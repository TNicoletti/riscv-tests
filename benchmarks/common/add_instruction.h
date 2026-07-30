#ifndef ADD_INSTRUCTION
#define ADD_INSTRUCTION

#include "float_operator.h"
#include "asm_functions.h"
#include "myutil.h"
#include "benchmarks.h"

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


/* ===== ENUMS ===== */
enum VEC_INSTRUCTIONS{
    VADD_VV       = 0,
    VSUB_VV       = 1,
    VDIV_VV       = 2,
    VMUL_VV       = 3,
    VSLL_VV       = 4,
    VSRL_VV       = 5,
    VAND_VV       = 6,
    VOR_VV        = 7,
    VXOR_VV       = 8,
    VADD_VI       = 9,
    VSLL_VI       = 10,
    VSRL_VI       = 11,
    VAND_VI       = 12,
    VOR_VI        = 13,
    VXOR_VI       = 14,
    VADD_VX       = 15,
    VSUB_VX       = 16,
    VDIV_VX       = 17,
    VMUL_VX       = 18,
    VSLL_VX       = 19,
    VSRL_VX       = 20,
    VAND_VX       = 21,
    VOR_VX        = 22,
    VXOR_VX       = 23,
    VFADD_VV      = 24,
    VFADD_VF      = 25,
    VFSUB_VV      = 26,
    VFSUB_VF      = 27,
    VFMUL_VV      = 28,
    VFMUL_VF      = 29,
    VFDIV_VV      = 30,
    VFDIV_VF      = 31,
    VFSQRT_V      = 32,
    VFMACC_VV     = 33,
    VSLIDEUP_VI   = 34,
    VSLIDEUP_VX   = 35,
    VSLIDEDOWN_VI = 36,
    VSLIDEDOWN_VX = 37,
    VMV_V_V       = 38,
    VMV_V_I       = 39,
    VMV_V_X       = 40,
    VWADD_VV      = 41,
    VWADD_VX      = 42,
    VWSUB_VV      = 43,
    VWSUB_VX      = 44,
    VWADDU_VV     = 45,
    VWADDU_VX     = 46,
    VWMUL_VV      = 47,
    VWMUL_VX      = 48,
    VWMACC_VV     = 49,
    VREDSUM_VS    = 50,
    VREDMAXU_VS   = 51,  
    VREDMAX_VS    = 52, 
    VREDMINU_VS   = 53,  
    VREDMIN_VS    = 54, 
    VREDAND_VS    = 55, 
    VREDOR_VS     = 56,
    VREDXOR_VS    = 57,
    VMAND_MM      = 58,
    VMOR_MM       = 59,
    VMNAND_MM     = 60,
    VMXOR_MM      = 61,
    VMSEQ_VV      = 62,
    VMSEQ_VI      = 63,
    VMSEQ_VX      = 64,
    VMSNE_VV      = 65,
    VMSNE_VI      = 66,
    VMSNE_VX      = 67,
    VMSLT_VV      = 68,
    VMSLT_VX      = 69,
    VMSLE_VV      = 70,
    VMSLE_VI      = 71,
    VMSLE_VX      = 72,
    VMSGT_VI      = 73,
    VMSGT_VX      = 74,
    VCOMPRESS_VM  = 75,
    VCPOP_M       = 76,
    VFIRST_M      = 77,
    VMFEQ_VV      = 78,     
    VMFNE_VV      = 79,     
    VMFLT_VV      = 80,     
    VMFLE_VV      = 81,     
    VMERGE_VVM    = 82,
    VMERGE_VXM    = 83,
    VMERGE_VIM    = 84,
    VMSLTU_VV     = 85,
    VMSLTU_VX     = 86,
    VMSLEU_VV     = 87,
    VMSLEU_VX     = 88,
    VDIVU_VV      = 89,
    VDIVU_VX      = 90,
    VREM_VV       = 91,
    VREM_VX       = 92,
    VREMU_VV      = 93,
    VREMU_VX      = 94,
    VMULH_VV      = 95,
    VMULHU_VV     = 96,
    VMULHU_VX     = 97,
    VMULHSU_VV    = 98,
    VMULHSU_VX    = 99,
    VMIN_VV      = 100,
    VMIN_VX      = 101,
    VMAX_VV      = 102,
    VMAX_VX      = 103,
    VMINU_VV     = 104,
    VMINU_VX     = 105,
    VMAXU_VV     = 106,
    VMAXU_VX     = 107,
    VSRA_VV      = 108,
    VSRA_VI      = 109,
    VSRA_VX      = 110,
    VMACC_VV     = 111,
    VLUXEI32_V   = 511,
    VLE32_V      = 512,
    VSE32_V      = 513,
    NOP = 555
};

enum INSTR_TYPES{
    VV = 0,
    VI = 1,
    VX = 2,
    VF = 3,
    V  = 4,
    VM = 5,
    NO_TYPE = 155
};

char* get_OP(int op);

/* ===== ERRORS DEFINITIONS ===== */
void help_errors();

char* get_err(int err);

#endif