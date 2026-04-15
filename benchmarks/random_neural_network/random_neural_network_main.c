#include <util.h>
#include "configs.h"
#include "mysrand.h"
#include "myutil.h"
#include "asm_functions.h"
#include "permute.h"
#include "parameters.h"
#include "add_instruction.h"

#define MAX_N 1024 * 16

#define NUM_RANDOM_OPS 4
#define QTD_HEURISTICS 8

int N = 4096;
int SEED = 0x1123456;

int32_t ADDRESS_VECTOR[255];
int allowed_instructions[SUPORTED_INSTRUCTIONS];
int fixed_suported_instructions = SUPORTED_INSTRUCTIONS;

/* ===== EXTERNALS ===== */

extern void clean_vector_scalar(int* v1, int n);
extern void STALL(int cycles);

extern int* load_OUT_t0_vet(int* address);
extern void load_value_ft0(float f);

extern void jump_to_vet(int* vet);

extern int* load_OUT_t0_vet(int* address);


/* ===== NORMALS ===== */

volatile int A[MAX_N];
volatile int B[MAX_N];
volatile int32_t OUT[MAX_N];
volatile int32_t scalar_res[NUM_REGISTERS][EL_PER_BLOCK];
volatile int32_t vet_res[NUM_REGISTERS][EL_PER_BLOCK];


/* ===== RANDOMIZERS ===== */
void generate_initial_values(){
    for (int i = 0; i < N; i++) {
        A[i] = mrand();
        B[i] = mrand();
        OUT[i] = mrand();
    }
}

int r[NUM_REGISTERS];
int rx[NUM_RANDOM_OPS][3];
int ops[NUM_RANDOM_OPS];

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

int fix_op(int x){
    int r = -1;
    for(int i = 0; i < SUPORTED_INSTRUCTIONS; i++){
        if(allowed_instructions[i] == 1)
            r++;
        if(r == x)
            return i;
    }
    return r;
}

