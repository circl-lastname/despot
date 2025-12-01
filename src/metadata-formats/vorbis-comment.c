#include <ctype.h>
#include <stdlib.h>

#include <despot-metadata.h>

#include "../ctx.h"
#include "../misc.h"
#include "../stb_ds.h"
#include "vorbis-comment.h"

despot_result_t vorbis_comment_parse(despot_ctx_t* ctx) {
  // Relevant: https://xiph.org/vorbis/doc/v-comment.html
  
  uint32_t vendor_length;
  MUST(ctx_read_u32_le(ctx, &vendor_length));
  
  ctx->vendor = malloc(vendor_length+1);
  TRY(ctx_read(ctx, ctx->vendor, vendor_length)) CATCH (
    free(ctx->vendor);
    ctx->vendor = NULL;
  );
  ctx->vendor[vendor_length] = '\0';
  
  uint32_t fields_amount;
  MUST(ctx_read_u32_le(ctx, &fields_amount));
  
  for (unsigned i = 0; i < fields_amount; i++) {
    uint32_t field_length;
    MUST(ctx_read_u32_le(ctx, &field_length));
    
    char* key = malloc(field_length+1);
    TRY(ctx_read(ctx, key, field_length)) CATCH (
      free(key);
    );
    key[field_length] = '\0';
    
    char* value = NULL;
    
    for (uint32_t j = 0; j < field_length; j++) {
      if (key[j] == '=') {
        key[j] = '\0';
        value = &key[j+1];
        break;
      }
      
      key[j] = tolower(key[j]);
    }
    
    if (!value) {
      free(key);
      continue;
    }
    
    shput(ctx->metadata, key, value);
  }
  
  return DESPOT_RESULT_SUCCESS;
}
