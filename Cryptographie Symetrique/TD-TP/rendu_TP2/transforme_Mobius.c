#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**
 * @brief Cette fonction permet de calculer Transformée de Möbius binaire (M_n) j'ai repris l'algorithme1 de la page 7 du poly.
 * @brief Complexité : O(n * 2^n) additions.
 * @param v Un pointeur vers le tableau d'entiers (0 ou 1) de taille 2^n.
 * @param n Le nombre de variables, tel que la taille du tableau est 2^n.
 */
void transfe_mobius(int *v, int n) {
    // La taille du tableau est N = 2^n
    int N = 1 << n;
    for (int k = 1; k <= n; k++) {
        // La taille d'un bloc à cette étape est 2^k
        int taille_bloc = 1 << k;
        // La taille d'une moitié de bloc (gauche ou droite) est 2^(k-1)
        int taille_demi_bloc = 1 << (k - 1); 
        // i itère sur le début de chaque bloc de taille_bloc (2^(n-k) blocs) 
        for (int i = 0; i < N; i += taille_bloc) {
            // j itère dans la moitié droite du bloc, de 0 à 2^(k-1) - 1 
            for (int j = 0; j < taille_demi_bloc; j++) {
                // L'élément G est dans la moitié gauche (indice i+j)
                int index_G = i + j;
                // L'élément D est dans la moitié droite (indice i + taille_demi_bloc + j)
                int index_D = i + taille_demi_bloc + j;
                // Opération : D <- G + D (mod 2)
                v[index_D] = (v[index_G] + v[index_D]) % 2;
            }
        }
    }
}


int main() {
    int n = 3; // Nombre de variables
    int N = 1 << n;
    int v_f[] = {1, 0, 1, 1, 0, 1, 0, 1};

    printf("Entrée (v_f / b) : ");
    for (int i = 0; i < N; i++) {
        printf("%d ", v_f[i]);
    }
    printf("\n");

    transfe_mobius(v_f, n);

    printf("Sortie (a_ANF / a) : ");
    for (int i = 0; i < N; i++) {
        printf("%d ", v_f[i]);
    }
    printf("\n");
    
    // Le résultat (1, 1, 0, 1, 1, 0, 0, 1) confirme le calcul fait à la main.
    // L'ANF est : 1 + x3 + x2*x3 + x1 + x1*x2*x3
    printf("\nLe vecteur de sortie est le vecteur des coefficients ANF (a_u).\n");
    printf("Resultat (1, 1, 0, 1, 1, 0, 0, 1) correspond a l'ANF :\n");
    printf("f(x1, x2, x3) = 1 + x3 + x2x3 + x1 + x1x2x3\n");
    
    return 0;
}