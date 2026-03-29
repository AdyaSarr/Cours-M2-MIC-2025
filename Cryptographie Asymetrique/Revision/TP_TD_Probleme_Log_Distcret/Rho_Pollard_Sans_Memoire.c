#include <stdio.h>
#include <stdlib.h>
#include "Rho_Pollard_Sans_Memoire.h"
#include "../TP_TD_Proto_a_clef_publique/algorithme_RSA.h"
#include "../TP_TD_Racine_nieme_unite_residu_quadratique/exponentiation_rapide.h"

long long fonction_mod(long long entier, long long mod){
    long long result = entier%mod;
    if (result<0)
    {
        result+=mod;
    }
    return result;
}

Etat *fonction_pseudo_aleatoire(const Etat *courant, const long long order, const long long modulus, const long long gen, const long long elem) {
    if (!courant) return NULL;
    
    long long valeur = courant->y;
    Etat *suivant = malloc(sizeof(Etat));
    if (!suivant) return NULL;


    if (valeur % 3 == 0) {
        suivant->y = (valeur * valeur) % modulus;
        suivant->alpha = (courant->alpha * 2) % order;
        suivant->beta = (courant->beta * 2) % order;
    } else if (valeur % 3 == 1) {
        suivant->y = (valeur * elem) % modulus;
        suivant->alpha = courant->alpha;
        suivant->beta = (courant->beta + 1) % order;
    } else {
        suivant->y = (valeur * gen) % modulus;
        suivant->alpha = (courant->alpha + 1) % order;
        suivant->beta = courant->beta;
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

long long attaque_Rho_Pollard(long long order, long long modulus, long long gen, long long elem) {
    Etat *etat_y_i = malloc(sizeof(Etat));
    if (!etat_y_i) return -1;
    etat_y_i->y = 1; 
    etat_y_i->alpha = 0; 
    etat_y_i->beta = 0;
    
    Etat *etat_y_2i = copy_etat(etat_y_i);
    do {
        Etat *tmp;
        tmp = fonction_pseudo_aleatoire(etat_y_i, order, modulus, gen, elem);
        free(etat_y_i);
        etat_y_i = tmp;
        tmp = fonction_pseudo_aleatoire(etat_y_2i, order, modulus, gen, elem);
        Etat *tmp2 = fonction_pseudo_aleatoire(tmp, order, modulus, gen, elem);
        free(tmp);
        free(etat_y_2i);
        etat_y_2i = tmp2;

        if (!etat_y_i || !etat_y_2i) return -1;

    } while (etat_y_i->y != etat_y_2i->y);
    long long diff_beta = fonction_mod(etat_y_i->beta - etat_y_2i->beta, order);
    long long diff_alpha = fonction_mod(etat_y_2i->alpha - etat_y_i->alpha, order);

    // 3. Gestion du cas où l'inverse n'existe pas directement
    ParamsEEA pgcd_result = euclideEtendu(diff_beta, order);
    long long d = pgcd_result.pgcd; 

    // Si diff_alpha n'est pas divisible par d, il n'y a pas de solution
    if (diff_alpha % d != 0) {
        free(etat_y_i); free(etat_y_2i);
        return -1;
    }

    // 4. Résolution du système réduit
    long long order_reduced = order / d;
    long long diff_beta_reduced = diff_beta / d;
    long long diff_alpha_reduced = diff_alpha / d;

    // L'inverse existe forcément modulo order_reduced
    long long inv = inverse_modulaire(diff_beta_reduced, order_reduced);
    long long x0 = fonction_mod(diff_alpha_reduced * inv, order_reduced);

    // 5. Test des d solutions candidates (x = x0 + k*(n/d))
    long long solution_finale = -1;
    for (int k = 0; k < d; k++) {
        long long candidat = x0 + k * order_reduced;
        // Vérification g^candidat == elem [mod modulus]
        if (exponentiation_rapide(gen, candidat, modulus) == elem) {
            solution_finale = candidat;
            break;
        }
    }

    free(etat_y_i);
    free(etat_y_2i);
    return solution_finale;
}
