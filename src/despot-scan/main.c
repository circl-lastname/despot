#include <despot.h>
#include <errno.h>
#include <ftw.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "database.h"
#include "main.h"
#include "shared/stb_ds.h"

const char* command_name;

track_t* tracks = NULL;

unsigned progress_file_count = 0;

unsigned stat_total_files = 0;
unsigned stat_unmodified_tracks = 0;
unsigned stat_new_or_modified_tracks = 0;
unsigned stat_other_files = 0;
unsigned stat_errors = 0;

void track_data_free(track_data_t* track) {
  if (track->title)
    free(track->title);
  
  if (track->artist)
    free(track->artist);
  
  if (track->album)
    free(track->album);
  
  if (track->album_artist)
    free(track->album_artist);
  
  free(track);
}

static void tracks_free() {
  size_t track_amount = shlenu(tracks);
  
  for (size_t i = 0; i < track_amount; i++) {
    free(tracks[i].key);
    track_data_free(tracks[i].value);
  }
  
  shfree(tracks);
}

static void print_progress() {
  progress_file_count++;
  if (progress_file_count == 50) {
    fprintf(stderr, "\rScanning... %u", stat_total_files);
    progress_file_count = 0;
  }
}

static int scan_func(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
  (void)ftwbuf;
  
  const char* path = &fpath[2];
  
  if (typeflag != FTW_F) {
    // errno not guaranteed, but try to show an error anyway
    if (typeflag == FTW_DNR) {
      fprintf(stderr, "\r%s: Failed to scan directory %s: %s\n", command_name, path, strerror(errno));
    } else if (typeflag == FTW_NS || typeflag == FTW_SLN) {
      stat_total_files++;
      stat_errors++;
      
      fprintf(stderr, "\r%s: Failed to scan file %s: %s\n", command_name, path, strerror(errno));
      
      print_progress();
    }
    
    return 0;
  }
  
  stat_total_files++;
  
  track_data_t* track = shget(tracks, path);
  
  int64_t last_modified = (sb->st_ctime > sb->st_mtime) ? sb->st_ctime : sb->st_mtime;
  
  bool update_metadata = false;
  bool is_new = false;
  
  // Determine if the track/file needs its metadata to be updated
  if (!track) {
    track = calloc(1, sizeof(track_data_t));
    update_metadata = true;
    is_new = true;
  } else {
    if (last_modified > track->last_modified) {
      update_metadata = true;
    } else {
      if (track->is_audio) {
        stat_unmodified_tracks++;
      } else {
        stat_other_files++;
      }
    }
  }
  
  track->scanned = true;
  
  if (update_metadata) {
    FILE* file = fopen(fpath, "rb");
    
    despot_ctx_t* ctx;
    despot_result_t result = despot_read_from_file(&ctx, file);
    
    if (result == DESPOT_RESULT_SUCCESS) {
      stat_new_or_modified_tracks++;
      
      const char* title = despot_get_basic_tag(ctx, DESPOT_TAG_TITLE);
      if (!title)
        // TODO: Cut down to filename?
        title = path;
      
      const char* artist = despot_get_basic_tag(ctx, DESPOT_TAG_ARTIST);
      if (!artist)
        artist = "Unknown artist";
      
      const char* album = despot_get_basic_tag(ctx, DESPOT_TAG_ALBUM);
      if (!album)
        album = title;
      
      const char* album_artist = despot_get_basic_tag(ctx, DESPOT_TAG_ALBUM_ARTIST);
      if (!album_artist)
        album_artist = artist;
      
      const char* track_num = despot_get_basic_tag(ctx, DESPOT_TAG_TRACK);
      const char* disc_num = despot_get_basic_tag(ctx, DESPOT_TAG_DISC);
      
      track->last_modified = last_modified;
      track->title = strdup(title);
      track->artist = strdup(artist);
      track->album = strdup(album);
      track->album_artist = strdup(album_artist);
      track->track = track_num ? atoi(track_num) : 0;
      track->disc = disc_num ? atoi(disc_num) : 0;
      track->is_audio = true;
      
      // TODO Picture system and processing for JSON output
      
      despot_free_ctx(ctx);
      fclose(file);
    } else if (result == DESPOT_RESULT_UNRECOGNIZED_FORMAT) {
      stat_other_files++;
      
      track->last_modified = last_modified;
      track->is_audio = false;
      
      despot_free_ctx(ctx);
      fclose(file);
    } else {
      stat_errors++;
      
      if (is_new) {
        free(track);
      }
      
      despot_free_ctx(ctx);
      fclose(file);
      
      fprintf(stderr, "\r%s: Failed to scan file %s: %s\n", command_name, path, despot_result_to_string(result));
      
      print_progress();
      
      return 0;
    }
  }
  
  if (is_new) {
    const char* allocated_path = strdup(path);
    shput(tracks, allocated_path, track);
  }
  
  print_progress();
  
  return 0;
}

