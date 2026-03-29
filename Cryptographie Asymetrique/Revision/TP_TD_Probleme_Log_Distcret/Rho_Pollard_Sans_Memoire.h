#if !defined(RHO_POLLARD_SANS_MEMOIRE_H)
#define RHO_POLLARD_SANS_MEMOIRE_H

typedef struct 
{
    long long y;
    int alpha;
    int beta;
}Etat;

Etat *fonction_pseudo_aleatoire(const Etat *courant, const long long order, const long long modulus, const long long gen, const long long elem);
long long attaque_Rho_Pollard(long long order, long long modulus, long long gen, long long elem);
Etat *copy_etat(Etat *src);
long long fonction_mod(long long entier, long long mod);
#endif // RHO_POLLARD_SANS_MEMOIRE_H
