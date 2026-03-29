#if !defined(ATTAQUE_LOG_DISCRET_H)
#define ATTAQUE_LOG_DISCRET_H

/**
 * @brief Cette structure contient tous les informations d'un etat donné
 * @param y: g^alpha * h^alpha mod N
 * @param alpha
 * @param beta
 */
typedef struct 
{
    int y;
    int alpha;
    int beta;
}Etat;

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
 * @brief Cette fonction permet de trouver le log discret d'un element dans une base g
 * @param order: ordre du groupe G
 * @param gen: generateur du goupe G
 * @param elem: l'element dont on veut sa log discret
 */
int attaque_Rho_Pollard(int order, int gen, int elem);

/**
 * @brief la fonction modulo du groupe cyclique
 * @param entier: l'entier dont on veut chercher sa correspondace sur le groupe
 * @param mod: l'ordre du groupe dans ce cas
 */
 int fonction_mod(int entier, int mod);


 /**
 * Cette fonction permet de chercher l'inverse d'un nombre entier modulo mod
 * @param entier l'entier qu'on cherche son inverse et mod le modulo
 * @return l'inverse de l'entier si celui-ci existe sinon il retourne -1
 */
int inverse_mod(const int entier,const int mod);

/**
 * Cette fonction permet de calculer le PGCD de deuc nombres en utilisant l'algorithme d'euclide
 * @param deux entiers
 * @return le PGCD
 */
int algo_Euclide(const int entier1,const int entier2);

/**
 * @brief La fonction pseudo-aleatoire qui permet de calculer l'etat suivant connaissant un etat courrant
 * @brief elle est definie comme suit
 *  - @brief F(y) = elem*y si y dans S_1
 *  - @brief F(y) = y^2 si y dans S_2
 *  - @brief F(y) = gen*y si y dans S_3
 * @brief dans ce cas particulier voici comment est partitionné le groupe:
 *  - @brief S_1 = {k dans le groupe tq  k congru 1 mod3}
 *  - @brief S_2 = {k dans le groupe tq  k congru 0 mod3}
 *  - @brief S_3 = {k dans le groupe tq  k congru 2 mod3}
 * @param courant: l'etap courant
 * @param order: c'est l'ordre du groupe
 * @param gen: le generateur du groupe 
 * @param elem: l'element dont on veut son log discret de base gen
 * @return etat suivant
 */

 Etat *fonction_pseudo_aleatoire(const Etat *courant,const int order, const int gen, const int elem);


 /**
  * @brief Cette fonction permet de copier de maniere propre
  * @param src: l'etat source
  * @return retourne un etat
  */
 Etat *copy_etat(Etat *src);
#endif // ATTAQUE_LOG_DISCRET_H
