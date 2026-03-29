#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../include/attaque_log_discret.h"



int fonction_mod(int entier, int mod){
    int result = entier%mod;
    if (result<0)
    {
        result+=mod;
    }
    return result;
}

params_euclide_etendu *euclide_etendu(int a, int b){
    params_euclide_etendu *resultat = malloc(sizeof(params_euclide_etendu));
    if (!resultat) {
        fprintf(stderr, "malloc: sur l'allocation des parametres d'Euclide etendu\n");
        return NULL;
    }

    int r0 = a;
    int r1 = b;
    int s0 = 1, s1 = 0;  // coeffs pour a
    int t0 = 0, t1 = 1;  // coeffs pour b

    while (r1 != 0) {
        int q  = r0 / r1;
        int r2 = r0 - q * r1;
        int s2 = s0 - q * s1;
        int t2 = t0 - q * t1;

        r0 = r1; r1 = r2;
        s0 = s1; s1 = s2;
        t0 = t1; t1 = t2;
    }

    resultat->pgcd = (r0 >= 0 ? r0 : -r0); // au cas où
    resultat->x = s0;  // coeff de a
    resultat->y = t0;  // coeff de b

    return resultat;
}

int inverse_mod(const int entier, const int mod){
    if (mod <= 0) {
        fprintf(stderr, "inverse_mod: modulo non strictement positif\n");
        return -1;
    }

    int a = fonction_mod(entier, mod);  // normalise dans [0,mod-1]
    params_euclide_etendu *result = euclide_etendu(a, mod);
    if (!result) {
        fprintf(stderr, "probleme d'allocation sur la fonction inverse_mod\n");
        return -1;
    }

    if (result->pgcd != 1) {
        // pas d'inverse modulaire
        free(result);
        return -1;
    }

    int inv = fonction_mod(result->x, mod); // s0 est coeff de 'a'
    free(result);
    return inv;
}



Etat *fonction_pseudo_aleatoire(const Etat *courant,const int order, const int gen, const int elem){
    if (!courant)
    {
        fprintf(stderr, "Erreur sur l'etat\n");
        return NULL;
    }
    int valeur = courant->y;
    Etat *suivant = malloc(sizeof(Etat));
    if (!suivant)
    {
        fprintf(stderr, "malloc: erreur d'allocation dynamique sur la fonction pseudo-alea\n");
        return NULL;
    }
    if (valeur%3==0)
    {
        suivant->alpha = fonction_mod(2*courant->alpha, order-1);
        suivant->beta = fonction_mod(2*courant->beta, order-1);
        suivant->y = fonction_mod((int)((long long)courant->y * courant->y), order);
    }else if (valeur%3==1)
    {
        suivant->alpha = fonction_mod(courant->alpha, order-1);
        suivant->beta = fonction_mod(courant->beta+1, order-1);
        suivant->y = fonction_mod((int)((long long)elem * courant->y), order);
    }else
    {
        suivant->alpha = fonction_mod(courant->alpha+1, order-1);
        suivant->beta = fonction_mod(courant->beta, order-1);
        suivant->y = fonction_mod((int)((long long)gen * courant->y), order);
    }    
    return suivant;
}


Etat *copy_etat(Etat *src){
    Etat *dst = malloc(sizeof(Etat));
    if (!dst)
    {
        fprintf(stderr, "malloc: sur la fonction copie\n");
        return NULL;
    }
    dst->y = src->y;
    dst->alpha = src->alpha;
    dst->beta = src->beta;
    return dst;
}

