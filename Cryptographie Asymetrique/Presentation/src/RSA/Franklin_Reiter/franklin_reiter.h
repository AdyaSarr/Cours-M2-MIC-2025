#ifndef FRANKLIN_REITER_H
#define FRANKLIN_REITER_H

#include <stdbool.h>
#include </opt/homebrew/opt/gmp/include/gmp.h> // L'en-tête de la bibliothèque GMP

/**
 * @brief Tente de retrouver le message M1 en utilisant l'attaque de Franklin-Reiter.
 * * L'attaque utilise le PGCD polynomial pour trouver le facteur linéaire (x - M1)
 * des deux polynômes cryptographiques g1(x) et g2(x) sur l'anneau Z_N[x].
 * Cette implémentation suppose E=3 (le cas le plus simple) et utilise la division
 * polynomiale simplifiée par des inverses modulaires.
 * * @param M1_out Le message clair M1 trouvé (sortie).
 * @param C1 Le chiffré du premier message (M1^E mod N).
 * @param C2 Le chiffré du second message (M2^E mod N).
 * @param E L'exposant public (doit être petit, E=3 est idéal).
 * @param N Le module RSA.
 * @param a Le coefficient a de la relation affine M2 = a*M1 + b.
 * @param b Le coefficient b de la relation affine M2 = a*M1 + b.
 * @return true si M1 est trouvé, false sinon.
 */
bool franklin_reiter_attack(mpz_t M1_out, const mpz_t C1, const mpz_t C2, const mpz_t E, const mpz_t N, const mpz_t a, const mpz_t b);

#endif // FRANKLIN_REITER_H