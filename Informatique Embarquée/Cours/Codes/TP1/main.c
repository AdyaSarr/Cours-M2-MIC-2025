#include <stdio.h>
#include <stdlib.h>
#include "tp_exercice2.h"


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <fichier_entree.asm> <fichier_sortie.hex>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *input_file = fopen(argv[1], "r");
    if (!input_file) {
        perror("Impossible d'ouvrir le fichier d'entrée");
        return EXIT_FAILURE;
    }

    FILE *output_file = fopen(argv[2], "wb"); 
    if (!output_file) {
        perror("Impossible d'ouvrir le fichier de sortie");
        fclose(input_file);
        return EXIT_FAILURE;
    }

    second_pass(input_file, output_file);

    fclose(input_file);
    fclose(output_file);
    printf("Assemblage réussi. Résultat dans %s\n", argv[2]);

    return EXIT_SUCCESS;
}