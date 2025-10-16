#include <dlist.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




/*================================wrapper pour les entiers=============================*/


void free_int(int element){
    free(element);
}

int *copy_int(int *element){
    int *cp_int = malloc(sizeof(int));
    memcmp(cp_int, element, sizeof(int));
    return cp_int;
}


void print_int(int *element, FILE *out){
    fprintf(out, "%d", element);
}

