#include <stdio.h>
#include "Algo_exmamen.h"

int main(){
    int p = 5;
    if (is_mersenne_prime(p)) { 
        printf("M_%d= 2^%d -1 est premier.\n", p, p); 
    }else { 
        printf("M_%d n'est pas premier.\n", p);
    }
}