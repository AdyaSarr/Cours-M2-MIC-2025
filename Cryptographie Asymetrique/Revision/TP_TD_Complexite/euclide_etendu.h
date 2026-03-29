#if !defined(EUCLIDE_ETENDU_H)
#define EUCLIDE_ETENDU_H


typedef struct 
{
    int x;
    int y;
    int pgcd;
}ParamsEEA;

ParamsEEA euclideEtendu(const int entier1, const int entier2);


#endif // EUCLIDE_ETENDU_H
