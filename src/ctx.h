#pragma once

#include <endian.h>
#include <stddef.h>
#include <stdint.h>

#include <despot-metadata.h>

#include "misc.h"

typedef enum {
  CTX_SOURCE_MEM,
  CTX_SOURCE_FD
} ctx_source_t;

typedef struct {
  char* key;
  char* value;
} ctx_metadata_t;

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
  
  char* vendor;
  ctx_metadata_t* metadata;
};

despot_result_t ctx_seek(despot_ctx_t* ctx, size_t offset);
despot_result_t ctx_skip(despot_ctx_t* ctx, size_t amount);
size_t ctx_tell(despot_ctx_t* ctx);
despot_result_t ctx_read(despot_ctx_t* ctx, void* buffer, size_t amount);

static inline despot_result_t ctx_read_u8(despot_ctx_t* ctx, uint8_t* integer) {
  return ctx_read(ctx, integer, 1);
}

static inline despot_result_t ctx_read_i8(despot_ctx_t* ctx, int8_t* integer) {
  return ctx_read(ctx, integer, 1);
}

static inline despot_result_t ctx_read_u16_le(despot_ctx_t* ctx, uint16_t* integer) {
  uint16_t integer_temp;
  MUST(ctx_read(ctx, &integer_temp, 2));
  *integer = le16toh(integer_temp);
  return DESPOT_RESULT_SUCCESS;
}

static inline despot_result_t ctx_read_i16_le(despot_ctx_t* ctx, int16_t* integer) {
  uint16_t integer_temp;
  MUST(ctx_read(ctx, &integer_temp, 2));
  *(uint16_t*)integer = le16toh(integer_temp);
  return DESPOT_RESULT_SUCCESS;
}

static inline despot_result_t ctx_read_u16_be(despot_ctx_t* ctx, uint16_t* integer) {
  uint16_t integer_temp;
  MUST(ctx_read(ctx, &integer_temp, 2));
  *integer = be16toh(integer_temp);
  return DESPOT_RESULT_SUCCESS;
}

static inline despot_result_t ctx_read_i16_be(despot_ctx_t* ctx, int16_t* integer) {
  uint16_t integer_temp;
  MUST(ctx_read(ctx, &integer_temp, 2));
  *(uint16_t*)integer = be16toh(integer_temp);
  return DESPOT_RESULT_SUCCESS;
}

static inline despot_result_t ctx_read_u24_be(despot_ctx_t* ctx, uint32_t* integer) {
  uint8_t integer_temp[3];
  MUST(ctx_read(ctx, &integer_temp, 3));
  *integer = integer_temp[2] | integer_temp[1] << 8 | integer_temp[0] << 16;
  return DESPOT_RESULT_SUCCESS;
}

static inline despot_result_t ctx_read_u32_le(despot_ctx_t* ctx, uint32_t* integer) {
  uint32_t integer_temp;
  MUST(ctx_read(ctx, &integer_temp, 4));
  *integer = le32toh(integer_temp);
  return DESPOT_RESULT_SUCCESS;
}

static inline despot_result_t ctx_read_i32_le(despot_ctx_t* ctx, int32_t* integer) {
  uint32_t integer_temp;
  MUST(ctx_read(ctx, &integer_temp, 4));
  *(uint32_t*)integer = le32toh(integer_temp);
  return DESPOT_RESULT_SUCCESS;
}

static inline despot_result_t ctx_read_u32_be(despot_ctx_t* ctx, uint32_t* integer) {
  uint32_t integer_temp;
  MUST(ctx_read(ctx, &integer_temp, 4));
  *integer = be32toh(integer_temp);
  return DESPOT_RESULT_SUCCESS;
}

static inline despot_result_t ctx_read_i32_be(despot_ctx_t* ctx, int32_t* integer) {
  uint32_t integer_temp;
  MUST(ctx_read(ctx, &integer_temp, 4));
  *(uint32_t*)integer = be32toh(integer_temp);
  return DESPOT_RESULT_SUCCESS;
}
