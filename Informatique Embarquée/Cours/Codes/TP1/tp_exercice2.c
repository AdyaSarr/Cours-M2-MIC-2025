#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "tp_exercice2.h"




const Map map_op[] = {
    // Les R-Type
    {"add", ADD}, {"sub", SUB}, {"mul", MUL}, {"div", DIV}, {"and", AND}, {"or", OR}, {"xor", XOR},
    // Les I-Type 
    {"addi", ADDI}, {"subi", SUBI}, {"muli", MULI}, {"divi", DIVI}, {"andi", ANDI}, {"ori", ORI}, {"xori", XORI},
    // Memoire 
    {"lw", LW}, {"sw", SW},
    // Branchement
    {"beq", BEQ}, {"bne", BNE}, {"blt", BLT}, {"bgt", BGT},
    {NULL, 0}
};
//Cette fonction me permet de netoyer le fichier a l'entrée dans un premier temps je considere que les commentaires sont faits avec #
void netoyage_file(char *str){
    char *search_char = strchr(str, '#');
    if (search_char)
    {
        //Si le caractere est trouvé on tronc le ligne
        *search_char = '\0';
    }

    //netoyer les espaces apres la chaine

    int taille_chaine = strlen(str);
    while (taille_chaine > 0 && isspace((unsigned char)str[taille_chaine-1]))
    {
        str[--taille_chaine]='\0';
    }
    
    //Nettoyage de l'avant de la chaine
    char *p =str;
    while (p && isspace((unsigned char)*p))
    {
        p++;
    }
    if (p != str) memmove(str, p, strlen(p) + 1);

    for (char *q = str; *q; q++) *q = (char)tolower((unsigned char)*q);

}

/*
    Retourne l'Opcode pour une instruction donnée
    op_name Nom de l'instruction
    L'Opcode (entier) ou -1 si non trouvé
*/
int get_opcode(const char *op_name){
    for (int i = 0; map_op[i].chaine; i++)
    {
        if (!strcmp(map_op[i].chaine, op_name))
        {
            return map_op[i].op;
        }
        
    }
    return -1;
}

/*
    Parse la chaine de registre ($rX) et retourne le numéro du registre.
    reg_str Chaine de registre (ex: "$1", "$0")
    Le numéro du registre (entier) ou -1 en cas d'erreur.
*/

int parse_register(const char *reg_str) {
    if (reg_str[0] == '$') {
        return atoi(reg_str + 1); // Décalage de 1 pour ignorer le '$'
    }
    return -1;
}

/*
    Trouve un label dans la table.
    Le PC (adresse) du label ou -1 si non trouvé.
 */
int find_label_pc(const char *label_name, const Label *label_table, int num_labels) {
    for (int i = 0; i < num_labels; i++) {
        if (strcmp(label_table[i].nom, label_name) == 0) {
            return label_table[i].pc;
        }
    }
    return -1;
}

/*
    Première passe de l'assembleur: Collecte des labels.
    0 en cas de succès, -1 en cas d'erreur.
 */
int first_pass(FILE *input_file, Label *label_table, int *num_labels, Row *source_rows, int *num_rows) {
    char line_buffer[MAX_LINE];
    int pc = 0;
    *num_labels = 0;
    *num_rows = 0;

    fseek(input_file, 0, SEEK_SET);

    while (fgets(line_buffer, MAX_LINE, input_file) != NULL) {
        if (*num_rows >= MAX_INSTRUCTIONS) {
            fprintf(stderr, "Erreur: Le programme d'entrée dépasse la limite de %d lignes (MAX_INSTRUCTIONS).\n", MAX_INSTRUCTIONS);
            return -1;
        }
        
        char clean_line[MAX_LINE];
        strcpy(clean_line, line_buffer);
        netoyage_file(clean_line);

        if (strlen(clean_line) == 0) {
            continue;
        }

        strcpy(source_rows[*num_rows].text, clean_line);
        source_rows[*num_rows].is_instruction = 1;

        char *label_end = strchr(clean_line, ':');
        if (label_end) {
            
            if (*num_labels >= MAX_LABELS) {
                fprintf(stderr, "Erreur: Le programme d'entrée dépasse la limite de %d labels (MAX_LABELS).\n", MAX_LABELS);
                return -1;
            }
            
            *label_end = '\0';
            strcpy(label_table[*num_labels].nom, clean_line);
            label_table[*num_labels].pc = pc;
            (*num_labels)++;
            source_rows[*num_rows].is_instruction = 0;
        } else {
            pc++;
        }
        (*num_rows)++;
    }
    return 0;
}



