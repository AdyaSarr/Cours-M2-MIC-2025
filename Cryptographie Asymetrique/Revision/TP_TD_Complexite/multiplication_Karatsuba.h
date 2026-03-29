#if !defined(MULTIPLICATION_KARATSUBA_H)
#define MULTIPLICATION_KARATSUBA_H

typedef struct 
{
    int x_0;
    int x_1;
}ParamsKaratsuba;

ParamsKaratsuba cutInteger(int x, int k);
int karatsuba(int entier1, int entier2);
#endif // MULTIPLICATION_KARATSUBA_H
