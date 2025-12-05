#pragma once

#include <despot-metadata.h>

despot_result_t vorbis_comment_parse(despot_ctx_t* ctx);
const char* vorbis_comment_get_basic_tag(despot_ctx_t* ctx, despot_tag_id_t tag);
