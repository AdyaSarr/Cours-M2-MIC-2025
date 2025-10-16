#if !defined(TP_EXERCICE2_H)
#define TP_EXERCICE2_H
#include <stdlib.h>
#include <stdint.h> // Pour uint32_t pour le mot d'instruction
#define MAX_LINE 256
#define MAX_LABELS 100 // Taille maximale de la table de labels
#define MAX_INSTRUCTIONS 1024 // Taille maximale du programme

//Une enumeration de toutes les operations
enum {
    // Type Registre (8 bits: 00000000 - 00000110) [cite: 112]
    ADD = 0x00, SUB = 0x01, MUL = 0x02, DIV = 0x03, AND = 0x04, OR = 0x05, XOR = 0x06,
    // Type Immediate (8 bits: 00001000 - 00001101) [cite: 114]
    ADDI = 0x08, SUBI = 0x09, MULI = 0x0A, DIVI = 0x0B, ANDI = 0x0C, ORI = 0x0D, XORI = 0x0E,
    // Memoire (8 bits: 00010000 - 00010001) [cite: 116]
    LW = 0x10, SW = 0x11,
    // Branchement (8 bits: 00011000 - 00011011) [cite: 118]
    BEQ = 0x18, BNE = 0x19, BLT = 0x1A, BGT = 0x1B,
};


// Mappage des operations avec les chaines de caracteres
typedef struct
{
    const char *chaine;
    unsigned op;
}Map;

// Structure pour stocker les informations du code source
typedef struct
{
    char text[MAX_LINE];
    int is_instruction;
}Row;




//Cette structure me permet de gerer les operations des branchements c'est a dire quand on fait des sauts
typedef struct label
{
    char nom[245];
    int pc;
}Label;


// Prototypes des fonctions
int get_opcode(const char *op_name);
int parse_register(const char *reg_str);
int first_pass(FILE *input_file, Label *label_table, int *num_labels, Row *source_rows, int *num_rows);
uint32_t assemble_instruction(int pc, const char *line, const Label *label_table, int num_labels) ;
void second_pass(FILE *input_file, FILE *output_file);
#endif // TP_EXERCICE2_H
