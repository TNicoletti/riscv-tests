#include "random_LMUL.h"

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

void generate_initial_values(){
    randomize_vector(OUT, N);
}

void generate_RIS(int index){
    shuffle_registers(r, NUM_REGISTERS, LMUL);
    randomize_instructions();
    load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);

    if(PRINTS >= 2)printf("STEP BY STEP RESULTS: \n");
    for(int i = 0; i < NUM_RANDOM_OPS; i++){
        ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        if (ADDRESS_VECTOR[i] == NOP){
            rx[i][0] = 0;
            rx[i][1] = 1;
            rx[i][2] = 2;
            ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        }
        if (ADDRESS_VECTOR[i] == NOP){
            rx[i][0] = 0;
            rx[i][1] = 2;
            rx[i][2] = 1;
            ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        }
        if (ADDRESS_VECTOR[i] == NOP){
            rx[i][0] = 1;
            rx[i][1] = 0;
            rx[i][2] = 2;
            ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        }
        if (ADDRESS_VECTOR[i] == NOP){
            rx[i][0] = 1;
            rx[i][1] = 2;
            rx[i][2] = 0;
            ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        }
        if (ADDRESS_VECTOR[i] == NOP){
            rx[i][0] = 2;
            rx[i][1] = 1;
            rx[i][2] = 0;
            ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        }
        if (ADDRESS_VECTOR[i] == NOP){
            rx[i][0] = 2;
            rx[i][1] = 0;
            rx[i][2] = 1;
            ADDRESS_VECTOR[i] = add_instruction(ops[i], rx[i], r);
        }
        if (ADDRESS_VECTOR[i] == NOP){
            ADDRESS_VECTOR[i] = RET_INSTR;
            return;
        }
    }
    
    ADDRESS_VECTOR[NUM_RANDOM_OPS] = RET_INSTR;
}

void random_LMUL(int seed){
    set_vet_settings();

    msrand(seed);
    generate_initial_values();
    printf("Done init values\n");
    
    msrand(seed); // Length of the values should not alter significantly the operations
    int inc = NUM_REGISTERS * EL_PER_BLOCK;
    for(int z = 0; z + inc <= N; z+= inc){
        int prev_error = error_count;
        printf("==== Begginning test  %d ======\n", z / inc);

    
        generate_RIS(z);
        printf("Execute_RIS\n");
        execute_RIS(&OUT[z], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);

        if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2){
            printf("Convergence %d-%d\n", z, z + inc);
        }else{
            printf("===== 1 =====\n");
            print_vector(&scalar_res[0][0], 32 * VLEN / SEW, VLEN / SEW);
            printf("===== 2 =====\n");
            print_vector(&vet_res[0][0], 32 * VLEN / SEW, VLEN / SEW);
            printf("===== 3 =====\n");

            printf("Divergence %d-%d\n", z, z + inc);
            
            exit(1);
        }
        printf("==== End test  %d        ======\n\n", z / inc);
    }

}

void digest_parameters(){
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
    
    update_LMUL(1);
    random_LMUL(SEED);    
    printf("LMUL: %d\n", LMUL);
    update_LMUL(2);
    random_LMUL(SEED);    
    printf("LMUL: %d\n", LMUL);
    update_LMUL(4);
    random_LMUL(SEED);    
    printf("LMUL: %d\n", LMUL);
    update_LMUL(8);
    random_LMUL(SEED);    
    printf("LMUL: %d\n", LMUL);    
    exit(0);
}