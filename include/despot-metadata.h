#pragma once

#include <stdio.h>

typedef enum {
  DESPOT_RESULT_SUCCESS,
  DESPOT_RESULT_INTERNAL_ERROR,
  DESPOT_RESULT_UNEXPECTED_EOF,
  DESPOT_RESULT_UNRECOGNIZED_FORMAT,
  DESPOT_RESULT_PARSING_ERROR,
  DESPOT_RESULT_SEE_ERRNO,
} despot_result_t;

typedef enum {
  DESPOT_TAG_TITLE,
  DESPOT_TAG_ARTIST,
  DESPOT_TAG_ALBUM,
  DESPOT_TAG_ALBUM_ARTIST,
  DESPOT_TAG_TRACK,
  DESPOT_TAG_DISC,
  DESPOT_TAG_TRACK_AMOUNT,
  DESPOT_TAG_DISC_AMOUNT,
  DESPOT_TAG_VENDOR,
} despot_tag_id_t;

typedef struct {
  char* key;
  char* value;
} despot_tag_t;

typedef struct despot_ctx_s despot_ctx_t;

despot_result_t despot_read_from_fd(despot_ctx_t** ctx, int fd);
despot_result_t despot_read_from_file(despot_ctx_t** ctx, FILE* file);
despot_result_t despot_read_from_mem(despot_ctx_t** ctx, void* buffer, size_t size);

despot_tag_t* despot_get_tags(despot_ctx_t* ctx, size_t** amount);
const char* despot_get_basic_tag(despot_ctx_t* ctx, despot_tag_id_t tag);
const char* despot_get_tag(despot_ctx_t* ctx, char* tag);

void despot_free_ctx(despot_ctx_t* ctx);

const char* despot_result_to_string(despot_result_t result);
