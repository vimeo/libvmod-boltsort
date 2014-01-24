#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vrt.h"
#include "cache/cache.h"

#define MAX_PARAM_COUNT 32
#define EQUALS(c, h) ((c == h) || (c == '\0' && h == '&') || (c == '&' && h == '\0'))
#define ENDS(s) (s == '&' || s == '\0') 
#define NOT_ENDS(s) (s != '&' && s != '\0') 

static const char TERMINATORS[2] = {'\0', '&'};

//since we dont store param length, we have to evaluate everytime
static inline int param_compare (char *s, char *t)
{

    for ( ;EQUALS(*s, *t); s++, t++) {
        if (ENDS(*s)) {
            return 0;
        }
    }
    return *s - *t;

}

//end of param is either first occurance of & or '\0'
static inline int param_copy(char *dst, char *src, char *last_param)
{

    int len = strchr(src, TERMINATORS[(src != last_param)]) - src;
    memcpy(dst, src, len);
    return len;

}

//Varnish vmod requires this
int init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{

    return 0;

}

//sort query string
const char * vmod_sort(const struct vrt_ctx *ctx, const char *url)
{

    if (url == NULL) {
        return NULL;
    }

    int qs_index = 0;
    int param_count = 0;

    char *dst_url = NULL;
    char *qs = NULL;

    //To avoid 1 pass for count calculations, assuming MAX_PARAM_COUNT as max
    char* params[MAX_PARAM_COUNT];

    int i, p;
    char *param = NULL;

    qs = strchr(url, '?');
    if(!qs) {
        return url;
    }

    //add first param and increment count
    params[param_count++] = ++qs;
    qs_index = qs - url;

    //Continue to find query string
    while((qs = strchr(qs, '&')) != NULL) {
        param = ++qs;

        for(p = 0; p < param_count; p++) {
            //if incoming param is < param at position then place it at p and then move up rest
            if(param[0] < params[p][0] || param_compare(param, params[p]) < 0) {
                for(i = param_count; i > p; i--) {
                    params[i] = params[i-1];
                }
                break;
            }
        }
        params[p] = param;
        param_count++;

        //if it exceed max params return as is
        if (param_count == MAX_PARAM_COUNT) {
            return url;
        }
    }

    //there is nothing after & 
    //eg: http://127.0.0.1/?me=1&
    if (param_count == 1) {
        return url;
    }

    //allocate space for sorted url
    struct ws *ws = ctx->ws;
    dst_url = WS_Alloc(ws, strchr(param, '\0') - url + 1);
    WS_Assert(ws);

    //if alloc fails return as is
    if(dst_url == NULL) {
        return url;
    }

    //copy data before query string
    char* cp = memcpy(dst_url, url, qs_index) + qs_index;

    //get rid of all empty params /test/?a&&&
    for(p = 0; p < param_count - 1; p++) { 
        if (params[p][0] != '\0' && params[p][0] != '&') {
            break;
        }
    }

    //copy sorted params
    for(; p < param_count - 1; p++) {
        //copy and increment
        cp += param_copy(cp, params[p], param);
        *cp++ = '&';
    }

    //copy the last param
    cp += param_copy(cp, params[p], param);
    *cp = '\0';

    return dst_url;

}
