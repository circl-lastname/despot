#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <despot.h>

#include "ctx.h"

despot_result_t ctx_seek(despot_ctx_t* ctx, size_t offset) {
  if (ctx->source == CTX_SOURCE_MEM) {
    if (offset >= ctx->mem.size) {
      return DESPOT_RESULT_UNEXPECTED_EOF;
    }
    
    ctx->mem.offset = offset;
    return DESPOT_RESULT_SUCCESS;
  } else if (ctx->source == CTX_SOURCE_FD) {
    if (lseek(ctx->fd, offset, SEEK_SET) < 0) {
      if (errno == EINVAL) {
        return DESPOT_RESULT_UNEXPECTED_EOF;
      } else {
        return DESPOT_RESULT_SEE_ERRNO;
      }
    }
    
    return DESPOT_RESULT_SUCCESS;
  } else {
    return DESPOT_RESULT_INTERNAL_ERROR;
  }
}

despot_result_t ctx_skip(despot_ctx_t* ctx, size_t amount) {
  if (amount == 0) {
    return DESPOT_RESULT_SUCCESS;
  }
  
  if (ctx->source == CTX_SOURCE_MEM) {
    if (ctx->mem.offset+amount > ctx->mem.size) {
      return DESPOT_RESULT_UNEXPECTED_EOF;
    }
    
    ctx->mem.offset += amount;
    return DESPOT_RESULT_SUCCESS;
  } else if (ctx->source == CTX_SOURCE_FD) {
    if (lseek(ctx->fd, amount, SEEK_CUR) < 0) {
      if (errno == EINVAL) {
        return DESPOT_RESULT_UNEXPECTED_EOF;
      } else {
        return DESPOT_RESULT_SEE_ERRNO;
      }
    }
    
    return DESPOT_RESULT_SUCCESS;
  } else {
    return DESPOT_RESULT_INTERNAL_ERROR;
  }
}

size_t ctx_tell(despot_ctx_t* ctx) {
  if (ctx->source == CTX_SOURCE_MEM) {
    return ctx->mem.offset;
  } else if (ctx->source == CTX_SOURCE_FD) {
    return lseek(ctx->fd, 0, SEEK_CUR);
  } else {
    return 0;
  }
}

despot_result_t ctx_read(despot_ctx_t* ctx, void* buffer, size_t amount) {
  if (amount == 0) {
    return DESPOT_RESULT_SUCCESS;
  }
  
  if (ctx->source == CTX_SOURCE_MEM) {
    if (ctx->mem.offset+amount > ctx->mem.size) {
      return DESPOT_RESULT_UNEXPECTED_EOF;
    }
    
    memcpy(buffer, ctx->mem.buffer+ctx->mem.offset, amount);
    ctx->mem.offset += amount;
    return DESPOT_RESULT_SUCCESS;
  } else if (ctx->source == CTX_SOURCE_FD) {
    size_t amount_read = 0;
    
    while (amount_read < amount) {
      ssize_t read_result = read(ctx->fd, buffer+amount_read, amount-amount_read);
      
      if (read_result == 0) {
        return DESPOT_RESULT_UNEXPECTED_EOF;
      } else if (read_result < 0) {
        if (errno == EINTR) {
          continue;
        } else {
          return DESPOT_RESULT_SEE_ERRNO;
        }
      }
      
      amount_read += read_result;
    }
    
    return DESPOT_RESULT_SUCCESS;
  } else {
    return DESPOT_RESULT_INTERNAL_ERROR;
  }
}
