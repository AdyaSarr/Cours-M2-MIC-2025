#include <fq.h>
#include <fp.h>
#include <stdio.h>


int main(void){
/* Example: p=3, f(x)=x^2+1, so F_9 */
int p = 3; int fcoeffs[] = {1,0,1}; /* 1 + 0x + 1x^2 */
FqCtx F = fqctx_make(p, fcoeffs, 2);


FqElem a = fq_from_coeffs(&F, (int[]){2,1}, 2); /* 2 + x */
FqElem b = fq_from_coeffs(&F, (int[]){1,2}, 2); /* 1 + 2x */


FqElem prod = fq_mul(&a, &b);
printf("(2+x)*(1+2x) = "); fq_print(&prod); printf("\n");


FqElem inva = fq_inv(&a);
printf("(2+x)^{-1} = "); fq_print(&inva); printf("\n");


FqElem div = fq_div(&a, &b);
printf("(2+x)/(1+2x) = "); fq_print(&div); printf("\n");


FqElem g = fq_from_coeffs(&F, (int[]){0,1}, 2); /* x */
printf("x generator? %s\n", fq_is_generator(g)?"yes":"no");


/* cleanup */
fq_free(&a); fq_free(&b); fq_free(&prod); fq_free(&inva); fq_free(&div); fq_free(&g);
poly_free(&F.f);
return 0;
}