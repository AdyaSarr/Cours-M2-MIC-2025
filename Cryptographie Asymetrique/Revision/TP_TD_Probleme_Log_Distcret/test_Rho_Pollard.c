#include <stdio.h>
#include <stdlib.h>
#include "Rho_Pollard_Sans_Memoire.h"

int main() {
    long long modulus = 181; // p [cite: 137]
    long long order = 180;   // n = p-1 [cite: 136, 139]
    long long gen = 2;       // g [cite: 134]
    long long elem = 153;    // h [cite: 132]

    long long result = attaque_Rho_Pollard(order, modulus, gen, elem);
    
    if (result != -1) {
        printf("Le logarithme discret est : %lld\n", result);
    } else {
        printf("Echec de l'attaque (diff_beta non inversible).\n");
    }
    return 0;
}