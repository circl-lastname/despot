#include <despot-metadata.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void try_get_metadata(despot_ctx_t* ctx, despot_tag_id_t tag, char* name) {
  const char* value = despot_get_basic_tag(ctx, tag);
  if (value) {
    printf("%s: %s\n", name, value);
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s FILE\n", argv[0]);
    return 1;
  }
  
  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "%s: Failed to open file: %s\n", argv[0], strerror(errno));
    return 1;
  }
  
  despot_ctx_t* ctx;
  despot_result_t result;
  if ((result = despot_read_from_fd(&ctx, fd)) != DESPOT_RESULT_SUCCESS) {
    fprintf(stderr, "%s: Failed to read metadata: %s\n", argv[0], despot_result_to_string(result));
    return 1;
  }
  
  close(fd);
  
  try_get_metadata(ctx, DESPOT_TAG_TITLE, "Title");
  try_get_metadata(ctx, DESPOT_TAG_ARTIST, "Artist");
  try_get_metadata(ctx, DESPOT_TAG_ALBUM, "Album");
  try_get_metadata(ctx, DESPOT_TAG_ALBUM_ARTIST, "Album artist");
  try_get_metadata(ctx, DESPOT_TAG_TRACK, "Track");
  try_get_metadata(ctx, DESPOT_TAG_DISC, "Disc");
  try_get_metadata(ctx, DESPOT_TAG_TRACK_AMOUNT, "Total tracks");
  try_get_metadata(ctx, DESPOT_TAG_DISC_AMOUNT, "Total discs");
  try_get_metadata(ctx, DESPOT_TAG_VENDOR, "Vendor");
  
  size_t pictures_amount;
  despot_picture_t* pictures = despot_get_pictures(ctx, &pictures_amount);
  
  for (size_t i = 0; i < pictures_amount; i++) {
    printf("\nPicture %zu:\n", i+1);
    printf("  Type: %s\n", despot_picture_type_to_string(pictures[i].type));
    printf("  Media type: %s\n", pictures[i].media_type);
    printf("  Description: %s\n", pictures[i].description);
    printf("  Width: %u\n", pictures[i].width);
    printf("  Height: %u\n", pictures[i].height);
    printf("  Size: %zu\n", pictures[i].size);
  }
  
  despot_free_ctx(ctx);
  
  return 0;
}
