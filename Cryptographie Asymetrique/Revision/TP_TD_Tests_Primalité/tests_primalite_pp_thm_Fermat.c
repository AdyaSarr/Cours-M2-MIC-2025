#include <stdio.h>
#include "tests_primalite_pp_thm_Fermat.h"



long long random_number(long long min, long long max){
    return min + arc4random_uniform((uint64_t)(max - min + 1));
}
bool is_prime_fermat(long long n, int k){
    if(n <= 1) return false;
    if(n <= 3) return true;
    if(k <= 0) return false;

    for (int i = 0; i < k; i++)
    {
        long long a = random_number(2, n - 2);
        int test = exponentiation_rapide(a, n - 1, n);
        printf("Test %d: a = %lld, a^(n-1) mod n = %d\n", i + 1, a, test);
        if(test != 1) return false;
    }
    return true;
}