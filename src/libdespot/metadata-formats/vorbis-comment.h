#pragma once

#include <despot.h>

#include "shared/io.h"

despot_result_t vorbis_comment_parse(despot_ctx_t* ctx, io_t* io);
const char* vorbis_comment_get_basic_tag(despot_ctx_t* ctx, despot_tag_id_t tag);
