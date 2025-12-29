#include <stdbool.h>

#include <despot.h>

#include "../ctx.h"
#include "../metadata-formats/flac-picture.h"
#include "../metadata-formats/vorbis-comment.h"
#include "flac.h"
#include "shared/misc.h"

enum {
  FLAC_STREAMINFO,
  FLAC_PADDING,
  FLAC_APPLICATION,
  FLAC_SEEK_TABLE,
  FLAC_VORBIS_COMMENT,
  FLAC_CUESHEET,
  FLAC_PICTURE,
};

despot_result_t flac_parse(despot_ctx_t* ctx) {
  // Relevant: https://www.rfc-editor.org/rfc/rfc9639.html
  
  ctx->get_basic_tag = vorbis_comment_get_basic_tag;
  
  bool blocks_remain = true;
  
  while (blocks_remain) {
    uint8_t block_type;
    MUST(ctx_read_u8(ctx, &block_type));
    
    blocks_remain = !(block_type & 0x80);
    block_type &= 0x7f;
    
    size_t next_block_address;
    
    if (blocks_remain) {
      uint32_t block_size;
      MUST(ctx_read_u24_be(ctx, &block_size));
      next_block_address = ctx_tell(ctx) + block_size;
    } else {
      MUST(ctx_skip(ctx, 3));
    }
    
    switch (block_type) {
      case FLAC_VORBIS_COMMENT:
        MUST(vorbis_comment_parse(ctx));
      break;
      case FLAC_PICTURE:
        MUST(flac_picture_parse(ctx));
      break;
    }
    
    if (blocks_remain) {
      MUST(ctx_seek(ctx, next_block_address));
    }
  }
  
  return DESPOT_RESULT_SUCCESS;
}
