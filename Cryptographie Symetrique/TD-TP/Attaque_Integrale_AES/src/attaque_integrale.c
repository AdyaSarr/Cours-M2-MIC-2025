#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/attaque_integrale.h"
#define NB_PLAIN_ET_CYPHER_TEXTE 256
#define CONSTANTE 0 // Pour avoir la propriete de debart de l'attaque
#define CONSTANTE1 0x01
#define WORD_SIZE 4
#define NB_WORD_KEY 4
#define NB_WORDS 80


/* const uint8_t key128[]        = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                     0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c}; */

/* const uint8_t key128[]        = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                     0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3b}; */
    
/* const uint8_t key128[]        = {0x2a, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                     0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3b}; */

/* const uint8_t key128[]           = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                     0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f}; */
                                
const uint8_t key128[]           = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                     0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3b};
//La sbox
static const uint8_t sbox[256] = {
    // 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,  // 0
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,  // 1
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,  // 2
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,  // 3
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,  // 4
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,  // 5
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,  // 6
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,  // 7
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,  // 8
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,  // 9
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,  // A
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,  // B
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,  // C
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,  // D
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,  // E
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}; // F






//! The Round Constant (Rcon) table used in the key schedule.
static const uint8_t rcon[] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36,
                               0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97,
                               0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39};

/**
 * @brief Performs a cyclic left shift on a 4-byte word.
 * @param[in,out] word A pointer to a 4-byte array to be rotated.
 */
static void word_rotate_left(uint8_t* word)
{
    uint8_t temp = word[0];
    word[0]      = word[1];
    word[1]      = word[2];
    word[2]      = word[3];
    word[3]      = temp;
}



static void calculer_core_inverse(uint8_t *out, const uint8_t *w_prec, int iteration) {
    uint8_t temp[WORD_SIZE];
    for (int k = 0; k < WORD_SIZE; ++k) temp[k] = w_prec[k];
    word_rotate_left(temp);
    for (int k = 0; k < WORD_SIZE; ++k) temp[k] = sbox[temp[k]];
    temp[0] ^= rcon[iteration];
    for (int k = 0; k < WORD_SIZE; ++k) out[k] = temp[k];
}

message *trouver_cle_maitre(const message *cle_k_4){
    //Stocker tous les mots des cles precedentes ici comme on a 4 tour le tableau contiendra 20 mots
    uint8_t words[NB_WORDS];
    //Je stocke les 4 dernieres case de de words ceux de cle_k_4
    for (size_t c = 0; c < AES_STATE_DIM; c++)
    {
        for (size_t r = 0; r < AES_STATE_DIM; r++)
        {
            //(4 * NB_WORD_KEY): correspond le nombre de mots dans la table avant le premier mot de la cle K_4
            //(4 * NB_WORD_KEY)*WORD_SIZE correspond au premier mot de K_4 qui est W_16
            //(c * WORD_SIZE): pour ce deplacer de 4 octet par exemple si on etait sur W_16 si c=1 on passe a W_17
            //r: pour la lecture ligne apres ligne comme le format Colum-Major
            //words[(4 * NB_WORD_KEY + c) * WORD_SIZE + r] = cle_k_4->tab[r][c];
            //Au lieu d'utiliser le format Colum-Major comme le mentionner son proprietaire sur l'entete de son code j'ai utiliser Row-major
            //pour charger la cle car pour le premier me donner des resultats faux donc j'ai basculer sur l'autre et cela a marché  j'ai lu le code 
            //mais j'arrive pas a donner une explication a cette reaction.
            words[(4 * NB_WORD_KEY + r) * WORD_SIZE + c] = cle_k_4->tab[r][c];
        }
        
    }
    for (int i = 19; i >= 4; i--)
    {
        uint8_t *w_i = &words[i*WORD_SIZE];
        uint8_t *w_moins_4 = &words[(i-4)*WORD_SIZE];
        uint8_t *w_moins_1 = &words[(i-1)*WORD_SIZE];

        if (i%4 != 0)
        {
            for (int k = 0; k < WORD_SIZE; k++) {
                w_moins_4[k] = w_i[k] ^ w_moins_1[k]; 
            }
        }else
        {
            //key_schedule_core(w_moins_1, i/WORD_SIZE);
            uint8_t Core_Wi_minus_1[WORD_SIZE];
            calculer_core_inverse(Core_Wi_minus_1, w_moins_1, i/WORD_SIZE);
            for (int k = 0; k < WORD_SIZE; k++) {
                w_moins_4[k] = w_i[k] ^ Core_Wi_minus_1[k];
            }
        }
    }
    message *cle_maitre = (message *)malloc(sizeof(message));
    if (!cle_maitre)
    {
        fprintf(stderr, "malloc: sur la cle maitre\n");
        return NULL;
    }
    for (size_t r = 0; r < AES_STATE_DIM; r++){
        for (size_t c = 0; c < AES_STATE_DIM; c++){
            cle_maitre->tab[r][c] = words[(c * WORD_SIZE) + r];
        }
    }

    return cle_maitre;
}
message *gen_alea_messages(size_t taille, const uint8_t constante){
    message *messages = malloc(taille*sizeof(message));

    if (!messages)
    {
        fprintf(stderr, "malloc: generation des messages\n");
        return NULL;
    }
    
    for (size_t i = 0; i < taille; i++)
    {
        messages[i].tab[0][0] = (uint8_t)i;
        for (size_t j = 0; j < AES_STATE_DIM; j++)
        {
            for (size_t k = 0; k < AES_STATE_DIM; k++)
            {
                if (j != 0 || k != 0)
                {
                    messages[i].tab[j][k] = constante;
                }
            }
            
        }
    }
    return messages; 
}

