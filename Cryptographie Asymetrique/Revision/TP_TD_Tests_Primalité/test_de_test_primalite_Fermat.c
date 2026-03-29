#include <stdio.h>
#include "tests_primalite_pp_thm_Fermat.h"

int main() {
    //long long n = exponentiation_rapide(2, 61, 0)-1; // Un nombre de Carmichael sans le -1
    long long n = exponentiation_rapide(2, 63, 0)-1; // Un nombre de Carmichael sans le -1
    int k = 100;

    if (is_prime_fermat(n, k)) {
        printf("%lld est probablement premier avec une probabilité de %.2f%%.\n", n, (1.0 - 1.0 / (1 << k)) * 100);
        printf("Mais il pourrait etre un entier de Carmichael.\n");
    } else {
        printf("%lld n'est pas premier.\n", n);
    }

    return 0;
}