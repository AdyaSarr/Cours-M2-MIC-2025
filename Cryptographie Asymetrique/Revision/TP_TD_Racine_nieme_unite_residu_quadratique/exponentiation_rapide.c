#include <stdio.h>
#include "exponentiation_rapide.h"


long long multiplication_modulaire(long long a, long long b, long long m) {
    return (long long)(((__int128)a * b) % m);
}

long long exponentiation_rapide(long long base, long long exponent, long long modulus) {
    long long result = 1;
    base = base % modulus;

    while (exponent > 0) {
        if (exponent % 2 == 1) {
            result = multiplication_modulaire(result, base, modulus);
        }
        exponent = exponent >> 1;
        base = multiplication_modulaire(base, base, modulus);
    }
    return result;
}