/*int attaque_Rho_Pollard(int order, int gen, int elem){
    Etat *etat_initial = malloc(sizeof(Etat));
    if (!etat_initial)
    {
        fprintf(stderr, "malloc: sur la fonction pollard\n");
        return -1;
    }
    etat_initial->y = 1;
    etat_initial->alpha = 0;
    etat_initial->beta = 0;

    Etat *etat_y_i = copy_etat(etat_initial);
    Etat *etat_y_2i = copy_etat(etat_initial);
    free(etat_initial);

    etat_y_i = fonction_pseudo_aleatoire(etat_y_i, order, gen, elem);
    etat_y_2i = fonction_pseudo_aleatoire(fonction_pseudo_aleatoire(etat_y_2i, order, gen, elem), order, gen, elem);

    do{
        Etat *old_y_i = etat_y_i; 
        etat_y_i = fonction_pseudo_aleatoire(old_y_i, order, gen, elem);
        free(old_y_i); 
        Etat *old_y_2i_step1 = etat_y_2i;
        Etat *new_y_2i_step1 = fonction_pseudo_aleatoire(old_y_2i_step1, order, gen, elem);
        free(old_y_2i_step1);
        Etat *old_y_2i_step2 = new_y_2i_step1;
        etat_y_2i = fonction_pseudo_aleatoire(old_y_2i_step2, order, gen, elem);
        free(old_y_2i_step2);

    } while (etat_y_2i->y != etat_y_i->y);
    printf("%d\n", etat_y_i->beta - etat_y_2i->beta);
    int trouve, inverse;
    inverse = inverse_mod(etat_y_i->beta - etat_y_2i->beta, order-1);
    if (inverse == -1)
    {
        fprintf(stdout, "La difference des betas n'est pas inversible du on relance l'attaque mais sur le PGCD\n");
        return -1;//pour l'instant j'arret le programme
    }
    
    trouve = (etat_y_2i->alpha - etat_y_i->alpha)*inverse;
    trouve = fonction_mod(trouve, order-1);
    free(etat_y_i);
    free(etat_y_2i);
    return trouve;
}*/
int attaque_Rho_Pollard(int order, int gen, int elem){
    Etat *etat_initial = malloc(sizeof(Etat));
    if (!etat_initial) {
        fprintf(stderr, "malloc: sur la fonction pollard\n");
        return -1;
    }

    etat_initial->y     = 1;
    etat_initial->alpha = 0;
    etat_initial->beta  = 0;

    Etat *etat_y_i  = copy_etat(etat_initial);
    Etat *etat_y_2i = copy_etat(etat_initial);

    if (!etat_y_i || !etat_y_2i) {
        fprintf(stderr, "Erreur d'allocation d'etat\n");
        free(etat_initial);
        free(etat_y_i);
        free(etat_y_2i);
        return -1;
    }

    free(etat_initial);

    /* Première avancée : on suit la même logique que dans la boucle pour éviter les fuites */
    Etat *tmp;

    // y_i <- f(y_i)
    tmp = fonction_pseudo_aleatoire(etat_y_i, order, gen, elem);
    if (!tmp) {
        free(etat_y_i);
        free(etat_y_2i);
        return -1;
    }
    free(etat_y_i);
    etat_y_i = tmp;

    // y_2i <- f(f(y_2i))
    tmp = fonction_pseudo_aleatoire(etat_y_2i, order, gen, elem);
    if (!tmp) {
        free(etat_y_i);
        free(etat_y_2i);
        return -1;
    }
    free(etat_y_2i);
    etat_y_2i = tmp;

    tmp = fonction_pseudo_aleatoire(etat_y_2i, order, gen, elem);
    if (!tmp) {
        free(etat_y_i);
        free(etat_y_2i);
        return -1;
    }
    free(etat_y_2i);
    etat_y_2i = tmp;

    // Boucle de Floyd
    do {
        Etat *old_y_i = etat_y_i;
        etat_y_i = fonction_pseudo_aleatoire(old_y_i, order, gen, elem);
        free(old_y_i);
        if (!etat_y_i) {
            free(etat_y_2i);
            return -1;
        }

        Etat *old_y_2i_step1 = etat_y_2i;
        Etat *new_y_2i_step1 = fonction_pseudo_aleatoire(old_y_2i_step1, order, gen, elem);
        free(old_y_2i_step1);
        if (!new_y_2i_step1) {
            free(etat_y_i);
            return -1;
        }

        Etat *old_y_2i_step2 = new_y_2i_step1;
        etat_y_2i = fonction_pseudo_aleatoire(old_y_2i_step2, order, gen, elem);
        free(old_y_2i_step2);
        if (!etat_y_2i) {
            free(etat_y_i);
            return -1;
        }

    } while (etat_y_2i->y != etat_y_i->y);

    int diff_beta = etat_y_i->beta - etat_y_2i->beta;
    int inverse   = inverse_mod(diff_beta, order - 1);
    if (inverse == -1) {
        fprintf(stdout, "La difference des betas n'est pas inversible, on pourrait relancer l'attaque avec le pgcd.\n");
        free(etat_y_i);
        free(etat_y_2i);
        return -1;
    }

    long long tmp_val = (long long)(etat_y_2i->alpha - etat_y_i->alpha) * inverse;
    int trouve = fonction_mod((int)tmp_val, order - 1);

    free(etat_y_i);
    free(etat_y_2i);

    return trouve;
}

int main(int argc, char const *argv[])
{
    int x = attaque_Rho_Pollard(383, 2, 229);
    printf("%d\n", x);
    return 0;
}
