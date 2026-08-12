#include "instructions_stats.h"
#define true 1
#define false 0

char* get_OP_name(int op){
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

        case 100: return "VMIN_VV     ";
        case 101: return "VMIN_VX     ";
        case 102: return "VMAX_VV     ";
        case 103: return "VMAX_VX     ";
        case 104: return "VMINU_VV    ";
        case 105: return "VMINU_VX    ";
        case 106: return "VMAXU_VV    ";
        case 107: return "VMAXU_VX    ";
        case 108: return "VSRA_VV     ";
        case 109: return "VSRA_VI     ";
        case 110: return "VSRA_VX     ";
        case 111: return "VMACC_VV    ";
        case 112: return "VMACC_VX    ";
        //FP?
        case 511: return "VLUXEI32_V  ";
        case 512: return "VLE32_V     ";
        case 555: return "NOP         ";
    }
    return "ERROR";
};

int is_float_instruction(int op){
    switch (op){
        case VFADD_VF: return true;
        case VFADD_VV: return true;
        case VFDIV_VV: return true;
        case VFDIV_VF: return true;
        case VFSUB_VV: return true;
        case VFSUB_VF: return true;
        case VFMUL_VV: return true;
        case VFMUL_VF: return true;

        case VFSQRT_V : return true;
        case VFMACC_VV: return true;
    }
    return false;
}

int is_merge_instruction(int op){
    switch (op){
        case VMERGE_VVM: return true;
        case VMERGE_VXM: return true;
        case VMERGE_VIM: return true;
    }
    return false;
}

int is_widening_instruction(int op){
    switch (op){
        case VWADD_VV  : return true;
        case VWADD_VX  : return true;
        case VWSUB_VV  : return true;
        case VWSUB_VX  : return true;
        case VWADDU_VV : return true;
        case VWADDU_VX : return true;
        case VWMUL_VV  : return true;
        case VWMUL_VX  : return true;
        case VWMACC_VV : return true;
    }
    return false;
}

int is_mask_instruction(int op){
    switch (op){
        case VMAND_MM  : return true;
        case VMOR_MM   : return true;
        case VMNAND_MM : return true;
        case VMXOR_MM  : return true;
        case VMSEQ_VV  : return true;
        case VMSEQ_VI  : return true;
        case VMSEQ_VX  : return true;
        case VMSNE_VV  : return true;
        case VMSNE_VI  : return true;
        case VMSNE_VX  : return true;
        case VMSLT_VV  : return true;
        case VMSLT_VX  : return true;
        case VMSLE_VV  : return true;
        case VMSLE_VI  : return true;
        case VMSLE_VX  : return true;
        case VMSGT_VI  : return true;
        case VMSGT_VX  : return true;
        case VMFEQ_VV  : return true;
        case VMFNE_VV  : return true;
        case VMFLT_VV  : return true;
        case VMFLE_VV  : return true;
        case VMSLTU_VV : return true;
        case VMSLTU_VX : return true;
        case VMSLEU_VV : return true;
        case VMSLEU_VX : return true;
    }
    return false;
}