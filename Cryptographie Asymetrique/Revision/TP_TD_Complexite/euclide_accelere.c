#include <stdio.h>
#include <stdlib.h>
#include "euclide_accelere.h"

int euclide_accelere(const int entier1, const int entier2){
    int a = abs(entier1);
    int b = abs(entier2);

    int r0 = a;
    int r1 = b;
    int r2, rprime2;

    while (r1!=0)
    {
        r2 = r0%r1;
        rprime2 = r1-r2;
        r0 = r1;
        if (rprime2<r2)
        {
            r1 = rprime2;
        }else
        {
            r1 = r2;
        }
    }
    return r0;
}

int main(int argc, char const *argv[])
{
    int entier1 = 2613;
    int entier2 = 2171;
    printf("Eucide Standard: PGCD(%d,%d)=%d\n", entier1, entier2, euclide_accelere(entier1, entier2));
    return 0;
}