message *chiffre_gen_alea_messages(size_t taille,const message *messages,const uint8_t *key){
    message *chiffres = malloc(taille*sizeof(message));
    if (!chiffres)
    {
        fprintf(stderr, "malloc: fonction de chiffrement des messages\n");
        return NULL;
    }
    for (size_t i = 0; i < taille; i++)
    {
        aes_error_t error_chif = aes_encrypt((const uint8_t *)messages[i].tab, (uint8_t *)chiffres[i].tab, key, AES_KEY_SIZE_128);
        if (error_chif!=AES_SUCCESS)
        {
            fprintf(stderr, "Chiffrement: Échec du chiffrement du message %zu\n", i);
            free(chiffres);
            return NULL;
        }
    }
    return chiffres;
}

void print_bloc(uint8_t tab[AES_STATE_DIM][AES_STATE_DIM]){
    for (size_t i = 0; i < AES_STATE_DIM; i++)
    {
        for (size_t j = 0; j < AES_STATE_DIM; j++)
        {
            fprintf(stdout, "%02X ", tab[i][j]);
        }
        printf("\n");
    }
}
uint8_t *valeurs_de_0_255(){
    uint8_t *valeurs = malloc(NB_PLAIN_ET_CYPHER_TEXTE*sizeof(uint8_t));
    if (!valeurs)
    {
        fprintf(stderr, "malloc: sur la generation des nombres de 0 a 255\n");
        return NULL;
    }
    
    for (size_t i = 0; i < NB_PLAIN_ET_CYPHER_TEXTE; i++)
    {
        valeurs[i] = (uint8_t)i;
    }
    return valeurs;
}

static uint8_t S_inv_octet(uint8_t octet){
    uint8_t rsbox_loc[256] = {
    // 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb, // 0
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb, // 1
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e, // 2
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25, // 3
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92, // 4
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84, // 5
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06, // 6
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b, // 7
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73, // 8
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e, // 9
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b, // A
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4, // B
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f, // C
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef, // D
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61, // E
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d, // F
};

    return rsbox_loc[octet];
}
uint8_t *trouver_4_ieme_cle_octet_octet(const message *chiffres, size_t taille, const int colonne_cible, const int ligne_cible, size_t *resultat_cles_trouvees){
    if (!chiffres)
    {
        fprintf(stderr, "Pas possibilite de faire l'attaque car vous ne donner pas les chiffrés attendu\n");
        return NULL;
    }
    if (taille<0)
    {
        fprintf(stderr, "La taille est non valide\n");
        return NULL;
    }
    
    if (colonne_cible < 0 || colonne_cible > 3 || ligne_cible < 0 || ligne_cible >3)
    {
        fprintf(stderr, "Erreur sur les entrees de la fonction %d et %d doivent etre comprise de 0 a 3\n", ligne_cible, colonne_cible);
        return NULL;
    }
    uint8_t *F_2_8 = valeurs_de_0_255();
    if (!F_2_8)
    {
        fprintf(stderr, "probleme de generation des entiers de 1 a 255\n");
        return NULL;
    }
    
    uint8_t *cle_trouvees = NULL;
    size_t nb_cles_trouvees = 0;
    for (uint8_t *k_ptr = F_2_8; k_ptr < F_2_8+NB_PLAIN_ET_CYPHER_TEXTE; k_ptr++)
    {
        uint8_t cle_candidat = *k_ptr; 
        uint8_t somme = 0;
        for (size_t i = 0; i < NB_PLAIN_ET_CYPHER_TEXTE; i++)
        {
            uint8_t sol_xor = cle_candidat ^ chiffres[i].tab[ligne_cible][colonne_cible];
            uint8_t sol_xor_inv = S_inv_octet(sol_xor);
            somme ^= sol_xor_inv;
        }
        if (somme == 0x00)
        {
            uint8_t *temp = (uint8_t *)realloc(cle_trouvees, (nb_cles_trouvees+1)*sizeof(uint8_t));
            if (!temp)
            {
                fprintf(stderr, "realloc: sur la table des cles trouvées\n");
                free(cle_trouvees);
                free(F_2_8);
                return NULL;
            }
            cle_trouvees = temp;
            cle_trouvees[nb_cles_trouvees] = cle_candidat;
            nb_cles_trouvees++;
        }
    }
    if (resultat_cles_trouvees)
    {
        *resultat_cles_trouvees = nb_cles_trouvees;
    }
    free(F_2_8);
    return cle_trouvees;
}

