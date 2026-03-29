#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operations_corps_finis.h"

// Fonction utilitaire pour automatiser les tests de générateurs
void test_generateur(field_F_q *alpha, extensionF_p *ext, char *nom) {
    if (est_generateur(alpha, ext)) {
        printf("[OK] %s est un generateur de F_q*\n", nom);
    } else {
        printf("[..] %s n'est pas un generateur\n", nom);
    }
}

int main() {
    // --- INITIALISATION DU CORPS F_2 ---
    field_F_p F2 = {2};

    // 1. Définition du polynôme primitif f(X) = X^3 + X + 1 (pour F_2^3)
    // Coeffs : {1, 1, 0, 1} -> 1 + X + X^3
    int coeffs_f[] = {1, 1, 0, 1};
    field_F_q *f = init_polynom(&F2, 4);
    memcpy(f->coefficients, coeffs_f, sizeof(coeffs_f));
    f->deg = 3;

    extensionF_p ext = {f};

    printf("--- TEST DU CORPS F_8 --- \n");
    printf("Polynome generateur : ");
    print_polynom(f);
    printf("--------------------------\n\n");

    // --- TEST 1 : ARITHMÉTIQUE DE BASE ---
    // a = X + 1, b = X^2 + 1
    int c_a[] = {1, 1, 0}; field_F_q *a = init_polynom(&F2, 3);
    memcpy(a->coefficients, c_a, sizeof(c_a)); a->deg = 1;
    
    int c_b[] = {1, 0, 1}; field_F_q *b = init_polynom(&F2, 3);
    memcpy(b->coefficients, c_b, sizeof(c_b)); b->deg = 2;

    field_F_q *res_add = addition_F_p(a, b);
    printf("Addition (X+1) + (X^2+1) : "); print_polynom(res_add);

    field_F_q *res_mult = multiplication_F_q(a, b, &ext);
    printf("Multiplication (X+1)*(X^2+1) mod f : "); print_polynom(res_mult);
    // (X+1)(X^2+1) = X^3 + X^2 + X + 1. Mod X^3+X+1 => X^2

    // --- TEST 2 : INVERSION ---
    printf("\n--- TEST DE L'INVERSE ---\n");
    field_F_q *inv_a = inverse_dans_F_q(a, &ext);
    if (inv_a) {
        printf("Inverse de (X+1) : "); print_polynom(inv_a);
        field_F_q *verif_inv = multiplication_F_q(a, inv_a, &ext);
        printf("Verification (a * a^-1) : "); print_polynom(verif_inv); // Doit afficher 1
        free_polynom(verif_inv);
    }

    // --- TEST 3 : PUISSANCE ---
    printf("\n--- TEST DE PUISSANCE ---\n");
    // (X+1)^7 doit être égal à 1 car l'ordre de F_8* est 7
    field_F_q *puiss_7 = puissance_dans_F_q(a, 7, &ext);
    printf("(X+1)^7 mod f : "); print_polynom(puiss_7); 
    
    field_F_q *puiss_neg = puissance_dans_F_q(a, -1, &ext);
    printf("(X+1)^-1 mod f : "); print_polynom(puiss_neg); // Doit être égal à l'inverse trouvé plus haut

    // --- TEST 4 : GÉNÉRATEUR ---
    printf("\n--- TEST DES GENERATEURS ---\n");
    // Dans F_8, X (coeffs {0, 1}) est souvent un générateur
    int c_alpha[] = {0, 1, 0}; field_F_q *alpha = init_polynom(&F2, 3);
    memcpy(alpha->coefficients, c_alpha, sizeof(c_alpha)); alpha->deg = 1;
    
    test_generateur(alpha, &ext, "X");
    test_generateur(a, &ext, "X+1");

    // --- TEST 5 : CAS LIMITES ---
    printf("\n--- TEST CAS LIMITES ---\n");
    field_F_q *zero = init_polynom(&F2, 1); // deg -1 par défaut
    printf("Test isZero sur polynome nul : %s\n", isZero(zero) ? "OUI" : "NON");
    
    field_F_q *inv_zero = inverse_dans_F_q(zero, &ext);
    if (!inv_zero) printf("Echec correct : On ne peut pas inverser 0.\n");

    // --- NETTOYAGE ---
    free_polynom(f);
    free_polynom(a);
    free_polynom(b);
    free_polynom(inv_a);
    free_polynom(res_add);
    free_polynom(res_mult);
    free_polynom(puiss_7);
    free_polynom(puiss_neg);
    free_polynom(alpha);
    free_polynom(zero);

    printf("\n--- TOUS LES TESTS SONT TERMINES ---\n");
    return 0;
}