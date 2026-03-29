#include <stdio.h>
#include <stdlib.h>
#include "operations_corps_finis.h"
#include <string.h>
#include <math.h>

int modulo_dans_F_p(const int entier, const int mod){
    int result = 0;
    result = entier%mod;
    if (result < 0)
    {
        result+=mod;
    }
    return result;
}


field_F_q *init_polynom(const field_F_p *p, int n){
    if (!p)
    {
        fprintf(stderr, "Erreur sur les parametres de la fonctions\n");
        return NULL;
    }
    int actual_n = (n <= 0) ? 1 : n;

    field_F_q *new_poly = malloc(sizeof(field_F_q));
    if (!new_poly)
    {
        fprintf(stderr, "malloc: probleme sur la fonction d'initialisation de polynome\n");
        return NULL;
    }

    new_poly->coefficients = calloc(actual_n, sizeof(int));
    if (!new_poly->coefficients)
    {
        fprintf(stderr, "calloc: probleme sur init_polynom allocation coeffeicients\n");
        free(new_poly);
        return NULL;
    }
    new_poly->k = malloc(sizeof(field_F_p));
    if (!new_poly->k)
    {
        fprintf(stderr, "malloc: sur init_polynom la caracteristique\n");
        free(new_poly);
        free(new_poly->coefficients);
        return NULL;
    }
    new_poly->k->p = p->p;
    new_poly->deg = -1;
    new_poly->n = actual_n;
    return new_poly;
}


void print_polynom(const field_F_q *poly){
    if (!poly || !poly->k || poly->k->p == 0)
    {
        fprintf(stderr, "Erreur: sur le parametre de la fonction print_polynom\n");
        return;
    }
    
    if (poly->deg==-1 || poly->n == 0)
    {
        fprintf(stdout, "O dans F_%d[X]\n", poly->k->p);
        return;
    }
    printf("F_%d[X] : ", poly->k->p);
    int flag_affichage_plus = 0;
    int *coeffs = poly->coefficients;
    for (int i = poly->deg; i >= 0; i--)
    {
        int coeff = modulo_dans_F_p(coeffs[i], poly->k->p);
        
        if (coeff == 0)
        {
            continue;
        }
        if (flag_affichage_plus)
        {
            printf(" + ");
        }
        if (coeff != 1 || i == 0) {
            printf("%d", coeff);
        }
        if (i == 1) {
            printf("X");
        } else if (i > 1) {
            printf("X^%d", i);
        }
        flag_affichage_plus = 1;
    }
    if (flag_affichage_plus == 0) {
        printf("0");
    }
    printf("\n");
}


field_F_q *addition_F_p(const field_F_q *poly1, const field_F_q *poly2){
    if (!poly1 || !poly2)
    {
        fprintf(stderr, "Erreur: Problemes de parametres\n");
        return NULL;
    }
    if (poly1->k->p != poly2->k->p)
    {
        fprintf(stdout, "Désolé sur on peut pas additionner les deux polynomes en parametres\n");
        return NULL;
    }
    
    int n = (poly1->n >= poly2->n)? poly1->n: poly2->n;
    field_F_q *resultat = init_polynom(poly1->k, n);
    if (!resultat)
    {
        fprintf(stderr, "probleme sur l'initialisation d'un polynome\n");
        return NULL;
    }
    
    resultat->deg = (poly1->deg > poly2->deg)? poly1->deg:poly2->deg;
    int k = resultat->deg;
    int max_deg = k;
    while (k >= 0)
    {
        int coeff_poly1 = (k<=poly1->deg)? poly1->coefficients[k]:0;
        int coeff_poly2 = (k<=poly2->deg)? poly2->coefficients[k]:0;
        resultat->coefficients[k] = modulo_dans_F_p(coeff_poly1+coeff_poly2, poly2->k->p);

        k--;
    }

    resultat->deg = -1; 
    for (int i = max_deg; i >= 0; i--) {
        if (resultat->coefficients[i] != 0) {
            resultat->deg = i;
            break;
        }
    }
    return resultat;
}


