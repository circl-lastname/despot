#include <string.h>
#include <unistd.h>

#include <despot-metadata.h>

#include "ctx.h"

uint8_t ctx_seek(despot_ctx_t* ctx, size_t offset) {
  if (ctx->source == CTX_SOURCE_MEM) {
    if (offset >= ctx->mem.size) {
      return -1;
    }
    
    ctx->mem.offset = offset;
    return 0;
  } else if (ctx->source == CTX_SOURCE_FD) {
    if (lseek(ctx->fd, offset, SEEK_SET) < 0) {
      return -1;
    }
    
    return 0;
  } else {
    return -1;
  }
}

uint8_t ctx_skip(despot_ctx_t* ctx, size_t amount) {
  if (ctx->source == CTX_SOURCE_MEM) {
    if (ctx->mem.offset+amount > ctx->mem.size) {
      return -1;
    }
    
    ctx->mem.offset += amount;
    return 0;
  } else if (ctx->source == CTX_SOURCE_FD) {
    if (lseek(ctx->fd, amount, SEEK_CUR) < 0) {
      return -1;
    }
    
    return 0;
  } else {
    return -1;
  }
}

uint8_t ctx_read(despot_ctx_t* ctx, void* buffer, size_t amount) {
  if (amount == 0) {
    return 0;
  }
  
  if (ctx->source == CTX_SOURCE_MEM) {
    if (ctx->mem.offset+amount > ctx->mem.size) {
      return -1;
    }
    
    memcpy(buffer, ctx->mem.buffer+ctx->mem.offset, amount);
    ctx->mem.offset += amount;
    return 0;
  } else if (ctx->source == CTX_SOURCE_FD) {
    size_t amount_read = 0;
    
    while (amount_read < amount) {
      ssize_t read_result = read(ctx->fd, buffer+amount_read, amount-amount_read);
      
      if (read_result <= 0) {
        return -1;
      }
      
      amount_read += read_result;
    }
    
    return 0;
  } else {
    return -1;
  }
}
