#include <stdio.h>
#include "algo_Miller_Rabin.h"

Decomposition *decompose(long long n){
    Decomposition *result = malloc(sizeof(Decomposition));
    if(!result) return NULL;
    result->t = n - 1;
    result->s = 0;
    if(n <= 1) return result;
    int r = 0;
    long long d = n - 1;
    while (d % 2 == 0) {
        d /= 2;
        r++;
    }
    result->t = d;
    result->s = r;

    return result;
}

bool is_prime_miller_rabin(long long n, int k){
    if(n <=1) return false;
    if(n <= 3) return true;
    if(k <= 0) return false;

    Decomposition *decomp = decompose(n);
    for (int i = 0; i < k; i++)
    {
        long long a = random_number(2, n - 2);
        ParamsEEA params = euclideEtendu(a, n);
        if (params.pgcd != 1){
            free(decomp);
            return false;
        }
        long long x = exponentiation_rapide(a, decomp->t, n);
        if(x%n == 1 || x%n == n-1) {
            continue;
        }else
        {
            for (int i = 1; i < decomp->s; i++)
            {
                x = exponentiation_rapide(x, 2 , n);
                if(x%n == n-1) break;
            }
            if(x%n != n-1){
                free(decomp);
                return false;
            }
        }
    }
    free(decomp);
    return true;
}