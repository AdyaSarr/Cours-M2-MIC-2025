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


int main(int argc, char const *argv[])
{
    // --- INITIALISATION DU CORPS F_3 ---
    field_F_p F3 = {3};
    //F(X) = X^7 + X^5 + X^4 -X^3 - X^2 - X + 1
    int coeffs_f[] = {1, 0, 1, 1, -1, -1, -1, 1};
    field_F_q *f = init_polynom(&F3, 8);
    memcpy(f->coefficients, coeffs_f, 8 * sizeof(int));
    f->deg = 7;

    //F'(X) = X^6 + 2X^4 + X^3 - 2X - 1
    int coeffs_f_prime[] = {1, 0, 2, 1, 0, -2, -1};
    field_F_q *f_prime = init_polynom(&F3, 7);
    memcpy(f_prime->coefficients, coeffs_f_prime, 7 * sizeof(int));
    f_prime->deg = 6;


    params_euclide_etendu_poly *res = euclide_etendu_poly(f, f_prime);
    printf("PGCD de f et f' : ");
    print_polynom(res->PGCD);
    printf("U : ");
    print_polynom(res->U);
    printf("V : ");
    print_polynom(res->V);
    return 0;
}
