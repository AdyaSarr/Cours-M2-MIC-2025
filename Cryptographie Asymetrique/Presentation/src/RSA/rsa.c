#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "rsa.h"
#define FILE_PUB_KEY  "pub.pem"
#define FILE_PRIV_KEY "priv.pem"
#define FILE_CYPHER   "cypher.txt"
#define E_VAL 3UL
#define KEY_SIZE_BITS 1024
#define REP 25
#define FILE_CYPHER_MODE3 "cypher1.txt"
#define FILE_DECH_BASE "dechiffré.txt"
#define FILE_DECH_MODE3 "dechiffré1.txt"


char *lire_message_fichier(const char *nom_fichier) {
    FILE *f = fopen(nom_fichier, "r");
    char *buffer = NULL;
    long file_size;

    if (f == NULL) {
        perror("Erreur: Impossible d'ouvrir le fichier message en lecture");
        return NULL;
    }

    // 1. Déterminer la taille du fichier
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    rewind(f); // Retour au début

    // 2. Allouer le buffer (taille du fichier + 1 pour le caractère nul '\0')
    buffer = (char *)malloc(file_size + 1);

    if (buffer == NULL) {
        perror("Erreur: Échec de l'allocation mémoire pour le message");
        fclose(f);
        return NULL;
    }

    // 3. Lire le fichier
    size_t result = fread(buffer, 1, file_size, f);

    if (result != file_size) {
        fprintf(stderr, "Erreur: Lecture incomplète du fichier message.\n");
        free(buffer);
        buffer = NULL;
    } else {
        // Ajouter le caractère nul pour le traiter comme une chaîne C
        buffer[file_size] = '\0';
    }

    fclose(f);
    return buffer;
}
unsigned long calculer_taille_sel(const mpz_t E, const mpz_t N, size_t taille_message_clair) {
    
    unsigned long N_bits = mpz_sizeinbase(N, 2);
    unsigned long M_bits = taille_message_clair * 8;
    unsigned long E_val = mpz_get_ui(E);

    // 1. Calculer la borne de sécurité (Borne de Hastad)
    unsigned long E_carre = E_val * E_val;
    
    // Pour éviter les flottants et garantir l'entier
    unsigned long m_max_theorique = N_bits / E_carre; 
    
    // 2. Contrainte pratique (Espace total disponible, sans marge fixe)
    unsigned long m_pratique = N_bits - M_bits; 

    // --- FORCER LA VULNÉRABILITÉ ---
    // m doit être au maximum le plus petit de : (Espace libre) et (Borne théorique - 2 bits)
    
    // La taille max du sel est la borne théorique moins une petite marge de 2 bits
    unsigned long m_force_securite = m_max_theorique - 2; 

    // Le sel m doit être faisable (plus petit que l'espace réel) et supérieur à 1.
    unsigned long m;
    
    if (m_pratique <= 1) { // Pas assez de place
        m = 1; 
    } else {
        // Le sel sera le minimum de l'espace libre et de la borne de sécurité
        m = (m_pratique < m_force_securite) ? m_pratique : m_force_securite;
    }
    
    // Assurer au moins 1 bit de sel si possible
    if (m == 0) m = 1; 
    
    return m;
}

void conversion_message_chiffrement_alea(
    const char *message_clair, 
    mpz_t C, 
    const mpz_t E, 
    const mpz_t N, 
    gmp_randstate_t etat_aleatoire
) {
    mpz_t M, M_prime, r, deux_puissance_m;
    
    mpz_init(M); mpz_init(M_prime); mpz_init(r); mpz_init(deux_puissance_m);

    size_t taille_message = strlen(message_clair);
    mpz_import(M, taille_message, 1, 1, 0, 0, (const void *)message_clair);

    if (mpz_cmp(M, N) >= 0) {
        fprintf(stderr, "Erreur RSA: Message trop long (M >= N).\n");
        goto cleanup;
    }
    
    // Utiliser la fonction simplifiée de calcul de m (synchronisation)
    unsigned long m = calculer_taille_sel(E, N, taille_message);

    if (m == 0) { // Devrait être m=1 grâce à la fonction de calcul
        fprintf(stderr, "Erreur RSA: Pas d'espace disponible pour le sel aléatoire (m=0).\n");
        goto cleanup;
    }
    
    // Générer le sel aléatoire r de m bits
    mpz_urandomb(r, etat_aleatoire, m);
    
    // Calculer M' = M * 2^m + r
    mpz_set_ui(deux_puissance_m, 0); 
    mpz_setbit(deux_puissance_m, m); 
    mpz_mul(M_prime, M, deux_puissance_m); 
    mpz_add(M_prime, M_prime, r); 
    
    // Chiffrement
    chiffrer_RSA(C, M_prime, E, N);
    
    cleanup:
    mpz_clear(M); mpz_clear(M_prime); mpz_clear(r); mpz_clear(deux_puissance_m);
}

