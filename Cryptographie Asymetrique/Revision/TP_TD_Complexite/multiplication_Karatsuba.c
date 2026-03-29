#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "multiplication_Karatsuba.h"

ParamsKaratsuba cutInteger(int entier, int k){
    ParamsKaratsuba result = {0, 0};
    int m = k/2;
    int mask = (1<<m)-1;
    result.x_0 = entier & mask;
    result.x_1 = entier >> m;
    return result;
}

int karatsuba(int entier1, int entier2){
    if (entier1==0 || entier2 ==0)
    {
        return 0;
    }
    int max = (entier1>entier2)? entier1:entier2;
    if (entier1==1)
    {
        return entier2;
    }
    if (entier2==1)
    {
        return entier1;
    }
    
    
    int k = (sizeof(unsigned int) * CHAR_BIT) - __builtin_clz(max);
    int m = k/2;
    ParamsKaratsuba karatEntier1 = cutInteger(entier1, k);
    ParamsKaratsuba karatEntier2 = cutInteger(entier2, k);

    int x0y0 = karatsuba(karatEntier1.x_0, karatEntier2.x_0);
    int x1y1 = karatsuba(karatEntier1.x_1, karatEntier2.x_1);

    int sommex0x1 = karatEntier1.x_0+karatEntier1.x_1;
    int sommey0y1 = karatEntier2.x_0+karatEntier2.x_1;
    int termeFacte = karatsuba(sommex0x1, sommey0y1);
    return (x1y1<<(2*m)) + ((termeFacte -x0y0 - x1y1)<<m) + x0y0;
}

int main(int argc, char const *argv[])
{
    //int entiersur1024 = 134883044242130517890147688432538206128495301630970723281032159252586181879067614942883691189848683124360564286281416252693398842572201326905116972893923490402683158555798421987143624342763837185502763138574874651348776312253371787058162134260891957014346243303165372393125123594400019261330736031379936784103322;
    int entier1 = 134;
    int entier2 = 343;
    printf("Le produit par karatsuba: %d * %d = %d\n", entier1, entier2, karatsuba(entier1, entier2));
    return 0;
}