field_F_q *soustraction_F_p(const field_F_q *poly1, const field_F_q *poly2){
    if (!poly1 || !poly2)
    {
        fprintf(stderr, "Erreurs: sur les parametres de la focntions\n");
        return NULL;
    }
    if (poly1->k->p != poly2->k->p)
    {
        fprintf(stdout, "Désole on peut pas faire la soustraction des deux polynomes car ils ne sont pas definis sur le meme corps de base F_%d != F_%d", poly1->k->p, poly2->k->p);
        return NULL;
    }
    int n = (poly1->n >= poly2->n)? poly1->n : poly2->n;
    field_F_q *resultat = init_polynom(poly1->k, n);
    if (!resultat)
    {
        fprintf(stderr, "probleme sur l'initialisation d'un polynome\n");
        return NULL;
    }
    resultat->deg = (poly1->deg >= poly2->deg)? poly1->deg: poly2->deg;

    int k = resultat->deg;
    int max_deg = k;
    while (k>=0)
    {
        int coeff_poly1 = (k<=poly1->deg)? poly1->coefficients[k]: 0;
        int coeff_poly2 = (k<=poly2->deg)? poly2->coefficients[k]: 0;
        resultat->coefficients[k] = modulo_dans_F_p(coeff_poly1-coeff_poly2, poly1->k->p);

        k--;
    }
    resultat->deg = -1; 
    for (int i = max_deg; i >= 0; i--) {
        if (resultat->coefficients[i] != 0) {
            resultat->deg = i;
            break;
        }
    }
    return resultat;
}


field_F_q *multiplication_F_p(const field_F_q *poly1, const field_F_q *poly2){
    if (!poly1 || !poly2)
    {
        fprintf(stderr, "Erreurs: sur les parametres d'entree de la fonction multiplication\n");
        return NULL;
    }
    if (poly1->k->p != poly2->k->p)
    {
        fprintf(stdout, "Désole on peut pas faire la multiplication des deux polynomes car ils ne sont pas definis sur le meme corps de base F_%d != F_%d", poly1->k->p, poly2->k->p);
        return NULL;
    }
    int n = poly1->deg + poly2->deg + 1;
    field_F_q *resultat = init_polynom(poly1->k, n);
    if (!resultat)
    {
        fprintf(stderr, "probleme sur l'initialisation d'un polynome\n");
        return NULL;
    }
    resultat->deg = poly1->deg+poly2->deg;
    int k = resultat->deg;
    int max_deg = k;
    while (k>=0)
    {
        int somme = 0;
        for (int i = 0; i <= k; i++)
        {
            int coeff_poly1 = (i<=poly1->deg)? poly1->coefficients[i]: 0;
            int coeff_poly2 = (k-i<=poly2->deg)? poly2->coefficients[k-i]: 0;
            somme += coeff_poly1*coeff_poly2;
        }
        resultat->coefficients[k] = modulo_dans_F_p(somme, poly1->k->p);
        k--;
    }
    resultat->deg = -1; 
    for (int i = max_deg; i >= 0; i--) {
        if (resultat->coefficients[i] != 0) {
            resultat->deg = i;
            break;
        }
    }
    return resultat;
    
}


field_F_q *copy_polynom(const field_F_q *source){
    if (!source)
    {
        fprintf(stderr, "Erreurs: sur les parametres d'entree de la fonction de la fonction copy\n");
        return NULL;
    }
    int n = source->n;
    field_F_q *destination = init_polynom(source->k, n);
    if (!destination)
    {
        fprintf(stderr, "Erreur: sur le polynome copie de la fonction copy\n");
        return NULL;
    }
    destination->deg = source->deg;
    size_t nb = (size_t) n*sizeof(int);
    memcpy(destination->coefficients, source->coefficients, nb);

    return destination;
}


void free_polynom(field_F_q *poly) {
    if (!poly) return;
    if (poly->coefficients) free(poly->coefficients);
    if (poly->k) free(poly->k);
    free(poly);
}


params_euclide_etendu *euclide_etendu(const int entier1, const int entier2){
    params_euclide_etendu *resultat = malloc(sizeof(params_euclide_etendu));
    if (!resultat)
    {
        fprintf(stderr, "malloc: sur l'allocation des parametres d'euclide etendu\n");
        return NULL;
    }
    int r0 = entier1, r1 = entier2;
    int x0 = 1, x1 = 0;
    int y0 = 0, y1 = 1;

    int q, r2, x2, y2;
    while (r1!=0)
    {
        q = r0/r1;
        r2 = r0 - q*r1;
        x2 = x0 - q*x1;
        y2 = y0 - q*y1;

        r0 = r1;
        r1 = r2;

        x0 = x1;
        x1 = x2;

        y0 = y1;
        y1 = y2;        
    }
    resultat->pgcd = r0;
    resultat->x = x0;
    resultat->y = y0;
    if (resultat->pgcd < 0)
    {
        resultat->pgcd *=-1;
        resultat->x *=-1;
        resultat->y *=-1;
    }
    
    return resultat;
}


