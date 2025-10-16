#include <stdio.h>
#include <stdlib.h>


int main(void)
{
    double *pr;

    pr = malloc(10*sizeof(double));
    if(!pr)
        return -1;
    
    for (int i = 0; i < 10; i++)
        *(pr+i) = i;
    /*Parcour en utilisant des indices*/
    // for(int i = 0; i < 10; i++)
    //     printf("the element %d est: %f\n", i, *(pr+i));
    
    /*Parcour en unitlisation un pointeur*/
    int i=0;
    for(double *index = pr; index - pr < 10; index++){
        printf("the element %d est: %f\n", i, *index);
        i++;
    }
    


    /*When we want to modify the size of a pointer here pr we can use realloc(void *, size_t)*/

    double *new_pr;

    new_pr = realloc(pr, 20*sizeof *pr);

    if(new_pr == NULL){
        printf("error of resizing\n");
        free(pr);
        return 1;
    }

    pr = new_pr;

    for (size_t k = 10; k < 20; k++) pr[k] = 0.0;
    for(double *index = pr; index-pr < 20; index++)
        printf("%f\n", *index);
    
    free(pr);
    return 0;
}
