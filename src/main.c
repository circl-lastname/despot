#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <despot-metadata.h>

#include "ctx.h"
#include "misc.h"

static despot_result_t begin(despot_ctx_t* ctx) {
  uint8_t n;
  
  while (ctx_read(ctx, &n, 1) == 0) {
    printf("%u\n", n);
  }
  
  return DESPOT_RESULT_SUCCESS;
}

EXPORT despot_result_t despot_read_from_fd(despot_ctx_t** ctx, int fd) {
  *ctx = calloc(1, sizeof(despot_ctx_t));
  
  (*ctx)->source = CTX_SOURCE_FD;
  (*ctx)->fd = dup(fd);
  
  if ((*ctx)->fd < 0) {
    free(*ctx);
    *ctx = NULL;
    return DESPOT_RESULT_SEE_ERRNO;
  }
  
  if (lseek((*ctx)->fd, 0, SEEK_SET) < 0) {
    free(*ctx);
    *ctx = NULL;
    return DESPOT_RESULT_SEE_ERRNO;
  }
  
  despot_result_t result = begin(*ctx);
  
  close((*ctx)->fd);
  
  return result;
}

EXPORT despot_result_t despot_read_from_file(despot_ctx_t** ctx, FILE* file) {
  int fd = fileno(file);
  if (fd < 0) {
    return DESPOT_RESULT_SEE_ERRNO;
  }
  
  return despot_read_from_fd(ctx, fd);
}

EXPORT despot_result_t despot_read_from_mem(despot_ctx_t** ctx, void* buffer, size_t size) {
  *ctx = calloc(1, sizeof(despot_ctx_t));
  
  (*ctx)->source = CTX_SOURCE_MEM;
  (*ctx)->mem.buffer = buffer;
  (*ctx)->mem.size = size;
  
  return begin(*ctx);
}

EXPORT void despot_free_ctx(despot_ctx_t* ctx) {
  free(ctx);
}
