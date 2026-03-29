#include <stdio.h>
#include <stdlib.h>
#include "algo_Shanks_Tonelli.h"
#include "exponentiation_rapide.h"
#include "symbole_Legendre.h"

// Recherche du plus petit non-résidu quadratique n tel que (n/p) = -1
int trouver_non_residu(long long prime) {
    for (int i = 2; i < prime; i++) {
        if (symbole_legendre(i, prime) == -1) {
            return i;
        }
    }
    return 0; // Ne devrait pas arriver pour un p premier impair
}

long long algo_Shanks_Tonelli(long long a, long long prime) {
    // Cas particulier : a = 0
    if (a % prime == 0) return 0;
    
    // Vérification de l'existence de la racine
    if (symbole_legendre(a, prime) != 1) return -1; 

    // 1. Décomposition de p-1 = s * 2^alpha
    long long s = prime - 1;
    int alpha = 0;
    while (s % 2 == 0) {
        s /= 2;
        alpha++;
    }

    // 2. Initialisation des variables
    int n = trouver_non_residu(prime);
    long long b = exponentiation_rapide(n, s, prime);      // Racine 2^alpha-ième de l'unité 
    long long r = exponentiation_rapide(a, (s + 1) / 2, prime); // Approximation initiale de la racine
    long long t = exponentiation_rapide(a, s, prime);      // Facteur de correction
    int m = alpha;

    // 3. Boucle de raffinement itérative 
    while (t != 1) {
        // Trouver le plus petit k tel que t^(2^k) = 1 
        int k = 0;
        long long temp_t = t;
        while (temp_t != 1 && k < m) {
            temp_t = (temp_t * temp_t) % prime;
            k++;
        }

        // Si k = m, cela signifie qu'il n'y a pas de racine (cas p non premier)
        if (k == m) return -1;

        // Calcul du facteur de correction L = b^(2^(m-k-1)) 
        long long L = b;
        for (int i = 0; i < m - k - 1; i++) {
            L = (L * L) % prime;
        }

        // Mise à jour des variables
        r = (r * L) % prime;          // Mise à jour de la racine
        b = (L * L) % prime;          // Mise à jour de la base de correction
        t = (t * b) % prime;          // Mise à jour du facteur de correction
        m = k;                        // Réduction de l'ordre
    }

    return r;
}


int main(int argc, char const *argv[])
{
    long long prime = 2081;
    long long a = 302;
    long long racine = algo_Shanks_Tonelli(a, prime);
    if (racine != -1) {
        printf("Une racine carrée de %lld modulo %lld est %lld\n", a, prime, racine);
    } else {
        printf("Aucune racine carrée de %lld modulo %lld n'existe.\n", a, prime);
    }
    return 0;
}
