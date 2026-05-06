#ifndef ADD_INSTRUCTION
#define ADD_INSTRUCTION

#include "float_operator.h"
#include "asm_functions.h"
#include "myutil.h"

#ifndef VLEN
    #define VLEN 128
#endif
#ifndef SEW
    #define SEW 32
#endif
#ifndef LMUL
    #define LMUL 8
#endif
#ifndef NUM_REGISTERS
    #define NUM_REGISTERS 3
#endif

#ifndef EL_PER_BLOCK
    #define EL_PER_BLOCK 128 / 32 * LMUL
#endif
#ifndef PRINTS
    #define PRINTS 0
#endif

int t0_VALUE = 16;
float f_vf = 0.15;

int imm = 7;

int compare_registers = -1;  //
int actual_t0 = 0;

int error_count = 0;
int last_hw_error = 0;

/* ===== EXTERNALS ===== */
extern int* load_OUT_t0_vet(int* address);
extern void load_value_ft0(float f);

extern void jump_to_vet(int* vet);


/* ===== RES ===== */
volatile int32_t scalar_res[32][VLEN / SEW];

/* ===== STORE LOAD VECTOR ===== */
int32_t SL_A_VECTOR[2];

void load_to_vet(int* vet, int reg);
void store_to_vet(int* vet, int reg);
void load_init_values_vector(int* vet, int regs[NUM_REGISTERS]);
void store_vet_values(int r[NUM_REGISTERS], int* vet_res, int el_per_block, int num_registers);
void load_init_values_scalar(int* vet, int r[3]);

int widening_forbid(int rx[3]);
int slideup_forbid(int vd, int vs2, int lmul);
void require_imm_positive();

int add_instruction(int op, int rxa[3], int r[3]);

int compare_solutions(int prev_error, int r[3], int* vet_res);
void execute_RIS(int* vet, int r[NUM_REGISTERS], int32_t address_vector[], int* vet_res);


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
    VLUXEI32_V    = 511,
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

char* get_OP(int op){
    switch(op){
        case  0: return "VADD_VV      ";
        case  1: return "VSUB_VV      ";
        case  2: return "VDIV_VV      ";
        case  3: return "VMUL_VV      ";
        case  4: return "VSLL_VV      ";
        case  5: return "VSRL_VV      ";
        case  6: return "VAND_VV      ";
        case  7: return "VOR_VV       ";
        case  8: return "VXOR_VV      ";
        case  9: return "VADD_VI      ";
        case 10: return "VSLL_VI      ";
        case 11: return "VSRL_VI      ";
        case 12: return "VAND_VI      ";
        case 13: return "VOR_VI       ";
        case 14: return "VXOR_VI      ";
        case 15: return "VADD_VX      ";
        case 16: return "VSUB_VX      ";
        case 17: return "VDIV_VX      ";
        case 18: return "VMUL_VX      ";
        case 19: return "VSLL_VX      ";
        case 20: return "VSRL_VX      ";
        case 21: return "VAND_VX      ";
        case 22: return "VOR_VX       ";
        case 23: return "VXOR_VX      ";
        case 24: return "VFADD_VV     ";
        case 25: return "VFADD_VF     ";
        case 26: return "VFSUB_VV     ";
        case 27: return "VFSUB_VF     ";
        case 28: return "VFMUL_VV     ";
        case 29: return "VFMUL_VF     ";
        case 30: return "VFDIV_VV     ";
        case 31: return "VFDIV_VF     ";
        case 32: return "VFSQRT_V     ";
        case 33: return "VFMACC_VV    ";
        case 34: return "VSLIDEUP_VI  ";
        case 35: return "VSLIDEUP_VX  ";
        case 36: return "VSLIDEDOWN_VI";
        case 37: return "VSLIDEDOWN_VX";
        case 38: return "VMV_V_V      ";
        case 39: return "VMV_V_I      ";
        case 40: return "VMV_V_X      ";
        case 41: return "VWADD_VV     ";
        case 42: return "VWADD_VX     ";
        case 43: return "VWSUB_VV     ";
        case 44: return "VWSUB_VX     ";
        case 45: return "VWADDU_VV    ";
        case 46: return "VWADDU_VX    ";
        case 47: return "VWMUL_VV     ";
        case 48: return "VWMUL_VX     ";
        case 49: return "VWMACC_VV    ";
        case 50: return "VREDSUM_VS   ";
        case 51: return "VREDMAXU_VS  ";  
        case 52: return "VREDMAX_VS   "; 
        case 53: return "VREDMINU_VS  ";  
        case 54: return "VREDMIN_VS   "; 
        case 55: return "VREDAND_VS   "; 
        case 56: return "VREDOR_VS    ";
        case 57: return "VREDXOR_VS   ";
        case 58: return "VMAND_MM     ";
        case 59: return "VMOR_MM      ";
        case 60: return "VMNAND_MM    ";
        case 61: return "VMXOR_MM     ";
        case 62: return "VMSEQ_VV     ";
        case 63: return "VMSEQ_VI     ";
        case 64: return "VMSEQ_VX     ";
        case 65: return "VMSNE_VV     ";
        case 66: return "VMSNE_VI     ";
        case 67: return "VMSNE_VX     ";
        case 68: return "VMSLT_VV     ";
        case 69: return "VMSLT_VX     ";
        case 70: return "VMSLE_VV     ";
        case 71: return "VMSLE_VI     ";
        case 72: return "VMSLE_VX     ";
        case 73: return "VMSGT_VI     ";
        case 74: return "VMSGT_VX     ";
        case 75: return "VCOMPRESS_VM ";
        case 76: return "VCPOP_M      ";
        case 77: return "VFIRST_M     ";
        case 78: return "VMFEQ_VV     ";   
        case 79: return "VMFNE_VV     ";   
        case 80: return "VMFLT_VV     ";   
        case 81: return "VMFLE_VV     ";   
        case 82: return "VMERGE_VVM   ";
        case 83: return "VMERGE_VXM   ";
        case 84: return "VMERGE_VIM   ";

        case 85: return "VMSLTU_VV    ";
        case 86: return "VMSLTU_VX    ";
        case 87: return "VMSLEU_VV    ";
        case 88: return "VMSLEU_VX    ";
        
        case 89: return "VDIVU_VV     ";
        case 90: return "VDIVU_VX     ";

        case 91: return "VREM_VV      ";
        case 92: return "VREM_VX      ";
        case 93: return "VREMU_VV     ";
        case 94: return "VREMU_VX     ";
        
        case 95: return "VMULH_VV     ";
        case 96: return "VMULHU_VV    ";
        case 97: return "VMULHU_VX    ";
        case 98: return "VMULHSU_VV   ";
        case 99: return "VMULHSU_VX   ";

        case 100: return "VMIN_VV      ";
        case 101: return "VMIN_VX      ";
        case 102: return "VMAX_VV      ";
        case 103: return "VMAX_VX      ";
        case 104: return "VMINU_VV     ";
        case 105: return "VMINU_VX     ";
        case 106: return "VMAXU_VV     ";
        case 107: return "VMAXU_VX     ";
        case 108: return "VSRA_VV      ";
        case 109: return "VSRA_VI      ";
        case 110: return "VSRA_VX      ";
        //FP?
        case 511: return "VLUXEI32_V  ";
        case 555: return "NOP         ";
    }
    return "ERROR";    
};

