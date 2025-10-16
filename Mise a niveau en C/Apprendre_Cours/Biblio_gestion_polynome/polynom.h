#if !defined(POLYNOM_H)
#define POLYNOM_H
#include <inttypes.h>
/*don't use a hard constant we use symbolique constant*/
#define MAX_DEGRE  32
typedef uint32_t poly_t;

poly_t addition_polynomiale(poly_t, poly_t);

#endif // POLYNOM_H
