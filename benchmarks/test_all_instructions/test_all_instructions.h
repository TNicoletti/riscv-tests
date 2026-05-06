#include <util.h>
#include "configs.h"
#include "myutil.h"
#include "asm_functions.h"
#include "mysrand.h"
#include "parameters.h"
#include "add_instruction.h"

#define MAX_REPEAT_INSTRUCTIONS 10
int repeat_instructions = 6;
#define MAX_N EL_PER_BLOCK * MAX_REPEAT_INSTRUCTIONS * SUPORTED_INSTRUCTIONS * NUM_REGISTERS

int allowed_instructions[SUPORTED_INSTRUCTIONS];

/* PARAMETERS */
int SEED = 0;
int sole_execution = -1;
int N = MAX_N;

int32_t ADDRESS_VECTOR[255];

/* ===== EXTERNALS ===== */

extern void clean_vector_scalar(int* v1, int n);
extern void STALL(int cycles);

extern void jump_to_vet(int* vet);

extern int return_t0();

extern void new_trap_handler(void);


/* ===== NORMALS ===== */

volatile int32_t OUT[MAX_N];
volatile int32_t vet_res[32][VLEN / SEW];

/* ===== Registers ===== */
int r[3] = {0, 8, 16};
int rx[1][3] = {{0, 1, 2}};
int ops[1];

/* ===== Register Instruction Strategy(RIS)  ===== */

void generate_RIS(int op, int index);

/* ===== Test Results ===== */
int res[SUPORTED_INSTRUCTIONS][MAX_REPEAT_INSTRUCTIONS];

void eval_results();

/* ===== Tests ===== */
int test_for_ls32();
void single_test(int op);
void all_test();

/* ===== Parameters ===== */
int digest_parameters();
