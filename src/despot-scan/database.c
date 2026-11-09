#include <despot.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "database.h"
#include "main.h"
#include "shared/io.h"
#include "shared/misc.h"
#include "shared/stb_ds.h"

// Database file format:
//
// u16_be path_length;
// char   path[path_length];
// i64_be last_modified;
// u16_be title_length;
//   if (title_length == 0)
//     is_audio = false;
//     continue;
// char   title[title_length];
// u16_be artist_length;
// char   artist[artist_length];
// u16_be album_length;
// char   album[album_length];
// u16_be album_artist_length;
// char   album_artist[album_artist_length];
// i32_be track;
// i32_be disc;
// u8     picture_md5[16];

despot_result_t database_load(FILE* database) {
  io_t io;
  MUST(io_init_for_reading(&io, database));
  
  while (true) {
    uint16_t path_length;
    despot_result_t read_result = io_read_u16_be(&io, &path_length);
    
    if (read_result == DESPOT_RESULT_UNEXPECTED_EOF) {
      break;
    } else if (read_result != DESPOT_RESULT_SUCCESS) {
      return read_result;
    }
    
    track_data_t* track = calloc(1, sizeof(track_data_t));
    
    char* path = malloc(path_length+1);
    TRY(io_read(&io, path, path_length)) CATCH (
      track_data_free(track);
    );
    path[path_length] = '\0';
    
    TRY(io_read_i64_be(&io, &track->last_modified)) CATCH (
      track_data_free(track);
    );
    
    uint16_t title_length;
    TRY(io_read_u16_be(&io, &title_length)) CATCH (
      track_data_free(track);
    );
    
    if (title_length == 0) {
      shput(tracks, path, track);
      continue;
    }
    
    track->is_audio = true;
    
    track->title = malloc(title_length+1);
    TRY(io_read(&io, track->title, title_length)) CATCH (
      track_data_free(track);
    );
    track->title[title_length] = '\0';
    
    uint16_t artist_length;
    TRY(io_read_u16_be(&io, &artist_length)) CATCH (
      track_data_free(track);
    );
    
    track->artist = malloc(artist_length+1);
    TRY(io_read(&io, track->artist, artist_length)) CATCH (
      track_data_free(track);
    );
    track->artist[artist_length] = '\0';
    
    uint16_t album_length;
    TRY(io_read_u16_be(&io, &album_length)) CATCH (
      track_data_free(track);
    );
    
    track->album = malloc(album_length+1);
    TRY(io_read(&io, track->album, album_length)) CATCH (
      track_data_free(track);
    );
    track->album[album_length] = '\0';
    
    uint16_t album_artist_length;
    TRY(io_read_u16_be(&io, &album_artist_length)) CATCH (
      track_data_free(track);
    );
    
    track->album_artist = malloc(album_artist_length+1);
    TRY(io_read(&io, track->album_artist, album_artist_length)) CATCH (
      track_data_free(track);
    );
    track->album_artist[album_artist_length] = '\0';
    
    TRY(io_read_i32_be(&io, &track->track)) CATCH (
      track_data_free(track);
    );
    
    TRY(io_read_i32_be(&io, &track->disc)) CATCH (
      track_data_free(track);
    );
    
    TRY(io_read(&io, &track->picture_md5, 16)) CATCH (
      track_data_free(track);
    );
    
    shput(tracks, path, track);
  }
  
  return DESPOT_RESULT_SUCCESS;
}

despot_result_t database_save(FILE* database) {
  io_t io;
  io.file = database;
  
  size_t track_amount = shlenu(tracks);
  
  for (size_t i = 0; i < track_amount; i++) {
    const char* path = tracks[i].key;
    track_data_t* track = tracks[i].value;
    
    // The scanned flag allows deleted files to be pruned from the database
    if (!track->scanned) {
      continue;
    }
    
    uint16_t path_length = strlen(path);
    MUST(io_write_u16_be(&io, path_length));
    MUST(io_write(&io, path, path_length));
    
    MUST(io_write_i64_be(&io, track->last_modified));
    
    if (!track->is_audio) {
      MUST(io_write_u16_be(&io, 0));
      continue;
    }
    
    uint16_t title_length = strlen(track->title);
    MUST(io_write_u16_be(&io, title_length));
    MUST(io_write(&io, track->title, title_length));
    
    uint16_t artist_length = strlen(track->artist);
    MUST(io_write_u16_be(&io, artist_length));
    MUST(io_write(&io, track->artist, artist_length));
    
    uint16_t album_length = strlen(track->album);
    MUST(io_write_u16_be(&io, album_length));
    MUST(io_write(&io, track->album, album_length));
    
    uint16_t album_artist_length = strlen(track->album_artist);
    MUST(io_write_u16_be(&io, album_artist_length));
    MUST(io_write(&io, track->album_artist, album_artist_length));
    
    MUST(io_write_i32_be(&io, track->track));
    MUST(io_write_i32_be(&io, track->disc));
    
    MUST(io_write(&io, track->picture_md5, 16));
  }
  
  return DESPOT_RESULT_SUCCESS;
}