void desceller_message(
    mpz_t M_clair, 
    const mpz_t M_chiffre_brut, 
    const mpz_t E, 
    const mpz_t N, 
    unsigned long taille_message_clair
) {
    
    mpz_t temp_M;
    mpz_init(temp_M);
    mpz_set(temp_M, M_chiffre_brut); 

    // OBTENIR LA TAILLE DU SEL M (Synchronisé avec le chiffrement)
    // C'est la même fonction de calcul de m qui doit être utilisée.
    unsigned long m = calculer_taille_sel(E, N, taille_message_clair); 
    
    if (m == 0) {
        fprintf(stderr, "Erreur de Descellage: Taille du sel m incorrecte (0).\n");
        mpz_clear(temp_M);
        return;
    }

    // DÉCALAGE : Retirer le sel r (M_clair = M_chiffre_brut / 2^m)
    mpz_tdiv_q_2exp(M_clair, temp_M, m);
    
    mpz_clear(temp_M);
}

// --- Fonctions courtes (pour la complétude) ---

void generation_aleatoire_nombre(mpz_t p, mpz_t q, unsigned long nbBit, gmp_randstate_t etat_aleatoire){
    mpz_urandomb(p, etat_aleatoire, nbBit);
    mpz_urandomb(q, etat_aleatoire, nbBit);
    mpz_setbit(p, nbBit - 1); mpz_setbit(q, nbBit - 1);
    mpz_setbit(p, 0); mpz_setbit(q, 0);
}

void keyGenRSA(mpz_t N, mpz_t D, const mpz_t E, unsigned long nbBit, gmp_randstate_t etat_aleatoire) {
    mpz_t p, q, phi_n, p_moins_1, q_moins_1, reste_mod; 
    unsigned long E_val = mpz_get_ui(E);
    mpz_init(p); mpz_init(q); mpz_init(phi_n); mpz_init(p_moins_1); mpz_init(q_moins_1); mpz_init(reste_mod);
    generation_loop: 
    do { generation_aleatoire_nombre(p, q, nbBit, etat_aleatoire);
    } while (mpz_probab_prime_p(p, REP) == 0 || mpz_probab_prime_p(q, REP) == 0 || mpz_cmp(p, q) == 0);
    mpz_mul(N, p, q); 
    mpz_sub_ui(p_moins_1, p, 1UL); mpz_sub_ui(q_moins_1, q, 1UL);
    mpz_mul(phi_n, p_moins_1, q_moins_1); 
    unsigned long reste = mpz_mod_ui(reste_mod, phi_n, E_val); 
    if (reste == 0 || mpz_invert(D, E, phi_n) == 0) goto generation_loop; 
    mpz_clear(p); mpz_clear(q); mpz_clear(phi_n); mpz_clear(p_moins_1); mpz_clear(q_moins_1); mpz_clear(reste_mod);
}

void chiffrer_RSA(mpz_t C, const mpz_t M, const mpz_t E, const mpz_t N){ mpz_powm(C, M, E, N); }
void dechiffrer_RSA(mpz_t M, const mpz_t C, const mpz_t D, const mpz_t N){ mpz_powm(M, C, D, N); }

