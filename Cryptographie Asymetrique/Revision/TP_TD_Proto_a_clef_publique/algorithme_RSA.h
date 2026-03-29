#if !defined(ALGORITHME_RSA_H)
#define ALGORITHME_RSA_H

#include <stdbool.h>

typedef struct 
{
    long long p;
    long long q;
    long long d;
    long long phi_n;
    long long n;
}private_key;


typedef struct{
    long long n;
    long long e;
}public_key;

typedef struct 
{
    public_key *public_key;
    private_key *private_key;
}keys_RSA;

typedef struct 
{
    long long x;
    long long y;
    long long pgcd;
}ParamsEEA;

ParamsEEA euclideEtendu(const long long entier1, const long long entier2);
long long inverse_modulaire(long long a, long long m);

long long fonction_modulaire(long long entierlong, long modulo);

keys_RSA *generation_keys_RSA(void);
bool is_prime(long long n);
long long random_prime_number(void);

long long encryption_RSA(long long message, public_key *public_key);
long long decryption_RSA(long long cipher, private_key *private_key);



#endif // ALGORITHME_RSA_H
