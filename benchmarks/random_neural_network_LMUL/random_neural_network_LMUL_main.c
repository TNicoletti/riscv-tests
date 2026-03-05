#include <util.h>
#include "configs.h"
#include "mysrand.h"
#include "myutil.h"
#include "asm_functions.h"
#include "permute.h"

#define N 1024

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
volatile int32_t scalar_res[3][EL_PER_BLOCK];
volatile int32_t vet_res[3][EL_PER_BLOCK];


/* ===== RANDOMIZERS ===== */
void generate_initial_values(){
    for (int i = 0; i < N; i++) {
        A[i] = mrand() % 0x7FF;
        B[i] = mrand() % 0x7FF;
        OUT[i] = mrand() % 0x7FF;
    }
}

int r[3];
int rx[6][3];
int ops[6];

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

void shuffle_registers(int r[3]){
    r[0] = get_random_reg();

    do
    r[1] = get_random_reg();
    while(r[1] == r[0]);
    
    do
    r[2] = get_random_reg();
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

    rx[0][0] = mrand() % 3; rx[0][1] = mrand() % 3; rx[0][2] = mrand() % 3;
    rx[1][0] = mrand() % 3; rx[1][1] = mrand() % 3; rx[1][2] = mrand() % 3;
    rx[2][0] = mrand() % 3; rx[2][1] = mrand() % 3; rx[2][2] = mrand() % 3;
    rx[3][0] = mrand() % 3; rx[3][1] = mrand() % 3; rx[3][2] = mrand() % 3;
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

int add_instruction(int op, int rx[3], int r[3]){
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
    instr = change_vet_rd(instr,  r[rx[0]]);
    instr = change_vet_rs1(instr, r[rx[1]]);
    instr = change_vet_rs2(instr, r[rx[2]]);
    return instr;
}

load_init_values_scalar(int* vet){
    for(int i = 0; i < EL_PER_BLOCK; i++)
    {
        scalar_res[0][i] = vet[i];
        scalar_res[1][i] = vet[EL_PER_BLOCK + i];
        scalar_res[2][i] = vet[2 * EL_PER_BLOCK + i];
    }
}

load_init_values_vector(int* vet, int regs[3]){    
    set_vet_settings();
    load_to_vet(&vet[0], regs[0]);
    load_to_vet(&vet[EL_PER_BLOCK], regs[1]);
    load_to_vet(&vet[2 * EL_PER_BLOCK], regs[2]);
}

store_vet_values(int r[3]){
    clean_vector_scalar(&vet_res[0][0], EL_PER_BLOCK * 3);

    set_vet_settings();
    store_to_vet(&vet_res[0][0], r[0]);
    store_to_vet(&vet_res[1][0], r[1]);
    store_to_vet(&vet_res[2][0], r[2]);
}

/*===== RANDOM TEST FUNCTIONS =====*/

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

    6 - Do the same operations with different configurations
*/
void error_discoverer(int index){
    int qtd_tests[6] = {0, 0, 0, 0, 0, 0};
    int passed[6]    = {0, 0, 0, 0, 0, 0};
    printf("\n===== Heuristic 0 ===== \n");
    for(int i = 0; i < 5; i++){
        qtd_tests[0]++;
        load_init_values_scalar(&OUT[index]);

        for(int i = 0; i < 4; i++){
            ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        }
    
        ADDRESS_VECTOR[4] = RET_INSTR;

        execute_RIS(&OUT[index], r);
        if(is_divergent_matrix(&scalar_res[0][0], &vet_res[0][0], 3, EL_PER_BLOCK)){
            printf("Convergence\n");
            passed[0]++;
        }else{
            printf("Divergence\n");
        }
        if(PRINTS >= 3){
            printf("SCALAR:\n");
            print_matrix(&scalar_res[0][0], 3, EL_PER_BLOCK);
            printf("VETORIAL:\n");
            print_matrix(&vet_res[0][0], 3, EL_PER_BLOCK);
        }
    }

    printf("\n===== Heuristic 1 ===== \n");
    qtd_tests[1] = 1;
    load_init_values_scalar(&OUT[index]);

    ADDRESS_VECTOR[0] = add_instruction(ops[0], rx[0], r);
    for(int i = 0; i < 5; i++) ADDRESS_VECTOR[i + 1] = add_instruction(NOP, rx[0], r);
    ADDRESS_VECTOR[6] = add_instruction(ops[1], rx[1], r);
    for(int i = 0; i < 5; i++) ADDRESS_VECTOR[i + 7] = add_instruction(NOP, rx[0], r);
    ADDRESS_VECTOR[11] = add_instruction(ops[2], rx[2], r);
    for(int i = 0; i < 5; i++) ADDRESS_VECTOR[i + 12] = add_instruction(NOP, rx[0], r);
    ADDRESS_VECTOR[16] = add_instruction(ops[3], rx[3], r);
    ADDRESS_VECTOR[17] = RET_INSTR;
    execute_RIS(&OUT[index], r);

    if(is_divergent_matrix(&scalar_res[0][0], &vet_res[0][0], 3, EL_PER_BLOCK)){
        printf("Convergence, probably data hazard problem\n");
        passed[1]++;
    }else{
        printf("Divergence => proceed to next test\n");
        print_matrix(&scalar_res[0][0], 3, EL_PER_BLOCK);
        print_matrix(&vet_res[0][0], 3, EL_PER_BLOCK);

    }

    printf("\n===== Heuristic 2 ===== \n\n");
    for(int i = 0; i < 4; i++){
        qtd_tests[2]++;        
        load_init_values_scalar(&OUT[index]);
        ADDRESS_VECTOR[0] = add_instruction(ops[i], rx[i], r);
        ADDRESS_VECTOR[1] = RET_INSTR;
        execute_RIS(&OUT[index], r);
        if(is_divergent_matrix(&scalar_res[0][0], &vet_res[0][0], 3, EL_PER_BLOCK)){
            printf("Convergence\n");
            passed[2]++;
        }
        else {
            printf("Divergence => problem single with instruction\n");
            printf("v = %d %s %d\n", rx[i][1], get_OP(ops[i]), rx[i][2]);
        }
    }


    printf("\n===== Heuristic 3 ===== \n\n");
    for(int i = 0; i < 4; i++){
        printf("Removed instruction %d\n", i);
        qtd_tests[3]++;        
        load_init_values_scalar(&OUT[index]);
        for(int j = 0; j < 4; j++){
            if(j == i)
                continue;
            if (j < i)
                ADDRESS_VECTOR[j] = add_instruction(ops[j], rx[j], r);
            else
                ADDRESS_VECTOR[j - 1] = add_instruction(ops[j], rx[j], r);
        }
        ADDRESS_VECTOR[3] = RET_INSTR;
        execute_RIS(&OUT[index], r);
        if(is_divergent_matrix(&scalar_res[0][0], &vet_res[0][0], 3, EL_PER_BLOCK)){
            printf("Convergence\n\n");
            passed[3]++;
        }
        else {
            printf("Divergence\n\n");
        }
    }

    printf("\n===== Heuristic 3.1 ===== \n\n");
    for(int i = 0; i < 4; i++)
        for(int j = i + 1; j < 4; j++){
            qtd_tests[3]++;
            printf("Removed instruction %d %d\n", i, j);        
            load_init_values_scalar(&OUT[index]);
            int t = 0;
            for(int z = 0; z < 4; z++){
                if(z == i || z == j)
                    continue;
                ADDRESS_VECTOR[t] = add_instruction(ops[z], rx[z], r);
                t++;
            }
            ADDRESS_VECTOR[2] = RET_INSTR;
            execute_RIS(&OUT[index], r);
            if(is_divergent_matrix(&scalar_res[0][0], &vet_res[0][0], 3, EL_PER_BLOCK)){
                printf("Convergence\n\n");
                passed[3]++;
            }
            else printf("Divergence\n\n");
        }
        
    printf("\n===== Heuristic 4 ===== \n\n");
    qtd_tests[4] = 1;
    int other_r[3] = {0, 8, 24};
    load_init_values_scalar(&OUT[index]);
    ADDRESS_VECTOR[0] = add_instruction(ops[0], rx[0], other_r);
    ADDRESS_VECTOR[1] = add_instruction(ops[1], rx[1], other_r);
    ADDRESS_VECTOR[2] = add_instruction(ops[2], rx[2], other_r);
    ADDRESS_VECTOR[3] = add_instruction(ops[3], rx[3], other_r);
    ADDRESS_VECTOR[4] = RET_INSTR;
    execute_RIS(&OUT[index], other_r);
    if(is_divergent_matrix(&scalar_res[0][0], &vet_res[0][0], 3, EL_PER_BLOCK)){
        printf("Convergence\n\n");
        passed[4]++;
    }
    else printf("Divergence\n\n");
    print_matrix(&vet_res[0][0], 3, EL_PER_BLOCK);
    print_matrix(&scalar_res[0][0], 3, EL_PER_BLOCK);
    
    printf("\n===== Heuristic 5 ===== \n\n");

    for(int one = 0; one < 24; one++){
        qtd_tests[5]++;
        int perm[4];
        get_permutation(one, 4, &perm[0]);
        printf("test %d\n", one);   
        printf("PERMUTATION %d %d %d %d\n", perm[0], perm[1], perm[2], perm[3]);
        load_init_values_scalar(&OUT[index]);
        ADDRESS_VECTOR[0] = add_instruction(ops[perm[0]], rx[perm[0]], r);
        ADDRESS_VECTOR[1] = add_instruction(ops[perm[1]], rx[perm[1]], r);
        ADDRESS_VECTOR[2] = add_instruction(ops[perm[2]], rx[perm[2]], r);
        ADDRESS_VECTOR[3] = add_instruction(ops[perm[3]], rx[perm[3]], r);
        ADDRESS_VECTOR[4] = RET_INSTR;
        execute_RIS(&OUT[index], r);
        if(is_divergent_matrix(&scalar_res[0][0], &vet_res[0][0], 3, EL_PER_BLOCK)){
            printf("Convergence\n\n");
            passed[5]++;
        }
        else printf("Divergence\n\n");
    }

    for(int i = 0; i < 6; i++)
        printf("RESULTS %d: %d out of %d tests converged\n\n", i, passed[i], qtd_tests[i]);
    
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
void generate_RIS(int index){
    shuffle_registers(r);
    randomize_instructions();
    load_init_values_scalar(&OUT[index]);

    if(PRINTS >= 2)printf("STEP BY STEP RESULTS: \n");
    for(int i = 0; i < 4; i++){
        ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
    }
    
    ADDRESS_VECTOR[4] = RET_INSTR;
}

void random_test(int seed) {
    set_vet_settings();

    msrand(seed);
    generate_initial_values();
    printf("Done init values\n");
    
    msrand(seed); // Length of the values should not alter significantly the operations

    int inc = 3 * EL_PER_BLOCK;
    for(int z = 0; z + inc < N; z+= inc){
        printf("==== Begginning test  %d ======\n\n", z / inc);

        generate_RIS(z);
        execute_RIS(&OUT[z], r);

        if(is_divergent_matrix(&scalar_res[0][0], &vet_res[0][0], 3, EL_PER_BLOCK)){
            printf("Convergence %d-%d\n", z, z + inc);
        }else{
            printf("Divergence %d-%d\n", z, z + inc);
            printf("Going to error discoverer\n");
            
            if(PRINTS >= 3){
                printf("OUT before modifications:\n");
                print_vector(&OUT[z], inc, EL_PER_BLOCK);
                printf("\n");
            }
            if(PRINTS){    
                printf("OUTPUT from vector:\n");
                print_matrix(&vet_res[0][0], 3, EL_PER_BLOCK);
                printf("\n");

                printf("SCALAR MATRIX:\n");
                print_matrix(&scalar_res[0][0], 3, EL_PER_BLOCK);
            }
            
            error_discoverer(z);
            exit(0);
        }
        printf("==== End test  %d ======\n", z / inc);
    }

}

int main(){
    printf("Doing random batch tests with registers v0-v7 with seed %d\n", SEED);
    random_test(SEED);
}