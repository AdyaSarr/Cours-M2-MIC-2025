#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


uint8_t conversion_string_uint8(const char *binary_str) {
    uint8_t result = 0;
    int i;
    
    if (strlen(binary_str) != 8) {
        fprintf(stderr, "Erreur sur les parametres de la fonction\n");
        return 0; 
    }
    for (i = 0; i < 8; i++) {
        result <<= 1;
        if (binary_str[i] == '1') {
            result |= 1; 
        }
    }
    return result;
}

void donnee_analyseur_uart(const char *data) {
    size_t len = strlen(data);
    size_t index_courant = 0;
    
    while (index_courant < len) {

        // Je me deplace jusqu'au premier caractere '0' de la chaine data 
        while (index_courant < len && data[index_courant] == '1') {
            index_courant++;
        }

        //si on arrive jusqu'a la fin de la chaine j'arret le programme car aucune donnée n'a ete transmise
        if (index_courant >= len) {
            break;
        }

        // dans la chaine du TP on se deplace jusqu'a ce que index_courant soit 3 et ce qui suit est 0 01000010 1 
        // puis on place l'index_courant sur le premier 0
        index_courant++;
        char data_bits[9] = {0};
        bool erreur = false;
        for (int i = 0; i < 8; i++) {
            if (index_courant >= len) {
                fprintf(stderr, "Erreur: la donnée est mal formée (manque bit de donnée %d).\n", i);
                erreur = true;
                break;
            }
            data_bits[i] = data[index_courant];
            index_courant++;
        }
        
        if (erreur) break;
        
        if (index_courant >= len || data[index_courant] != '1') {
            fprintf(stderr, "La donnée donnée par l'analyseur ne respecte les contrats predefinie sur le TP\n");
            break;
        }
        index_courant++;//on decalle pour le bit stop
        // on cherche le mirroire du mot
        char mirroire_bits[9] = {0};
        for (int i = 0; i < 8; i++) {
            mirroire_bits[i] = data_bits[7 - i];
        }

        uint8_t valeur_ascii = conversion_string_uint8(mirroire_bits);
        printf("%c", valeur_ascii);
    }
    printf("\n");
}

int main() {
    const char *tp_data = "1110010000101011110110100111011010010101101011110110101010111010010011101111";
    donnee_analyseur_uart(tp_data);
    return 0;
}