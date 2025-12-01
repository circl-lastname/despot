#include <stdbool.h>

#include <despot-metadata.h>

#include "../ctx.h"
#include "../metadata-formats/vorbis-comment.h"
#include "../misc.h"
#include "flac.h"

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
  
  bool blocks_remain = true;
  
  while (blocks_remain) {
    uint8_t block_type;
    MUST(ctx_read_u8(ctx, &block_type));
    
    blocks_remain = !(block_type & 0x80);
    block_type &= 0x7f;
    
    uint32_t block_size;
    MUST(ctx_read_u24_be(ctx, &block_size));
    
    switch (block_type) {
      case FLAC_VORBIS_COMMENT: {
        size_t next_block_address = ctx_tell(ctx) + block_size;
        MUST(vorbis_comment_parse(ctx));
        MUST(ctx_seek(ctx, next_block_address));
      } break;
      case FLAC_PICTURE:
        // TODO
        MUST(ctx_skip(ctx, block_size));
      break;
      default:
        MUST(ctx_skip(ctx, block_size));
    }
  }
  
  return DESPOT_RESULT_SUCCESS;
}
