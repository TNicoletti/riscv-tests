#include <util.h>
#include "config.h"
#include "add_instruction.h"
#include "mysrand.h"
#include "parameters.h"

intSEW ADDRESS_VECTOR1[255];
intSEW ADDRESS_VECTOR2[255];
intSEW ADDRESS_VECTOR4[255];
intSEW ADDRESS_VECTOR8[255];


int SEED = 0x1123456;

/* ===== NORMALS ===== */
#define MAX_N 1024 * 16 * 2
volatile intSEW OUT[MAX_N];
volatile intSEW vet_res1[32][VLEN / SEW];
volatile intSEW vet_res2[32][VLEN / SEW];
volatile intSEW vet_res4[32][VLEN / SEW];
volatile intSEW vet_res8[32][VLEN / SEW];

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


