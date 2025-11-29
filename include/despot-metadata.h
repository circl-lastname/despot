#pragma once

#include <stdio.h>

typedef enum {
  DESPOT_RESULT_SUCCESS,
  DESPOT_RESULT_SEE_ERRNO,
} despot_result_t;

typedef struct despot_ctx_s despot_ctx_t;

despot_result_t despot_read_from_fd(despot_ctx_t** ctx, int fd);
despot_result_t despot_read_from_file(despot_ctx_t** ctx, FILE* file);
despot_result_t despot_read_from_mem(despot_ctx_t** ctx, void* buffer, size_t size);

void despot_free_ctx(despot_ctx_t* ctx);
