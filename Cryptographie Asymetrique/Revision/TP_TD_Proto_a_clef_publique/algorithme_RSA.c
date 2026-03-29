#include <stdio.h>
#include <stdlib.h>
#include "algorithme_RSA.h"
#include "../TP_TD_Racine_nieme_unite_residu_quadratique/exponentiation_rapide.h"
#include <math.h>

bool is_prime(long long n){
    if(n<=1) return false;
    for (long long i = 2; i <= sqrt(n); i++)
    {
        if(n%i == 0) return false;
    }
    return true;
}

long long random_prime_number(void){
    long long prime_number;
    while (true)
    {
        uint32_t small_prime;
        arc4random_buf(&small_prime, sizeof(small_prime));
        prime_number = small_prime % 2147483647; // Max 2^31 - 1
        
        if (prime_number < 100000) continue; // Pour garder une certaine sécurité
        
        if (is_prime(prime_number)) {
            break;
        }
    }
    return prime_number;
}

ParamsEEA euclideEtendu(const long long entier1, const long long entier2){
    ParamsEEA result;

    long long r0 = entier1, r1 = entier2;
    long long x0 = 1, x1 = 0;
    long long y0 = 0, y1 = 1;

    long long q, r2, x2, y2;
    while (r1!=0)
    {
        q = r0/r1;
        r2 = r0 - q*r1;
        x2 = x0 - q*x1;
        y2 = y0 - q*y1;

        r0 = r1;
        r1 = r2;

        x0 = x1;
        x1 = x2;

        y0 = y1;
        y1 = y2;        
    }
    result.pgcd = r0;
    result.x = x0;
    result.y = y0;
    if (result.pgcd < 0)
    {
        result.pgcd *=-1;
        result.x *=-1;
        result.y *=-1;
    }
    
    return result;
}
long long fonction_modulaire(long long entierlong, long modulo){
    long long result = entierlong % modulo;
    if (result < 0)
    {
        result += modulo;
    }
    return result;
}

long long inverse_modulaire(long long a, long long m){
    ParamsEEA result = euclideEtendu(a, m);
    if (result.pgcd != 1) return -1;
    else return fonction_modulaire(result.x, m); 
}

keys_RSA *generation_keys_RSA(void){
    keys_RSA *keys = malloc(sizeof(keys_RSA));
    keys->private_key = malloc(sizeof(private_key));
    keys->public_key = malloc(sizeof(public_key));

    keys->private_key->p = random_prime_number();
    keys->private_key->q = random_prime_number();
    keys->private_key->phi_n = (keys->private_key->p - 1) * (keys->private_key->q - 1);
    keys->private_key->n = keys->public_key->n = keys->private_key->p * keys->private_key->q;
    keys->public_key->e = 65537;
    
    keys->private_key->d = inverse_modulaire(keys->public_key->e, keys->private_key->phi_n);
    if (keys->private_key->d == -1)
    {
        fprintf(stderr, "Error: e and phi_n are not coprime.\n");
        free(keys->private_key);
        free(keys->public_key);
        free(keys);
        return NULL;
    }
    return keys;
}

long long encryption_RSA(long long message, public_key *public_key){
    return exponentiation_rapide(message, public_key->e, public_key->n);
}

long long decryption_RSA(long long cipher, private_key *private_key){
    return exponentiation_rapide(cipher, private_key->d, private_key->n);
}


void free_keys_RSA(keys_RSA *keys){
    free(keys->private_key);
    free(keys->public_key);
    free(keys);
}
