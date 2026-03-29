#include <stdio.h>
#include "exponentiation_rapide.h"

int main(int argc, char const *argv[])
{
    long long base = 1926;
    long long exponent = 4;
    long long modulus = 2081;

    fprintf(stdout, "%lld^%lld[%lld] = %lld\n", base, exponent, modulus, exponentiation_rapide(base, exponent, modulus));
    return 0;
}