/* ===== ERRORS DEFINITIONS ===== */
void help_errors(){
    printf("Error reference:\n");
    printf("'XX'  : correct answer\n\n");
    printf("'WA' : wrong answer (no hardware errors)\n");
    printf("Hardware errors\n");
    printf("'IM': Instruction Address Misaligned\n");
    printf("'IF': Instruction Access Fault\n");
    printf("'II' : Illegal Instruction / Unimplemented Instruction\n");
    printf("'BP' : Breakpoint (ebreak)\n");
    printf("'LM': Load Address Misaligned\n");
    printf("'LF': Load Access Fault\n");
    printf("'SM': Store/AMO Address Misaligned\n");
    printf("'SF': Store/AMO Access Fault\n");
    printf("'EU': Environment Call from U-mode\n");
    printf("'ES': Environment Call from S-mode\n");
    printf("'EM': Environment Call from M-mode\n");
    printf("'IP': Instruction Page Fault\n");
    printf("'LP': Load Page Fault\n");
    printf("'SP': Store/AMO Page Fault\n");
    printf("'HE' : Hardware Error (implementation specific)\n");
    printf("'UK': Unknown / Reserved\n");
}

char* get_err(int err){
    switch(err){
        case   2: return "XX"; // correct answer
        case   1: return "WA"; // wrong answer
        case   0: return "IM";  // Instruction Address Misaligned
        case  -1: return "IF";  // Instruction Access Fault
        case  -2: return "II";   // Illegal Instruction
        case  -3: return "BP";   // Breakpoint (ebreak)
        case  -4: return "LM";  // Load Address Misaligned
        case  -5: return "LF";  // Load Access Fault
        case  -6: return "SM";  // Store/AMO Address Misaligned
        case  -7: return "SF";  // Store/AMO Access Fault
        case  -8: return "EU";  // Environment Call from U-mode
        case  -9: return "ES";  // Environment Call from S-mode
        case -11: return "EM";  // Environment Call from M-mode
        case -12: return "IP";  // Instruction Page Fault
        case -13: return "LP";  // Load Page Fault
        case -15: return "SP";  // Store/AMO Page Fault
        case -16: return "HE";   // Hardware Error (implementation specific)
        default: return "UK";  // Unknown / Reserved
    }
};

#endif