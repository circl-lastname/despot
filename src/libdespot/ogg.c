#include <stdbool.h>

#include <despot.h>

#include "ctx.h"
#include "ogg.h"
#include "shared/misc.h"
#include "vorbis-comment.h"

despot_result_t ogg_parse(despot_ctx_t* ctx, io_t* io) {
  // Relevant: https://www.rfc-editor.org/rfc/rfc3533.html
  
  ctx->get_basic_tag = vorbis_comment_get_basic_tag;
  
  MUST(io_seek(io, 0));
  
  return DESPOT_RESULT_SUCCESS;
}
