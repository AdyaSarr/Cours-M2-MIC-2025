#include <stdio.h>
#include "Algo_exmamen.h"

bool is_mersenne_prime(int p){
    if(p < 1) return false;
    if(p == 1) return true;
    long long Mp = (1LL << p) - 1;
    long long S_de_p_moin_2=4;
    for (int i = 0; i < p-2; i++)
    {
        S_de_p_moin_2 = (S_de_p_moin_2*S_de_p_moin_2 -2)%Mp;
    }
    if (S_de_p_moin_2==0)
    {
        return true;
    }
    return false;
}