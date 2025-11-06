#if !defined(ATTAQUE_INTEGRALE_H)
#define ATTAQUE_INTEGRALE_H
#include "aes.h"

/**
 * La structure des messages
 */
typedef struct 
{
    uint8_t tab[AES_STATE_DIM][AES_STATE_DIM];
}message;

/** 
*@brief Cette fonction me permet de generer aleatoirement des  (256 dans ce cas) messages et les stocker sur une table 
*@param prend en entree la taille du pointeurs qui doit contenir les message
**/
message *gen_alea_messages(size_t taille, const uint8_t constante);


/**
 * @brief Cette fonction permet d'afficher un bloc de 128 bits sur forme d'un tableau 4x4
 * @param elle prend en entrée un tableau d'entier 4x4
 */
void print_bloc(uint8_t tab[AES_STATE_DIM][AES_STATE_DIM]);

/**
 * @brief cette fonction permet de chiffrer (en appelant la fonction de chiffrement de AES de ce git https://github.com/m3y54m/aes-in-c/tree/main)
 *          et les stocker tous dans une table
 * @param prend en entrée la taille (le nombre) des messages a chiffrer et le tableau des messages a chiffrer et la clé de chiffrement
 */
message *chiffre_gen_alea_messages(size_t taille,const message *messages,const uint8_t *key);

/**
 * Cette fonction permet de me generer des nombre de 0 a 255.
 * @return un pointeur vers ces nombre (mais si on fait appel a cette fonction on oublie de le free)
 */
uint8_t *valeurs_de_0_255();
/**
 * Puisque le proprietaire du code de AES a rendu static l'inverse de la matrice du sbox c'est pourquoi je reprend cette constante ici
 * pour pouvoir donner l'inverse d'un octet donné, j'ai definie la fonction suivante dans le point c static uint8_t S_inv_octet(uint8_t octet);
 */

/**
 * @brief cette fonction permet de trouver la quatrieme clé générée par l'algorithme de cadancement des clés.
 *          en procedant octet par octet
 * @param une table contenant tous les chiffrés des 256 messages, le nombre de chiffrés, l'octet de la cle qu'on veut determiner simboliser par la colonne et la ligne, a la fin pour chaque octet le nombre de cle candidates resultat_cles_trouvees 
 * @return la quatrieme cle dans son entierté
 */
uint8_t *trouver_4_ieme_cle_octet_octet(const message *chiffres, 
                                            size_t taille,
                                        const int colonne_cible,
                                        const int ligne_cible,
                                        size_t *resultat_cles_trouvees);

/**
 * @brief Cette fonction me donne l'intersection de deux ensembles de cles candidates
 * @param les deux ensembles avec leurs tailles ainsi que une variable qui stockera la taille de l'intersection
 */
uint8_t *intersection_deux_ensemble(const uint8_t *ptr1,const size_t taille_ptr1,const uint8_t *ptr2, const size_t taille_ptr2,size_t *taille_intersection);
/**
 * @brief Cette fonction permet de donner l'integralité de la quatrieme cle en s'appuyant sur la fonction precedente puisqu'elle me permet de trouver des candidats octets par octets
 * @param l'ensemble des 256 chiffrés sur le tableau chiffres
 * @return la quatrieme cle avec le format Row-MAjor
 */
message *trouver_integralite_de_k_4(const message *chiffres);

/**
 * @brief Cette fonction me permet de trouver m'entierte de cle maitre en me basant uniquement sur la quatrieme cle
 *          Mais comme je sait que l'algo d'expansion travaille sur des mot(pour la cle numero n chacune de ses colonne est considerer comme un mot)
 *          Donc K_n = (w_4n, w_4n-1, w_4n-2, w_4n-3) et dans notre cas on s'arrete a 4 tour le nombre de mots est 20 = 4*5(le nombre de cle)
 *          K_4=(w_16, w_17, w_18, w_19) est connue
 * @param la quatrieme a partir de la quelle je determinie progressivement la cle entiere
 * @return la cle maitre
 */
message *trouver_cle_maitre(const message *cle_k_4);
#endif // ATTAQUE_INTEGRALE_H
