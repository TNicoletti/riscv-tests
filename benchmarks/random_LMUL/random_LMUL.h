#include <util.h>
#include "config.h"
#include "add_instruction.h"
#include "mysrand.h"
#include "parameters.h"

int32_t ADDRESS_VECTOR[255];
int SEED = 0x1123456;

/* ===== NORMALS ===== */
#define MAX_N 1024 * 16
volatile intSEW OUT[MAX_N];
volatile intSEW vet_res[NUM_REGISTERS][VLEN / SEW * 8];

int r[NUM_REGISTERS];
int rx[NUM_RANDOM_OPS][3];
int ops[NUM_RANDOM_OPS];

void generate_initial_values();
void generate_RIS(int index);
void random_LMUL(int seed);

int allowed_instructions[SUPORTED_INSTRUCTIONS];
int fixed_suported_instructions = SUPORTED_INSTRUCTIONS;

int fix_op(int x);
void randomize_instructions();


