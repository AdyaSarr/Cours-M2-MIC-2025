#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "rsa.h"






// Verifie la primalité d'un nombre en utlisant la bibliotheque gmp
/* bool est_premier(const char *strNumber){

    //Declaration d'une variable pour le nombre de grande precision
    mpz_t nombre_a_tester;

    //Initialisation de la variable GMP
    mpz_init(nombre_a_tester);

    //Conversion de la chaine de caractere vers le type de GMP en base 10
    if (mpz_set_str(nombre_a_tester, strNumber, 10) == -1)
    {
        fprintf(stderr, "mpz_set_str: la chaine que vous avez entré n'est pas un nombre");
        mpz_clear(nombre_a_tester);
        return false;
    }
    // AJOUT DE DÉBOGAGE
    printf("DEBUG: Chaîne d'entrée : %s\n", strNumber);
    printf("DEBUG: Chaîne d'sortie : ");
    mpz_out_str(stdout, 10, nombre_a_tester); // Affiche la valeur actuelle de mpz_t
    printf("\n");
    
    //Appel de la fonction de test de primalité GMP
    // mpz_probab_prime_p(nombre, reps)
    // reps est le nombre d'itérations
    int est_prime = mpz_probab_prime_p(nombre_a_tester, REP);

    //Nettoyage de la memoire
    mpz_clear(nombre_a_tester);

    // Le résultat de GMP est : 0 (composé pas premier), 1 (probablement premier) ou 2 (premier certain pour petits nombres)
    return (est_prime > 0);
} */

void generation_aleatoire_nombre(mpz_t p, mpz_t q, unsigned long nbBit, gmp_randstate_t etat_aleatoire){
    // Utiliser le temps et l'ID du processus pour un meilleur seed aléatoire
    // REMARQUE : CE SEED NE DOIT ÊTRE FAIT QU'UNE FOIS DANS MAIN !
    // Si vous le laissez ici, vous re-semez le générateur à chaque appel, ce qui n'est pas idéal.
    // gmp_randseed_ui(etat_aleatoire, time(NULL) + getpid()); // <-- À SUPPRIMER OU PASSER EN MAIN

    // Génération du nombre
    mpz_urandomb(p, etat_aleatoire, nbBit);
    mpz_urandomb(q, etat_aleatoire, nbBit);

    // ... (le reste du code de setbit est correct) ...
    mpz_setbit(p, nbBit - 1); 
    mpz_setbit(q, nbBit - 1);
    mpz_setbit(p, 0); 
    mpz_setbit(q, 0);

    // SUPPRIMER : On ne nettoie pas l'état ici, car il doit être réutilisé.
    // gmp_randclear(etat_aleatoire);
}

void keyGenRSA(mpz_t N, mpz_t D, unsigned long nbBit, gmp_randstate_t etat_aleatoire) {
    
    mpz_t p, q, phi_n, E;
    mpz_t p_moins_1, q_moins_1;
    mpz_t reste_mod; 

    //Initialisation de toutes les variables
    mpz_init(p);
    mpz_init(q);
    mpz_init(phi_n);
    mpz_init(E);
    mpz_init(p_moins_1);
    mpz_init(q_moins_1);
    mpz_init(reste_mod);

    // Définir l'exposant public E = 3 (choix courant et efficace)
    //unsigned long E_val = 3UL;
    unsigned long E_val = 65537UL;
    mpz_set_ui(E, E_val);
    
    //Étiquette pour le saut (si phi_n est divisible par E) ---
    generation_loop: 
    
    //Génération de P et Q jusqu'à ce qu'ils soient premiers
    do {
        // La fonction generation_aleatoire_nombre doit être appelée deux fois
        // ou être modifiée pour garantir deux nombres distincts de même taille
        generation_aleatoire_nombre(p, q, nbBit, etat_aleatoire);
    } while (mpz_probab_prime_p(p, REP) == 0|| mpz_probab_prime_p(q, REP) == 0 || mpz_cmp(p, q) == 0);

    //Calcul de N et de la fonction Totient d'Euler phi_n
    
    // N = P * Q
    mpz_mul(N, p, q); 
    
    // phi_n = (P - 1) * (Q - 1)
    mpz_sub_ui(p_moins_1, p, 1UL); // mpz_sub_ui pour soustraire 1
    mpz_sub_ui(q_moins_1, q, 1UL);
    mpz_mul(phi_n, p_moins_1, q_moins_1); 

    //Vérification de la coprimalité : PGCD(E, phi_n) doit être 1
    // Simplification: E=3 est copremier si phi_n n'est pas divisible par 3 (phi_n mod E != 0).
    
    // Calcule le reste: reste_mod = phi_n mod E
    unsigned long reste = mpz_mod_ui(reste_mod, phi_n, E_val); 

    if (reste == 0) {
        // Problème : PGCD(E, phi_n) n'est pas 1. Nous devons regénérer P et Q.
        // Utiliser GOTO pour un saut propre au début du processus de génération.
        goto generation_loop; 
    }

    // Calcul de l'exposant privé D
    // D = E^-1 mod phi_n
    // Si nous arrivons ici, l'inverse existe (mpz_invert retournera 1).
    if (mpz_invert(D, E, phi_n) == 0) {
        fprintf(stderr, "Erreur critique: mpz_invert a échoué.\n");
        // Gérer cette erreur pourrait être de regénérer ou de quitter.
        goto generation_loop; 
    }

    // 6. Nettoyage de la mémoire interne (les clés N et D ne sont PAS nettoyées)
    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(phi_n);
    mpz_clear(E);
    mpz_clear(p_moins_1);
    mpz_clear(q_moins_1);
    mpz_clear(reste_mod);
}


