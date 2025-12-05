#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <despot-metadata.h>

#include "ctx.h"
#include "formats/flac.h"
#include "misc.h"
#include "stb_ds.h"

static despot_result_t parse(despot_ctx_t* ctx) {
  uint32_t magic_number;
  
  // FLAC
  // Relevant: https://www.rfc-editor.org/rfc/rfc9639.html
  MUST(ctx_read_u32_be(ctx, &magic_number));
  if (magic_number == 0x664c6143) {
    return flac_parse(ctx);
  }
  
  return DESPOT_RESULT_UNRECOGNIZED_FORMAT;
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
  
  despot_result_t result = parse(*ctx);
  
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
  
  return parse(*ctx);
}

EXPORT const char* despot_get_basic_tag(despot_ctx_t* ctx, despot_tag_id_t tag) {
  return ctx->get_basic_tag(ctx, tag);
}

EXPORT void despot_free_ctx(despot_ctx_t* ctx) {
  if (ctx->vendor) {
    free(ctx->vendor);
  }
  
  size_t metadata_keys = shlenu(ctx->metadata);
  for (size_t i = 0; i < metadata_keys; i++) {
    free(ctx->metadata[i].key);
  }
  
  shfree(ctx->metadata);
  
  free(ctx);
}

EXPORT const char* despot_result_to_string(despot_result_t result) {
  switch (result) {
    case DESPOT_RESULT_SUCCESS:
      return "Success";
    case DESPOT_RESULT_INTERNAL_ERROR:
      return "Internal error, please report a bug to despot";
    case DESPOT_RESULT_UNEXPECTED_EOF:
      return "Unexpected end of file";
    case DESPOT_RESULT_UNRECOGNIZED_FORMAT:
      return "Unrecognized format";
    case DESPOT_RESULT_PARSING_ERROR:
      return "Failed to parse metadata";
    case DESPOT_RESULT_SEE_ERRNO:
      return strerror(errno);
    default:
      return "Unknown result";
  }
}
