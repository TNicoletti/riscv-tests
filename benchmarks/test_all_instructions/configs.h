#define SEW 32
#define VLEN 128
#define LMUL 1
#define EL_PER_BLOCK (VLEN / SEW) * LMUL
#define NUM_REGISTERS 3
#define SUPORTED_INSTRUCTIONS (110 + 1)

#define MEM_PROTECION 0

int PRINTS = 4;
