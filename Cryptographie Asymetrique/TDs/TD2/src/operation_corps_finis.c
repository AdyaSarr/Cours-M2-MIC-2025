#include <stdio.h>
#include <stdlib.h>
#include "../include/operation_corps_finis.h"
#include <string.h>

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
    if (!p || n == 0)
    {
        fprintf(stderr, "Erreur sur les parametres de la fonctions\n");
        return NULL;
    }
    field_F_q *new_poly = malloc(sizeof(field_F_q));
    if (!new_poly)
    {
        fprintf(stderr, "malloc: probleme sur la fonction d'initialisation de polynome\n");
        return NULL;
    }

    new_poly->coefficients = calloc(n, sizeof(int));
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
    new_poly->n = n;
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
        fprintf(stderr, "Erreur: sur les problemes de parametres\n");
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
    while (k >= 0)
    {
        int coeff_poly1 = (k<=poly1->deg)? poly1->coefficients[k]:0;
        int coeff_poly2 = (k<=poly2->deg)? poly2->coefficients[k]:0;
        resultat->coefficients[k] = modulo_dans_F_p(coeff_poly1+coeff_poly2, poly2->k->p);

        k--;
    }

    for (int i = resultat->deg; i >= 0; i--)
    {
        if (resultat->coefficients[i]!=0)
        {
            resultat->deg = i;
            break;
        }
        if (i == 0)
        {
            resultat->deg = -1;
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
    while (k>=0)
    {
        int coeff_poly1 = (k<=poly1->deg)? poly1->coefficients[k]: 0;
        int coeff_poly2 = (k<=poly2->deg)? poly2->coefficients[k]: 0;
        resultat->coefficients[k] = modulo_dans_F_p(coeff_poly1-coeff_poly2, poly1->k->p);

        k--;
    }
    for (int i = resultat->deg; i >=0; i--)
    {
        if (resultat->coefficients[i]!=0)
        {
            resultat->deg = i;
            break;
        }
        if (i==0)
        {
            resultat->deg = -1;
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
    for (int i = resultat->deg; i >= 0; i--)
    {
        if (resultat->coefficients[i] != 0)
        {
            resultat->deg = i;
            break;
        }
        if (i == 0)
        {
            resultat->deg = -1;
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

params_euclide_etendu *euclide_etendu(int entier1, int entier2){
    params_euclide_etendu *resultat = malloc(sizeof(params_euclide_etendu));
    if (!resultat)
    {
        fprintf(stderr, "malloc: sur l'allocation des parametres d'euclide etendu\n");
        return NULL;
    }
    int max = (entier1>entier2? entier1:entier2);
    int min;
    if (max == entier1)
    {
        min = entier2;
    }else
    {
        min = entier1;
    }
    int quotien = 0;
    int reste = 0;
    int x_0 = 1;
    int x_1 = 0;
    int y_0 = 0;
    int y_1 = 1;
    int x_2, y_2; 
    do
    {
        quotien = max/min;
        reste = max - min*quotien;
        x_2 = x_0 - x_1*quotien;
        y_2 = y_0 - y_1*quotien;
        
        max = min;
        min = reste;

        x_0 = x_1;
        x_1 = x_2;

        y_0 = y_1;
        y_1 = y_2;
    } while (reste!=0);
    resultat->pgcd = max;
    resultat->x = x_0;
    resultat->y = y_0;

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

    result_division *resultat = malloc(sizeof(result_division));

    if (!resultat)
    {
        fprintf(stderr, "malloc: sur la fonction de division euclidienne\n");
        return NULL;
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
    resultat->reste = init_polynom(dividende->k, n_reste);
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
        resultat->quotient->coefficients[k] = R_partiel->coefficients[R_partiel->deg]*inverse_mod(diviseur->coefficients[diviseur->deg], diviseur->k->p);
        field_F_q *Coeff_dominant = init_polynom(dividende->k, k+1);
        Coeff_dominant->coefficients[k] = resultat->quotient->coefficients[k];
        Coeff_dominant->deg = k;
        field_F_q *P_partiel = multiplication_F_p(Coeff_dominant, diviseur);
        field_F_q *temp = soustraction_F_p(R_partiel, P_partiel);
        free(R_partiel);
        free(Coeff_dominant);
        free(P_partiel);
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
    resultat->reste = R_partiel;
    return resultat;
}


int main()
{
    field_F_p *F_2 = malloc(sizeof(field_F_p));
    F_2->p = 2;
    // --- INITIALISATION SÉCURISÉE DES DONNÉES ---
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
}