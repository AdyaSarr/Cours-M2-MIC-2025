#include <fp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

Poly poly_new(FpCtx *K, int n){
    Poly A;
    A.n = n;
    A.deg = -1;
    A.K = K;
    A.c = (int*)calloc(n, sizeof(int));
    return A;
}

void poly_free(Poly *A){
    if (A->c) free(A->c);
    A->c = NULL;
    A->n = 0;
    A->deg = -1;
}

void poly_trim(Poly *A){
    int d = A->n - 1;
    while (d >= 0 && A->c[d] == 0) d--;
    A->deg = d;
}

void poly_normalize(Poly *A){
    for (int i = 0; i < A->n; ++i) 
        A->c[i] = imod(A->c[i], A->K->p);
    poly_trim(A);
}

Poly poly_from_coeffs(FpCtx *K, const int *coeffs, int m){
    Poly A = poly_new(K, m);
    for(int i=0;i<m;i++)
        A.c[i] = imod(coeffs[i], K->p);
    poly_trim(&A);
    return A;
}

Poly poly_clone(const Poly *A){
    Poly B = poly_new(A->K, A->n);
    memcpy(B.c, A->c, sizeof(int)*A->n);
    B.deg = A->deg; B.K = A->K;
    return B;
}

void poly_reserve(Poly *A, int n){
    if (n <= A->n) return;
    A->c = (int*)realloc(A->c, sizeof(int)*n);
    for(int i=A->n;i<n;i++)
        A->c[i]=0;
    A->n = n;
}

Poly poly_add(const Poly *A, const Poly *B){
    assert(A->K==B->K);//verifie si les poly sont compatible
    int n = (A->n > B->n) ? A->n : B->n;
    Poly C = poly_new(A->K, n);
    for(int i=0;i<n;i++){
        int ai = (i<=A->deg)?A->c[i]:0;
        int bi = (i<=B->deg)?B->c[i]:0;
        C.c[i] = imod(ai + bi, A->K->p);
    }
    poly_trim(&C);
    return C;
}

Poly poly_sub(const Poly *A, const Poly *B){
    assert(A->K==B->K);
    int n = (A->n > B->n) ? A->n : B->n;
    Poly C = poly_new(A->K, n);
    for(int i=0;i<n;i++){
        int ai = (i<=A->deg)?A->c[i]:0;
        int bi = (i<=B->deg)?B->c[i]:0;
        C.c[i] = imod(ai - bi, A->K->p);
    }
    poly_trim(&C);
    return C;
}

Poly poly_scalar_mul(const Poly *A, int s){
    Poly C = poly_new(A->K, A->n);
    for(int i=0;i<=A->deg;i++)
        C.c[i] = imod((long long)A->c[i]*s, A->K->p);
    poly_trim(&C);
    return C;
}

Poly poly_shift(const Poly *A, int k){//A*x^k
    if (A->deg < 0){
        Poly Z = poly_new(A->K, 1);
        return Z;
    }
    Poly C = poly_new(A->K, A->deg + k + 1);
    for(int i=0;i<=A->deg;i++)
        C.c[i+k] = A->c[i];
    C.deg = A->deg + k;
    return C;
}

Poly poly_mul(const Poly *A, const Poly *B){
    assert(A->K==B->K);
    if (A->deg < 0 || B->deg < 0){
        Poly Z = poly_new(A->K, 1);
        return Z;
    }
    int m = A->deg, n = B->deg;
    Poly C = poly_new(A->K, m+n+2);
    for(int i=0;i<=m;i++){
        long long ai = A->c[i];
        for(int j=0;j<=n;j++){
            C.c[i+j] = imod(C.c[i+j] + ai*B->c[j], A->K->p);
        }
    }
    poly_trim(&C);
    return C;
}

void poly_divmod(const Poly *A, const Poly *B, Poly *Q, Poly *R){
    //assert(A->K==B->K);
    int p = A->K->p;
    if (B->deg < 0){
        fprintf(stderr, "Division par un polynome zero\n");
        exit(1);
    }
    Poly r = poly_clone(A);
    Poly q = poly_new(A->K, (A->deg - B->deg + 2>0)?(A->deg - B->deg + 2):1);


    int inv_lead = inv_modp(B->c[B->deg], p);
    while(r.deg >= B->deg && r.deg >= 0){
        int k = r.deg - B->deg;
        int coeff = imod((long long)r.c[r.deg]*inv_lead, p);
        if (q.n <= k) poly_reserve(&q, k+1);
        q.c[k] = imod(q.c[k] + coeff, p);
        for(int j=0;j<=B->deg;j++){
            int idx = j + k;
            if (r.n <= idx) poly_reserve(&r, idx+1);
            r.c[idx] = imod(r.c[idx] - (long long)coeff*B->c[j], p);
        }
        poly_trim(&r);
    }
    poly_trim(&q);
    *Q = q; *R = r;
}





static Poly poly_make_monic(const Poly *G){
    if (G->deg < 0) {
        Poly Z = poly_new(G->K, 1);
        return Z;
    }
    int inv = inv_modp(G->c[G->deg], G->K->p);
    Poly M = poly_scalar_mul(G, inv);
    poly_trim(&M);
    return M;
}


EGCD poly_egcd(Poly A, Poly B){
    /* Iterative extended Euclid */
    FpCtx *K = A.K;
    //assert(K==B.K);
    Poly r0 = A, r1 = B;
    Poly s0 = poly_new(K,1); s0.c[0]=1; s0.deg=0;
    Poly s1 = poly_new(K,1); /* 0 */
    Poly t0 = poly_new(K,1); /* 0 */
    Poly t1 = poly_new(K,1); t1.c[0]=1; t1.deg=0;
    while(r1.deg >= 0){
        Poly q, r; poly_divmod(&r0, &r1, &q, &r);
        Poly tmp;
        tmp = r0; r0 = r1; r1 = r; /* r0,r1 */
        /* s0, s1 = s1, s0 - q*s1 */
        Poly qs1 = poly_mul(&q, &s1);
        Poly s0_minus_qs1 = poly_sub(&s0, &qs1);
        tmp = s0; s0 = s1; s1 = s0_minus_qs1;
        poly_free(&qs1); poly_free(&tmp);
        /* t0, t1 = t1, t0 - q*t1 */
        Poly qt1 = poly_mul(&q, &t1);
        Poly t0_minus_qt1 = poly_sub(&t0, &qt1);
        tmp = t0; t0 = t1; t1 = t0_minus_qt1;
        poly_free(&qt1); poly_free(&tmp);
        poly_free(&q);
    }
    Poly g = poly_make_monic(&r0);
    /* scale s0,t0 accordingly to make g monic */
    int lead = r0.c[r0.deg];
    int inv = inv_modp(lead, K->p);
    Poly u = poly_scalar_mul(&s0, inv);
    Poly v = poly_scalar_mul(&t0, inv);
    /* cleanup */
    poly_free(&r0); poly_free(&r1);
    poly_free(&s0); poly_free(&s1);
    EGCD out = {g,u,v};
    return out;
}
Poly poly_gcd(const Poly *A, const Poly *B){
    EGCD e = poly_egcd(poly_clone(A), poly_clone(B));
    Poly g = e.g; /* steal g */
    poly_free(&e.u); poly_free(&e.v);
    return g;
}


void poly_print(const Poly *A){
    if (A->deg < 0){
        printf("0");
        return;
    }
    for(int i=A->deg;i>=0;i--){
        int ai = A->c[i];
        if(i!=A->deg) printf(" + ");
        printf("%d", ai);
        if(i>0) printf("x");
        if(i>1) printf("^%d", i);
    }
}