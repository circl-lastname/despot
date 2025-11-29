#pragma once

#include <stddef.h>
#include <stdint.h>

#include <despot-metadata.h>

typedef enum {
  CTX_SOURCE_MEM,
  CTX_SOURCE_FD
} ctx_source_t;

struct despot_ctx_s {
  ctx_source_t source;
  union {
    struct {
      void* buffer;
      size_t size;
      size_t offset;
    } mem;
    int fd;
  };
};

uint8_t ctx_seek(despot_ctx_t* ctx, size_t offset);
uint8_t ctx_skip(despot_ctx_t* ctx, size_t amount);
uint8_t ctx_read(despot_ctx_t* ctx, void* buffer, size_t amount);
