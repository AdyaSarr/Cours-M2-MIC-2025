#include <stdio.h>
#include "algoritheme_Chif_dechi_Elgamal.h"
#include "algorithme_RSA.h"
#include "../TP_TD_Racine_nieme_unite_residu_quadratique/exponentiation_rapide.h"
#include <stdlib.h>


Keys_Elgamal *generate_keys_Elgamal(Field_q *field){
    Keys_Elgamal *keys = malloc(sizeof(Keys_Elgamal));
    keys->private_key = malloc(sizeof(Private_key_Elgamal));
    keys->public_key = malloc(sizeof(Public_key_Elgamal));

    keys->private_key->key_private = random_prime_number() % (field->q - 1) + 1;
    keys->public_key->key_public = exponentiation_rapide(field->generator, keys->private_key->key_private, field->q);

    return keys;
}

Cipher_Elgamal *encryption_Elgamal(long long message, Public_key_Elgamal *public_key, Field_q *field){
    if(!public_key || !field) return NULL;
    Cipher_Elgamal *cipher = malloc(sizeof(Cipher_Elgamal));
    long long k = random_prime_number() % (field->q - 1) + 1;
    cipher->gen_power_k = exponentiation_rapide(field->generator, k, field->q);
    long long shared_secret = exponentiation_rapide(public_key->key_public, k, field->q);
    cipher->cipher = multiplication_modulaire(message, shared_secret, field->q);
    return cipher;
}

long long decryption_Elgamal(Cipher_Elgamal *cipher, Private_key_Elgamal *private_key, Field_q *field){
    if(!cipher || !private_key || !field) return -1;
    long long shared_secret = exponentiation_rapide(cipher->gen_power_k, private_key->key_private, field->q);
    long long inverse_shared_secret = inverse_modulaire(shared_secret, field->q);
    if (inverse_shared_secret == -1) {
        fprintf(stderr, "Error: Shared secret has no modular inverse.\n");
        return -1;
    }
    return multiplication_modulaire(cipher->cipher, inverse_shared_secret, field->q);
}

int main(int argc, char const *argv[])
{
    Field_q field = { .q = 104729, .generator = 2 };
    Keys_Elgamal *keys = generate_keys_Elgamal(&field);
    long long message = 12345;
    Cipher_Elgamal *cipher = encryption_Elgamal(message, keys->public_key, &field);
    long long decrypted_message = decryption_Elgamal(cipher, keys->private_key, &field);
    printf("Message: %lld\n", message);
    printf("Cipher: (gen_power_k: %lld, cipher: %lld)\n", cipher->gen_power_k, cipher->cipher);
    printf("Decrypted Message: %lld\n", decrypted_message);
    free(keys->private_key);
    free(keys->public_key);
    free(keys);
    free(cipher); 
    return 0;
}