void randomize_instructions(){
    for(int i = 0; i < NUM_RANDOM_OPS; i++){
        ops[i] = mrand() % fixed_suported_instructions;
        ops[i] = fix_op(ops[i]);
        rx[i][0] = mrand() % NUM_REGISTERS; rx[i][1] = mrand() % NUM_REGISTERS; rx[i][2] = mrand() % NUM_REGISTERS;
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

void execute_RIS(int* vet, int r[NUM_REGISTERS]){
    set_vet_settings();
    load_init_values_vector(vet, r);
    set_vet_settings();

    load_OUT_t0_vet((int*)t0_VALUE);// Gambiarra simples para ter t0 com t0_VALUE
    load_value_ft0(f_vf);

    jump_to_vet(&ADDRESS_VECTOR[0]);
    store_vet_values(r);
}

load_init_values_vector(int* vet, int regs[NUM_REGISTERS]){    
    set_vet_settings();
    for(int i = 0; i < NUM_REGISTERS; i++)
        load_to_vet(&vet[i * EL_PER_BLOCK], regs[i]);
}

store_vet_values(int r[NUM_REGISTERS]){
    clean_vector_scalar(&vet_res[0][0], EL_PER_BLOCK * NUM_REGISTERS);

    set_vet_settings();
    for(int i = 0; i < NUM_REGISTERS; i++)
        store_to_vet(&vet_res[i][0], r[i]);
    
}

/*===== RANDOM TEST FUNCTIONS =====*/
int wrong_op = -1;
void analyze_results(int passed[QTD_HEURISTICS], int qtd_tests[QTD_HEURISTICS]){
    printf("===== ERROR RESULTS ANALISIS =====\n");
    if(PRINTS >= 0){
        printf("Instructions: \n");
        for(int i = 0; i < NUM_RANDOM_OPS; i++)
            printf("%s; (%d, %d, %d)\n", get_OP(ops[i]), rx[i][0], rx[i][1], rx[i][2]);
        printf("t0: %d\n", t0_VALUE);
            printf("\n");

    }
    int all_passed = 0;
    for(int i = 0; i < QTD_HEURISTICS; i++){
        all_passed += passed[i] / qtd_tests[i];
        printf("RESULTS %d: %d out of %d tests converged\n", i, passed[i], qtd_tests[i]);
    }

    if(passed[0] == 0){
        printf("ERROR IS COMPULSORY\n");
    }
    else
        printf("ERROR IS NOT COMPULSORY\n");
    
    if(all_passed >= QTD_HEURISTICS){
        printf(" => could be cold start or random error\n");
        printf("No further conclusions to be made as it passed all tests(including simply repeating the test)\n");
        return;
    }
    
    if (passed[1] == 1)
        printf("Problem is probably related to data hazards, put nops between instructions solves the issue\n");
    
    if (passed[4] == 1)
        printf("Problem is probably related to registers, as changing them solves the problem\n");

    if(passed[2] == NUM_RANDOM_OPS - 1){
        printf("Problem is probably related to a single instruction\n");
        if(wrong_op != -1)
            printf("Probable problematic instruction: %s\n", get_OP(wrong_op));
        else
            printf("Problematic instruction could not be identified\n");
    }
}

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
void error_discoverer(int index){
    int qtd_tests[QTD_HEURISTICS] = {0, 0, 0, 0, 0, 0, 0};
    int passed[QTD_HEURISTICS]    = {0, 0, 0, 0, 0, 0, 0};
    if(PRINTS >= 1) printf("\n===== Heuristic 0 ===== \n");
    for(int i = 0; i < 5; i++){
        qtd_tests[0]++;
        load_init_values_scalar(&OUT[index]);

        for(int j = 0; j < NUM_RANDOM_OPS; j++){
            ADDRESS_VECTOR[j] = add_instruction(ops[j], rx[j], r);
        }
    
        ADDRESS_VECTOR[NUM_RANDOM_OPS] = RET_INSTR;

        execute_RIS(&OUT[index], r);
        if(manual_convergence(&scalar_res[0][0], &vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK)){
            if(PRINTS >= 1) printf("Convergence\n");
            passed[0]++;
        }else{
            if(PRINTS >= 1) printf("Divergence\n");
        }
        if(PRINTS >= 3){
            printf("SCALAR:\n");
            print_matrix(&scalar_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);
            printf("VETORIAL:\n");
            print_matrix(&vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);
        }
    }

    if(PRINTS >= 1) printf("\n===== Heuristic 1 ===== \n");
    qtd_tests[1] = 1;
    int qtd_nops = 32;
    load_init_values_scalar(&OUT[index]);
    for(int i = 0; i < NUM_RANDOM_OPS; i++){
        ADDRESS_VECTOR[i * (qtd_nops + 1)] = add_instruction(ops[i], rx[i], r);
        for(int j = 1; j <= qtd_nops; j++) ADDRESS_VECTOR[j + i * (qtd_nops + 1)] = add_instruction(NOP, rx[0], r);
        
    }
    ADDRESS_VECTOR[NUM_RANDOM_OPS * (qtd_nops + 1)] = RET_INSTR;
    execute_RIS(&OUT[index], r);

    if(manual_convergence(&scalar_res[0][0], &vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK)){
        if(PRINTS >= 1) printf("Convergence, probably data hazard problem\n");
        passed[1]++;
    }else{
        if(PRINTS >= 1) printf("Divergence => proceed to next test\n");
        if(PRINTS >= 3) print_matrix(&scalar_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);
        if(PRINTS >= 3) print_matrix(&vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);

    }

    if(PRINTS >= 1) printf("\n===== Heuristic 2 ===== \n\n");
    for(int i = 0; i < 4; i++){
        qtd_tests[2]++;        
        load_init_values_scalar(&OUT[index]);
        ADDRESS_VECTOR[0] = add_instruction(ops[i], rx[i], r);
        ADDRESS_VECTOR[1] = RET_INSTR;
        execute_RIS(&OUT[index], r);
        if(manual_convergence(&scalar_res[0][0], &vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK)){
            if(PRINTS >= 1) printf("Convergence\n");
            passed[2]++;
        }
        else {
            if(PRINTS >= 1) printf("Divergence => problem single with instruction\n");
            if(PRINTS >= 3) printf("v = %d %s %d\n", rx[i][1], get_OP(ops[i]), rx[i][2]);
            wrong_op = ops[i];
        }
    }


    if(PRINTS >= 1) printf("\n===== Heuristic 3 ===== \n\n");
    for(int i = 0; i < 4; i++){
        if(PRINTS >= 1) printf("Removed instruction %d\n", i);
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
        if(manual_convergence(&scalar_res[0][0], &vet_res[0][0], 3, EL_PER_BLOCK)){
            if(PRINTS >= 1) printf("Convergence\n");
            passed[3]++;
        }
        else {
            if(PRINTS >= 1) printf("Divergence\n");
        }
    }

    if(PRINTS >= 1) printf("\n===== Heuristic 3.1 ===== \n\n");
    for(int i = 0; i < 4; i++)
        for(int j = i + 1; j < 4; j++){
            qtd_tests[3]++;
            if(PRINTS >= 1) printf("Removed instruction %d %d\n", i, j);        
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
            if(manual_convergence(&scalar_res[0][0], &vet_res[0][0], 3, EL_PER_BLOCK)){
                if(PRINTS >= 1) printf("Convergence\n");
                passed[3]++;
            }
            else if(PRINTS >= 1) printf("Divergence\n");
        }
        
    if(PRINTS >= 1) printf("\n===== Heuristic 4 ===== \n\n");
    qtd_tests[4] = 1;
    int other_r[3] = {0, 8, 24};
    load_init_values_scalar(&OUT[index]);
    ADDRESS_VECTOR[0] = add_instruction(ops[0], rx[0], other_r);
    ADDRESS_VECTOR[1] = add_instruction(ops[1], rx[1], other_r);
    ADDRESS_VECTOR[2] = add_instruction(ops[2], rx[2], other_r);
    ADDRESS_VECTOR[3] = add_instruction(ops[3], rx[3], other_r);
    ADDRESS_VECTOR[4] = RET_INSTR;
    execute_RIS(&OUT[index], other_r);
    if(manual_convergence(&scalar_res[0][0], &vet_res[0][0], 3, EL_PER_BLOCK)){
        if(PRINTS >= 1) printf("Convergence\n\n");
        passed[4]++;
    }
    else{
        if(PRINTS >= 1) printf("Divergence\n\n");
        if(PRINTS >= 3) print_matrix(&vet_res[0][0], 3, EL_PER_BLOCK);
        if(PRINTS >= 3) print_matrix(&scalar_res[0][0], 3, EL_PER_BLOCK);
    } 
    
    if(PRINTS >= 1) printf("\n===== Heuristic 5 ===== \n\n");

    for(int one = 0; one < 24; one++){
        qtd_tests[5]++;
        int perm[4];
        get_permutation(one, 4, &perm[0]);
        if(PRINTS >= 1) printf("PERMUTATION %d %d %d %d\n", perm[0], perm[1], perm[2], perm[3]);
        load_init_values_scalar(&OUT[index]);
        ADDRESS_VECTOR[0] = add_instruction(ops[perm[0]], rx[perm[0]], r);
        ADDRESS_VECTOR[1] = add_instruction(ops[perm[1]], rx[perm[1]], r);
        ADDRESS_VECTOR[2] = add_instruction(ops[perm[2]], rx[perm[2]], r);
        ADDRESS_VECTOR[3] = add_instruction(ops[perm[3]], rx[perm[3]], r);
        ADDRESS_VECTOR[4] = RET_INSTR;
        execute_RIS(&OUT[index], r);
        if(manual_convergence(&scalar_res[0][0], &vet_res[0][0], 3, EL_PER_BLOCK)){
            if(PRINTS >= 1) printf("Convergence\n\n");
            passed[5]++;
        }
        else{
            if(PRINTS >= 1) printf("Divergence\n\n");
        } 
    }

    if(PRINTS >= 1) printf("\n===== Heuristic 6 ===== \n");
    for(int i = 0; i < NUM_RANDOM_OPS; i++){
        int signatures[5][3] = {
            {0, 0, 0},
            {0, 0, 1},
            {0, 1, 0},
            {0, 1, 1},
            {0, 1, 2}
        };
        for(int z = 0; z < 5; z++){
            qtd_tests[6]++;
            int* s = &signatures[z][0];
            int op = ops[i];

            int invalid_instruction = 0;
            load_init_values_scalar(&OUT[index]);
            for(int j = 0; j < 30; j++){
                ADDRESS_VECTOR[j] = add_instruction(op, s, other_r);
                if(ADDRESS_VECTOR[j] == -1){
                    invalid_instruction = 1;
                    break;
                }
            }
            if(invalid_instruction){
                passed[6]++;
                break;
            }
            ADDRESS_VECTOR[30] = RET_INSTR;
            execute_RIS(&OUT[index], other_r);
            if(manual_convergence(&scalar_res[0][0], &vet_res[0][0], 3, EL_PER_BLOCK)){
                if(PRINTS >= 1) printf("Convergence\n");
                passed[6]++;
            }
            else{
                if(PRINTS >= 1) printf("Divergence\n");
                if(PRINTS >= 1) printf("PROBLEMATIC OP: %s\n", get_OP(op));
                wrong_op = op;  
            } 
        }
    }
    printf("\n");

    if(PRINTS >= 1) printf("\n===== Heuristic 7 ===== \n");
    for(int i = 0; i < NUM_RANDOM_OPS; i++){
       if(PRINTS >= 1) printf("Test %d\n\n", i);

        qtd_tests[7]++;
        load_init_values_scalar(&OUT[index]);

        for(int j = 0; j <= i; j++){
            ADDRESS_VECTOR[j] = add_instruction(ops[j], rx[j], r);
        }
    
        ADDRESS_VECTOR[i + 1] = RET_INSTR;

        execute_RIS(&OUT[index], r);
        if(manual_convergence(&scalar_res[0][0], &vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK)){
            if(PRINTS >= 1) printf("Convergence\n");
            passed[7]++;
        }else{
            if(PRINTS >= 1) printf("Divergence\n");
        }
        if(PRINTS >= 3){
            printf("SCALAR:\n");
            print_matrix(&scalar_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);
            printf("VETORIAL:\n");
            print_matrix(&vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);
        }
    }
    printf("\n");

    analyze_results(passed, qtd_tests);
    
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
    for(int i = 0; i < NUM_RANDOM_OPS; i++){
        ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        if (ADDRESS_VECTOR[i] == -1){
            rx[i][0] = 0;
            rx[i][1] = 1;
            rx[i][2] = 2;
            ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        }
        if (ADDRESS_VECTOR[i] == -1){
            rx[i][0] = 0;
            rx[i][1] = 2;
            rx[i][2] = 1;
            ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        }
        if (ADDRESS_VECTOR[i] == -1){
            rx[i][0] = 1;
            rx[i][1] = 0;
            rx[i][2] = 2;
            ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        }
        if (ADDRESS_VECTOR[i] == -1){
            rx[i][0] = 1;
            rx[i][1] = 2;
            rx[i][2] = 0;
            ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        }
        if (ADDRESS_VECTOR[i] == -1){
            rx[i][0] = 2;
            rx[i][1] = 1;
            rx[i][2] = 0;
            ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        }
        if (ADDRESS_VECTOR[i] == -1){
            rx[i][0] = 2;
            rx[i][1] = 0;
            rx[i][2] = 1;
            ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        }
        if (ADDRESS_VECTOR[i] == -1){
            ADDRESS_VECTOR[i] = RET_INSTR;
            return;
        }
    }
    
    ADDRESS_VECTOR[NUM_RANDOM_OPS] = RET_INSTR;
}

void random_test(int seed) {
    set_vet_settings();

    msrand(seed);
    generate_initial_values();
    printf("Done init values\n");
    
    msrand(seed); // Length of the values should not alter significantly the operations
    int inc = NUM_REGISTERS * EL_PER_BLOCK;
    for(int z = 0; z + inc <= N; z+= inc){
        printf("==== Begginning test  %d ======\n", z / inc);

        generate_RIS(z);
        execute_RIS(&OUT[z], r);

        if(manual_convergence(&scalar_res[0][0], &vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK)){
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
                print_matrix(&vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);
                printf("\n");

                printf("SCALAR MATRIX:\n");
                print_matrix(&scalar_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);
            }
            
            error_discoverer(z);
            exit(0);
        }
        printf("==== End test  %d        ======\n\n", z / inc);
    }

}

void print_params(){
    printf("argc: %d\n", parameter.argc);
    for(int i = 0; i < parameter.argc; i++){
        printf("argv[%d] = %d;\n", i, parameter.argv[i]);
    }
    printf("\n");
}

void digest_parameters(){
    //print_params();
    if (parameter.argc > 0){
        SEED = parameter.argv[0];
    }

    if(parameter.argc > 1)
        N = parameter.argv[1];

    if(parameter.argc > 2)
        PRINTS = parameter.argv[2];

    if(parameter.argc >= 3 + SUPORTED_INSTRUCTIONS){
        fixed_suported_instructions = 0;
        for(int i = 0; i < SUPORTED_INSTRUCTIONS; i++){

            allowed_instructions[i] = parameter.argv[3 + i];

            if(allowed_instructions[i]) fixed_suported_instructions++;
        }
    }else{
        for(int i = 0; i < SUPORTED_INSTRUCTIONS; i++){
            allowed_instructions[i] = 1;
        }
    }
    
}

int main(){
    digest_parameters();

    printf("Doing random batch tests with registers v0-v7 with seed %d\n", SEED);
    random_test(SEED);
    exit(0);
}