#include <util.h>
#include "configs.h"
#include "mysrand.h"
#include "parameters.h"
#include "add_instruction.h"

#define MAX_REPEAT_INSTRUCTIONS 16
int repeat_instructions = 6;
#define MAX_N MAX_EL_PER_BLOCK * MAX_REPEAT_INSTRUCTIONS * SUPORTED_INSTRUCTIONS * NUM_REGISTERS

int allowed_instructions[SUPORTED_INSTRUCTIONS];

int correct_instructions = 0;
int wrong_instructions   = 0;

/* PARAMETERS */
int SEED = 0;
int sole_execution = -1;
int N = MAX_N;

int32_t ADDRESS_VECTOR[255];

/* ===== EXTERNALS ===== */

extern void clean_vector_scalar(int* v1, int n);
extern void STALL(int cycles);

extern void jump_to_vet(void* vet);

extern void new_trap_handler(void);


/* ===== NORMALS ===== */

volatile intSEW OUT[MAX_N];
volatile intSEW vet_res[32][VLEN / SEW];

/* ===== Registers ===== */
int r[3] = {0, 8, 16};
int rx[1][3] = {{0, 1, 2}};
int ops[1];

/* ===== Register Instruction Strategy(RIS)  ===== */

/*
r1 = OUT[index + 0]
r2 = OUT[index + 1 * EL_PER_BLOCK]
r3 = OUT[index + 2 * EL_PER_BLOCK]

rx1 = rx2 op1 rx3
rx4 = rx5 op1 rx6
rx7 = rx8 op1 rx9
rx10 = rx11 op12 rx13

r1 = OUT[index + 0]
r2 = OUT[index + 1 * EL_PER_BLOCK]
r3 = OUT[index + 2 * EL_PER_BLOCK]

1 - picks 3 different registers
2 - picks 4 operations
3 - picks
*/
int generate_RIS(int op, int index);

/* ===== Test Results ===== */
int res[SUPORTED_INSTRUCTIONS][MAX_REPEAT_INSTRUCTIONS];

void eval_results();

/* ===== Tests ===== */
int test_for_ls32();
void single_test(int op);
void all_test();

/* ===== Parameters ===== */
int digest_parameters();
