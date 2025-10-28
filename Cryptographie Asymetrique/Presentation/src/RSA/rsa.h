#if !defined(RSA_H)
#define RSA_H

#include <stdbool.h>
#include </opt/homebrew/opt/gmp/include/gmp.h> // L'en-tête de la bibliothèque GMP


unsigned long calculer_taille_sel(const mpz_t E, const mpz_t N, size_t taille_message_clair);
void conversion_message_chiffrement_alea(const char *message_clair, mpz_t C, const mpz_t E, const mpz_t N, gmp_randstate_t etat_aleatoire);
void generation_aleatoire_nombre(mpz_t p, mpz_t q, unsigned long nbBit, gmp_randstate_t etat_aleatoire);
void keyGenRSA(mpz_t N, mpz_t D, const mpz_t E, unsigned long nbBit, gmp_randstate_t etat_aleatoire);
void chiffrer_RSA(mpz_t C, const mpz_t M, const mpz_t E, const mpz_t N);
void dechiffrer_RSA(mpz_t M, const mpz_t C, const mpz_t D, const mpz_t N);
char *exporter_message(const mpz_t M_dechiffre_propre, size_t taille_message_clair);
int ecrire_mpz_fichier(const char *nom_fichier, const mpz_t cle);
int lire_mpz_fichier(const char *nom_fichier, mpz_t cle);
void desceller_message(mpz_t M_clair, const mpz_t M_chiffre_brut, const mpz_t E, const mpz_t N, unsigned long taille_message_clair);
char *lire_message_fichier(const char *nom_fichier);
#endif // RSA_H