#if !defined(algo_Solovay_Strasseu_h)
#define algo_Solovay_Strasseu_h
#include <stdbool.h>
#include "tests_primalite_pp_thm_Fermat.h"
#include "../TP_TD_Racine_nieme_unite_residu_quadratique/exponentiation_rapide.h"
#include "../TP_TD_Racine_nieme_unite_residu_quadratique/symbole_Legendre.h"
#include "../TP_TD_Proto_a_clef_publique/algorithme_RSA.h"


bool is_prime_solovay_strasseu(long long n, int k);


#endif // algo_Solovay_Strasseu_h
