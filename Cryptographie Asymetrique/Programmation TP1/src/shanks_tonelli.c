#include <stdio.h>
#include <stdlib.h>
#include <time.h>




static unsigned long long multiplication_modulo(unsigned long long a, unsigned long long b, unsigned long long mod);

// Calcule (a * b) % mod
static unsigned long long multiplication_modulo(unsigned long long a, unsigned long long b, unsigned long long mod) {
    return (a * b) % mod; 
}

/*
    Cette fonction permet de calculer (base^exp) % mod
*/
unsigned long long exponentiation_rapide(unsigned long long base, unsigned long long exp, unsigned long long mod) {
    unsigned long long res = 1;
    base %= mod;
    while (exp > 0) {
        // Si le bit actuel est 1, on multiplie le résultat par la base(impair)
        if (exp & 1){
            res = multiplication_modulo(res, base, mod);
        }
        //On élève la base au carré pour le bit suivant
        base = multiplication_modulo(base, base, mod);
        //On passe au bit suivant avec le decalage
        exp >>= 1;
    }
    return res;
}




/**
    Cette fonction permet de calculer une racine carrée de n modulo p (x tel que x^2 = n mod p).
    n est le résidu quadratique.
    p est le nombre premier impair.
    elle retourne racine carrée x, ou 0 si n n'est pas un résidu quadratique.
 */
unsigned long long shanks_tonelli(unsigned long long n, unsigned long long p) {

    // Si p est 2, la seule solution est n (puisque (n)^2 = n mod 2).
    if (p == 2) {
        return n % 2; 
    }

    // Vérification du Critère d'Euler (Symbole de Legendre)
    if (exponentiation_rapide(n, (p - 1) / 2, p) != 1) {
        return 0; 
    }

    // Cas simple: p = 3 mod 4
    if (p % 4 == 3) {
        return exponentiation_rapide(n, (p + 1) / 4, p); 
    }

    // Décomposition dyadique p-1 = Q * 2^S
    unsigned long long Q = p - 1;
    int S = 0;
    
    while (Q % 2 == 0) {
        Q /= 2;
        S++;
    }


    // Trouver un non-résidu quadratique z
    unsigned long long z = 0;
    unsigned long long p_minus_1_div_2 = (p - 1) / 2;
    for (unsigned long long i = 2; i < p; i++) {
        if (exponentiation_rapide(i, p_minus_1_div_2, p) == p - 1) {
            z = i;
            break;
        }
    }
    
    // Initialisation
    unsigned long long c = exponentiation_rapide(z, Q, p); 
    unsigned long long x = exponentiation_rapide(n, (Q + 1) / 2, p);
    unsigned long long b = exponentiation_rapide(n, Q, p);
    int k = S;



    while (b != 1) {
        // Trouver le plus petit 'm' tel que b^(2^m) = 1 mod p. (L'ordre de b est 2^m)
        int m = 0;
        unsigned long long b_test = b;
        // On effectue des carrés successifs
        while (b_test != 1) {
            b_test = multiplication_modulo(b_test, b_test, p); 
            m++;
        }
        
        // Calcul du terme correctif t = c^(2^(k-m-1))
        unsigned long long exponent = 1ULL << (k - m - 1);
        unsigned long long t = exponentiation_rapide(c, exponent, p);

        x = multiplication_modulo(x, t, p); 
        
        // c est mis à jour (c devient t^2)
        c = multiplication_modulo(t, t, p); 
        
        // b est réduit par le facteur c (b devient b * t^2)
        b = multiplication_modulo(b, c, p); 
        
        // k est mis à jour à m (le nouvel ordre de c)
        k = m;
    }

    return x;
}


int main() {
    unsigned long long n1 = 302;
    unsigned long long p1 = 2081;
    unsigned long long r1 = shanks_tonelli(n1, p1);

    printf("Racine carrée de %llu mod %llu: ", n1, p1);
    if (r1 != 0) {
        printf("%llu et %llu\n", r1, p1 - r1);
    } else {
        printf("Aucune solution (non-résidu quadratique).\n");
    }
    return 0;
}