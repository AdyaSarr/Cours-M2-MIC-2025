#include <stdio.h>
#include <stdlib.h>
#include "algorithme_RSA.h"


int main(){
    keys_RSA *keys = generation_keys_RSA();
    if (keys == NULL) {
        fprintf(stderr, "Failed to generate RSA keys.\n");
        return EXIT_FAILURE;
    }
    long long message = 123456789;
    long long cipher = encryption_RSA(message, keys->public_key);
    long long decrypted_message = decryption_RSA(cipher, keys->private_key);
    printf("Message: %lld\n", message);
    printf("Cipher: %lld\n", cipher);
    printf("Decrypted Message: %lld\n", decrypted_message);
    free_keys_RSA(keys);
    return 0;
}