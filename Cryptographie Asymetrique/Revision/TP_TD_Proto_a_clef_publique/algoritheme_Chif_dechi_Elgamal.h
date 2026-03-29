#if !defined(algoritheme_Chif_dechi_Elgamal_H)
#define algoritheme_Chif_dechi_Elgamal_H
#include <stdbool.h>
typedef struct 
{
    long long q;
    long long generator;
}Field_q;

typedef struct
{
    long long key_private;
}Private_key_Elgamal;

typedef struct
{
    long long key_public;
}Public_key_Elgamal;

typedef struct
{
    Public_key_Elgamal *public_key;
    Private_key_Elgamal *private_key;
}Keys_Elgamal;

typedef struct 
{
    long long gen_power_k;
    long long cipher;
}Cipher_Elgamal;

Keys_Elgamal *generate_keys_Elgamal(Field_q *field);
Cipher_Elgamal *encryption_Elgamal(long long message, Public_key_Elgamal *public_key, Field_q *field);
long long decryption_Elgamal(Cipher_Elgamal *cipher, Private_key_Elgamal *private_key, Field_q *field);

Cipher_Elgamal *signature_Elgamal(long long message, Private_key_Elgamal *private_key, Field_q *field);
bool verification_signature_Elgamal(long long message, Cipher_Elgamal *signature, Public_key_Elgamal *public_key, Field_q *field);




#endif // algoritheme_Chif_dechi_Elgamal_H