/* void keyGenRSA(unsigned long nbBit){
    mpz_t p;
    mpz_t q;
    mpz_t n;
    mpz_t difp;
    mpz_t difq;
    mpz_t phi_de_n;

    mpz_init(p);
    mpz_init(q);
    mpz_init(n);

    mpz_init(difp);
    mpz_init(difq);
    mpz_init(phi_de_n);

    generation_aleatoire_nombre(p, q, nbBit);
    while (!est_premier(p) || !est_premier(q) )
    {
        generation_aleatoire_nombre(p, q, nbBit);
    }
    
    mpz_mul(n, p, q);
    mpz_sub(difp,p,1UL); //1UL force la valeur a unsigned 1
    mpz_sub(difq,q,1UL);
    mpz_mul(phi_de_n, difp, difq);

    int e = 3UL;
    mpz_t rest_mod;
    mpz_init(rest_mod);

    unsigned long int reste = mpz_mod_ui(rest_mod, phi_de_n, e);
    while (reste == 0)
    {
        mpz_clear(p);
        mpz_clear(q);
        mpz_clear(n);
        mpz_clear(difp);
        mpz_clear(difq);
        mpz_clear(phi_de_n);
        mpz_clear(rest_mod);
        keyGenRSA(nbBit);
    }

    mpz_t d, trois;
    mpz_init(trois);
    mpz_init(d);
    mpz_set_ui(trois, 3UL);
    mpz_invert(d, trois, phi_de_n);


    

    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(difp);
    mpz_clear(difq);
    mpz_clear(phi_de_n);
    //mpz_clear(n);
    //mpz_clear(d);
}
 */
void chiffrer_RSA(mpz_t C, const mpz_t M, const mpz_t E, const mpz_t N){
    mpz_powm(C, M, E, N);
}

void dechiffrer_RSA(mpz_t M, const mpz_t C, const mpz_t D, const mpz_t N){
    mpz_powm(M, C, D, N);
}

void conversion_message_chiffrement(const char *message_clair, mpz_t C, const mpz_t E, const mpz_t N){
    //Clés et Messages GMP
    mpz_t M;
    mpz_init(M);
    size_t taille_message = strlen(message_clair);
    const void *buffer = (const void *)message_clair;
    
    // On importe les octets. L'ordre des octets est important pour la décodabilité.
    // L'ordre 1 (gros-boutien des éléments) et size=1 (octet par octet) est souvent le plus sûr
    // pour que la lecture soit toujours la même.
    mpz_import(M, taille_message, 1, 1, 0, 0, buffer);

    if (mpz_cmp(M, N) >= 0)
    {
        fprintf(stderr, "Erreur RSA: Le message (%zu octets) est trop long (M >= N). Utiliser le chaînage de blocs.\n", taille_message);
        mpz_clear(M);
        return;
    }
    chiffrer_RSA(C, M, E, N);
    mpz_clear(M);
}

