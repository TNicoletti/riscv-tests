#ifndef ADD_INSTRUCTION
#define ADD_INSTRUCTION

#include "float_operator.h"
#include "asm_functions.h"
#include "myutil.h"
#include "benchmarks.h"
#include "instructions_stats.h"

extern intSEW t0_VALUE;
extern floaSEW f_vf;
extern int imm;
extern int compare_registers;
extern intSEW actual_t1;
extern int error_count;
extern int last_hw_error;

/* ===== EXTERNALS ===== */
extern int* load_OUT_t0_vet(void* address);

extern void jump_to_vet(void* vet);
extern int return_reg(int reg);


/* ===== RES ===== */
extern volatile intSEW scalar_res[32][VLEN / SEW];

/* ===== STORE LOAD VECTOR ===== */
extern INTXLEN SL_A_VECTOR[2];

void load_to_vet(intSEW* vet, int reg);
void store_to_vet(intSEW* vet, int reg);
void load_init_values_vector(intSEW* vet, int* regs, int num_registers);
void store_vet_values(INTXLEN* r, intSEW* vet_res, int num_registers);
void load_init_values_scalar(intSEW* vet, int* r, int num_registers);

int widening_forbid(INTXLEN rx[3]);
int slideup_forbid(int vd, int vs2, int lmul);
void require_imm_positive();

int add_instruction(int op, int rxa[3], int r[3]);

int compare_solutions(int prev_error, int r[3], intSEW* vet_res);
void execute_RIS(intSEW* vet_init, INTXLEN* r, INT_INST address_vector[], intSEW* vet_res, int num_registers);

/* ===== ERRORS DEFINITIONS ===== */
void help_errors();

char* get_err(int err);

#endif