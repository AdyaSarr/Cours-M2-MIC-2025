/*Operation on the pointers:
We have four operations on the pointers(except on the pointers of function or pointer of type void *)*/

#include <stdio.h>

int main(void)
{
    int entier;//declaration of integer
    int *pointer; //declaration of pointer

    entier = 3; //initialization of the integer entier
    pointer = &entier;// initialization of the pointer we cannot manupilate a pointer without initialization otherwise on the execution we will have segmentation fault

    printf("The integer is: %d\n", entier);
    printf("The pointer points to : %p\n", (void *)pointer);
    printf("The integer is: %d\n", *pointer);

    *pointer = 5;
    printf("The integer is: %d\n", entier);


    /*Addition pointer with an integer*/
    /*let type *p and an integer i so p +i is an other pointer which points in &p + i*sizeof(typr)*/
    int i = 2;
    int *q = pointer + i;

    *q = 5;
    printf("The pointer points to : %p\n", (void *)q);
    printf("The integer is: %d\n", *q);

    return 0;
}
