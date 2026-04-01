#include <util.h>
#include "configs.h"
#include "myutil.h"
#include "asm_functions.h"
#include "mysrand.h"
#include "float_operator.h"
#include "parameters.h"
#include "add_instruction.h"

#define MAX_REPEAT_INSTRUCTIONS 10
int repeat_instructions = 6;
#define MAX_N EL_PER_BLOCK * MAX_REPEAT_INSTRUCTIONS * SUPORTED_INSTRUCTIONS * NUM_REGISTERS

/* PARAMETERS */
int SEED = 0;
int sole_execution = -1;
int N = MAX_N;

int32_t ADDRESS_VECTOR[255];

/* ===== EXTERNALS ===== */

extern void clean_vector_scalar(int* v1, int n);
extern void STALL(int cycles);

extern void jump_to_vet(int* vet);

extern int* load_OUT_t0_vet(int* address);
extern void load_value_ft0(float f);
extern int return_t0();

extern void new_trap_handler(void);


/* ===== NORMALS ===== */

volatile int A[MAX_N];
volatile int B[MAX_N];
volatile int32_t OUT[MAX_N];


/* ===== RANDOMIZERS ===== */
void generate_initial_values(){
    msrand(SEED);
    for (int i = 0; i < N; i++) {
        A[i] = mrand();
        B[i] = mrand();
        OUT[i] = mrand();
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

void execute_RIS(int* vet, int r[NUM_REGISTERS]){
    set_vet_settings();
    load_init_values_vector(vet, r);
    set_vet_settings();

    load_OUT_t0_vet((int*)t0_VALUE);// Gambiarra simples para ter t0 com t0_VALUE
    load_value_ft0(f_vf);
    
    jump_to_vet(&ADDRESS_VECTOR[0]);
    actual_t0 = return_t0();
    if(PRINTS >= 2 && compare_registers != -1) 
        printf("Compare_registers: %d, actual_t0: %d \n", compare_registers, actual_t0);
    store_vet_values(r);
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

int res[SUPORTED_INSTRUCTIONS][MAX_REPEAT_INSTRUCTIONS];
void eval_results(){
    int qtt_errors = 0;
    if(PRINTS >= 4) help_errors();

    printf("\nRESULTS\n");
    for(int i = 0; i < SUPORTED_INSTRUCTIONS; i++){
        printf("%d - %s\t ", i, get_OP(i));
        for(int j = 0; j < repeat_instructions; j++){
            printf("[%s] ", get_err(res[i][j]));
            if(res[i][j] != 2)
                qtt_errors++;
        }
        printf("\n");
    }

    printf("\nErrors: %d\n", qtt_errors);
    printf("Hardware errors: %d\n", error_count);
    printf("Wrong answers: %d\n", qtt_errors - error_count);
}

int test_for_ls32(){
    if(PRINTS >= 3)printf("==== Testing required instructions ======\n\n");
    int res[repeat_instructions];
    int inc = NUM_REGISTERS * EL_PER_BLOCK;
    for(int j = 0; j < repeat_instructions; j++){
        int prev_error = error_count;

        if(PRINTS >= 3)printf("Executing instruction VLE32_V VSE32_V %d\n", j);
        load_init_values_scalar(&OUT[j * inc]);
        set_vet_settings();
        load_init_values_vector(&OUT[j * inc], r);
        store_vet_values(r);

        if(PRINTS >= 3)print_matrix(&scalar_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);
        if(PRINTS >= 3)print_matrix(&vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);
        
        if(prev_error < error_count){
            if(PRINTS >= 1) printf("Hardware error detected\n");
            res[j] = -last_hw_error;
        } else {
            if(manual_convergence(&scalar_res[0][0], &vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK)){
                if(PRINTS >= 1)printf("Convergence \n");
                res[j] = 2;
            }else{
                if(PRINTS >= 1)printf("Divergence\n");
                res[j] = 1;
            }
        }
    }
    int ret = 1;
    printf("\nRESULTS\n");
    printf("VLE32_V VSE32_V\t ");
    for(int j = 0; j < repeat_instructions; j++){
        printf("[%s] ", get_err(res[j]));
        if(res[j] != 2) ret = 0;
    }
    printf("\n\n");

    return ret;
}

void single_test(int op){
    set_vet_settings();
    int inc = NUM_REGISTERS * EL_PER_BLOCK;
    N = inc * repeat_instructions;
    generate_initial_values();
    printf("Done init values\n");
    
    for(int i = 0; i < repeat_instructions; i++)
        res[0][i] = 2;
    

    for(int j = 0; j < repeat_instructions; j++){
        int prev_error = error_count;
        if(PRINTS >= 3)printf("==== Begginning test ======\n\n");

        if(PRINTS >= 3)printf("Executing instruction %s\n", get_OP(op));
        generate_RIS(op, j * inc);
        execute_RIS(&OUT[j * inc], r);

        if(PRINTS >= 3){printf("IN:\n");print_vector(&OUT[0], NUM_REGISTERS * EL_PER_BLOCK, EL_PER_BLOCK);}
        if(PRINTS >= 3){printf("Scalar:\n");print_matrix(&scalar_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);}
        if(PRINTS >= 3){printf("Vetorial:\n");print_matrix(&vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);}
        
        if(prev_error < error_count){
            if(PRINTS >= 1) printf("Hardware error detected\n");
            res[0][j] = -last_hw_error;
        } else {
            res[0][j] = compare_solutions(prev_error);   
        } 
    }

    printf("Results: - %s\t ", get_OP(op));
    for(int j = 0; j < repeat_instructions; j++){
        printf("[%s] ", get_err(res[0][j]));
    }
    printf("\n");
}

void all_test() {
    set_vet_settings();
    N = EL_PER_BLOCK * repeat_instructions * SUPORTED_INSTRUCTIONS * NUM_REGISTERS;
    printf("N: %d\n", N);
    generate_initial_values();
    printf("Done init values\n");

    if(!test_for_ls32()){
        if(MEM_PROTECION){
            printf("Store and load not working properly, impossible to continue\n");
            printf("Disable MEM_PROTECTION to continue anyway");
            exit(1);
        }
        printf("WARNING: some memory tests were incorrect, could generate random errors\n");        
    }

    int inc = NUM_REGISTERS * EL_PER_BLOCK;
    int z = 0;
    for(; z < SUPORTED_INSTRUCTIONS; z++){
        for(int j = 0; j < repeat_instructions; j++){
            int prev_error = error_count;
            if(PRINTS >= 3)printf("==== Begginning test  %d ======\n\n", z * repeat_instructions + j);

            if(PRINTS >= 3)printf("Executing instruction %s\n", get_OP(z));

            generate_RIS(z, (z * repeat_instructions + j) * inc);
            execute_RIS(&OUT[(z * repeat_instructions + j) * inc], r);
            
            if(PRINTS >= 3){printf("Scalar:\n");print_matrix(&scalar_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);}
            if(PRINTS >= 3){printf("Vetorial:\n");print_matrix(&vet_res[0][0], NUM_REGISTERS, EL_PER_BLOCK);}


            if(prev_error < error_count){
                if(PRINTS >= 1) printf("Hardware error detected\n");
                res[z][j] = -last_hw_error;
            } else {
                res[z][j] = compare_solutions(prev_error);   
            }  
        }
    }
    if(PRINTS >= 3) printf("index: %d\n", z * repeat_instructions * inc);

    eval_results(res);
}

int digest_parameters(){
    if(parameter.argc > 0) SEED                = parameter.argv[0];
    if(parameter.argc > 1) sole_execution      = parameter.argv[1];
    if(parameter.argc > 2) PRINTS              = parameter.argv[2];
    if(parameter.argc > 3) repeat_instructions = parameter.argv[3];
}

int main(){
    digest_parameters();
    asm volatile("csrw mtvec, %0" : : "r" (new_trap_handler));
    
    if (sole_execution != -1){
        printf("Executing single instruction\n");
        single_test(sole_execution);
    }
    else{
        printf("Executing all instructions once\n");
        all_test();
    } 
    exit(0);
}