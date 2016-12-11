/*
 * Copyright (C) 2016 flano_yuki
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#define NGX_HTTP_EARLY_HINTS_STATUS "HTTP/1.1 103 Early Hints"

typedef struct {
    u_char *headers;
} ngx_http_early_hints_loc_conf_t;
