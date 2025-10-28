#include <stdio.h>
#include <stdlib.h>
#include </opt/homebrew/opt/gmp/include/gmp.h>
#include <stdbool.h>
#include <math.h>

 
typedef struct {
    int deg;
    mpz_t *coeffs;
} poly_t;

void poly_init(poly_t *p, int deg) {
    p->deg = deg;
    p->coeffs = malloc((deg + 1) * sizeof(mpz_t));
    for (int i = 0; i <= deg; i++) 
        mpz_init(p->coeffs[i]);
}

void poly_clear(poly_t *p) {
    for (int i = 0; i <= p->deg; i++) 
        mpz_clear(p->coeffs[i]);
    free(p->coeffs);
}

void poly_zero(poly_t *p) {
    for (int i = 0; i <= p->deg; i++) 
        mpz_set_ui(p->coeffs[i], 0);
}
// evalu le polynome en x
void poly_eval_integer(mpz_t out, poly_t *p, mpz_t x) {
    mpz_t acc; 
    mpz_init(acc); 
    mpz_set_ui(acc,0);
    for (int i = p->deg; i >= 0; i--) {
        mpz_mul(acc, acc, x);
        mpz_add(acc, acc, p->coeffs[i]);
    }
    mpz_set(out, acc);
    mpz_clear(acc);
}

void poly_mul_scalar(poly_t *p, mpz_t c) {
    for (int i = 0; i <= p->deg; i++) 
        mpz_mul(p->coeffs[i],p->coeffs[i],c);
}
//p^e
void poly_pow(poly_t *res, poly_t *p, int e) {
    if (e == 0) { 
        poly_zero(res); 
        mpz_set_ui(res->coeffs[0],1); 
        res->deg=0; 
        return; 
    }
    poly_t acc;
    poly_init(&acc,0);  
    mpz_set_ui(acc.coeffs[0],1);

    poly_t base; 
    poly_init(&base,p->deg);
    for(int i=0;i<=p->deg;i++) 
        mpz_set(base.coeffs[i],p->coeffs[i]);

    int exp=e;
    while(exp){
        if(exp&1){
            poly_t tmp;
            poly_init(&tmp, acc.deg+base.deg); 
            poly_zero(&tmp);
            for(int i=0;i<=acc.deg;i++)
                for(int j=0;j<=base.deg;j++){
                    mpz_t t; 
                    mpz_init(t);
                    mpz_mul(t, acc.coeffs[i], base.coeffs[j]);
                    mpz_add(tmp.coeffs[i+j], tmp.coeffs[i+j], t);
                    mpz_clear(t);
                }
            poly_clear(&acc);
            poly_init(&acc,tmp.deg);
            for(int i=0;i<=tmp.deg;i++) 
                mpz_set(acc.coeffs[i], tmp.coeffs[i]);
            poly_clear(&tmp);
        }
        exp >>= 1;
        if(exp){
            poly_t tmp; 
            poly_init(&tmp, base.deg*2); 
            poly_zero(&tmp);
            for(int i=0;i<=base.deg;i++)
                for(int j=0;j<=base.deg;j++){
                    mpz_t t; 
                    mpz_init(t);
                    mpz_mul(t, base.coeffs[i], base.coeffs[j]);
                    mpz_add(tmp.coeffs[i+j], tmp.coeffs[i+j], t);
                    mpz_clear(t);
                }
            poly_clear(&base); 
            poly_init(&base,tmp.deg);
            for(int i=0;i<=tmp.deg;i++) 
                mpz_set(base.coeffs[i], tmp.coeffs[i]);
            poly_clear(&tmp);
        }
    }
    poly_clear(res); 
    poly_init(res,acc.deg);
    for(int i=0;i<=acc.deg;i++) 
        mpz_set(res->coeffs[i], acc.coeffs[i]);
    poly_clear(&acc); 
    poly_clear(&base);
}

// parti matrice
typedef struct {
    int rows, cols;
    mpz_t **a;
} matrice;

