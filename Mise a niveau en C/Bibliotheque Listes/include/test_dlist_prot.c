#include "dlist.h"
#include <string.h>

void *int_copy(const void *in)     { (void)in; return NULL; } // test de signature
void  int_free(void *p)            { (void)p; }
int   int_cmp(const void *a,const void *b){ (void)a;(void)b; return 0; }
void  int_print(const void *e, FILE *out){ fprintf(out, "%p", e); }

int main(void){ return 0; }
