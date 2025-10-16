#include "error.h"
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

static char buf_error[256];
void error_sys(const char *func, const char *sys){
    snprintf(buf_error, sizeof(buf_error), "%s(): %s(): [%d, %s]", func, sys, errno, strerror(errno));
}

const char *error_what(void){
    return buf_error;
}

void error(const char *fun, const char *fmt, ...){
    size_t off = strlen(fun) + 4;//
    va_list ap;
    snprintf(buf_error, sizeof(buf_error), "%s(): ", fun);
    va_start(ap, fmt);
    vsnprintf(buf_error + off, sizeof(buf_error)-off, fmt, ap);
    va_end(ap);
}