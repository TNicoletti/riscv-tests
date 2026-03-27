#include <util.h>
#include "configs.h"
#include "myutil.h"
#include "asm_functions.h"
#include "parameters.h"
#include "mysrand.h"
#include "float_operator.h"

#define MAX_N 4096

#define REPEAT_INSTRUCTIONS 4

int N = 4096;
int error_count = 0;
int last_hw_error = 0;

int32_t ADDRESS_VECTOR[255];

/* ===== EXTERNALS ===== */

extern void clean_vector_scalar(int* v1, int n);
extern void STALL(int cycles);

extern void jump_to_vet(int* vet);

extern int* load_OUT_t0_vet(int* address);
extern void load_value_ft0(float f);

#define t0_VALUE 16

extern void new_trap_handler(void);


/* ===== NORMALS ===== */

volatile int A[MAX_N];
volatile int B[MAX_N];
volatile int32_t OUT[MAX_N];
volatile int32_t scalar_res[NUM_REGISTERS][EL_PER_BLOCK];
volatile int32_t vet_res[NUM_REGISTERS][EL_PER_BLOCK];


/* ===== RANDOMIZERS ===== */
void generate_initial_values(){
    msrand(11234);
    for (int i = 0; i < N; i++) {
        A[i] = mrand() % 2147000000;
        B[i] = mrand() % 2147000000;
        OUT[i] = mrand() % 2147000000;
    }
}

int r[3] = {0, 1, 2};
int rx[1][3] = {{0, 1, 2}};
int ops[1];

int get_random_reg(){
    #if LMUL == 1
        return mrand() % 32;
    #elif LMUL == 2
        return mrand() % 16 * 2;
    #elif LMUL == 4
        return mrand() % 8 * 4;
    #elif LMUL == 8
        return mrand() % 4 * 8;
    #endif
}

int is_register_different_from_previous(int* r, int idx){
    for(int i = 0; i < idx; i++)
        if(r[i] == r[idx])
            return false;
    return true;
}

void shuffle_registers(int r[NUM_REGISTERS]){
    for(int i = 0; i < NUM_REGISTERS; i++){
        do
        r[i] = get_random_reg();
        while(!is_register_different_from_previous(&r[0], i));
    }
    if(PRINTS >= 2){
        printf("Register choice:\n");
        for(int i = 0; i < NUM_REGISTERS; i++)
            printf("r[%d]= %d;", i, r[i]);
        printf("\n\n");

    }
}

/* ===== VECTOR LOADERS =====*/


int32_t SL_A_VECTOR[2];
void load_to_vet(int* vet, int reg){
    SL_A_VECTOR[1] = RET_INSTR;
    int32_t instr = VLE32_V_INSTR;
    instr = change_vet_rd(instr, reg);
    load_OUT_t0_vet(vet);
    SL_A_VECTOR[0] = instr;
    jump_to_vet(&SL_A_VECTOR[0]);
}

void store_to_vet(int* vet, int reg){
    SL_A_VECTOR[1] = RET_INSTR;
    int32_t instr = VSE32_V_INSTR;
    instr = change_vet_rd(instr, reg);
    load_OUT_t0_vet(vet);
    SL_A_VECTOR[0] = instr;
    jump_to_vet(&SL_A_VECTOR[0]);
}

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
        case  0: return "VADD_VV";
        case  1: return "VSUB_VV";
        case  2: return "VDIV_VV";
        case  3: return "VMUL_VV";
        case  4: return "VSLL_VV";
        case  5: return "VSRL_VV";
        case  6: return "VAND_VV";
        case  7: return "VOR_VV";
        case  8: return "VXOR_VV";
        case  9: return "VADD_VI";
        case 10: return "VSLL_VI";
        case 11: return "VSRL_VI";
        case 12: return "VAND_VI";
        case 13: return "VOR_VI";
        case 14: return "VXOR_VI";
        case 15: return "VADD_VX";
        case 16: return "VSUB_VX";
        case 17: return "VDIV_VX";
        case 18: return "VMUL_VX";
        case 19: return "VSLL_VX";
        case 20: return "VSRL_VX";
        case 21: return "VAND_VX";
        case 22: return "VOR_VX";
        case 23: return "VXOR_VX";
        case 24: return "VFADD_VV";
        case 25: return "VFADD_VF";
        case 26: return "VFSUB_VV";
        case 27: return "VFSUB_VF";
        case 28: return "VFMUL_VV";
        case 29: return "VFMUL_VF";
        case 30: return "VFDIV_VV";
        case 31: return "VFDIV_VF";
        case 32: return "VFSQRT_V";
        case 33: return "VFMACC_VV";
        case 34: return "VSLIDEUP_VI";
        case 35: return "VSLIDEUP_VX";
        case 36: return "VSLIDEDOWN_VI";
        case 37: return "VSLIDEDOWN_VX";
        case 38: return "VMV_V_V";
        case 39: return "VMV_V_I";
        case 40: return "VMV_V_X";
    }
    return "ERROR";    
}

