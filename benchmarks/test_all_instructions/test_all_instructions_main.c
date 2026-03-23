#include <util.h>
#include "configs.h"
#include "myutil.h"
#include "asm_functions.h"
#include "parameters.h"
#include "mysrand.h"

#define MAX_N 1024 * 16

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
        A[i] = mrand() % 32;
        B[i] = mrand() % 32;
        OUT[i] = mrand() % 32;
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
    VADD_VV = 0,
    VSUB_VV = 1,
    VDIV_VV = 2,
    VMUL_VV = 3,
    VSLL_VV = 4,
    VSRL_VV = 5,
    VAND_VV = 6,
    VOR_VV  = 7,
    VXOR_VV = 8,
    VADD_VI = 9,
    VSLL_VI = 10,
    VSRL_VI = 11,
    VAND_VI = 12,
    VOR_VI  = 13,
    VXOR_VI = 14,
    VADD_VX = 15,
    VSUB_VX = 16,
    VDIV_VX = 17,
    VMUL_VX = 18,
    VSLL_VX = 19,
    VSRL_VX = 20,
    VAND_VX = 21,
    VOR_VX  = 22,
    VXOR_VX = 23,
    NOP = 555
};

enum INSTR_TYPES{
    VV = 0,
    VI = 1,
    VX = 2
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
        load_OUT_t0_vet((int*)t0_VALUE);
    }
    if(instr_type == VI){
        instr = change_vet_rd(instr, r[rx[0]]);
        
        instr = change_vet_rs1(instr, r[rx[1]]);
        
        instr = change_vet_rs2(instr, imm);
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
        case   2: return "X"; // correct answer
        case   1: return "WA"; // wrong answer
        case   0: return "IAM";  // Instruction Address Misaligned
        case  -1: return "IAF";  // Instruction Access Fault
        case  -2: return "II";   // Illegal Instruction
        case  -3: return "BP";   // Breakpoint (ebreak)
        case  -4: return "LAM";  // Load Address Misaligned
        case  -5: return "LAF";  // Load Access Fault
        case  -6: return "SAM";  // Store/AMO Address Misaligned
        case  -7: return "SAF";  // Store/AMO Access Fault
        case  -8: return "ECU";  // Environment Call from U-mode
        case  -9: return "ECS";  // Environment Call from S-mode
        case -11: return "ECM";  // Environment Call from M-mode
        case -12: return "IPF";  // Instruction Page Fault
        case -13: return "LPF";  // Load Page Fault
        case -15: return "SPF";  // Store/AMO Page Fault
        case -16: return "HE";   // Hardware Error (implementation specific)
        default: return "UNK";  // Unknown / Reserved
    }
}
int res[SUPORTED_INSTRUCTIONS][REPEAT_INSTRUCTIONS];
void eval_results(){
    printf("\nRESULTS\n");
    for(int i = 0; i < SUPORTED_INSTRUCTIONS; i++){
        printf("%s\t ", get_OP(i));
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

    for(int z = 0; z < SUPORTED_INSTRUCTIONS; z++){
        for(int j = 0; j < REPEAT_INSTRUCTIONS; j++){
            int prev_error = error_count;
            if(PRINTS >= 3)printf("==== Begginning test  %d ======\n\n", z * REPEAT_INSTRUCTIONS + j);

            if(PRINTS >= 3)printf("Executing instruction %s\n", get_OP(z));
            generate_RIS(z, (z * REPEAT_INSTRUCTIONS + j) * inc);
            execute_RIS(&OUT[(z * REPEAT_INSTRUCTIONS + j) * inc], r);

            if(PRINTS >= 3)print_matrix(&scalar_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);
            if(PRINTS >= 3)print_matrix(&vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);

            if(manual_convergence(&scalar_res[0][0], &vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK)){
                if(PRINTS >= 1)printf("Convergence \n");
            }else{
                if(PRINTS >= 1)printf("Divergence\n");
                res[z][j] = 1;
                if(prev_error < error_count){
                    printf("Hardware error detected, might be unimplemented instruction\n");
                    res[z][j] = -last_hw_error;
                } 
            }
        }
    }

    eval_results(res);
}

int main(){
    asm volatile("csrw mtvec, %0" : : "r" (new_trap_handler));
    printf("Executing all instructions once\n");
    all_test();
    exit(0);
}