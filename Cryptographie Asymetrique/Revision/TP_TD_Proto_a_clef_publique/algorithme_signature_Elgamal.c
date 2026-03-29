#include <stdio.h>
#include <stdlib.h>
#include </opt/homebrew/etc/openssl@3/include/openssl/sha.h> //opt/homebrew/etc/openssl@3/include/openssl/sha.h
#include "algoritheme_Chif_dechi_Elgamal.h"
#include "algorithme_RSA.h"

Cipher_Elgamal *signature_Elgamal(long long message, Private_key_Elgamal *private_key, Field_q *field){
    if(!private_key || !field) return NULL;
    Cipher_Elgamal *signature = malloc(sizeof(Cipher_Elgamal));
    do
    {  
        long long k = random_prime_number() % (field->q - 1) + 1;
        signature->gen_power_k = exponentiation_rapide(field->generator, k, field->q);
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1((unsigned char *)&message, sizeof(message), hash);
        long long hashed_val = 0;
        memcpy(&hashed_val, hash, sizeof(long long));
        hashed_val = hashed_val % (field->q - 1);
        if (hashed_val < 0) hashed_val += (field->q - 1);
        long long invers_k = inverse_modulaire(k, field->q - 1);
        long long shared_secret = multiplication_modulaire(private_key->key_private, signature->gen_power_k, field->q);
        long long precalculus = (hashed_val-shared_secret) % (field->q - 1);
        signature->cipher = multiplication_modulaire(invers_k, precalculus, field->q);
    } while (signature->cipher == 0);
    
    return signature;
}

bool verification_signature_Elgamal(long long message, Cipher_Elgamal *signature, Public_key_Elgamal *public_key, Field_q *field){
    if(!signature || !public_key || !field) return false;
    if(signature->cipher <= 0 || signature->cipher >= field->q || signature->gen_power_k <= 0 || signature->gen_power_k >= field->q) return false;
    SHA1((unsigned char *)&message, sizeof(message), (unsigned char *)&message);
    long long left_side = exponentiation_rapide(public_key->key_public, signature->gen_power_k, field->q);
    long long right_side = multiplication_modulaire(exponentiation_rapide(field->generator, signature->cipher, field->q), exponentiation_rapide(signature->gen_power_k, signature->cipher, field->q), field->q);
    return left_side == right_side;
}


