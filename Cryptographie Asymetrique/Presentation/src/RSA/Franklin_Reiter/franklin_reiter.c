#include </opt/homebrew/opt/gmp/include/gmp.h> // L'en-tête de la bibliothèque GMP
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Définition de l'exposant public pour cette attaque (E=3)
#define E_VAL 3UL

/**
 * @brief Tente de retrouver les messages M1, M2 et M (original) en utilisant l'attaque de Franklin-Reiter.
 * M est calculé par division entière M = M1 / 2^m.
 * * @param M_out Message clair M trouvé (sortie).
 * @param M1_out Message scellé M1 trouvé (sortie).
 * @param M2_out Message scellé M2 trouvé (sortie).
 * @param C1 Le chiffré C1.
 * @param C2 Le chiffré C2.
 * @param N Le module RSA.
 * @param b Le coefficient b (Delta) de la relation M2 = M1 + b.
 * @return true si l'attaque réussit, false sinon.
 */
bool franklin_reiter_attack(mpz_t M_out, mpz_t M1_out, mpz_t M2_out, const mpz_t C1, const mpz_t C2, const mpz_t N, const mpz_t b) {
    mpz_t A, B, b_pow3, temp1, temp2, b_inv;
    
    mpz_inits(A, B, b_pow3, temp1, temp2, b_inv, NULL);

    // Vérification E=3 (simplifiée)
    if (E_VAL != 3UL) {
        fprintf(stderr, "ATTENTION: Implémentation optimisée pour E=3.\n");
        mpz_clears(A, B, b_pow3, temp1, temp2, b_inv, NULL);
        return false;
    }

    // --- Étape 1: Résolution de M1 via la formule Franklin-Reiter ---

    // Calcul de b^3
    mpz_pow_ui(b_pow3, b, E_VAL); 
    
    // Calcul de A (Dénominateur)
    mpz_mul_ui(temp1, b_pow3, 2UL); 
    mpz_sub(temp1, temp1, C1);      
    mpz_add(temp1, temp1, C2);      
    mpz_mul_ui(temp2, b, 3UL);      
    mpz_mul(A, temp2, temp1);       
    mpz_mod(A, A, N);               

    // Calcul de B (Numérateur)
    mpz_mul(temp1, b, b);           
    mpz_mul_ui(temp1, temp1, 3UL);  
    mpz_mul_ui(temp2, C1, 2UL);     
    mpz_sub(temp2, b_pow3, temp2);  
    mpz_sub(temp2, temp2, C2);      
    mpz_mul(B, temp1, temp2);       
    mpz_mod(B, B, N);               

    // Résolution M1 = -B * A^-1 (mod N)
    if (mpz_invert(b_inv, A, N) == 0) {
        fprintf(stderr, "ÉCHEC: Inversion modulaire de A échouée. Module N ou données incorrectes.\n");
        mpz_clears(A, B, b_pow3, temp1, temp2, b_inv, NULL);
        return false;
    }

    mpz_neg(M1_out, B);             
    mpz_mul(M1_out, M1_out, b_inv); 
    mpz_mod(M1_out, M1_out, N);     

    // M2 = M1 + b (Delta) mod N
    mpz_add(M2_out, M1_out, b);
    mpz_mod(M2_out, M2_out, N);

    // --- Étape 2: Calcul de M (Message clair original) ---
    
    // Pour l'attaque, le message est scellé par M_sealed = M * 2^m + r.
    // M = M_sealed / 2^m (division entière par décalage de bits).
    
    // Calculer la taille du sel m (simplifié) :
    // On suppose une clé de 512 bits pour simplifier l'exemple.
    // m est au maximum floor(N_bits / E^2) = 512 / 9 = 56 bits.
    // En pratique, m est la taille réelle du sel. Supposons 56 bits pour la démo.
    unsigned long M_BIT_SIZE = 512;
    unsigned long M_SEL_BITS = M_BIT_SIZE / (E_VAL * E_VAL) - 1; // e.g., 56 bits de sel max
    
    // Division entière par 2^m : M = M1_out >> m
    mpz_tdiv_q_2exp(M_out, M1_out, M_SEL_BITS);
    
    printf("DEBUG: Sel m utilisé pour le descellement (estimation) : %lu bits\n", M_SEL_BITS);

    mpz_clears(A, B, b_pow3, temp1, temp2, b_inv, NULL);
    return true;
}


int main() {
    // --- Initialisation des variables ---
    mpz_t C1, C2, N, b_delta, M_original, M1_out, M2_out, M_final_out;
    mpz_inits(C1, C2, N, b_delta, M_original, M1_out, M2_out, M_final_out, NULL);
    
    // VALEURS DE L'ATTAQUE FOURNIES PAR L'UTILISATEUR
    const char *C1_HEX = "4ddae2b6b6ebd1fe668d3e4c13c27832e95eb2183cdebe141c667897c2af90fd025168417d41e84061268742562b87ba0d3c0880a2c82b512c5ee267dfad18c2";
    const char *C2_HEX = "301fcf906266b3aa65ef75bbbeee6e8377fee08b26fc5da6f75c7a0bb0846aaa609448498ab881ceb01d633de00fe27a317dca6b1d30f7996bb5fcca0ccf3611";
    const char *DELTA_HEX = "f4251"; 
    const char *N_HEX = "6e764a64c216aee832db7dc0310c3cc8e0bb336179c1a2d0874f34828a2cf9c1e078360ca5d2f6046bffab830ce100f8d996d7205a9f1bcb823e2008f930049d";

    // Chargement des valeurs en GMP (Base 16)
    if (mpz_set_str(C1, C1_HEX, 16) == -1 || 
        mpz_set_str(C2, C2_HEX, 16) == -1 || 
        mpz_set_str(b_delta, DELTA_HEX, 16) == -1 || 
        mpz_set_str(N, N_HEX, 16) == -1) {
        fprintf(stderr, "Erreur lors du chargement des chaînes GMP.\n");
        mpz_clears(C1, C2, N, b_delta, M_original, M1_out, M2_out, M_final_out, NULL);
        return EXIT_FAILURE;
    }

    printf("--- Test Isolé de l'Attaque de Franklin-Reiter ---\n");
    printf("E = %lu, Delta (b) = %s\n", E_VAL, DELTA_HEX);
    
    // --- Exécution de l'attaque ---
    if (franklin_reiter_attack(M_final_out, M1_out, M2_out, C1, C2, N, b_delta)) {
        printf("\nRESULTAT DE L'ATTAQUE (RÉUSSITE) :\n");
        printf("--------------------------------------------------\n");
        
        printf("1. M1 (Message scellé) :\n");
        gmp_printf("   %Zx\n\n", M1_out);

        printf("2. M2 (Message scellé M1 + Delta) :\n");
        gmp_printf("   %Zx\n\n", M2_out);
        
        printf("3. M (Message clair original) :\n");
        gmp_printf("   %Zx\n", M_final_out);
        printf("--------------------------------------------------\n");
        
    } else {
        printf("\nÉCHEC CRITIQUE : L'attaque n'a pas pu trouver M1.\n");
        printf("Vérifiez le module N et si le message est bien lié par M2 = M1 + Delta.\n");
    }

    // Libération des variables
    mpz_clears(C1, C2, N, b_delta, M_original, M1_out, M2_out, M_final_out, NULL);
    return EXIT_SUCCESS;
}