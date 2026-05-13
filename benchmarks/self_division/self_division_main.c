#include <util.h>
#include "configs.h"
#include "mysrand.h"
#include "myutil.h"
#include "asm_functions.h"
#include "permute.h"
#include "add_instruction.h"

#define REPEAT_TESTS 4

#define N 4 * REPEAT_TESTS * VLEN / SEW * NUM_REGS

int32_t ADDRESS_VECTOR[30];

/* ===== EXTERNALS ===== */

extern void clean_vector_scalar(int* v1, int n);
extern void STALL(int cycles);

extern void jump_to_vet(int* vet);

extern int* load_OUT_t0_vet(int* address);


/* ===== NORMALS ===== */

volatile int32_t OUT[N];
volatile int32_t vet_res[32][4];


/* ===== RANDOMIZERS ===== */
void generate_initial_values(){
    for (int i = 0; i < N; i++) {
        OUT[i] = mrand() % 0x7FF;
    }
}

int ops[6];

void randomize_instructions(){
    ops[0] = mrand() % 4;
    ops[1] = mrand() % 4;
    ops[2] = mrand() % 4;
    ops[3] = mrand() % 4;
    ops[4] = -1;
    ops[5] = -1;

}

/* ===== VECTOR LOADERS =====*/

load_random_values_registers(int* vet){
    set_vet_settings();
    for(int i = 0; i < 32; i++){
        load_to_vet(&vet[i * EL_PER_BLOCK], i);
    }
}

/*
1 - picks 3 different registers
2 - picks 4 operations
3 - picks
*/
void execute_batch_tests(int index, int rx[NUM_REGS][3]){
    int r[NUM_REGS] = {0, 8, 16, 24};
    int prev_error = error_count;


    char operation_value[4] = {'+', '-', '/', '*'};

    for(int i = 0; i < REPEAT_TESTS; i++){
        load_init_values_scalar(&OUT[index], r, NUM_REGS);
        printf("Executing test of 4 operations (r%d = r%d %c r%d)\n", rx[0][0], rx[0][1], operation_value[i], rx[0][2]);
        for(int j = 0; j < 4; j++){
            ADDRESS_VECTOR[j] = add_instruction(i, rx[j], r);
        }
        ADDRESS_VECTOR[NUM_REGS] = RET_INSTR;

        execute_RIS(&OUT[index], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGS);
        if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2)
            printf("Convergence %d-%d\n", index, index + 4 * EL_PER_BLOCK);
        else
            printf("Divergence\n");
        if(PRINTS > 3){
            printf("SCALAR MATRIX:\n");
            print_regs(&scalar_res[0][0], NUM_REGS, r);
        
            printf("OUTPUT from vector:\n");
            print_regs(&vet_res[0][0], NUM_REGS, r);
            printf("\n");
        }
        index +=  NUM_REGS * EL_PER_BLOCK;
    }


}

void test(int seed) {
    set_vet_settings();

    msrand(seed);
    generate_initial_values();
    printf("Done init values\n");
    
    msrand(seed); // Length of the values should not alter significantly the operations

    int inc = NUM_REGS * EL_PER_BLOCK * REPEAT_TESTS;
    int z = 0;
    int rx2[NUM_REGS][3] = {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}};
    int rx1[NUM_REGS][3] = {{0, 1, 0}, {1, 2, 1}, {2, 3, 2}, {3, 0, 3}};
    int rx3[NUM_REGS][3] = {{0, 0, 1}, {1, 1, 2}, {2, 2, 3}, {3, 3, 0}};
    

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