#include <stdio.h>
#include "symbole_Legendre.h"

int main(int argc, char const *argv[])
{
    long long a = 3;
    long long p = 2081;
    if (p%2==0)
    {
        fprintf(stderr, "Erreur : p doit etre un nombre premier impair.\n");
        return 0;
    }
    fprintf(stdout, "Le symbole de Legendre (%lld/%lld) est : %lld\n", a, p, symbole_legendre(a, p));
    return 0;
}
