#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <despot.h>

#include "io.h"

despot_result_t io_seek(io_t* io, size_t offset) {
  if (io->source == IO_SOURCE_MEM) {
    if (offset >= io->mem.size) {
      return DESPOT_RESULT_UNEXPECTED_EOF;
    }
    
    io->mem.offset = offset;
    return DESPOT_RESULT_SUCCESS;
  } else if (io->source == IO_SOURCE_FD) {
    if (lseek(io->fd, offset, SEEK_SET) < 0) {
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

despot_result_t io_skip(io_t* io, size_t amount) {
  if (amount == 0) {
    return DESPOT_RESULT_SUCCESS;
  }
  
  if (io->source == IO_SOURCE_MEM) {
    if (io->mem.offset+amount > io->mem.size) {
      return DESPOT_RESULT_UNEXPECTED_EOF;
    }
    
    io->mem.offset += amount;
    return DESPOT_RESULT_SUCCESS;
  } else if (io->source == IO_SOURCE_FD) {
    if (lseek(io->fd, amount, SEEK_CUR) < 0) {
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

size_t io_tell(io_t* io) {
  if (io->source == IO_SOURCE_MEM) {
    return io->mem.offset;
  } else if (io->source == IO_SOURCE_FD) {
    return lseek(io->fd, 0, SEEK_CUR);
  } else {
    return 0;
  }
}

despot_result_t io_read(io_t* io, void* buffer, size_t amount) {
  if (amount == 0) {
    return DESPOT_RESULT_SUCCESS;
  }
  
  if (io->source == IO_SOURCE_MEM) {
    if (io->mem.offset+amount > io->mem.size) {
      return DESPOT_RESULT_UNEXPECTED_EOF;
    }
    
    memcpy(buffer, io->mem.buffer+io->mem.offset, amount);
    io->mem.offset += amount;
    return DESPOT_RESULT_SUCCESS;
  } else if (io->source == IO_SOURCE_FD) {
    size_t amount_read = 0;
    
    while (amount_read < amount) {
      ssize_t read_result = read(io->fd, buffer+amount_read, amount-amount_read);
      
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
