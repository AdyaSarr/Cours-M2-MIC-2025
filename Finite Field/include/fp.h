#ifndef FP_H
#define FP_H
#include <stdint.h>
#include "ff_util.h"


typedef struct { 
    int p;
} FpCtx;


typedef struct {
int n; //taille allouée de c[], n > deg + 1
int deg;// deg == -1 poly null
int *c;
FpCtx *K;
} Poly;


/* Construction de polynome et gestion de la memoire*/
Poly poly_new(FpCtx *K, int n);
Poly poly_from_coeffs(FpCtx *K, const int *coeffs, int m);
Poly poly_clone(const Poly *A);
void poly_free(Poly *A);
void poly_trim(Poly *A);
void poly_normalize(Poly *A);


/* Operations Basiques */
Poly poly_add(const Poly *A, const Poly *B);
Poly poly_sub(const Poly *A, const Poly *B);
Poly poly_scalar_mul(const Poly *A, int s);
Poly poly_shift(const Poly *A, int k);
Poly poly_mul(const Poly *A, const Poly *B);
void poly_divmod(const Poly *A, const Poly *B, Poly *Q, Poly *R);


/* calcul du PGCD */
typedef struct{
    Poly g,u,v;
} EGCD; // u*A + v*B = g (monic g)
EGCD poly_egcd(Poly A, Poly B);
Poly poly_gcd(const Poly *A, const Poly *B);


/* I/O helpers */
void poly_print(const Poly *A);


#endif