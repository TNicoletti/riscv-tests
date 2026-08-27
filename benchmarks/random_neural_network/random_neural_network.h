#include <util.h>
#include "configs.h"
#include "mysrand.h"
#include "permute.h"
#include "parameters.h"
#include "add_instruction.h"
#include "issue_finder.h"

#define MAX_N 1024 * 16 * 2

#define NUM_RANDOM_OPS 4
#define QTD_HEURISTICS 8
#define MAX_TESTS_PER_HEURISTIC 24

int N = 8192;
//int SEED = 0x1123456;

int32_t ADDRESS_VECTOR[255];
int allowed_instructions[SUPORTED_INSTRUCTIONS];
int fixed_suported_instructions = SUPORTED_INSTRUCTIONS;

/* ===== EXTERNALS ===== */

extern void clean_vector_scalar(int* v1, int n);
extern void STALL(int cycles);

extern void new_trap_handler(void);

/* ===== NORMALS ===== */

__attribute__((section(".OUT_SECTION"))) volatile intSEW OUT[MAX_N];
volatile intSEW vet_res[32][VLEN / SEW];

int r[NUM_REGISTERS];
int rx[NUM_RANDOM_OPS][3];
int ops[NUM_RANDOM_OPS];

/* ===== Randomized instructions ===== */
int fix_op(int x);
void randomize_instructions();


/*===== RANDOM TEST FUNCTIONS =====*/
int wrong_op[4] = {-1, -1, -1, -1};
void analyze_results(int passed[QTD_HEURISTICS][MAX_TESTS_PER_HEURISTIC], int qtd_tests[QTD_HEURISTICS]);

/*
    Uses some heuristics to uncover the error

    0- Do the same operation 5 times to confirm compulsory error
    If one converge the error is not compulsory

    1- Do the same operation with 5 nops between instructions
    If converges it is probably data hazards or sync problems

    2 - Do the same operations, one by one
    If all converge the problem is with a stream of instructions
    If one diverge the problem is with a specific instruction with given configurations

    3 - Do the same operations, except by removing one of them each iteraction
    If all converge the problem is with the 4 instructions stream
    If one diverge the problem happens with 3 instructions => go to heuristic 3.1

    3.1 - Do the same operations, except by removing two of them each iteraction
    If all converge the problem is with the 3 instructions that diverged with heuristic 3
    If one diverge the problem happens with 2 instructions in sequence

    4 - Do the same operations on vector registers v0, v1 and v2
    If one converge the problem is with the selected registers
    If all diverge the problem is probably not related to specific registers

    5 - Do the same operations in different orders
    If one converge the problem is related to specific operation sequences
    If all diverge the problem might be related to specific instructions

    6 - Do the operations one by one with the same signature multiple times in a row
    Seeks to identify eventually faulty single instructions

    6.1 - Do the operations one by one with differente "signatures"
    Seeks to identify eventually faulty single instructions

    7 - Do the operations:
        7.1 - only the first instruction 
        7.2 - the first and the second instructions 
        7.3 - the first, second and third instructions
        7.4 - all instructions
    
*/
void error_discoverer(int index);

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
void generate_RIS(int index);

void random_test();

void digest_parameters();
int main();
