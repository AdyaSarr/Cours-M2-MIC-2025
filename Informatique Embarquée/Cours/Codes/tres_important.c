// sim.c — Emulateur minimal pour le jeu d'instructions de l'Exo 1
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

#define NUM_REGS 64
#define MEM_WORDS 1024           // mémoire d'instructions + données (au choix)
#define INSTR_SIZE 4             // 32 bits

// Champs (big picture 32 bits): [31:24]=OP (8b), [23:18]=A (6b), [17:12]=B (6b), [11:0]=C/IMM/LBL (12b ou 6b+6rsvd)
static inline uint8_t  get_op (uint32_t ins) { return (ins >> 24) & 0xFF; }
static inline uint8_t  get_A  (uint32_t ins) { return (ins >> 18) & 0x3F; }
static inline uint8_t  get_B  (uint32_t ins) { return (ins >> 12) & 0x3F; }
static inline uint16_t get_U12(uint32_t ins) { return  ins        & 0x0FFF; }   // non signé
static inline int16_t  get_S12(uint32_t ins) { // signe sur 12b
    int16_t v = (int16_t)(ins & 0x0FFF);
    if (v & 0x0800) v |= ~0x0FFF; // sign-extend
    return v;
}
// Pour type R, le 3e registre est dans [11:6], et [5:0] réservé
static inline uint8_t get_C(uint32_t ins) { return (ins >> 6) & 0x3F; }

enum {
    // R-type
    OP_ADD = 0x00, OP_SUB = 0x01, OP_MUL = 0x02, OP_DIV = 0x03,
    OP_AND = 0x04, OP_OR  = 0x05, OP_XOR = 0x06,
    // I-type
    OP_ADDI= 0x08, OP_SUBI= 0x09, OP_DIVI= 0x0A, OP_ANDI=0x0B, OP_ORI=0x0C, OP_XORI=0x0D,
    // MEM
    OP_LW  = 0x10, OP_SW  = 0x11,
    // BR
    OP_BEQ = 0x18, OP_BNE = 0x19, OP_BLT = 0x1A, OP_BGT = 0x1B,
};

typedef struct {
    uint32_t mem[MEM_WORDS];   // mémoire (vue mots 32b) — on peut partager code/données pour un TP simple
    int32_t  R[NUM_REGS];      // registres 32b signés (adapte si tu veux 64b)
    uint32_t PC;               // en octets (adresse de l'instruction courante)
    bool     running;
} CPU;

static inline uint32_t load_word(CPU* cpu, uint32_t addr_bytes) {
    uint32_t idx = addr_bytes / 4;
    if (idx >= MEM_WORDS) { fprintf(stderr, "Load OOB @0x%08x\n", addr_bytes); exit(1); }
    return cpu->mem[idx];
}
static inline void store_word(CPU* cpu, uint32_t addr_bytes, uint32_t val) {
    uint32_t idx = addr_bytes / 4;
    if (idx >= MEM_WORDS) { fprintf(stderr, "Store OOB @0x%08x\n", addr_bytes); exit(1); }
    cpu->mem[idx] = val;
}

// Accès "données" 32 bits (LW/SW) : on réutilise mem[] pour simplifier.
// Adresse effective = R[B] + IMM12 (en octets). On force l’alignement mot.
static inline int32_t data_load(CPU* cpu, uint32_t addr) {
    if (addr % 4) { fprintf(stderr, "Unaligned LW @0x%08x\n", addr); exit(1); }
    return (int32_t)load_word(cpu, addr);
}
static inline void data_store(CPU* cpu, uint32_t addr, int32_t v) {
    if (addr % 4) { fprintf(stderr, "Unaligned SW @0x%08x\n", addr); exit(1); }
    store_word(cpu, addr, (uint32_t)v);
}

