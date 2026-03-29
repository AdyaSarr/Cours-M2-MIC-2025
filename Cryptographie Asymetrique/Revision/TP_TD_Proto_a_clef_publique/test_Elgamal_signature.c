#include <stdio.h>
#include <stdlib.h>
#include "algorithme_signature_Elgamal.h"

int main(int argc, char const *argv[])
{
    Field_q field = { .q = 104729, .generator = 2 };
    Keys_Elgamal *keys = generate_keys_Elgamal(&field);
    long long message = 12345;
    Cipher_Elgamal *signature = signature_Elgamal(message, keys->private_key, &field);
    bool is_valid = verification_signature_Elgamal(message, signature, keys->public_key, &field);
    printf("Message: %lld\n", message);
    printf("Signature: (gen_power_k: %lld, cipher: %lld)\n", signature->gen_power_k, signature->cipher);
    printf("Is the signature valid? %s\n", is_valid ? "Yes" : "No");
    free(keys->private_key);
    free(keys->public_key);
    free(keys);
    free(signature); 
    return 0;
}