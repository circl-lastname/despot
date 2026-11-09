#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  int64_t last_modified;
  char* title;
  char* artist;
  char* album;
  char* album_artist;
  int32_t track;
  int32_t disc;
  uint8_t picture_md5[16];
  bool is_audio;
  bool scanned;
} track_data_t;

typedef struct {
  char* key;
  track_data_t* value;
} track_t;

extern track_t* tracks;

void track_data_free(track_data_t* track);
