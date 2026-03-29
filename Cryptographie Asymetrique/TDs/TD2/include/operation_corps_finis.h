#if !defined(OPERATION_CORPS_FINIS)
#define OPERATION_CORPS_FINIS

/**
 * @brief Definition du corps F_p avec p un nombre premier donné
 * @param p: est le nombre premier
 */
typedef struct 
{
    int p;
}field_F_p;

/**
 * @brief definition de la structure du corps F_q ou q = p^d====> F_q est isomorphe a F_p[X]/(f(X)) ou f(X) est un polynome irreductible de degre d
 * @param coefficiens: se sont les coefficients du polynome irreductible ca sera sous forme d'un pointeur
 * @param deg: le degre du polynome qui est egal a -1 quand c'est le polynome null
 * @param n : la taille en memoire des coefficients du polynome f(X) pourquoi ce choix immagine qu'on veut faire la multiplication de deux polynimes
 *              on a besoin d'allouer prealablement la taille en memoire des coefficients sinon on est obliger des faire des realloc a chaque fois.
 * @param k : un pointeur vers la caracteristique du corps F_q bon il point vers p
 */
typedef struct 
{
    int n;
    int deg;
    int *coefficients;
    field_F_p *k;
}field_F_q;

/**
 * @brief Cette fonction permet d'afficher un polynome donné
 * @param poly: polynome dont on doit afficher veiller a entre un polynome correcte
 */
void print_polynom(const field_F_q *poly);
/**
 * @brief Cette fonction permet de faire des reductions modulo p dans F_p
 * @param entier: correspond a l'entier dont on veut chercher sa correspondance dans F_p
 * @param mod: se sera le modulo du corps dans ce cas c'est p
 * @return elle retourne la correspondance de entier dans F_p
 */

 int modulo_dans_F_p(const int entier, const int mod);

 /**
  * @brief Cette fonction permet creer ou d'initialiser un polynome null
  * @param p: un pointeur vers la caracteristique du corps
  * @param n: la taille du polynome en terme de coefficients
  * @return retourne un pointeur vers un polynome nul
  */
field_F_q *init_polynom(const field_F_p *p, int n);
/**
 * @brief Cette fonction permet de faire l'addition de deux elements de F_p[X] avec une complexite de max{degP1, degP2}log(p)
 * @param poly1: la representation polynomiale d'un element de F_q
 * @param poly2: la representation polynomiale d'un element de F_q aussi
 * @return un polynome dans F_p[X]/(f(X))
 */
field_F_q *addition_F_p(const field_F_q *poly1, const field_F_q *poly2);

/**
 * @brief Cette fonction permet de faire la soustraction deux elements de F_p[X] avec une complexite idem de max{degP1, degP2}log(p)
 * @param poly1: la reprensation polynomiale d'un element de F_p[X]
 * @param poly2: la reprensation polynomiale d'un element de F_p[X]
 * @return la representation polynomiale du resultat de la soustraction
 */
field_F_q *soustraction_F_p(const field_F_q *poly1, const field_F_q *poly2);

/**
 * @brief Cette fonction permet de faire la multiplication deux elements de F_p[X] avec une complexite de log(p)
 * @param poly1: le premier polynome
 * @param poly2: le deuxieme
 */
field_F_q *multiplication_F_p(const field_F_q *poly1, const field_F_q *poly2);

/**
 * Cette structure permet de stocker a la fois le quotient et le reste de la division deux polynome
 */
typedef struct 
{
    field_F_q *quotient;
    field_F_q *reste;
}result_division;
/**
 * Cette fonction permet de chercher l'inverse d'un nombre entier modulo mod
 * @param entier l'entier qu'on cherche son inverse et mod le modulo
 * @return l'inverse de l'entier si celui-ci existe sinon il retourne -1
 */
int inverse_mod(const int entier,const int mod);
/**
 * @brief Cette fonction permet de faire la divisions euclidienne de deux polynomes
 * @param dividende: la dividende de la division euclidienne
 * @param diviseur: le diviseur
 * @return retourne a la fois le quotient et le reste de la division
 */
result_division *division_eucliduenne(field_F_q *dividende, field_F_q *diviseur);

/**
 * @brief Cette fonction permet de faire une copie d'un polynome vers un autre
 * @param source: le polynome dont on veut faire sa copie
 * @return un polynome contenant la copie de la premiere
 */
field_F_q *copy_polynom(const field_F_q *source);

/**
 * 
 */


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
 * @brief Cette fonction permet de faire la multiplication deux elements de F_q avec une complexite de (log(q))^2
 * @param poly1: la reprensation polynomiale d'un element de F_q
 * @param poly2: la reprensation polynomiale d'un element de F_q
 * @return la representation polynomiale su resultat
 */
field_F_q *multiplication_F_q(const field_F_q *poly1, const field_F_q *poly2);
#endif // OPERATION_CORPS_FINIS
