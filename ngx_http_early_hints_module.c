/*
 * Copyright (C) 2016 flano_yuki
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "ngx_http_early_hints_module.h"

static char *ngx_http_early_hints_handler_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_early_hints_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_early_hints_init(ngx_conf_t *cf);
static void *ngx_http_early_hints_loc_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_early_hints_add_variables(ngx_conf_t *cf);
static ngx_int_t ngx_http_early_hints_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);



static ngx_command_t ngx_http_early_hints_commands[] = {
    {
        ngx_string("add_early_header"),
        NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_TAKE2,
        ngx_http_early_hints_handler_phase,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    ngx_null_command
};


static ngx_http_module_t ngx_http_early_hints_module_ctx = {
    ngx_http_early_hints_add_variables,/* preconfiguration */
    ngx_http_early_hints_init,         /* postconfiguration */

    NULL,                              /* create main configuration */
    NULL,                              /* init main configuration */

    NULL,                              /* create server configuration */
    NULL,                              /* merge server configuration */

    ngx_http_early_hints_loc_conf,     /* create location configuration */
    NULL                               /* merge location configuration */
};

ngx_module_t ngx_http_early_hints_module = {
    NGX_MODULE_V1,
    &ngx_http_early_hints_module_ctx, /* module context */
    ngx_http_early_hints_commands,    /* module directives */
    NGX_HTTP_MODULE,                  /* module type */
    NULL,                             /* init master */
    NULL,                             /* init module */
    NULL,                             /* init process */
    NULL,                             /* init thread */
    NULL,                             /* exit thread */
    NULL,                             /* exit process */
    NULL,                             /* exit master */
    NGX_MODULE_V1_PADDING
};

static ngx_http_variable_t  ngx_http_early_hints_vars[] = {
    {
      ngx_string("early_hints"), NULL,
          ngx_http_early_hints_variable, 0, 0, 0 }, //Is flag ok?

    { ngx_null_string, NULL, NULL, 0, 0, 0 }
};

static ngx_int_t
ngx_http_early_hints_add_variables(ngx_conf_t *cf)
{
    ngx_http_variable_t  *var, *v;

    for (v = ngx_http_early_hints_vars; v->name.len; v++) {
        var = ngx_http_add_variable(cf, &v->name, v->flags);
        if (var == NULL) {
            return NGX_ERROR;
        }

        var->get_handler = v->get_handler;
        var->data = v->data;
    }

    return NGX_OK;
}

static ngx_int_t
ngx_http_early_hints_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_http_early_hints_loc_conf_t *clcf = ngx_http_get_module_loc_conf(r, ngx_http_early_hints_module);
    v->len = sizeof("0") - 1;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = clcf->headers ? (u_char *) "1" : (u_char *) "0";

    return NGX_OK;
}

static ngx_int_t ngx_http_early_hints_handler(ngx_http_request_t *r)
{
    ngx_chain_t  *out;
    ngx_chain_t  *cl;

    ngx_buf_t    *b;
    ngx_str_t    body;

    ngx_http_early_hints_loc_conf_t *clcf = ngx_http_get_module_loc_conf(r, ngx_http_early_hints_module);

    if (clcf->headers == NULL)
        return NGX_DECLINED;
    if (r->internal)
        return NGX_DECLINED;
    if (r->method != NGX_HTTP_GET && r->method != NGX_HTTP_HEAD) {
        return NGX_HTTP_NOT_ALLOWED;
    }

#if (NGX_HTTP_V2)
    //TODO support http2
    if (r->http_connection->addr_conf->http2)
        return NGX_DECLINED;
#endif

    body.data = ngx_pcalloc(r->pool, sizeof(u_char)*(50 + ngx_strlen(clcf->headers)));
    if (body.data == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_sprintf(body.data, "%s" CRLF "%s" CRLF CRLF ,NGX_HTTP_EARLY_HINTS_STATUS, clcf->headers);
    body.len  = ngx_strlen(body.data);

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    out = ngx_pcalloc(r->pool, sizeof(ngx_chain_t));
    if (b == NULL || out == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    //TODO suppport two or more headers
    b->pos      = body.data;
    b->last     = b->pos + body.len;
    b->memory   = 1;
    b->last_buf = 1;
    out->buf     = b;
    out->next    = NULL;

    cl = r->connection->send_chain(r->connection, out, 0);
    if (cl == NGX_CHAIN_ERROR) {
        return NGX_ERROR;
    }
    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "Send 103 Early Hints");

    return NGX_OK;
}

static ngx_int_t ngx_http_early_hints_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    // use NGX_HTTP_POST_ACCESS_PHASE? but, it is not called...
    h = ngx_array_push(&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_early_hints_handler;
    return NGX_OK;
}

static void *ngx_http_early_hints_loc_conf(ngx_conf_t *cf)
{
    ngx_http_early_hints_loc_conf_t *conf;
    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_early_hints_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }
    return conf;
}

static char *ngx_http_early_hints_handler_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t *value;
    ngx_http_early_hints_loc_conf_t *flcf = conf;
    char *k;
    char *v;
    u_char *tmp;

    value = cf->args->elts;
    //TODO add error handling
    k = (char *)value[1].data;
    v = (char *)value[2].data;

    if (flcf->headers == NULL){
       flcf->headers = ngx_pcalloc(cf->pool, sizeof(u_char)*200);
       ngx_sprintf(flcf->headers, "%s: %s", k, v);
    }
    else {
       tmp = flcf->headers;
       flcf->headers = ngx_pcalloc(cf->pool, sizeof(u_char)*(200 + ngx_strlen(flcf->headers)));
       ngx_sprintf(flcf->headers, "%s" CRLF "%s: %s", tmp, k, v);
       ngx_pfree(cf->pool, tmp);
    }

    return NGX_CONF_OK;
}