char *exporter_message(const mpz_t M_dechiffre) {
    
    size_t taille_octets_reels;
    char *message_dechiffre = NULL;
    
    // Déterminer la taille nécessaire
    // On appelle mpz_export une première fois avec NULL pour obtenir la taille d'exportation.
    // Les paramètres (1, 1, 0, 0) doivent correspondre à ceux utilisés dans mpz_import.
    mpz_export(NULL, &taille_octets_reels, 1, 1, 0, 0, M_dechiffre);

    //Allocation de la mémoire
    // Allouer la taille trouvée + 1 pour le caractère nul de fin de chaîne '\0' (nécessaire pour printf).
    message_dechiffre = (char *)malloc(taille_octets_reels + 1);

    if (message_dechiffre == NULL) {
        perror("Erreur: Échec de l'allocation mémoire pour le message déchiffré");
        return NULL;
    }
    
    // Exporter le nombre vers le tampon alloué
    // On appelle mpz_export une deuxième fois pour effectuer l'exportation réelle.
    mpz_export(message_dechiffre, &taille_octets_reels, 1, 1, 0, 0, M_dechiffre);

    // Ajouter le caractère nul de fin de chaîne
    // Cela permet de traiter le tampon d'octets comme une chaîne C.
    message_dechiffre[taille_octets_reels] = '\0';
    
    return message_dechiffre;
}
int main() {
    const char *message = "Hello World!";
    size_t taille_message = strlen(message);
    //unsigned long E_val = 3UL;
    unsigned long E_val = 65537UL;

    mpz_t cle_N_public, cle_D_prive, E, C, M_dechiffre;
    
    // Initialisation du générateur aléatoire (PRNG)
    gmp_randstate_t etat_aleatoire;
    gmp_randinit_default(etat_aleatoire);

    // CRITIQUE: UTILISER /dev/urandom POUR UN SEED UNIQUE ET SÉCURISÉ
    unsigned long seed;
    int fd = open("/dev/urandom", O_RDONLY); // Tente d'ouvrir la source d'aléa

    if (fd != -1) {
        // Lire 8 octets d'aléa cryptographique dans 'seed'
        if (read(fd, &seed, sizeof(seed)) == sizeof(seed)) {
            gmp_randseed_ui(etat_aleatoire, seed);
            // printf("DEBUG: Seed cryptographique utilisé.\n");
        } else {
            // Revenir à une méthode moins bonne si la lecture échoue
            gmp_randseed_ui(etat_aleatoire, time(NULL) + getpid());
            // printf("DEBUG: Seed temporel de secours utilisé.\n");
        }
        close(fd);
    } else {
        // Si open échoue, utiliser le seed temporel simple
        gmp_randseed_ui(etat_aleatoire, time(NULL) + getpid());
        // printf("DEBUG: Seed temporel simple utilisé.\n");
    }

    mpz_init(cle_N_public);
    mpz_init(cle_D_prive);
    mpz_init(E);
    mpz_init(M_dechiffre);
    mpz_init(C);
    mpz_set_ui(E, E_val);

    // Correction de l'appel : Passage de l'état
    keyGenRSA(cle_N_public, cle_D_prive, 1024, etat_aleatoire); //
    
    printf("--- Clés RSA Générées ---\n");

    printf("Module N (Clé Publique) : \n");

    mpz_out_str(stdout, 10, cle_N_public);
    printf("\n\nExposant D (Clé Privée) : \n");

    mpz_out_str(stdout, 10, cle_D_prive);

    printf("\n");

    conversion_message_chiffrement(message, C,E,cle_N_public);
    char *message_chiffre_str = mpz_get_str(NULL, 16, C); 

    printf("Le chiffrement du message (Hex) est : \n");
    printf("%s\n", message_chiffre_str); // Affichage de la chaîne capturée
    printf("\n");
    dechiffrer_RSA(M_dechiffre, C, cle_D_prive, cle_N_public);
    char *message_dechiffre = exporter_message(M_dechiffre);
    char *message_dechiffre_final = message_dechiffre; // Pointeur vers la chaîne complète (avec les zéros)

    // Avancer le pointeur pour sauter les zéros non significatifs
    while (*message_dechiffre_final == '\0' && message_dechiffre_final < (message_dechiffre + taille_message)) {
        // Note: 'taille_message' est la longueur du message clair original, non accessible directement ici
        // Il est plus simple de le faire une seule fois pour cette démo.
        message_dechiffre_final++; 
    }

    // Pour une solution simple dans main (en sachant que le message fait 12 octets) :
    char *message_debut = message_dechiffre;

    // Le message a 12 octets. L'exportation a (approx) 128 octets.
    // Nous devons sauter (128 - 12) = 116 octets.
    // Une méthode simple est de sauter les zéros :

    while (*message_debut == '\0' && (message_debut - message_dechiffre) < 128) {
        message_debut++;
    }

    fprintf(stdout, "Le dechiffre du message est %s\n", message_debut); // Affiche à partir du premier caractère non-nul
    mpz_clear(cle_N_public);
    mpz_clear(cle_D_prive);
    mpz_clear(E);
    mpz_clear(C);
    mpz_clear(M_dechiffre);
    free(message_dechiffre);
    free(message_chiffre_str);
    // NOUVELLE PARTIE CRUCIALE : Nettoyage de l'état aléatoire
    gmp_randclear(etat_aleatoire); 
    return 0;
}