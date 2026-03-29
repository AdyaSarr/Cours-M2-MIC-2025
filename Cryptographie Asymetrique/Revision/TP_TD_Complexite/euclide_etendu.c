#include <stdio.h>
#include <stdlib.h>
#include "euclide_etendu.h"


ParamsEEA euclideEtendu(const int entier1, const int entier2){
    ParamsEEA result;

    int r0 = entier1, r1 = entier2;
    int x0 = 1, x1 = 0;
    int y0 = 0, y1 = 1;

    int q, r2, x2, y2;
    while (r1!=0)
    {
        q = r0/r1;
        r2 = r0 - q*r1;
        x2 = x0 - q*x1;
        y2 = y0 - q*y1;

        r0 = r1;
        r1 = r2;

        x0 = x1;
        x1 = x2;

        y0 = y1;
        y1 = y2;        
    }
    result.pgcd = r0;
    result.x = x0;
    result.y = y0;
    if (result.pgcd < 0)
    {
        result.pgcd *=-1;
        result.x *=-1;
        result.y *=-1;
    }
    
    return result;
}

int main(int argc, char const *argv[])
{
    int entier1 = 252;
    int entier2 = 198;
    ParamsEEA result = euclideEtendu(entier1, entier2);
    printf("Theoreme de Bezout: %d*%d + %d*%d = %d\n", entier1, result.x, entier2, result.y, result.pgcd);
    return 0;
}