void matrice_init(matrice *M, int r, int c) {
    M->rows = r; 
    M->cols = c;
    M->a = malloc(r * sizeof(mpz_t*));
    for(int i = 0; i < r; i++){
        M->a[i] = malloc(c * sizeof(mpz_t));
        for(int j = 0; j < c; j++) 
            mpz_init(M->a[i][j]);
    }
}

void matrice_clear(matrice *M) {
    for(int i=0;i<M->rows;i++){
        for(int j=0;j<M->cols;j++) 
            mpz_clear(M->a[i][j]);
        free(M->a[i]);
    }
    free(M->a);
}

void matrice_print(matrice *M) {
    for(int i=0;i<M->rows;i++){
        for(int j=0;j<M->cols;j++) {
            gmp_printf("%Zd ", M->a[i][j]);
        }
        printf("\n");
    }
}

//LLL
void lll_reduce(matrice *B, double delta) {
    int n = B->rows;

    mpf_t *b_star = malloc(n * sizeof(mpf_t));
    for (int i = 0; i < n; i++) 
        mpf_init(b_star[i]);

    mpf_t **mu = malloc(n * sizeof(mpf_t*));
    for (int i = 0; i < n; i++) {
        mu[i] = malloc(n * sizeof(mpf_t));
        for (int j = 0; j < n; j++) 
            mpf_init(mu[i][j]);
    }

    mpf_t tmp1, tmp2, tmp_delta; 
    mpf_inits(tmp1, tmp2, tmp_delta, NULL);
    mpz_t dot; 
    mpz_init(dot);

    int k = 1;
    while (k < n) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < i; j++) {
                mpz_set_ui(dot, 0);
                for (int l = 0; l < n; l++) {
                    mpz_t t; 
                    mpz_init(t);
                    mpz_mul(t, B->a[i][l], B->a[j][l]);
                    mpz_add(dot, dot, t);
                    mpz_clear(t);
                }
                mpf_set_z(tmp1, dot);
                mpf_div(tmp1, tmp1, b_star[j]);
                mpf_set(mu[i][j], tmp1);
            }

            mpf_set_ui(b_star[i], 0);
            for (int l = 0; l < n; l++) {
                mpf_set_z(tmp1, B->a[i][l]);
                for (int j = 0; j < i; j++) {
                    mpf_mul(tmp2, mu[i][j], tmp1);
                    mpf_sub(tmp1, tmp1, tmp2);
                }
                mpf_mul(tmp2, tmp1, tmp1);
                mpf_add(b_star[i], b_star[i], tmp2);
            }
        }

        for (int j = k-1; j < k; j++) {
            double q_d = mpf_get_d(mu[k][j]);
            mpz_t q; 
            mpz_init(q);
            if (q_d >= 0) 
                mpz_set_d(q, floor(q_d));
            else           
                mpz_set_d(q, ceil(q_d));

            for (int i_col = 0; i_col < n; i_col++) {
                mpz_t tmp; 
                mpz_init(tmp);
                mpz_mul(tmp, q, B->a[j][i_col]);
                mpz_sub(B->a[k][i_col], B->a[k][i_col], tmp);
                mpz_clear(tmp);
            }
            mpz_clear(q);
        }

        mpf_set_d(tmp_delta, delta);
        mpf_mul(tmp1, b_star[k-1], tmp_delta);

        if (mpf_cmp(b_star[k], tmp1) >= 0) 
            k++;
        else {
            for (int i_col = 0; i_col < n; i_col++) 
                mpz_swap(B->a[k][i_col], B->a[k-1][i_col]);
            if (k>1) 
                k--;
            else 
                k=1;
        }
    }

    for (int i = 0; i < n; i++) {
        mpf_clear(b_star[i]);
        for (int j = 0; j < n; j++)
             mpf_clear(mu[i][j]);
        free(mu[i]);
    }
    free(mu);
    free(b_star);
    mpf_clears(tmp1, tmp2, tmp_delta, NULL);
    mpz_clear(dot);
}


