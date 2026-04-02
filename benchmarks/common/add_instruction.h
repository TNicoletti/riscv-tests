#include "float_operator.h"

int t0_VALUE = 16;
float f_vf = 0.15;

int imm = 7;

int compare_registers = -1;  //
int actual_t0 = 0;

int error_count = 0;
int last_hw_error = 0;


volatile int32_t scalar_res[NUM_REGISTERS][EL_PER_BLOCK];
volatile int32_t vet_res[NUM_REGISTERS][EL_PER_BLOCK];

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
        case 511: return "VLUXEI32_V  ";
    }
    return "ERROR";    
}

int add_instruction(int op, int rx[3], int r[3]){
    int instr = 0;
    int instr_type = VV;
    int imm = 7;

    int i = 1;
    switch (op){
        case VADD_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] + scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VADD_VV_INSTR;
            break;
        case VSUB_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d - [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] - scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VSUB_VV_INSTR;
            break;
        case VDIV_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d / [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] / scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VDIV_VV_INSTR;
            break;
        case VMUL_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d * [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] * scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMUL_VV_INSTR;
            break;
        case VSLL_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d << [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] << scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VSLL_VV_INSTR;
            break;
        case VSRL_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d >> [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] >> scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VSRL_VV_INSTR;
            break;
        case VAND_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d & [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] & scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VAND_VV_INSTR;
            break;
        case VOR_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d | [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] | scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VOR_VV_INSTR;
            break;
        case VXOR_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d ^ [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] ^ scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VXOR_VV_INSTR;
            break;
        case VADD_VI:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + imm(%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] + imm);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VADD_VI_INSTR;
            instr_type = VI;
            break;
        case VSLL_VI:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d << imm(%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] << imm);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VSLL_VI_INSTR;
            instr_type = VI;
            break;
        case VSRL_VI:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d >> imm(%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] >> imm);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VSRL_VI_INSTR;
            instr_type = VI;
            break;
        case VAND_VI:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d & imm(%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] & imm);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VAND_VI_INSTR;
            instr_type = VI;
            break;
        case VOR_VI:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d | imm(%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] | imm);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VOR_VI_INSTR;
            instr_type = VI;
            break;
        case VXOR_VI:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d ^ imm(%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] ^ imm);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VXOR_VI_INSTR;
            instr_type = VI;
            break;
        case VADD_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] + t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VADD_VX_INSTR;
            instr_type = VX;
            break;
        case VSUB_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d - %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] - t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VSUB_VX_INSTR;
            instr_type = VX;
            break;
        case VDIV_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d / %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] / t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VDIV_VX_INSTR;
            instr_type = VX;
            break;
        case VMUL_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d * %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] * t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMUL_VX_INSTR;
            instr_type = VX;
            break;
        case VSLL_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d << %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] << t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VSLL_VX_INSTR;
            instr_type = VX;
            break;
        case VSRL_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d >> %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] >> t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VSRL_VX_INSTR;
            instr_type = VX;
            break;
        case VAND_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d & %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] & t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VAND_VX_INSTR;
            instr_type = VX;
            break;
        case VOR_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d | %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] | t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VOR_VX_INSTR;
            instr_type = VX;
            break;
        case VXOR_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d ^ %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j] ^ t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VXOR_VX_INSTR;
            instr_type = VX;
            break;
        case VFADD_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) + bits_to_float(scalar_res[rx[2]][j]));
            }
            if(PRINTS >= 2) printf("\n");
            instr = VFADD_VV_INSTR;
            break;
        case VFADD_VF:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + 0.15;\n", rx[0], j, rx[1], scalar_res[rx[1]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) + f_vf);
            }
            instr = VFADD_VF_INSTR;
            instr_type = VF;
            break;
        case VFSUB_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d - [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) - bits_to_float(scalar_res[rx[2]][j]));
            }
            instr = VFSUB_VV_INSTR;
            break;

        case VFSUB_VF:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d - 0.15;\n", rx[0], j, rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) - f_vf);
            }
            instr = VFSUB_VF_INSTR;
            instr_type = VF;
            break;

        case VFMUL_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d * [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) * bits_to_float(scalar_res[rx[2]][j]));
            }
            instr = VFMUL_VV_INSTR;
            break;

        case VFMUL_VF:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d * 0.15;\n", rx[0], j, rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) * f_vf);
            }
            instr = VFMUL_VF_INSTR;
            instr_type = VF;
            break;

        case VFDIV_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d / [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) / bits_to_float(scalar_res[rx[2]][j]));
            }
            instr = VFDIV_VV_INSTR;
            break;

        case VFDIV_VF:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d / 0.15;\n", rx[0], j, rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) / (double)f_vf);
            }
            instr = VFDIV_VF_INSTR;
            instr_type = VF;
            break;

        case VFSQRT_V:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = sqrt([%d]%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j]);
                // Note que sqrt() precisa da math.h
                scalar_res[rx[0]][j] = float_to_bits(sqrtf(bits_to_float(scalar_res[rx[1]][j])));
            }
            instr = VFSQRT_V_INSTR;
            instr_type = V;
            break;
        case VFMACC_VV: // TODO: fix very big numbers issue
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d * [%d]%d) + [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j], rx[0], scalar_res[rx[0]][j]);
                // Note que sqrt() precisa da math.h
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) * bits_to_float(scalar_res[rx[2]][j]) + bits_to_float(scalar_res[rx[0]][j]));
            }
            instr = VFMACC_VV_INSTR;
            break;
        case VSLIDEUP_VI:
            imm = 2;
            for(int j = EL_PER_BLOCK - 1; j >= imm; j--){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j - imm]);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j - imm]);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VSLIDEUP_VI_INSTR;
            instr_type = VI;
            break;
        case VSLIDEUP_VX:
            for(int j = EL_PER_BLOCK - 1; j >= t0_VALUE; j--){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j - t0_VALUE]);
                scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j - t0_VALUE]);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VSLIDEUP_VX_INSTR;
            instr_type = VX;
            break;
        case VSLIDEDOWN_VI:
            imm = 2;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(j >= EL_PER_BLOCK - imm){
                    if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = 0;\n", rx[0], j);
                    scalar_res[rx[0]][j] = 0;
                }else{
                    if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j + imm]);
                    scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j + imm]);
                }
            }
            if(PRINTS >= 2) printf("\n");
            instr = VSLIDEDOWN_VI_INSTR;
            instr_type = VI;
            break;
        case VSLIDEDOWN_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(j >= EL_PER_BLOCK - imm){
                    if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = 0;\n", rx[0], j);
                    scalar_res[rx[0]][j] = 0;
                }else{
                    if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j + t0_VALUE]);
                    scalar_res[rx[0]][j] = (int32_t)(scalar_res[rx[1]][j + t0_VALUE]);
                }
            }
            if(PRINTS >= 2) printf("\n");
            instr = VSLIDEDOWN_VX_INSTR;
            instr_type = VX;
            break;
        case VMV_V_V:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = %d;\n", rx[0], j, scalar_res[rx[1]][j]);
                scalar_res[rx[0]][j] = scalar_res[rx[1]][j];
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMV_V_V_INSTR;
            instr = change_vet_rd(instr, r[rx[0]]);
            instr = change_vet_rs1(instr, 0);
            instr = change_vet_rs2(instr, r[rx[1]]);
            instr_type = NO_TYPE;
            break;
        case VMV_V_I:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = %d;\n", rx[0], j, imm);
                scalar_res[rx[0]][j] = imm;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMV_V_I_INSTR;

            instr = change_vet_rd(instr, r[rx[0]]);
            instr = change_vet_rs1(instr, 0);
            instr = change_vet_rs2(instr, imm);

            instr_type = NO_TYPE;
            break;
        case VMV_V_X:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = %d;\n", rx[0], j, t0_VALUE);
                scalar_res[rx[0]][j] = t0_VALUE;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMV_V_X_INSTR;
            instr = change_vet_rd(instr, r[rx[0]]);
            instr = change_vet_rs1(instr, 0);
            instr = change_vet_rs2(instr, 5);
            instr_type = NO_TYPE;
            break;
        case VWADD_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                int64_t res = (int64_t)((int64_t)scalar_res[rx[1]][j] + (int64_t)scalar_res[rx[2]][j]);
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VWADD_VV_INSTR;
            break;
        case VWADD_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                int64_t res = (int64_t)((int64_t)scalar_res[rx[1]][j] + (int64_t)t0_VALUE);
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VWADD_VX_INSTR;
            instr_type = VX;
            break;
        case VWSUB_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d - [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                int64_t res = (int64_t)((int64_t)scalar_res[rx[1]][j] - (int64_t)scalar_res[rx[2]][j]);
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VWSUB_VV_INSTR;
            break;
        case VWSUB_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d - %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                int64_t res = (int64_t)((int64_t)scalar_res[rx[1]][j] - (int64_t)t0_VALUE);
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VWSUB_VX_INSTR;
            instr_type = VX;
            break;
        case VWADDU_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                uint64_t res = (uint64_t)((uint64_t)scalar_res[rx[1]][j] + (uint64_t)scalar_res[rx[2]][j]);
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VWADDU_VV_INSTR;
            break;
        case VWADDU_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                uint64_t res = (uint64_t)((uint64_t)scalar_res[rx[1]][j] + (uint64_t)t0_VALUE);
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VWADDU_VX_INSTR;
            instr_type = VX;
            break;
        case VWMUL_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d * [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                int64_t res = (int64_t)((int64_t)scalar_res[rx[1]][j] * (int64_t)scalar_res[rx[2]][j]);
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VWMUL_VV_INSTR;
            break;
        case VWMUL_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d * %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                uint64_t res = (uint64_t)((uint64_t)scalar_res[rx[1]][j] * (uint64_t)t0_VALUE);
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VWMUL_VX_INSTR;
            instr_type = VX;
            break;
        case VWMACC_VV:
            int64_t res[EL_PER_BLOCK];
            for(int j = 0; j < EL_PER_BLOCK; j++) {
                int64_t product = (int64_t)scalar_res[rx[1]][j] * (int64_t)scalar_res[rx[2]][j];

                // 2. Reconstruct the EXISTING 64-bit accumulator from the dest registers
                int row_idx = rx[j / (EL_PER_BLOCK / 2)];
                int col_idx = (2 * j) % EL_PER_BLOCK;

                uint32_t lo = (uint32_t)scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK];
                uint32_t hi = (uint32_t)scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK + 1];                
                int64_t existing_acc = (int64_t)(((uint64_t)hi << 32) | lo);

                // 3. Accumulate
                res[j] = product + existing_acc;
            }
            for(int j = 0; j < EL_PER_BLOCK; j++){

                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK]     = \
                res[j] & 0xFFFFFFFF;
                scalar_res[rx[j / (EL_PER_BLOCK / 2)]][(2 * j) % EL_PER_BLOCK + 1 ] = \
                (res[j] >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            instr = VWMACC_VV_INSTR;
            break;
        /*case VLUXEI32_V:
            // VLUXEI32.V: Indexed load (unordered). vd, (rs1), vs2.
            // Maps rx[1] -> rs1 (base address) and rx[2] -> vs2 (vector of indices).
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = MEM[rs1 + [%d]%d];\n", rx[0], j, rx[2], scalar_res[rx[2]][j]);
                
                scalar_res[rx[0]][j] = 0; 
            }
            if(PRINTS >= 2) printf("\n");
            instr = VLUXEI32_V_INSTR;
            instr_type = V; 
            break;*/

        case VREDSUM_VS:
            // VREDSUM.VS: Vector reduction sum. vd[0] = sum(vs2[0..VL-1]) + vs1[0]
            // Maps rx[1] -> vs1 (scalar accumulator) and rx[2] -> vs2 (vector to reduce).
            int32_t sum = scalar_res[rx[2]][0];
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = [%d]%d", rx[0], rx[1], scalar_res[rx[1]][0]);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf(" + [%d]%d", rx[2], scalar_res[rx[2]][j]);
                sum += scalar_res[rx[1]][j];
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = (int32_t)sum;

            instr = VREDSUM_VS_INSTR;
            instr_type = VV; 
            break;
        case VREDMAXU_VS:
            // VREDSUM.VS: Vector reduction sum. vd[0] = sum(vs2[0..VL-1]) + vs1[0]
            // Maps rx[1] -> vs1 (scalar accumulator) and rx[2] -> vs2 (vector to reduce).
            uint32_t mx = scalar_res[rx[2]][0];
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = MAX [%d]%d", rx[0], rx[2], scalar_res[rx[2]][0]);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("; [%d]%d", rx[1], scalar_res[rx[1]][j]);
                mx = max(mx, scalar_res[rx[1]][j]);
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = (uint32_t)mx;

            instr = VREDMAXU_VS_INSTR;
            instr_type = VV; 
            break;

        case VREDMAX_VS:
            int32_t mx_s = scalar_res[rx[2]][0]; // vs1[0] is the scalar accumulator
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = MAX_S [%d]%d", rx[0], rx[2], mx_s);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                int32_t val = (int32_t)scalar_res[rx[1]][j];
                if(PRINTS >= 2) printf("; [%d]%d", rx[1], val);
                mx_s = max(mx_s, val);
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = (int32_t)mx_s;
            instr = VREDMAX_VS_INSTR;
            instr_type = VV; 
            break;

        case VREDMINU_VS:
            // VREDMINU.VS: Vector reduction minimum (unsigned)
            uint32_t mn_u = (uint32_t)scalar_res[rx[2]][0];
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = MIN_U [%d]%u", rx[0], rx[2], mn_u);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                uint32_t val = (uint32_t)scalar_res[rx[1]][j];
                if(PRINTS >= 2) printf("; [%d]%u", rx[1], val);
                if(val < mn_u) mn_u = val;
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = (uint32_t)mn_u;
            instr = VREDMINU_VS_INSTR;
            instr_type = VV; 
            break;

        case VREDMIN_VS:
            // VREDMIN.VS: Vector reduction minimum (signed)
            int32_t mn_s = (int32_t)scalar_res[rx[2]][0];
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = MIN_S [%d]%d", rx[0], rx[2], mn_s);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                int32_t val = (int32_t)scalar_res[rx[1]][j];
                if(PRINTS >= 2) printf("; [%d]%d", rx[1], val);
                if(val < mn_s) mn_s = val;
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = (int32_t)mn_s;
            instr = VREDMIN_VS_INSTR;
            instr_type = VV; 
            break;

        case VREDAND_VS:
            // VREDAND.VS: Vector reduction bitwise AND
            uint32_t and_res = (uint32_t)scalar_res[rx[2]][0];
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = AND [%d]%x", rx[0], rx[2], and_res);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                uint32_t val = (uint32_t)scalar_res[rx[1]][j];
                if(PRINTS >= 2) printf(" & [%d]%x", rx[1], val);
                and_res &= val;
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = and_res;
            instr = VREDAND_VS_INSTR;
            instr_type = VV; 
            break;

        case VREDOR_VS:
            // VREDOR.VS: Vector reduction bitwise OR
            uint32_t or_res = (uint32_t)scalar_res[rx[2]][0];
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = OR [%d]%x", rx[0], rx[2], or_res);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                uint32_t val = (uint32_t)scalar_res[rx[1]][j];
                if(PRINTS >= 2) printf(" | [%d]%x", rx[1], val);
                or_res |= val;
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = or_res;
            instr = VREDOR_VS_INSTR;
            instr_type = VV; 
            break;

        case VREDXOR_VS:
            // VREDXOR.VS: Vector reduction bitwise XOR
            uint32_t xor_res = (uint32_t)scalar_res[rx[2]][0];
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = XOR [%d]%x", rx[0], rx[2], xor_res);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                uint32_t val = (uint32_t)scalar_res[rx[1]][j];
                if(PRINTS >= 2) printf(" ^ [%d]%x", rx[1], val);
                xor_res ^= val;
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = xor_res;
            instr = VREDXOR_VS_INSTR;
            instr_type = VV;
            break;
        case VMAND_MM:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d & [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                scalar_res[rx[0]][j / SEW] &= (0xFFFFFFFF ^ i);
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][0] & i) & (scalar_res[rx[2]][0] & i);
                if(i == 1 << SEW - 1)
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMAND_MM_INSTR;
            break;

        case VMOR_MM:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d | [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                scalar_res[rx[0]][j / SEW] &= (0xFFFFFFFF ^ i);
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][0] & i) | (scalar_res[rx[2]][0] & i);
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMOR_MM_INSTR;
            break;

        case VMNAND_MM:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d NAND [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (~(scalar_res[rx[1]][j / SEW] & scalar_res[rx[2]][j / SEW])) & i;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMNAND_MM_INSTR;
            break;

        case VMXOR_MM:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d ^ [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= ((scalar_res[rx[1]][j / SEW] ^ scalar_res[rx[2]][j / SEW])) & i;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMXOR_MM_INSTR;
            break;

        case VMSEQ_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d == [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][j] == scalar_res[rx[2]][j])?i:0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMSEQ_VV_INSTR;
            break;

        case VMSEQ_VI:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d == imm(%d)) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][j] == imm)?i:0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMSEQ_VI_INSTR;
            instr_type = VI;
            break;

        case VMSEQ_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d == %d) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][j] == t0_VALUE)?i:0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMSEQ_VX_INSTR;
            instr_type = VX;
            break;

        case VMSNE_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d != [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][j] != scalar_res[rx[2]][j])?i:0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMSNE_VV_INSTR;
            break;

        case VMSNE_VI:
           for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d != imm(%d)) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][j] != imm)?i:0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMSNE_VI_INSTR;
            instr_type = VI;
            break;

        case VMSNE_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d != %d) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][j] != t0_VALUE)?i:0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMSNE_VX_INSTR;
            instr_type = VX;
            break;

        case VMSLT_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d < [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][j] < scalar_res[rx[2]][j])?i:0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMSLT_VV_INSTR;
            break;
        case VMSLT_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d < %d) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][j] < t0_VALUE)?i:0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMSLT_VX_INSTR;
            instr_type = VX;
            break;

        case VMSLE_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d <= [%d]%d) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][j] <= scalar_res[rx[2]][j])?i:0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMSLE_VV_INSTR;
            break;

        case VMSLE_VI:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d <= %d) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][j] <= t0_VALUE)?i:0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMSLE_VI_INSTR;
            instr_type = VI;
            break;

        case VMSLE_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d < %d) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][j] < imm)?i:0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMSLE_VX_INSTR;
            instr_type = VX;
            break;
        case VMSGT_VI:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d > %d) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][j] > t0_VALUE)?i:0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMSGT_VI_INSTR;
            instr_type = VI;
            break;

        case VMSGT_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d > %d) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (scalar_res[rx[1]][j] > imm)?i:0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMSGT_VX_INSTR;
            instr_type = VX;
            break;

        case VCOMPRESS_VM: {
            // Compressão baseada na máscara vs1 (rx[2]), filtrando elementos de vs2 (rx[1])
            int compress_idx = 0;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if((scalar_res[rx[2]][j / SEW] & i) != 0){ // Se o bit da máscara está ativo
                    scalar_res[rx[0]][compress_idx] = scalar_res[rx[1]][j];
                    compress_idx++;
                }
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VCOMPRESS_VM_INSTR;
            break;
        }

        case VCPOP_M: {
            // Conta bits setados na máscara (population count). Resultado é um escalar em vd.
            int popcount = 0;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if((scalar_res[rx[1]][j / SEW] & i) != 0){ 
                    popcount++;
                }
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("SCALAR_RESULT:t0 = POPCOUNT([%d]) = %d;\n\n", rx[0], popcount);
            compare_registers = popcount;
            instr = VCPOP_M_INSTR;
            instr = change_vet_rd(instr, 5);
            instr = change_vet_rs1(instr, r[rx[1]]);    
            instr_type = NO_TYPE;
            break;
        }

        case VFIRST_M: {
            // Encontra o primeiro bit setado na máscara. Retorna índice, ou -1 se não encontrar.
            int first_idx = -1;
            for(int j = 0; j < VLEN; j++){
                if((scalar_res[rx[1]][j / SEW] & i) != 0){
                    first_idx = j;
                    break;
                }
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("SCALAR_RESULT:t0 = VFIRST([%d]) = %d;\n\n", rx[1], first_idx);
            instr = VFIRST_M_INSTR;
            compare_registers = first_idx;
            instr = change_vet_rd(instr, 5);
            instr = change_vet_rs1(instr, r[rx[1]]);    
            instr_type = NO_TYPE;
            break;
        }
        case VMFEQ_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                float f1 = bits_to_float(scalar_res[rx[1]][j]);
                float f2 = bits_to_float(scalar_res[rx[2]][j]);
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = (%d == %d) ? 1 : 0;\n", rx[0], j / SEW, f1, f2);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (f1 == f2) ? i : 0;

                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMFEQ_VV_INSTR;
            break;

        case VMFNE_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                float f1 = bits_to_float(scalar_res[rx[1]][j]);
                float f2 = bits_to_float(scalar_res[rx[2]][j]);
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = (%f != %f) ? 1 : 0;\n", rx[0], j / SEW, f1, f2);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (f1 != f2) ? i : 0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMFNE_VV_INSTR;
            break;

        case VMFLT_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                float f1 = bits_to_float(scalar_res[rx[1]][j]);
                float f2 = bits_to_float(scalar_res[rx[2]][j]);
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = (%f < %f) ? 1 : 0;\n", rx[0], j / SEW, f1, f2);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (f1 < f2) ? i : 0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMFLT_VV_INSTR;
            break;

        case VMFLE_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                float f1 = bits_to_float(scalar_res[rx[1]][j]);
                float f2 = bits_to_float(scalar_res[rx[2]][j]);
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = (%d <= %d) ? 1 : 0;\n", rx[0], j / SEW, f1, f2);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (f1 <= f2) ? i : 0;
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMFLE_VV_INSTR;
            break;

       

        case VMERGE_VVM:
            // vmerge utiliza a máscara de v0. Assumindo que v0 está mapeado em scalar_res[0]
            for(int j = 0; j < EL_PER_BLOCK; j++){
                int mask_active = (scalar_res[0][j / SEW] & i) == 0;

                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = mask(%d) ? [%d]%d : [%d]%d;\n", rx[0], j, mask_active, rx[2], scalar_res[rx[2]][j], rx[1], scalar_res[rx[1]][j]);
                
                scalar_res[1][j] = mask_active ? scalar_res[2][j]: 0/* MASK 3 */;
                
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMERGE_VVM_INSTR;
            instr = change_vet_rd(instr,  1);
            instr = change_vet_rs1(instr, 2);
            instr = change_vet_rs2(instr, 3);
            instr_type = NO_TYPE;
            break;

        case VMERGE_VXM:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                int mask_active = (scalar_res[0][j / SEW] & i) == 0;

                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = mask(%d) ? [%d]%d : %d;\n", rx[0], j, mask_active, rx[2], scalar_res[rx[2]][j], t0_VALUE);
                
                scalar_res[1][j] = mask_active ? scalar_res[2][j]: t0_VALUE;
                
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMERGE_VXM_INSTR;
            instr = change_vet_rd(instr,  1);
            instr = change_vet_rs1(instr, 2);
            instr = change_vet_rs2(instr, 5);
            instr_type = NO_TYPE;
            break;

        case VMERGE_VIM:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                int mask_active = (scalar_res[0][j / SEW] & i) == 0;

                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = mask(%d) ? [%d]%d : %d;\n", rx[0], j, mask_active, rx[2], scalar_res[rx[2]][j], imm);
                
                scalar_res[1][j] = mask_active ? scalar_res[2][j]: imm/* MASK 3 */;
                
                if(i == (1 << SEW - 1))
                    i = 1;
                else
                    i = i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            instr = VMERGE_VIM_INSTR;
            instr = change_vet_rd(instr,  1);
            instr = change_vet_rs1(instr, 2);
            instr = change_vet_rs2(instr, imm);
            instr_type = NO_TYPE;
            break;
        case NOP:
            return ADDI_ZZZ_INSTR;
        default:
            break;
    }
    if(instr_type == VV){
        instr = change_vet_rd(instr,  r[rx[0]]);
        instr = change_vet_rs1(instr, r[rx[1]]);
        instr = change_vet_rs2(instr, r[rx[2]]);
    }
    if(instr_type == VX){
        instr = change_vet_rd(instr, r[rx[0]]);
        instr = change_vet_rs1(instr, r[rx[1]]);
        instr = change_vet_rs2(instr, 5);
    }
    if(instr_type == VI){
        instr = change_vet_rd(instr, r[rx[0]]);
        
        instr = change_vet_rs1(instr, r[rx[1]]);
        
        instr = change_vet_rs2(instr, imm);
    }
    if(instr_type == VF){
        instr = change_vet_rd(instr, r[rx[0]]);
        
        instr = change_vet_rs1(instr, r[rx[1]]);
        
        instr = change_vet_rs2(instr, 0); 
        load_value_ft0(f_vf);
    }
    if(instr_type == V){
        instr = change_vet_rd(instr, r[rx[0]]);
        instr = change_vet_rs1(instr, r[rx[1]]);
    }
    return instr;
}

int compare_solutions(int prev_error){
    int cmr = compare_registers;
    compare_registers = -1;

    if(cmr != -1){
        if(actual_t0 == cmr){
            if(PRINTS >= 1)printf("Convergence \n");
            if(PRINTS >= 2)printf("registers compared\b");
            return 2;
        }else{
            if(PRINTS >= 1)printf("Divergence\n");
            if(PRINTS >= 2)printf("t0: %d\n", actual_t0);
            return 1;
        }
    }
    else{
        if(manual_convergence(&scalar_res[0][0], &vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK)){
            if(PRINTS >= 1)printf("Convergence \n");
            return 2;
        }else{
            if(PRINTS >= 1)printf("Divergence\n");
            return 1;
        }
    }
}