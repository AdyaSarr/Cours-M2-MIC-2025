#if !defined(OPERATIONS_BINAIRES_H)
#define OPERATIONS_BINAIRES_H
#include <stdio.h>

/**  Cette fonction permet de faire l'addition de deux entiers 
*@param deux chaines de caracteres entier1 et entier2 mais en bianires
*@return une chaine en bianire qui contient la somme des deux entiers
*@brief veiller a entrez des chaines valides car je fais de precondition pour verifier et n'oublier pas de faire free a chaque appel de la fonction
*/
char *addition_binaire(const char *entier1, const char *entier2);

/** 
 * Cette fonction permet de faire le complement a deux d'entier donnee sous forme de chaine 
 * Cette operation permet de faire une soustration en utiulisant l'addition
 * Le complemment a deux fonctionne ainsi sur un exemple -3:
 *      -on donne l'ecriture en binaire de 3
 *      -On inverse les bites
 *      -On ajoute le resultat par un 
 * @param prend en entrée une chaine je ne fait les verificaion necessaire et ovaleaf pour 
 * @return une chaine qui est le complement de l'entier
*/
char *complement_a_deux(const char *entier, size_t ovaleaf);

/**
 * Cette fonction permet de faire la soustraction de entier1 et entier2 en utilisant le complement a deux
 * @param deux chaine de bits (pas de teste sur ces chaines)
 * @return retourne une chaine contenant la difference des deux entiers 
*/
char *soustraction_avec_cpmt_deux(char *entier1,char *entier2);

/**
 * Cette fonction permet de faire du padding sur la gauche de l'entier
 * @param une chaine de caractere representant un entier et la taille a la quelle on veut son padding*/
char *padding_left(char *entier, size_t ovoleaf);

/**
 * Cette fonction permet de faire la soustracrtion naive sans utiliser le complement a deux
 * @param deux chaines de caracteres bon je ne fait pas les testes pour verifier la correctitude de tes chaines
 * @return une chaine contenant la soustraction des deux chaines
*/
char *soustraction_naive(const char *entier1, const char *entier2);

/**
 * Cette fonction permet de faire la multiplication binaire deux chaines
 * @param deux chaines 
 * @return une chaine de taille au plus la somme des taille + 1*/
char *multiplication_naive(const char *entier1, const char *entier2);


/**
 * Cette fonction permet de faire la multiplication rapide avec la methode dse Karatsuba
 * @param prend en entrée deux entiers 
 * @return la multiplication des deux chaines*/
int karatsuba(int entier1, int entier2);

/**
 * Structure pour spliter un entier en deux entiers a = a_1* 2^k/2 + a_2*/

typedef struct 
{
    int a_1;
    int a_2;
}split_ent;


/**
 * Cette fonction permet de calculer le PGCD de deuc nombres en utilisant l'algorithme d'euclide
 * @param deux entiers
 * @return le PGCD
 */
int algo_Euclide(const int entier1,const int entier2);
/**
 * Cette fonction fait la meme chose que la precedent mais de maniere plus rapide
 */
int algo_Euclide_accelerer(int entier1, int entier2);

/**
 * Je passe a l'algorithme d'euclide etendue ou je calcule a la fois le pgcd deux entiers a et b en plus deux entier x et y tels
 * que ax + by = PGCD(a, b)
 * Etape 1: Definir la structure me permettant de le faire
 * Etape 2: l'implementation de la fonction
 */
typedef struct 
{
    int pgcd;
    int x;
    int y;
}params_euclide_etendu;
/**
 * @param deux entiers entier1 et entier2
 * @return le pgcd de entier1 et entier2 ainsi que x et y
 */
params_euclide_etendu *euclide_etendu(int entier1, int entier2);
/**
 * Fonction d'affichage de la structure
 */
void affichage_param_euclide(params_euclide_etendu *resultat, const int a, const int b);


/**
 * Cette fonction permet de chercher l'inverse d'un nombre entier modulo mod
 * @param entier l'entier qu'on cherche son inverse et mod le modulo
 * @return l'inverse de l'entier si celui-ci existe sinon il retourne -1
 */
int inverse_mod(const int entier,const int mod);
#endif // OPERATIONS_BINAIRES_H