// exécute UNE instruction
static void step(CPU* c) {
    uint32_t pc = c->PC;
    uint32_t ins = load_word(c, pc);
    uint8_t  op  = get_op(ins);
    uint8_t  A   = get_A(ins);
    uint8_t  B   = get_B(ins);
    int16_t  imm = get_S12(ins);
    uint8_t  C   = get_C(ins);

    // Valeur de PC par défaut (séquentiel)
    uint32_t nextPC = pc + INSTR_SIZE;

    // R0 câblé à 0 : lecture retourne 0, écriture ignorée
    auto getR = [&](uint8_t r){ return (r==0) ? 0 : c->R[r]; };
    auto setR = [&](uint8_t r, int32_t v){ if (r!=0) c->R[r] = v; };

    switch (op) {
        // ===== R-type =====
        case OP_ADD: setR(A, getR(B) + getR(C)); break;
        case OP_SUB: setR(A, getR(B) - getR(C)); break;
        case OP_MUL: setR(A, getR(B) * getR(C)); break;
        case OP_DIV: setR(A, getR(C)==0 ? 0 : (getR(B) / getR(C))); break;
        case OP_AND: setR(A, getR(B) & getR(C)); break;
        case OP_OR : setR(A, getR(B) | getR(C)); break;
        case OP_XOR: setR(A, getR(B) ^ getR(C)); break;

        // ===== I-type =====
        case OP_ADDI: setR(A, getR(B) + imm); break;
        case OP_SUBI: setR(A, getR(B) - imm); break;
        case OP_DIVI: setR(A, imm==0 ? 0 : (getR(B) / imm)); break;
        case OP_ANDI: setR(A, getR(B) & (int32_t)imm); break;
        case OP_ORI : setR(A, getR(B) | (int32_t)imm); break;
        case OP_XORI: setR(A, getR(B) ^ (int32_t)imm); break;

        // ===== Mémoire =====
        case OP_LW: {
            uint32_t addr = (uint32_t)(getR(B)) + (int32_t)imm;
            int32_t val = data_load(c, addr);
            setR(A, val);
        } break;
        case OP_SW: {
            uint32_t addr = (uint32_t)(getR(B)) + (int32_t)imm;
            data_store(c, addr, getR(A));
        } break;

        // ===== Branch =====
        case OP_BEQ: if (getR(A) == getR(B)) nextPC = pc + INSTR_SIZE + ((int32_t)imm << 2); break;
        case OP_BNE: if (getR(A) != getR(B)) nextPC = pc + INSTR_SIZE + ((int32_t)imm << 2); break;
        case OP_BLT: if (getR(A) <  getR(B)) nextPC = pc + INSTR_SIZE + ((int32_t)imm << 2); break;
        case OP_BGT: if (getR(A) >  getR(B)) nextPC = pc + INSTR_SIZE + ((int32_t)imm << 2); break;

        default:
            fprintf(stderr, "Opcode inconnu 0x%02X @PC=0x%08x\n", op, pc);
            c->running = false;
            return;
    }

    c->PC = nextPC;
}

// Boucle d'exécution
static void run(CPU* c, uint32_t max_steps) {
    c->running = true;
    for (uint32_t i=0; i<max_steps && c->running; i++) {
        step(c);
    }
}

// --------- Exemple minimal d’utilisation ----------
static uint32_t ENCODE_R(uint8_t op, uint8_t A, uint8_t B, uint8_t C) {
    return ((uint32_t)op << 24) | ((uint32_t)A << 18) | ((uint32_t)B << 12) | ((uint32_t)C << 6);
}
static uint32_t ENCODE_I(uint8_t op, uint8_t A, uint8_t B, int16_t imm12) {
    uint32_t u12 = ((uint16_t)imm12) & 0x0FFF;
    return ((uint32_t)op << 24) | ((uint32_t)A << 18) | ((uint32_t)B << 12) | u12;
}

int main(void) {
    CPU cpu = {0};
    // R0=0 par convention. On initialise la mémoire à 0.
    memset(cpu.mem, 0, sizeof(cpu.mem));
    memset(cpu.R,   0, sizeof(cpu.R));
    cpu.PC = 0;

    // --- Démo: charge un petit programme en mémoire (tu peux remplacer par n'importe quelles instructions valides) ---
    // Exemple logique "test == 42"
    // lw  r1, 0(r0)
    cpu.mem[0] = ENCODE_I(OP_LW,  1, 0, 0);
    // addi r2, r0, 42
    cpu.mem[1] = ENCODE_I(OP_ADDI,2, 0, 42);
    // addi r3, r0, 1
    cpu.mem[2] = ENCODE_I(OP_ADDI,3, 0, 1);
    // beq r1, r2, +2 (saut par rapport à l'instruction suivante) → offset en mots = +2
    cpu.mem[3] = ENCODE_I(OP_BEQ, 1, 2, +2);
    // xor r3, r3, r3
    cpu.mem[4] = ENCODE_R(OP_XOR, 3, 3, 3);
    // sw  r3, 0(r0)
    cpu.mem[5] = ENCODE_I(OP_SW,  3, 0, 0);

    // Donnée à l'adresse 0 (en mots) : mets 42 ou autre pour tester
    cpu.mem[0] = cpu.mem[0]; // instruction déjà mise
    cpu.mem[10] = 0; // libre
    // Place une valeur initiale à l'adresse 0 (données) — ici on réutilise mem[256] comme “data @0”
    // Pour la démo simple, on va plutôt écrire dans mem[0] en tant que données:
    // => Pour éviter de mélanger code et données dans ce squelette,
    //    on va choisir l'adresse 0x100 (mot 64) comme zone "données @0".
    // Modifie data_load/store pour mapper si tu veux séparer proprement code/données.
    // (Garde ce commentaire si tu remplaces par un vrai chargeur.)

    // Exécution
    run(&cpu, 100);

    // Affichage registre/mémoire (debug)
    printf("PC=0x%08" PRIx32 "\n", cpu.PC);
    for (int i=0; i<8; i++) printf("R%-2d = %d\n", i, cpu.R[i]);
    return 0;
}