/*
    Assemble une seule instruction en un mot de 32 bits.
    pc Program Counter actuel (adresse de l'instruction)
    line Ligne d'assembleur (déjà nettoyée)
    label_table Table de labels
    num_labels Nombre de labels
    Le mot d'instruction de 32 bits (uint32_t)
 */

uint32_t assemble_instruction(int pc, const char *line, const Label *label_table, int num_labels) {
    uint32_t instruction = 0;
    char line_copy[MAX_LINE];
    strcpy(line_copy, line);

    
    char *token = strtok(line_copy, " \t,()");
    if (token == NULL) return 0;

    char op_name[20];
    strcpy(op_name, token);
    int opcode = get_opcode(op_name);

    if (opcode == -1) {
        fprintf(stderr, "Erreur à l'adresse %d: Instruction inconnue '%s'\n", pc, op_name);
        return 0;
    }

    // Début de l'assemblage: Opcode (8 bits) en position [31:24]
    instruction |= ((uint32_t)opcode) << 24;


    // Instructions de Type R: OP r1, r2, r3  (add $1, $2, $3)
    if (opcode >= ADD && opcode <= XOR) { // R-Type: 0x00 à 0x06
        // --- Vérification des 3 opérandes R-Type (r1, r2, r3) ---
        char *r1_str = strtok(NULL, " \t,()");
        char *r2_str = strtok(NULL, " \t,()");
        char *r3_str = strtok(NULL, " \t,()");
        
        if (r1_str == NULL || r2_str == NULL || r3_str == NULL) {
            fprintf(stderr, "Erreur à l'adresse %d: Opérande manquante pour l'instruction R-Type '%s'.\n", pc, op_name);
            return 0;
        }

        int r1 = parse_register(r1_str);
        int r2 = parse_register(r2_str);
        int r3 = parse_register(r3_str);

        if (r1 == -1 || r2 == -1 || r3 == -1) {
            fprintf(stderr, "Erreur à l'adresse %d: Registre R-Type invalide.\n", pc);
            return 0;
        }

        // r1 [23:18], r2 [17:12], r3 [11:6], Réservé [5:0]
        instruction |= ((uint32_t)r1 & 0x3F) << 18; // 6 bits (0-63)
        instruction |= ((uint32_t)r2 & 0x3F) << 12; // 6 bits
        instruction |= ((uint32_t)r3 & 0x3F) << 6;  // 6 bits
    }
    // Instructions de Type I (Immédiat) et Branchement: OP r1, r2, cst/label
    else if ((opcode >= ADDI && opcode <= XORI) || (opcode >= BEQ && opcode <= BGT)) {
        // --- Vérification des 3 opérandes I-Type/Branch (r1, r2, cst/label) ---
        char *r1_str = strtok(NULL, " \t,()");
        char *r2_str = strtok(NULL, " \t,()");
        char *last_token = strtok(NULL, " \t,()");

        if (r1_str == NULL || r2_str == NULL || last_token == NULL) {
            fprintf(stderr, "Erreur à l'adresse %d: Opérande manquante pour l'instruction I/Branchement '%s'.\n", pc, op_name);
            return 0;
        }

        int r1 = parse_register(r1_str);
        int r2 = parse_register(r2_str);

        if (r1 == -1 || r2 == -1) {
            fprintf(stderr, "Erreur à l'adresse %d: Registre I/Branchement invalide.\n", pc);
            return 0;
        }

        int immediate = 0;

        if (opcode >= BEQ && opcode <= BGT) {
            // ... (logique de branchement existante utilisant last_token)
            int target_pc = find_label_pc(last_token, label_table, num_labels);
            if (target_pc == -1) {
                fprintf(stderr, "Erreur à l'adresse %d: Label de branchement inconnu '%s'\n", pc, last_token);
                return 0;
            }
            immediate = target_pc - (pc + 1);
            if (immediate > 2047 || immediate < -2048) {
                 fprintf(stderr, "Erreur: Décalage de branchement trop grand pour 12 bits\n");
                 return 0;
            }
        } else {
            immediate = atoi(last_token);
            if (immediate > 2047 || immediate < -2048) {
                 fprintf(stderr, "Erreur: Constante immédiate trop grande pour 12 bits\n");
                 return 0;
            }
        }

        // r1 [23:18], r2 [17:12], Immediate/Label [11:0]
        instruction |= ((uint32_t)r1 & 0x3F) << 18;
        instruction |= ((uint32_t)r2 & 0x3F) << 12;
        instruction |= ((uint32_t)immediate & 0xFFF);
    }
    // Instructions Mémoire: lw r1, cst(r2) / sw r1, cst(r2)
    else if (opcode == LW || opcode == SW) { // Mémoire (0x10, 0x11)
        // --- Vérification des 3 opérandes Mémoire (r1, cst, r2) ---
        char *r1_str = strtok(NULL, " \t,()");
        char *cst_str = strtok(NULL, " \t,()");
        char *r2_str = strtok(NULL, " \t,()");

        if (r1_str == NULL || cst_str == NULL || r2_str == NULL) {
            fprintf(stderr, "Erreur à l'adresse %d: Opérande manquante pour l'instruction Mémoire '%s'.\n", pc, op_name);
            return 0;
        }
        
        int r1 = parse_register(r1_str);
        int r2 = parse_register(r2_str);

        if (r1 == -1 || r2 == -1) {
            fprintf(stderr, "Erreur à l'adresse %d: Registre Mémoire invalide.\n", pc);
            return 0;
        }

        int immediate = atoi(cst_str);
        if (immediate > 2047 || immediate < -2048) {
             fprintf(stderr, "Erreur: Décalage mémoire trop grand pour 12 bits\n");
             return 0;
        }

        // r1 [23:18], r2 [17:12], cst [11:0]
        instruction |= ((uint32_t)r1 & 0x3F) << 18;
        instruction |= ((uint32_t)r2 & 0x3F) << 12;
        instruction |= ((uint32_t)immediate & 0xFFF);

    } else {
        fprintf(stderr, "Erreur interne: Type d'instruction non géré pour l'opcode 0x%X\n", opcode);
        return 0;
    }

    return instruction;
}

/*
    Seconde passe de l'assembleur: Traduction des instructions.
    input_file Fichier assembleur
    output_file Fichier binaire de sortie
 */
void second_pass(FILE *input_file, FILE *output_file) {
    Label label_table[MAX_LABELS];
    int num_labels;
    Row source_rows[MAX_INSTRUCTIONS];
    int num_rows;
    int pc = 0;

    if (first_pass(input_file, label_table, &num_labels, source_rows, &num_rows) != 0) {
        return;
    }
    for (int i = 0; i < num_rows; i++) {
        const char *line = source_rows[i].text;
        
        if (source_rows[i].is_instruction) {
            // Assembler et écrire l'instruction
            uint32_t machine_code = assemble_instruction(pc, line, label_table, num_labels);
            fwrite(&machine_code, sizeof(uint32_t), 1, output_file);
            pc++;
        }
    }
}