int inverse_mod(const int entier,const int mod){
    int inv = 0;
    params_euclide_etendu *result = euclide_etendu(entier, mod);
    if (!result)
    {
        fprintf(stderr, "probleme d'allocation sur la fonction inverse_mod\n");
        return -1;
    }

    if (result->pgcd != 1)
    {
        return -1;
    }
    inv = result->x;
    if (inv < 0) {
        inv += mod;
    }
    free(result);
    return inv;
}


result_division *division_eucliduenne(field_F_q *dividende, field_F_q *diviseur){
    if (!dividende || !diviseur)
    {   
        fprintf(stderr, "Erreurs: sur les parametres de la fonction division euclidienne\n");
        return NULL;
    }
    if (dividende->k->p != diviseur->k->p)
    {
        fprintf(stdout, "Désole on peut pas faire la division euclidienne des deux polynomes car ils ne sont pas definis sur le meme corps de base F_%d != F_%d", dividende->k->p, diviseur->k->p);
        return NULL;
    }
    if (diviseur->deg < 0) {
        fprintf(stderr, "Erreur : Division par le polynôme nul\n");
        return NULL;
    }

    result_division *resultat = malloc(sizeof(result_division));

    if (!resultat)
    {
        fprintf(stderr, "malloc: sur la fonction de division euclidienne\n");
        return NULL;
    }
    if (dividende->deg < diviseur->deg) {
        resultat->quotient = init_polynom(dividende->k, 1); // Taille 1 pour polynôme nul
        resultat->reste = copy_polynom(dividende);
        return resultat;
    }

    int n_quotient = dividende->deg - diviseur->deg + 1;
    int n_reste = diviseur->deg;

    resultat->quotient = init_polynom(dividende->k, n_quotient);
    if (!resultat->quotient)
    {
        fprintf(stderr, "Probleme d'initialisation du quotient sur la fonction division euclidienne\n");
        free(resultat);
        return NULL;
    }
    resultat->reste = init_polynom(dividende->k, n_reste+1);
    if (!resultat->reste)
    {
        fprintf(stderr, "Probleme d'initialisation du reste sur la fonction division euclidienne\n");
        free(resultat->quotient);
        free(resultat);
        return NULL;
    }
    if (dividende->deg < diviseur->deg)
    {
        free_polynom(resultat->reste);
        resultat->reste = copy_polynom(dividende);
        return resultat;
    }

    // Preparation de mes polynomes
    for (int  i = 0; i <= dividende->deg; i++)
    {
        dividende->coefficients[i]= modulo_dans_F_p(dividende->coefficients[i], dividende->k->p);
    }
    for (int  i = 0; i <= diviseur->deg; i++)
    {
        diviseur->coefficients[i]= modulo_dans_F_p(diviseur->coefficients[i], diviseur->k->p);
    }
    
    field_F_q *R_partiel = copy_polynom(dividende);
    while (R_partiel->deg >= diviseur->deg)
    {
        int k = R_partiel->deg - diviseur->deg;
        int inv = inverse_mod(diviseur->coefficients[diviseur->deg], diviseur->k->p);
        resultat->quotient->coefficients[k] = modulo_dans_F_p(R_partiel->coefficients[R_partiel->deg]* inv, diviseur->k->p);
        field_F_q *Coeff_dominant = init_polynom(dividende->k, k+1);
        Coeff_dominant->coefficients[k] = resultat->quotient->coefficients[k];
        Coeff_dominant->deg = k;
        field_F_q *P_partiel = multiplication_F_p(Coeff_dominant, diviseur);
        field_F_q *temp = soustraction_F_p(R_partiel, P_partiel);
        free_polynom(R_partiel);
        free_polynom(Coeff_dominant);
        free_polynom(P_partiel);
        R_partiel = temp;
        for (int i = R_partiel->deg; i >=0; i--)
        {
            if (R_partiel->coefficients[i]!=0)
            {
                R_partiel->deg = i;
                break;
            }
            if (i==0)
            {
                R_partiel->deg = -1;
            }
        }
    }
    free(resultat->reste);
    R_partiel->deg = -1; 
    for (int i = R_partiel->n - 1; i >= 0; i--) {
        if (R_partiel->coefficients[i] != 0) {
            R_partiel->deg = i;
            break;
        }
    }
    resultat->quotient->deg = -1;
    for (int i = resultat->quotient->n - 1; i >= 0; i--) {
        if (resultat->quotient->coefficients[i] != 0) {
            resultat->quotient->deg = i;
            break;
        }
    }
    resultat->reste = R_partiel;
    return resultat;
}


