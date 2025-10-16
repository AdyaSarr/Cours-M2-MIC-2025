#include "polynom.h"

/*For example when I like to define an auxilier function that using only this module. We difine it in the static class */
/*Example search the maximum between two numbers a and b. We define the prototype of the function at the top of the module*/

//static int max(int a , int b);


poly_t addition_polynomiale(poly_t poly1, poly_t poly2){
    return poly1^poly2;
}

// static int max(int a , int b){
//     return a>b?a:b;
// }