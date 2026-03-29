#if !defined(Baby_Steps_Giant_Steps_Shanks_h)
#define Baby_Steps_Giant_Steps_Shanks_h
#include "../TP_TD_Proto_a_clef_publique/algorithme_RSA.h"
typedef struct {
    int order_group;   // n = 180
    long long modulus; // p = 181
    long long generator; // g = 2
} Cycle_group;

typedef struct 
{
    long long r;
    long long q;
}Decomposition_x;

Decomposition_x *decompose_x(const long long x, const int t);

typedef struct {
    long long key;   // Stocke la valeur g^i mod p
    int exponent;    // Stocke l'exposant i
} HashEntry;

typedef struct 
{
    HashEntry *structure;
    int size;
}Baby_steps;

Baby_steps *compute_baby_steps(const Cycle_group *group, const int t);
long long compute_giant_step(const long long h, const Cycle_group *group, const Baby_steps *baby_steps, const int t);
#endif // Baby_Steps_Giant_Steps_Shanks_h
