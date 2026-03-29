#include <stdio.h>
#include "algo_Miller_Rabin.h"

int main() {
    long long n = 17;
    int k = 10;
    if (is_prime_miller_rabin(n, k)) {
        printf("%lld est probablement premier avec une probabilité de %.2f%%.\n", n, (1.0 - 1.0 / (1 << k)) * 100);
    } else {
        printf("%lld n'est pas premier.\n", n);
    }
    return 0;
}