void execute_RIS(int* vet, int r[NUM_REGISTERS]){
    set_vet_settings();
    load_init_values_vector(vet, r);
    set_vet_settings();
    load_OUT_t0_vet((int*)t0_VALUE);
    jump_to_vet(&ADDRESS_VECTOR[0]);
    store_vet_values(r);
}

int add_instruction(int op, int rx[3], int r[3]){
    int instr = 0;
    int instr_type = VV;
    int imm = 7;
    float f_vf = 0.15;
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
        //load_OUT_t0_vet((int*)t0_VALUE);
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

void load_init_values_scalar(int* vet){
    for(int i = 0; i < EL_PER_BLOCK; i++)
    {
        scalar_res[0][i] = vet[i];
        scalar_res[1][i] = vet[EL_PER_BLOCK + i];
        scalar_res[2][i] = vet[2 * EL_PER_BLOCK + i];
    }
}

void load_init_values_vector(int* vet, int regs[NUM_REGISTERS]){    
    set_vet_settings();
    for(int i = 0; i < NUM_REGISTERS; i++)
        load_to_vet(&vet[i * EL_PER_BLOCK], regs[i]);
}

void store_vet_values(int r[NUM_REGISTERS]){
    clean_vector_scalar(&vet_res[0][0], EL_PER_BLOCK * NUM_REGISTERS);

    set_vet_settings();
    for(int i = 0; i < NUM_REGISTERS; i++)
        store_to_vet(&vet_res[i][0], r[i]);
    
}

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
void generate_RIS(int op, int index){
    load_init_values_scalar(&OUT[index]);

    ADDRESS_VECTOR[0] = add_instruction(op, rx[0], r);    
    
    ADDRESS_VECTOR[1] = RET_INSTR;
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
}

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

int res[SUPORTED_INSTRUCTIONS][REPEAT_INSTRUCTIONS];
void eval_results(){
    if(PRINTS >= 4) help_errors();

    printf("\nRESULTS\n");
    for(int i = 0; i < SUPORTED_INSTRUCTIONS; i++){
        printf("%d - %s\t ", i, get_OP(i));
        for(int j = 0; j < REPEAT_INSTRUCTIONS; j++){
            printf("[%s] ", get_err(res[i][j]));
        }
        printf("\n");
    }

    printf("\nHardware errors: %d\n", error_count);
}

void all_test() {

    set_vet_settings();
    generate_initial_values();
    printf("Done init values\n");
    
    for(int i = 0; i < SUPORTED_INSTRUCTIONS; i++){
        for(int j = 0; j < REPEAT_INSTRUCTIONS; j++)
            res[i][j] = 2;
    }

    int inc = NUM_REGISTERS * EL_PER_BLOCK;
    int z = 0;
    for(; z < SUPORTED_INSTRUCTIONS; z++){
        for(int j = 0; j < REPEAT_INSTRUCTIONS; j++){
            int prev_error = error_count;
            if(PRINTS >= 3)printf("==== Begginning test  %d ======\n\n", z * REPEAT_INSTRUCTIONS + j);

            if(PRINTS >= 3)printf("Executing instruction %s\n", get_OP(z));
            generate_RIS(z, (z * REPEAT_INSTRUCTIONS + j) * inc);
            execute_RIS(&OUT[(z * REPEAT_INSTRUCTIONS + j) * inc], r);

            if(PRINTS >= 3)print_matrix(&scalar_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);
            if(PRINTS >= 3)print_matrix(&vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);
            
            if(prev_error < error_count){
                if(PRINTS >= 1) printf("Hardware error detected\n");
                res[z][j] = -last_hw_error;
            } else {
                if(manual_convergence(&scalar_res[0][0], &vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK)){
                    if(PRINTS >= 1)printf("Convergence \n");
                }else{
                    if(PRINTS >= 1)printf("Divergence\n");
                    res[z][j] = 1;
                }
            } 
        }
    }
    printf("index: %d\n", z * REPEAT_INSTRUCTIONS * inc);

    eval_results(res);
}

int main(){
    asm volatile("csrw mtvec, %0" : : "r" (new_trap_handler));
    printf("Executing all instructions once\n");
    all_test();
    exit(0);
}