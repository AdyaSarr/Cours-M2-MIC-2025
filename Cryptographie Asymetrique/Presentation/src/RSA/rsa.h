#if !defined(RSA_H)
#define RSA_H

#include <stdbool.h>
#include </opt/homebrew/opt/gmp/include/gmp.h> // L'en-tête de la bibliothèque GMP
#define REP 25


//bool est_primier(const char *strNumber);
void generation_aleatoire_nombre(mpz_t p, mpz_t q, unsigned long nbBit, gmp_randstate_t etat_aleatoire);
void keyGenRSA(mpz_t N, mpz_t D, unsigned long nbBit, gmp_randstate_t etat_aleatoire);
void chiffrer_RSA(mpz_t C, const mpz_t M, const mpz_t E, const mpz_t N);
void dechiffrer_RSA(mpz_t M, const mpz_t C, const mpz_t D, const mpz_t N);
void conversion_message(const char *message, mpz_t corresp);
char *exporter_message(const mpz_t M_dechiffre);


#endif // RSA_H
