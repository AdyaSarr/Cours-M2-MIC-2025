#include <stdio.h>
#include "Adya_Sarr.h"
#include <math.h>


long long operation_mod(long long nombre, long long modulo){
    long long result = nombre%modulo;
    if (result<0)
    {
        result+=modulo;
    }
    return;
}


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
long long calcul_terme_prochain(int i, long long y_i, long long d_i ,int r,long long p){
    y_i = operation_mod(y_i, p);
    long long mod = pow(p, i+1);
    return operation_mod(y_i+d_i*exponentiation_rapide(p, i, mod), mod);
}