uint8_t *intersection_deux_ensemble(const uint8_t *ptr1,const size_t taille_ptr1,const uint8_t *ptr2, const size_t taille_ptr2,size_t *taille_intersection){
    bool ptr_contient[NB_PLAIN_ET_CYPHER_TEXTE] = {false};
    uint8_t *intersection = NULL;
    size_t cal_taille_inter = 0;

    // je mets sur la table de hachage toutes les valeurs de ptr1 a true
    for (size_t i = 0; i < taille_ptr1; i++)
    {
        ptr_contient[ptr1[i]] = true;
    }

    for (size_t i = 0; i < taille_ptr2; i++)
    {
        uint8_t element = ptr2[i];
        if (ptr_contient[element])
        {
            uint8_t *temp = realloc(intersection, (cal_taille_inter+1)*sizeof(uint8_t));
            if (!temp)
            {
                fprintf(stderr, "realloc: sur l'intersection des deux sous ensembles\n");
                free(intersection);
                return NULL;
            }
            intersection = temp;
            intersection[cal_taille_inter] = element;
            cal_taille_inter++;
            ptr_contient[element] = false;
        }
    }
    if (taille_intersection)
    {
        *taille_intersection = cal_taille_inter;
    }
    return intersection;
}

message *trouver_integralite_de_k_4(const message *chiffres){
    message *cle_k_4 =(message *) malloc(sizeof(message));
    if (!cle_k_4)
    {
        fprintf(stderr, "malloc: sur la allocation de cle_k_4\n");
        return NULL;
    }

    size_t resultat_cles_trouvees = 0;
    for (size_t i = 0; i < AES_STATE_DIM; i++)
    {
        for (size_t j = 0; j < AES_STATE_DIM; j++)
        {
            uint8_t *tab_octet = trouver_4_ieme_cle_octet_octet(chiffres, NB_PLAIN_ET_CYPHER_TEXTE, j, i, &resultat_cles_trouvees);
            if (!tab_octet)
            {
                fprintf(stderr, "probleme sur l'appel de la fonction trouver_4_ieme_cle_octet_octet pour la premiere fois\n");
                free(cle_k_4);
                return NULL;
            }
            
            if (resultat_cles_trouvees==0 || resultat_cles_trouvees > 1)
            {
                message *messages1 = gen_alea_messages(NB_PLAIN_ET_CYPHER_TEXTE, CONSTANTE1);
                message *chiffres1 = chiffre_gen_alea_messages(NB_PLAIN_ET_CYPHER_TEXTE, messages1, key128);
                size_t resultat_cles_trouvees1 = 0;
                uint8_t *tab_octet1 = trouver_4_ieme_cle_octet_octet(chiffres1, NB_PLAIN_ET_CYPHER_TEXTE, j, i, &resultat_cles_trouvees1);
                size_t taille_intersection = 0;
                uint8_t *intersection = intersection_deux_ensemble(tab_octet, resultat_cles_trouvees, tab_octet1, resultat_cles_trouvees1, &taille_intersection);
                //Bon a partir de la je triche pour l'instant mais je suppose que le nombre d'element de l'intersection est 1
                if (taille_intersection == 1) 
                {
                    cle_k_4->tab[i][j] = intersection[0]; 
                } else {
                    fprintf(stderr, "Échec de l'attaque : Intersection ne donne pas 1 seule clé pour [%zu][%zu]\n", i, j);
                    
                    // Nettoyage complet
                    free(cle_k_4);
                    free(messages1);
                    free(chiffres1);
                    free(tab_octet); 
                    free(tab_octet1); 
                    free(intersection);
                    return NULL;
                }
                free(messages1);
                free(chiffres1);
                free(tab_octet1);
                free(intersection);
            }else
            {
                cle_k_4->tab[i][j] = tab_octet[0];
            }
            free(tab_octet);
        }
    }
    return cle_k_4;
}
int main(void)
{
    //certaines variables 

    
    //appel aux fonctions
    message *messages = gen_alea_messages(NB_PLAIN_ET_CYPHER_TEXTE, CONSTANTE);
    /* for (size_t i = 0; i < NB_PLAIN_ET_CYPHER_TEXTE; i++)
    {
        print_bloc(messages[i].tab);
        printf("\n\n");
    } */
    message *chiffres = chiffre_gen_alea_messages(NB_PLAIN_ET_CYPHER_TEXTE, messages, key128);
   /*  for (size_t i = 0; i < NB_PLAIN_ET_CYPHER_TEXTE; i++)
    {
        print_bloc(chiffres[i].tab);
        printf("\n\n");
    } */
    message *cle_k_4 = trouver_integralite_de_k_4(chiffres);
    printf("La quatrieme cle est\n");
    print_bloc(cle_k_4->tab);
    message *cle_maitre = trouver_cle_maitre(cle_k_4);
    printf("La cle maitre est\n");
    print_bloc(cle_maitre->tab);
    //Nettoyage de la memeoire
    free(messages);
    free(chiffres);
    free(cle_k_4);
    free(cle_maitre);
    return 0;
}

