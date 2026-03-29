#include <stdio.h>
#include <stdlib.h>
#include "Baby_Steps_Giant_Steps_Shanks.h"

int main(int argc, char const *argv[])
{
    Cycle_group group = {180, 181, 2};
    int t = 13; 
    Baby_steps *baby_steps = compute_baby_steps(&group, t);
    long long h = 153; 
    long long result = compute_giant_step(h, &group, baby_steps, t);
    if (result != -1) {
        printf("Le logarithme discret de %lld est : %lld\n", h, result);
    } else {
        printf("Aucun résultat trouvé pour le logarithme discret de %lld\n", h);
    }
    free(baby_steps->structure);
    free(baby_steps);
    return 0;
}