void cooperS(mpz_t racine, poly_t f,mpz_t N){
    
   
    mpz_t X; 
    mpz_inits(X,NULL);
    // Calcul X = floor(N^(1/d)) 
    mpz_root(X, N, f.deg); //f.deg  2
    
    
    // Determination de m  log(N)/(d*log(X))
    int d = f.deg;
    double logN = log(mpz_get_d(N));
    double logX = log(mpz_get_d(X));
    int m = (int)(logN / (d * logX)) ;

    int rows = (m+1)*d;
    int cols = rows;

    matrice M; 
    matrice_init(&M, rows, cols);
    int row_idx = 0;

    //Construction de la matrice pour LLL
    for(int v=0; v < m+1; v++){
        mpz_t Npow; 
        mpz_init(Npow); 
        mpz_pow_ui(Npow, N, m-v);
        poly_t f_pow; 
        poly_init(&f_pow, d*v); 
        poly_pow(&f_pow, &f, v);
        

        for(int u=0; u < d; u++){
            mpz_t temp;
            mpz_init(temp);
            mpz_t X_pow; 
            mpz_init(X_pow); 
            mpz_pow_ui(X_pow, N, u);
            poly_t P;
            poly_init(&P, f_pow.deg + u); 
            poly_zero(&P);
            for( int i=0; i <= f_pow.deg; i++){
                mpz_mul( temp,f_pow.coeffs[i],X);

                mpz_set(P.coeffs[i+u], temp); 

            }

            P.deg = f_pow.deg + u;
            poly_mul_scalar(&P, Npow);
            poly_mul_scalar(&P, X_pow);
            
            for(int c=0; c <= P.deg && c < cols; c++)
                mpz_set(M.a[row_idx][c], P.coeffs[c]);

            row_idx++; 
            poly_clear(&P);
            mpz_clear(temp);
            mpz_clear(X_pow);
        }
        poly_clear(&f_pow);
        mpz_clear(Npow);
        
    }

     

    //Reduction LLL 
    lll_reduce(&M, 0.75);

    

    //Recuperer le vecteur court
    poly_t h; 
    poly_init(&h, cols-1);
    for(int i=0; i < cols; i++) 
        mpz_set(h.coeffs[i], M.a[0][i]);
    h.deg = cols-1;


    //Recherche brute de racine dans [-X, X]
    mpz_t val,moinX; 
    mpz_inits(val,moinX,NULL);
    mpz_neg(moinX,X);
    mpz_set(racine,moinX);
    bool flag=false;
    while(mpz_cmp(racine,X) <= 0){
        poly_eval_integer(val,&h,racine);
        if(mpz_cmp_ui(val,0)==0){ 
            //gmp_printf("racine trouve: %Zd\n",racine); 
            flag=true; 
            break; 
        }
        mpz_add_ui(racine,racine,1);
    }
    if(!flag) {

        //printf("Pas de racine trouve dans [-X,X]\n");

    }
        
    // --- Cleanup ---
    mpz_clears(X,val,moinX,NULL);
    poly_clear(&h);
    matrice_clear(&M);

    
}


int main (){

    poly_t f; 
    poly_init(&f,3);
    mpz_set_si(f.coeffs[0], -1728000000000); //-125000000000  1728000000000
    mpz_set_ui(f.coeffs[1], 0);
    mpz_set_ui(f.coeffs[2], 0);
    mpz_set_ui(f.coeffs[3], 1);

   
    mpz_t N; 
    mpz_t racine;
    mpz_inits(racine,N,NULL);

    
    
    mpz_set_ui(N,1000036000099); 

    cooperS(racine,f,N);
    gmp_printf("racine trouve: %Zd\n",racine); 

    //printf("racine therique");//12000     //5000

    mpz_clear(N);
    poly_clear(&f);

    return 0;


    
}