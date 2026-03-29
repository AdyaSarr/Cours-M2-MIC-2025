#if !defined(FF_UTIL_H)
#define FF_UTIL_H

static inline int imod(int a, int p){
    int r = a%p;
    if(r<0) r +=p;
    return r;
}

int inv_modp(int a, int p);

#endif // FF_UTIL_H
