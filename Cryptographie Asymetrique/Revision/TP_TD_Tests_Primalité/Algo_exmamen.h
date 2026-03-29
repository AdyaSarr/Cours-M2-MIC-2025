#if !defined(Algo_exmamen_h)
#define Algo_exmamen_h
#include <stdbool.h>
//M_p = 2^p -1 est premier sssi s_{p-2} = 0 mod(M_p) ou s_0 = 4 et s_{n+1} = s_n^2 - 2
bool is_mersenne_prime(int p);
#endif // Algo_exmamen_h