field_F_q *multiplication_F_q(const field_F_q *poly1, const field_F_q *poly2, const extensionF_p *polyprimitif){
    if (!poly1 || !poly2 || !polyprimitif)
    {
        fprintf(stderr, "Erreur(multiplication_F_q): sur les parametres\n");
        return NULL;
    }
    field_F_q *multi = multiplication_F_p(poly1, poly2);
    if (!multi)
    {
        fprintf(stderr, "Erreur(multiplication_F_q): calcul du produit dans F_p[X]\n");
        return NULL;
    }
    result_division *resultat = division_eucliduenne(multi, polyprimitif->polygenerateur);
    if (!resultat)
    {
        fprintf(stderr, "Erreur(multiplication_F_q): de la division");
        free_polynom(multi);
        return NULL;
    }

    field_F_q *result = copy_polynom(resultat->reste);
    free_polynom(resultat->quotient);
    free_polynom(resultat->reste);
    free(resultat);
    free_polynom(multi);
    return result;
}


bool isZero(field_F_q *poly){
    if (!poly)
    {
        return true;
    }
    if (poly->deg==-1)
    {
        return true;
    }
    
    return false;
}


params_euclide_etendu_poly *euclide_etendu_poly(const field_F_q *poly1, const field_F_q *poly2){
    if (!poly1 || !poly2)
    {
        fprintf(stderr, "Erreur(multiplication_F_q): sur les parametres\n");
        return NULL;
    }
    field_F_q *R0 = copy_polynom(poly1), *R1 = copy_polynom(poly2);
    field_F_q *U0 = init_polynom(poly1->k, 1);
    if (!U0)
    {
        fprintf(stderr, "Erreur(euclide_etendu_poly): creation du polynome 1");
        return NULL;
    }
    U0->coefficients[0]=1;
    U0->deg = 0;
    field_F_q *U1 = init_polynom(poly1->k, 1);
    if (!U1)
    {
        fprintf(stderr, "Erreur(euclide_etendu_poly): creation du polynome 0");
        return NULL;
    }
    
    field_F_q *V0 = init_polynom(poly1->k, 1);
    if (!V0)
    {
        fprintf(stderr, "Erreur(euclide_etendu_poly): creation du polynome 0");
        return NULL;
    }
    field_F_q *V1 = init_polynom(poly1->k, 1);
    if (!V1)
    {
        fprintf(stderr, "Erreur(euclide_etendu_poly): creation du polynome 1");
        return NULL;
    }
    V1->coefficients[0]=1;
    V1->deg = 0;

    field_F_q *Q1, *R2, *U2, *V2;

    params_euclide_etendu_poly *resultat = malloc(sizeof(params_euclide_etendu_poly));
    if (!resultat)
    {
        fprintf(stderr, "Erreur(euclide_etendu_poly): d'allocation pour le resultat\n");
        return NULL;
    }
    
    while (!isZero(R1))
    {
        result_division *resultDiv = division_eucliduenne(R0, R1);
        Q1 = copy_polynom(resultDiv->quotient);
        R2 = copy_polynom(resultDiv->reste);

        field_F_q *temp1 = multiplication_F_p(Q1, U1);
        U2 = soustraction_F_p(U0, temp1);
        free_polynom(temp1);
        field_F_q *temp2 = multiplication_F_p(Q1, V1);
        V2 = soustraction_F_p(V0, temp2);
        free_polynom(temp2);

        free_polynom(R0);
        R0 = R1;
        R1 = R2;

        free_polynom(U0);
        U0 = U1;
        U1 = U2;

        free_polynom(V0);
        V0 = V1;
        V1 = V2;

        free_polynom(Q1);
        free_polynom(resultDiv->quotient);
        free_polynom(resultDiv->reste);
        free(resultDiv);
    }
    resultat->PGCD = R0;
    resultat->U = U0;
    resultat->V = V0;
    if (!isZero(resultat->PGCD))
    {
        int max_deg_pgcd = resultat->PGCD->deg;
        if (resultat->PGCD->coefficients[max_deg_pgcd]!=1)
        {
            int inv = inverse_mod(resultat->PGCD->coefficients[max_deg_pgcd], poly1->k->p);
            for (int i = 0; i <= max_deg_pgcd; i++)
            {
                resultat->PGCD->coefficients[i]=modulo_dans_F_p(resultat->PGCD->coefficients[i]*inv, poly1->k->p);
            }
            for (int i = 0; i <= resultat->U->deg; i++)
            {
                resultat->U->coefficients[i]=modulo_dans_F_p(resultat->U->coefficients[i]*inv, poly1->k->p);
            }
            for (int i = 0; i <= resultat->V->deg; i++)
            {
                resultat->V->coefficients[i]=modulo_dans_F_p(resultat->V->coefficients[i]*inv, poly1->k->p);
            }
        }
    }
    
    free_polynom(R1);
    free_polynom(U1);
    free_polynom(V1);
    return resultat;
}


