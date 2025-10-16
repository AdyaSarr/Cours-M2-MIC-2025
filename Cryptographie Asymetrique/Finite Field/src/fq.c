#include <fq.h>
#include <stdio.h>
#include <assert.h>

FqCtx fqctx_make(int p, const int *fcoeffs, int d){
    FqCtx C; C.K.p = p; C.f = poly_from_coeffs(&C.K, fcoeffs, d+1);
    assert(C.f.deg == d && "f must be degree d");
    /* naive q = p^d */
    int64_t q = 1; for(int i=0;i<d;i++) q *= p; C.q = (int)q;
    return C;
}

void fq_free(FqElem *x){
    poly_free(&x->a); x->F=NULL;
}


FqElem fq_add(const FqElem *x, const FqElem *y){
    Poly s = poly_add(&x->a, &y->a);
    FqElem z = {s, x->F}; return z;
}


FqElem fq_sub(const FqElem *x, const FqElem *y){
    Poly s = poly_sub(&x->a, &y->a);
    FqElem z = {s, x->F}; return z;
}


FqElem fq_mul(const FqElem *x, const FqElem *y){
    Poly p = poly_mul(&x->a, &y->a);
    FqElem z = fq_from_poly(x->F, &p);
    poly_free(&p);
    return z;
}


FqElem fq_pow(FqElem base, uint64_t e){
    FqElem r = fq_from_coeffs(base.F, (int[]){1}, 1); /* 1 */
    while(e){
        if(e&1){
            FqElem t = fq_mul(&r, &base);
            fq_free(&r); r=t;
        }
        e >>= 1;
        if(e){
            FqElem t2 = fq_mul(&base, &base);
            fq_free(&base); base=t2;
        }
    }
    return r;
}

FqElem fq_from_coeffs(FqCtx *F, const int *coeffs, int m){
    Poly A = poly_from_coeffs(&F->K, coeffs, m);
    return fq_from_poly(F, &A);
}

FqElem fq_from_poly(FqCtx *F, const Poly *A){
    Poly Q,R; poly_divmod(A, &F->f, &Q, &R);
    poly_free(&Q);
    FqElem x; x.a = R; x.F = F; return x;
}


FqElem fq_inv(const FqElem *x){
    /* extended Euclid on a(x) and f */
    EGCD e = poly_egcd(poly_clone(&x->a), poly_clone(&x->F->f));
    /* inverse is e.u mod f */
    FqElem z = fq_from_poly(x->F, &e.u);
    poly_free(&e.g); poly_free(&e.u); poly_free(&e.v);
    return z;
}


FqElem fq_div(const FqElem *x, const FqElem *y){
    FqElem yin = fq_inv(y);
    FqElem z = fq_mul(x, &yin);
    fq_free(&yin);
    return z;
}


/* Return 1 if g is a generator of Fq^*, 0 otherwise */
int fq_is_generator(FqElem g){
    if (g.a.deg < 0) return 0; /* 0 */
    if (g.a.deg==0 && g.a.c[0]==1) return 0; /* 1 is not */
    int phi = g.F->q - 1;
    /* factor phi by trial division */
    int primes[64]; int pc=0; int n = phi;
    for(int p=2;p*p<=n;p++) if(n%p==0){ primes[pc++]=p; while(n%p==0) n/=p; }
    if(n>1) primes[pc++]=n;
    for(int i=0;i<pc;i++){
        int ell = primes[i];
        uint64_t exp = (uint64_t)phi / (uint64_t)ell;
        FqElem h = fq_pow(fq_from_poly(g.F, &g.a), exp);
        /* check h == 1 */
        int is_one = (h.a.deg==0 && h.a.c[0]==1);
        fq_free(&h);
        if (is_one) return 0;
    }
    return 1;
}

void fq_print(const FqElem *x){
    poly_print(&x->a);
}