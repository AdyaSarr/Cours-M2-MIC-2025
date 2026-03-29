#ifndef FQ_H
#define FQ_H
#include "fp.h"


typedef struct{
    FpCtx K;
    Poly f;
    int q;
} FqCtx;//definition de Fq


typedef struct{
    Poly a;
    FqCtx *F; 
} FqElem; // represeentant mod f


FqCtx fqctx_make(int p, const int *fcoeffs, int d);
void fqctx_free(FqCtx *F);


FqElem fq_from_coeffs(FqCtx *F, const int *coeffs, int m);
FqElem fq_from_poly(FqCtx *F, const Poly *A);
void fq_free(FqElem *x);


FqElem fq_add(const FqElem *x, const FqElem *y);
FqElem fq_sub(const FqElem *x, const FqElem *y);
FqElem fq_mul(const FqElem *x, const FqElem *y);
FqElem fq_div(const FqElem *x, const FqElem *y);
FqElem fq_inv(const FqElem *x);
FqElem fq_pow(FqElem base, uint64_t e);
int fq_is_generator(FqElem g); // 1 if generator of Fq^*


void fq_print(const FqElem *x);


#endif