field_F_q *inverse_dans_F_q(field_F_q *poly, extensionF_p *polyprimitif){
    if (!poly || !polyprimitif || !polyprimitif->polygenerateur)
    {
        fprintf(stderr, "Erreur(inverse_dans_F_q): sur les arguments de la focntion\n");
        return NULL;
    }
    if (isZero(poly)) {
        fprintf(stderr, "Erreur : Tentative d'inversion du polynôme nul\n");
        return NULL;
    }
    params_euclide_etendu_poly *resultat = euclide_etendu_poly(poly, polyprimitif->polygenerateur);
    if (!resultat || !resultat->PGCD || !resultat->U || !resultat->V)
    {
        fprintf(stderr, "Erreur(inverse_dans_F_q): sur calcul des coefficients de Bezout\n");
        return NULL;
    }
    if (resultat->PGCD->deg!=0 || resultat->PGCD->coefficients[0] != 1)
    {
        fprintf(stderr, "Erreur(inverse_dans_F_q): le PCGD n'est pas le polynome constant");
        free_polynom(resultat->U);
        free_polynom(resultat->V);
        free_polynom(resultat->PGCD);
        free(resultat);
        return NULL;
    }
    field_F_q *inverse = copy_polynom(resultat->U);
    free_polynom(resultat->U);
    free_polynom(resultat->V);
    free_polynom(resultat->PGCD);
    free(resultat);
    return inverse;
}


field_F_q *puissance_dans_F_q(const field_F_q *current_base, long long exposant, extensionF_p *polyprimitif){
    if (!current_base || !polyprimitif || !polyprimitif->polygenerateur)
    {
        fprintf(stderr, "Erreur(puissance_dans_F_q): sur les arguements de la fonction\n");
        return NULL;
    }
    field_F_q *reste = init_polynom(current_base->k, 1);
    if (!reste)
    {
        fprintf(stderr, "Erreur(puissance_dans_F_q): polynome reste\n");
        return NULL;
    }
    reste->deg = 0;
    reste->coefficients[reste->deg] = 1;
    field_F_q *temp_base = copy_polynom(current_base);
    result_division *resultatDiv = division_eucliduenne(temp_base, polyprimitif->polygenerateur);
    free_polynom(temp_base);

    field_F_q *base = copy_polynom(resultatDiv->reste);
    free_polynom(resultatDiv->quotient);
    free_polynom(resultatDiv->reste);
    free(resultatDiv);
    if (!base)
    {
        fprintf(stderr, "Erreur(puissance_dans_F_q)");
        return NULL;
    }
    
    if (exposant < 0) {
        field_F_q *inv_base = inverse_dans_F_q(base, polyprimitif);
        if (!inv_base)
        {
            fprintf(stderr, "Erreur(puissance_dans_F_q): calcul de l'inverse\n");
            return NULL;
        }
        
        free_polynom(base);
        base = inv_base;
        exposant = -exposant;
    }
    
    while (exposant>0)
    {
        if (exposant&1)
        {
            field_F_q *copyReste = multiplication_F_q(reste, base, polyprimitif);
            free_polynom(reste);
            reste = copyReste;
        }
        exposant >>= 1;
        if (exposant>0)
        {
            field_F_q *baseMultcopy = multiplication_F_q(base, base, polyprimitif);
            free_polynom(base);
            base = baseMultcopy;
        }
    }
    free_polynom(base);
    return reste;
}


