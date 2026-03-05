#include <util.h>
#include "configs.h"
#include "mysrand.h"
#include "myutil.h"
#include "asm_functions.h"
#include "permute.h"

#define N 310

int32_t ADDRESS_VECTOR[30];

/* ===== EXTERNALS ===== */

extern void clean_vector_scalar(int* v1, int n);
extern void STALL(int cycles);

extern void jump_to_vet(int* vet);

extern int* load_OUT_t0_vet(int* address);


/* ===== NORMALS ===== */

volatile int A[N];
volatile int B[N];
volatile int32_t OUT[N];
volatile int32_t scalar_res[5][4];
volatile int32_t vet_res[5][4];


/* ===== RANDOMIZERS ===== */
void generate_initial_values(){
    for (int i = 0; i < N; i++) {
        A[i] = mrand() % 0x7FF;
        B[i] = mrand() % 0x7FF;
        OUT[i] = mrand() % 0x7FF;
    }
}

int ops[6];

void shuffle_registers(int r[3]){
    r[0] = mrand() % 32;

    do
    r[1] = mrand() % 32;
    while(r[1] == r[0]);
    
    do
    r[2] = mrand() % 32;
    while(r[2] == r[0] || r[2] == r[1]);

    if(PRINTS >= 2) printf("Register choice:\nr[0]= %d; r[1]= %d; r[2]= %d;\n\n", r[0], r[1], r[2]);
}

void randomize_instructions(){
    ops[0] = mrand() % 4;
    ops[1] = mrand() % 4;
    ops[2] = mrand() % 4;
    ops[3] = mrand() % 4;
    ops[4] = -1;
    ops[5] = -1;

    /*rx[0][0] = mrand() % 3; rx[0][1] = mrand() % 3; rx[0][2] = mrand() % 3;
    rx[1][0] = mrand() % 3; rx[1][1] = mrand() % 3; rx[1][2] = mrand() % 3;
    rx[2][0] = mrand() % 3; rx[2][1] = mrand() % 3; rx[2][2] = mrand() % 3;
    rx[3][0] = mrand() % 3; rx[3][1] = mrand() % 3; rx[3][2] = mrand() % 3;*/
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
    NOP = 555
};
char* get_OP(int op){
    switch(op){
        case 0: return "+";
        case 1: return "-";
        case 2: return "/";
        case 3: return "*";
    }
    return "ERROR";    
}

void execute_RIS(int* vet, int r[3]){
    set_vet_settings();
    load_init_values_vector(vet, r);
    set_vet_settings();
    jump_to_vet(&ADDRESS_VECTOR[0]);
    store_vet_values(r);
}

int add_instruction(int op, int rx[3]){
    int instr = 0;
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
        case NOP:
            return ADDI_ZZZ_INSTR;
        default:
            break;
    }
    instr = change_vet_rd(instr,  rx[0]);
    instr = change_vet_rs1(instr, rx[1]);
    instr = change_vet_rs2(instr, rx[2]);
    return instr;
}

load_init_values_scalar(int* vet){
    for(int i = 0; i < EL_PER_BLOCK; i++)
    {
        scalar_res[0][i] = vet[i];
        scalar_res[1][i] = vet[EL_PER_BLOCK + i];
        scalar_res[2][i] = vet[2 * EL_PER_BLOCK + i];
        scalar_res[3][i] = vet[3 * EL_PER_BLOCK + i];
        scalar_res[4][i] = vet[4 * EL_PER_BLOCK + i];
    }
}

load_init_values_vector(int* vet, int regs[5]){    
    set_vet_settings();
    load_to_vet(&vet[0], regs[0]);
    load_to_vet(&vet[EL_PER_BLOCK], regs[1]);
    load_to_vet(&vet[2 * EL_PER_BLOCK], regs[2]);
    load_to_vet(&vet[3 * EL_PER_BLOCK], regs[3]);
    load_to_vet(&vet[4 * EL_PER_BLOCK], regs[4]);

}

load_random_values_registers(int* vet){
    set_vet_settings();
    for(int i = 0; i < 32; i++){
        load_to_vet(&vet[i * EL_PER_BLOCK], i);
    }
}

store_vet_values(int r[5]){
    clean_vector_scalar(&vet_res[0][0], 12);

    set_vet_settings();
    store_to_vet(&vet_res[0][0], r[0]);
    store_to_vet(&vet_res[1][0], r[1]);
    store_to_vet(&vet_res[2][0], r[2]);
    store_to_vet(&vet_res[3][0], r[3]);
    store_to_vet(&vet_res[4][0], r[4]);
}

/*
1 - picks 3 different registers
2 - picks 4 operations
3 - picks
*/
void execute_batch_tests(int index, int rx[5][3]){
    int r[5] = {0, 1, 2, 3, 4, 5};
    ADDRESS_VECTOR[5] = RET_INSTR;

    char operation_value[4] = {'+', '-', '/', '*'};

    for(int i = 0; i < 4; i++){
        load_init_values_scalar(&OUT[index]);
        printf("Executing test of 5 operations of op %c(r%d = r%d %c r%d)\n", operation_value[i], rx[0][1], rx[0][2], operation_value[i], rx[0][3]);
        for(int j = 0; j < 5; j++){
            ADDRESS_VECTOR[j] = add_instruction(i, rx[j]);
        }
        set_vet_settings();
        load_init_values_vector(&OUT[index], r);
        jump_to_vet(ADDRESS_VECTOR);
        store_vet_values(r);
        index += 5 * 4;
        
        if (is_divergent_matrix(vet_res, scalar_res, 5, 4) == 1){
            printf("Error on test of operation %c\n", operation_value[i]);
            printf("VET MATRIX:\n");
            print_matrix(&vet_res[0][0], 5, 4);
            printf("SCALAR MATRIX:\n");
            print_matrix(&scalar_res[0][0], 5, 4);
            //exit(0);
        }
    }    
}

void test(int seed) {
    set_vet_settings();

    msrand(seed);
    generate_initial_values();
    load_random_values_registers(&OUT[0]);
    printf("Done init values\n");
    
    msrand(seed); // Length of the values should not alter significantly the operations

    int inc = 5 * (VLEN / SEW) * 4;
    int z = 0;
    int rx2[5][3] = {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}};
    int rx1[5][3] = {{0, 1, 0}, {1, 2, 1}, {2, 3, 2}, {3, 4, 3}, {4, 0, 4}};
    int rx3[5][3] = {{0, 0, 1}, {1, 1, 2}, {2, 2, 3}, {3, 3, 4}, {4, 4, 0}};
    

    printf("\nTEST %d\n", 0);
    execute_batch_tests(z, rx1);
    z += inc;

    printf("\nTEST %d\n", 1);
    execute_batch_tests(z, rx2);
    z += inc;

    printf("\nTEST %d\n", 2);
    execute_batch_tests(z, rx3);
}

int main(){
    test(SEED);

    exit(0);
}