static void print_usage() {
  fprintf(stderr, "Usage: %s -d DATABASE -o OUTPUT -p PICTURES_DIR [DIRECTORY]\n", command_name);
}

int main(int argc, char** argv) {
  command_name = argv[0];
  
  // Parse command line arguments
  const char* directory = NULL;
  const char* database = NULL;
  const char* output = NULL;
  const char* pictures_dir = NULL;
  
  for (int i = 1; i < argc; i++) {
    const char* arg = argv[i];
    
    if (arg[0] == '-') {
      if (arg[1] == 0 || arg[2] != 0) {
        print_usage();
        return 1;
      }
      
      i++;
      const char* value = argv[i];
      
      switch (arg[1]) {
        case 'd':
          if (!database) {
            database = value;
          } else {
            print_usage();
            return 1;
          }
        break;
        case 'o':
          if (!output) {
            output = value;
          } else {
            print_usage();
            return 1;
          }
        break;
        case 'p':
          if (!pictures_dir) {
            pictures_dir = value;
          } else {
            print_usage();
            return 1;
          }
        break;
        default:
          print_usage();
          return 1;
      }
    } else {
      if (!directory) {
        directory = arg;
      } else {
        print_usage();
        return 1;
      }
    }
  }
  
  if (!database || !output || !pictures_dir) {
    print_usage();
    return 1;
  }
  
  fputs("despot by Nexa | Despot-ify your music library\n\n", stderr);
  
  // Open files, load database, and enter directory
  FILE* database_file = fopen(database, "rb");
  
  if (!database_file) {
    if (errno != ENOENT) {
      fprintf(stderr, "%s: Failed to open database file: %s\n", command_name, strerror(errno));
      return 1;
    }
  } else {
    despot_result_t load_result = database_load(database_file);
    
    if (load_result != DESPOT_RESULT_SUCCESS) {
      fprintf(stderr, "%s: Failed to load database file: %s\n", command_name, despot_result_to_string(load_result));
      return 1;
    }
  }
  
  /*FILE* output_file = fopen(output, "wb");
  if (!output_file) {
    fprintf(stderr, "%s: Failed to open output file: %s\n", command_name, strerror(errno));
    return 1;
  }*/
  
  if (directory) {
    if (chdir(directory) < 0) {
      fprintf(stderr, "%s: Failed to enter directory: %s\n", command_name, strerror(errno));
      //fclose(output_file);
      return 1;
    }
  }
  
  fputs("Scanning...", stderr);
  
  if (nftw(".", scan_func, 512, 0) < 0) {
    fprintf(stderr, "\r%s: Failed to scan directory: %s\n", command_name, strerror(errno));
    //fclose(output_file);
    return 1;
  }
  
  fprintf(stderr, "\rScanned %u files\n\n", stat_total_files);
  
  fprintf(stderr, "     Unmodified tracks: %u\n", stat_unmodified_tracks);
  fprintf(stderr, "New or modified tracks: %u\n", stat_new_or_modified_tracks);
  fprintf(stderr, "           Other files: %u\n", stat_other_files);
  fprintf(stderr, "                Errors: %u\n", stat_errors);
  
  // Doesn't work when creating it for the first time
  database_file = freopen(NULL, "wb", database_file);
  if (!database_file) {
    fprintf(stderr, "%s: Failed to open database file: %s\n", command_name, strerror(errno));
    //fclose(output_file);
    return 1;
  } 
  
  despot_result_t save_result = database_save(database_file);
  
  fclose(database_file);
  
  if (save_result != DESPOT_RESULT_SUCCESS) {
    fprintf(stderr, "%s: Failed to save database file: %s\n", command_name, despot_result_to_string(save_result));
    //fclose(output_file);
    return 1;
  }
  
  //fclose(output_file);
  tracks_free();
  
  return 0;
}