bool isOne(field_F_q *poly) {
    if (!poly) return false;
    return (poly->deg == 0 && poly->coefficients[0] == 1);
}


long long *trouver_diviseurs_premiers(long long n, int *nb_facteurs){
    long long *facteurs = malloc(64*sizeof(long long));
    *nb_facteurs = 0;
    long long d = n;

    if (d%2==0)
    {
        facteurs[(*nb_facteurs)++] = 2;
        while(d%2==0) d /= 2;
    }
    for (long long i = 3; i * i <= d; i += 2) {
        if (d % i == 0) {
            facteurs[(*nb_facteurs)++] = i;
            while (d % i == 0) d /= i;
        }
    }
    if (d > 1) {
        facteurs[(*nb_facteurs)++] = d;
    }
    return facteurs;
}


bool est_generateur(field_F_q *alpha, extensionF_p *polyprimitif) {
    if (isZero(alpha)) return false;


    long long p = polyprimitif->polygenerateur->k->p;
    long long d = polyprimitif->polygenerateur->deg;
    long long n = 1;
    for(int i=0; i<d; i++) n *= p;
    n = n - 1;

    int nb_fact = 0;
    long long *facteurs = trouver_diviseurs_premiers(n, &nb_fact);


    bool ok = true;
    for (int i = 0; i < nb_fact; i++) {
        long long exposant_test = n / facteurs[i];
        field_F_q *res = puissance_dans_F_q(alpha, exposant_test, polyprimitif);
        
        if (isOne(res)) {
            ok = false;
            free_polynom(res);
            break; 
        }
        free_polynom(res);
    }

    free(facteurs);
    return ok;
}

/* int main()
{
    field_F_p *F_2 = malloc(sizeof(field_F_p));
    F_2->p = 2;
    int coeff_f_4_data[] = {1, 1, 1};
    field_F_q *f_4 = init_polynom(F_2, sizeof(coeff_f_4_data) / sizeof(int));
    if (f_4 && f_4->coefficients){
        memcpy(f_4->coefficients, coeff_f_4_data, sizeof(coeff_f_4_data));
        f_4->deg = 2;
    }

    int coeff_f_5_data[] = {1, 0, 1, 1, 1};
    field_F_q *f_5 = init_polynom(F_2, sizeof(coeff_f_5_data) / sizeof(int));
    if (f_5 && f_5->coefficients){
        memcpy(f_5->coefficients, coeff_f_5_data, sizeof(coeff_f_5_data));
        f_5->deg = 4;
    }

    int coeff_q_1_data[] = {1,0,0,1};
    field_F_q *q_1 = init_polynom(F_2, sizeof(coeff_q_1_data) / sizeof(int));
    if (q_1 && q_1->coefficients){
        memcpy(q_1->coefficients, coeff_q_1_data, sizeof(coeff_q_1_data));
        q_1->deg = 3;
    }
    
    // 1. Les copies pour la division sont déplacées ici pour la lisibilité
    field_F_q *f_5_div = copy_polynom(f_5); 
    field_F_q *q_1_div = copy_polynom(q_1);

    // --- APPEL AUX FONCTIONS ---
    field_F_q *add_test = addition_F_p(f_4, f_5);
    field_F_q *sub_test = soustraction_F_p(f_5, q_1);
    field_F_q *mult_test = multiplication_F_p(f_5, q_1);
    
    // 2. L'appel à la division utilise les copies pour la non-destruction des originaux
    result_division *div_test = division_eucliduenne(q_1_div, f_5_div);
    
    print_polynom(add_test);
    print_polynom(sub_test);
    print_polynom(mult_test);
    print_polynom(div_test->reste);

    // --- NETTOYAGE FINAL ---
    free(F_2);
    free_polynom(f_4);
    free_polynom(f_5);
    free_polynom(q_1);
    
    // 3. Libération des copies utilisées pour la division
    free_polynom(f_5_div); 
    free_polynom(q_1_div); 

    // Libération des résultats des opérations
    free_polynom(add_test);
    free_polynom(sub_test);
    free_polynom(mult_test);
    
    // Libération du résultat de la division
    free_polynom(div_test->quotient);
    free_polynom(div_test->reste);
    free(div_test);
    return 0;
} */