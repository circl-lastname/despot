#include <despot.h>

#include "../ctx.h"
#include "flac-picture.h"
#include "shared/misc.h"
#include "shared/stb_ds.h"

static void cleanup(despot_picture_t* picture) {
  if (picture->media_type) {
    free((char*)picture->media_type);
  }
  
  if (picture->media_type) {
    free((char*)picture->description);
  }
}

despot_result_t flac_picture_parse(despot_ctx_t* ctx) {
  // Relevant: https://www.rfc-editor.org/rfc/rfc9639.html
  
  despot_picture_t picture = { 0 };
  
  MUST(ctx_read_u32_be(ctx, &picture.type));
  if (picture.type > DESPOT_PICTURE_PUBLISHER_OR_STUDIO_LOGO) {
    picture.type = DESPOT_PICTURE_OTHER;
  }
  
  uint32_t media_type_length;
  MUST(ctx_read_u32_be(ctx, &media_type_length));
  
  picture.media_type = malloc(media_type_length+1);
  TRY(ctx_read(ctx, (char*)picture.media_type, media_type_length)) CATCH (
    cleanup(&picture);
  );
  ((char*)picture.media_type)[media_type_length] = '\0';
  
  uint32_t description_length;
  TRY(ctx_read_u32_be(ctx, &description_length)) CATCH (
    cleanup(&picture);
  );
  
  picture.description = malloc(description_length+1);
  TRY(ctx_read(ctx, (char*)picture.description, description_length)) CATCH (
    cleanup(&picture);
  );
  ((char*)picture.description)[description_length] = '\0';
  
  TRY(ctx_read_u32_be(ctx, &picture.width)) CATCH (
    cleanup(&picture);
  );
  
  TRY(ctx_read_u32_be(ctx, &picture.height)) CATCH (
    cleanup(&picture);
  );
  
  TRY(ctx_skip(ctx, 8)) CATCH (
    cleanup(&picture);
  );
  
  uint32_t size;
  TRY(ctx_read_u32_be(ctx, &size)) CATCH (
    cleanup(&picture);
  );
  picture.size = size;
  
  picture.source_offset = ctx_tell(ctx);
  
  arrput(ctx->pictures, picture);
  
  return DESPOT_RESULT_SUCCESS;
}