char *exporter_message(const mpz_t M_dechiffre_propre, size_t taille_message_clair) {
    size_t taille_a_exporter = taille_message_clair;
    char *message_dechiffre = (char *)malloc(taille_a_exporter + 1);
    if (message_dechiffre == NULL) return NULL;
    mpz_export(message_dechiffre, NULL, 1, 1, 0, 0, M_dechiffre_propre);
    message_dechiffre[taille_a_exporter] = '\0';
    return message_dechiffre;
}

int ecrire_mpz_fichier(const char *nom_fichier, const mpz_t cle) {
    FILE *f = fopen(nom_fichier, "w");
    if (f == NULL) { perror("Erreur"); return 0; }
    mpz_out_str(f, 16, cle); fprintf(f, "\n"); fclose(f); return 1;
}
int lire_mpz_fichier(const char *nom_fichier, mpz_t cle) {
    FILE *f = fopen(nom_fichier, "r");
    if (f == NULL) { perror("Erreur"); return 0; }
    if (mpz_inp_str(cle, f, 16) == 0) { fprintf(stderr, "Erreur MPZ invalide.\n"); fclose(f); return 0; }
    fclose(f); return 1;
}

// --- FONCTION MAIN ---
int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        fprintf(stderr, "Utilisation:\n");
        fprintf(stderr, "  1. Génération et Chiffrement: %s <fichier_message_clair>\n", argv[0]);
        fprintf(stderr, "  2. Chiffrement (Clé existante): %s <fichier_message_clair> %s\n", argv[0], FILE_PUB_KEY);
        fprintf(stderr, "  3. Déchiffrement: %s <fichier_chiffré> %s\n", argv[0], FILE_PRIV_KEY);
        return 1;
    }

    // --- 1. INITIALISATION DES RESSOURCES ---
    mpz_t N, D, E, C, M_dechiffre_brut, M_dechiffre_propre;
    gmp_randstate_t etat_aleatoire;
    char *message_chiffre_str = NULL;
    char *message_dechiffre_str = NULL;
    char *message_clair_buffer = NULL; 
    
    // Initialisation des mpz_t
    mpz_init(N); mpz_init(D); mpz_init(E); mpz_init(C); 
    mpz_init(M_dechiffre_brut); mpz_init(M_dechiffre_propre);

    // Seed Aléatoire Robuste
    gmp_randinit_default(etat_aleatoire);
    unsigned long seed; int fd = open("/dev/urandom", O_RDONLY); 
    if (fd != -1 && read(fd, &seed, sizeof(seed)) == sizeof(seed)) { gmp_randseed_ui(etat_aleatoire, seed); close(fd); } 
    else { gmp_randseed_ui(etat_aleatoire, time(NULL) + getpid()); if (fd != -1) close(fd); }
    
    mpz_set_ui(E, E_VAL);

    // Lecture du fichier message clair (si nécessaire)
    const char *message_file = argv[1];
    if (argc == 2 || (argc == 3 && strstr(argv[2], "pub.pem") != NULL)) {
        message_clair_buffer = lire_message_fichier(message_file);
        if (message_clair_buffer == NULL) {
            fprintf(stderr, "Échec de la lecture du fichier message: %s\n", message_file);
            goto cleanup;
        }
    }
    
    const size_t TAILLE_MESSAGE_CLAIRE = (message_clair_buffer != NULL) ? strlen(message_clair_buffer) : strlen("Hello World! C'est le message à restaurer."); 
    

    // --- 2. ANALYSE DES MODES ---

    // MODE 1 : Génération de Clés et Chiffrement (./rsa message.txt)
    if (argc == 2) {
        printf("--- MODE 1: GÉNÉRATION & CHIFFREMENT (Nouvelles Clés) ---\n");
        
        keyGenRSA(N, D, E, KEY_SIZE_BITS / 2, etat_aleatoire); 
        conversion_message_chiffrement_alea(message_clair_buffer, C, E, N, etat_aleatoire);

        ecrire_mpz_fichier(FILE_PUB_KEY, N);
        ecrire_mpz_fichier(FILE_PRIV_KEY, D);
        ecrire_mpz_fichier(FILE_CYPHER, C);
        
        message_chiffre_str = mpz_get_str(NULL, 16, C); 
        printf("Clés générées (%d bits) et chiffré dans %s:\n%s\n", KEY_SIZE_BITS, FILE_CYPHER, message_chiffre_str);
    }
    
    // MODES 2 & 3 : Clés existantes
    else if (argc == 3) {
        
        const char *key_file = argv[2];       
        
        // --- MODE 3 : CHIFFREMENT AVEC CLÉ PUBLIQUE EXISTANTE ---
        if (strstr(key_file, "pub.pem") != NULL) {
            
            printf("--- MODE 3: CHIFFREMENT (Clé Publique existante) ---\n");
            
            if (!lire_mpz_fichier(key_file, N)) goto cleanup; 
            conversion_message_chiffrement_alea(message_clair_buffer, C, E, N, etat_aleatoire);

            ecrire_mpz_fichier(FILE_CYPHER_MODE3, C);
            
            message_chiffre_str = mpz_get_str(NULL, 16, C);
            printf("Message chiffré avec %s, résultat dans %s:\n%s\n", key_file, FILE_CYPHER_MODE3, message_chiffre_str);
        } 
        
        // --- MODE 2 : DÉCHIFFREMENT AVEC CLÉ PRIVÉE ---
        else if (strstr(key_file, "priv.pem") != NULL) {
            
            printf("--- MODE 2: DÉCHIFFREMENT (Clé Privée existante) ---\n");

            const char *dechiffre_output_file = strstr(message_file, "cypher1.txt") ? FILE_DECH_MODE3 : FILE_DECH_BASE;
            
            if (!lire_mpz_fichier(message_file, C) || !lire_mpz_fichier(key_file, D) || !lire_mpz_fichier(FILE_PUB_KEY, N)) goto cleanup;

            // DÉCHIFFREMENT BRUT
            dechiffrer_RSA(M_dechiffre_brut, C, D, N);
            
            // DESCELLAGE (UNPADDDING)
            desceller_message(M_dechiffre_propre, M_dechiffre_brut, E, N, TAILLE_MESSAGE_CLAIRE);

            // Exportation
            message_dechiffre_str = exporter_message(M_dechiffre_propre, TAILLE_MESSAGE_CLAIRE);
            
            if (message_dechiffre_str != NULL) {
                
                // Sauter les octets nuls (qui viennent du début du bloc)
                char *message_debut = message_dechiffre_str;
                size_t octets_a_sauter = 0;
                
                // Déterminer combien de zéros non significatifs il y a
                while (*message_debut == '\0' && octets_a_sauter < (size_t)KEY_SIZE_BITS / 8) {
                    message_debut++;
                    octets_a_sauter++;
                }

                // La taille réelle à écrire est la taille originale du message (en octets)
                // Le nombre d'octets à écrire est TAILLE_MESSAGE_CLAIRE.
                size_t octets_a_ecrire = TAILLE_MESSAGE_CLAIRE; 

                FILE *f_dech = fopen(dechiffre_output_file, "w");
                if (f_dech != NULL) {
                    // CORRECTION CRITIQUE: Utiliser fwrite pour écrire le nombre exact d'octets.
                    fwrite(message_debut, 1, octets_a_ecrire, f_dech);
                    
                    fclose(f_dech);
                    printf("Déchiffrement réussi. Message écrit dans %s.\n", dechiffre_output_file);
                } else { perror("Erreur lors de l'ouverture du fichier de sortie déchiffré"); }
            }
        }
    }

    // --- 3. NETTOYAGE GENERAL ---
    cleanup:
    mpz_clear(N); mpz_clear(D); mpz_clear(E); mpz_clear(C); 
    mpz_clear(M_dechiffre_brut); mpz_clear(M_dechiffre_propre); 
    if (message_chiffre_str != NULL) free(message_chiffre_str);
    if (message_dechiffre_str != NULL) free(message_dechiffre_str);
    if (message_clair_buffer != NULL) free(message_clair_buffer); 
    gmp_randclear(etat_aleatoire); 
    
    return 0;
}