#if !defined(ADYA_SARR_H)
#define ADYA_SARR_H

long long operation_mod(long long nombre, long long modulo);
long long calcul_terme_prochain(int indice, int r,long long modulo);
long long exponentiation_rapide(long long base, long long exponent, long long modulus);
long long multiplication_modulaire(long long a, long long b, long long m);

#endif // ADYA_SARR_H
