#include <fp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>



static int mod_p(int a, int p);

/*Creer un polynome nul*/
Poly *poly_creat(Fpctx *p, int n){
    if(!p){
        fprintf(stdout, "Erreur sur les arguments de la fonction\n");
        exit(1);
    }
    Poly *new_poly;

    new_poly = malloc(sizeof*new_poly);
    if(!new_poly){
        fprintf(stderr, "malloc");
        exit(1);
    }

    new_poly->n = n;
    new_poly->coefficient = calloc(n, sizeof(int));
    new_poly->deg = -1;
    new_poly->k = p;
    return new_poly;
}

void poly_free(Poly *p_de_x){
    if(!p_de_x) return;
    free(p_de_x->coefficient);//on libere d'abord les données(coefficients du polynome)
    free(p_de_x->k);//puis on libre la place memoire de k
    free(p_de_x);//on libere la structure a la fin 
}

/*Cherche le representant de a dans {0,....p-1}*/
static int mod_p(int a, int p){
    int r = a%p;
    if(r<0) return r+=p;
    return r;
}

Poly *poly_multiplication(Poly *p_de_x, Poly *q_de_x){
    if(!p_de_x || !q_de_x) return NULL;

    assert(p_de_x->k==q_de_x->k);

    int m = p_de_x->n, l = q_de_x->n;//On sait que le resultat de la multiplicatin deux polynomes ne depasse pas la somme des tailles
    Poly *new_poly = poly_creat(p_de_x->k, m+l);
}