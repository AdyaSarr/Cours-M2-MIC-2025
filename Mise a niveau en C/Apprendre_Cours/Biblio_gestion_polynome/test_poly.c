#include <stdio.h>
#include "polynom.h"

int main(void)
{
    poly_t poly1 =123456789, poly2 = 42;

    printf("The result of the addition between the two numbers is: %" PRIu32 "\n", addition_polynomiale(poly1, poly2));
    return 0;
}
