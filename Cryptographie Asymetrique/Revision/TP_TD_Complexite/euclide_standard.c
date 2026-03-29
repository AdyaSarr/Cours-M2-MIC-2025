#include <stdio.h>
#include "euclide_standard.h"


int euclideStandard(const int entier1,const int entier2){
    int max = abs(entier1);
    int min = abs(entier2);

    int r0 = max;
    int r1 = min;
    int r2;
    while (r1!=0)
    {
        r2 = r0%r1;
        r0 = r1;
        r1 = r2; 
    }
    return r0;
}
int main(int argc, char const *argv[])
{
    int entier1 = 252;
    int entier2 = 198;
    printf("Eucide Standard: PGCD(%d,%d)=%d\n", entier1, entier2, euclideStandard(entier1, entier2));
    return 0;
}
