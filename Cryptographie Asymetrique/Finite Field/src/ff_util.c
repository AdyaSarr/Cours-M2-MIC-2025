#include <ff_util.h>


int inv_modp(int a, int p){ // Fermat :p prime et p non div a alors a^(p-1) congru a 1modp alors l'inverse est a^(p-2) modp
    long long base = imod(a, p);
    long long res = 1;
    int e = p-2;
    while(e){ 
        if(e&1) res = (res*base)%p;
        base = (base*base)%p;
        e >>= 1;
    }
    return (int)res;
}