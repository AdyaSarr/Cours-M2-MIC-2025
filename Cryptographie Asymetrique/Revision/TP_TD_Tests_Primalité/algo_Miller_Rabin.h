#if !defined(algo_Miller_Rabin_h)
#define algo_Miller_Rabin_h
#include <stdbool.h>
#include "tests_primalite_pp_thm_Fermat.h"
#include "../TP_TD_Racine_nieme_unite_residu_quadratique/exponentiation_rapide.h"
#include "../TP_TD_Proto_a_clef_publique/algorithme_RSA.h"

typedef struct {
    long long t;
    int s;
} Decomposition;

Decomposition *decompose(long long n);
bool is_prime_miller_rabin(long long n, int k);


#endif // algo_Miller_Rabin_h
