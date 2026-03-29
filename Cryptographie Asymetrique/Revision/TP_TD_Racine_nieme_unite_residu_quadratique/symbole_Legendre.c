#include <stdio.h>
#include "exponentiation_rapide.h"
#include "symbole_Legendre.h"

long long symbole_legendre(long long a, long long p) {
    
    a%=p;
    if (a<0) a+=p;
    

    if (a==1) return 1;
    if(a==0) return 0;
    if (a == 2) {
        if (p % 8 == 1 || p % 8 == 7) return 1;
        else return -1;
    }
    if(a==p-1){
        if (p%4==1) return 1;
        if(p%4==3) return -1;   
    }

    if (a%2==0)
    {
        return symbole_legendre(a/2, p) * symbole_legendre(2, p);
    }

    if (a%4==3 && p%4==3)
    {
        return -symbole_legendre(p%a, a);
    }
    return symbole_legendre(p%a, a);
    
}