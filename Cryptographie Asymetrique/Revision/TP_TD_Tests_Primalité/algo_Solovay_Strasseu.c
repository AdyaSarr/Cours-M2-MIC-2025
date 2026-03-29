#include <stdio.h>
#include <stdlib.h>
#include "algo_Solovay_Strasseu.h"

bool is_prime_solovay_strasseu(long long n, int k){
    if(n <= 1) return false;
    if(n <= 3) return true;
    if(k <= 0) return false;

    for (int i = 0; i < k; i++)
    {
        long long a = random_number(2, n - 2);
        ParamsEEA params = euclideEtendu(a, n);
        if (params.pgcd != 1) return false;
        int x = symbole_legendre(a, n);
        int mod_exp = exponentiation_rapide(a, (n - 1) / 2, n);
        printf("Test %d: a = %lld, symbole_legendre(a, n) = %d, a^((n-1)/2) mod n = %d\n", i + 1, a, x, mod_exp);
        if(((x+=n)%n)!=mod_exp) return false;
    }
    return true;
}

