#include "benchmark_LMUL.h"

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

void add_instruction_prevent_NOP(int i){
    int instr = 0;
    instr = add_instruction(ops[i], rx[i], r);
    if (instr == NOP){
        rx[i][0] = 0;
        rx[i][1] = 1;
        rx[i][2] = 2;
        instr = add_instruction(ops[i], rx[i], r);
    }
    if (instr == NOP){
        rx[i][0] = 0;
        rx[i][1] = 2;
        rx[i][2] = 1;
        instr = add_instruction(ops[i], rx[i], r);
    }
    if (instr == NOP){
        rx[i][0] = 1;
        rx[i][1] = 0;
        rx[i][2] = 2;
        instr = add_instruction(ops[i], rx[i], r);
    }
    if (instr == NOP){
        rx[i][0] = 1;
        rx[i][1] = 2;
        rx[i][2] = 0;
        instr = add_instruction(ops[i], rx[i], r);
    }
    if (instr == NOP){
        rx[i][0] = 2;
        rx[i][1] = 1;
        rx[i][2] = 0;
        instr = add_instruction(ops[i], rx[i], r);
    }
    if (instr == NOP){
        rx[i][0] = 2;
        rx[i][1] = 0;
        rx[i][2] = 1;
        instr = add_instruction(ops[i], rx[i], r);
    }
    if (instr == NOP){
        instr = RET_INSTR;
        return;
    }

    ADDRESS_VECTOR8[i] = instr;
}

void add_all_instructions(int index){
    update_LMUL(8);
    int delta_cyc8, delta_cyc4, delta_cyc2, delta_cyc1;
    int delta_ins8, delta_ins4, delta_ins2, delta_ins1;


    load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);
    for(int i = 0; i < NUM_RANDOM_OPS; i++){
        add_instruction_prevent_NOP(i);
    }
    ADDRESS_VECTOR8[NUM_RANDOM_OPS] = RET_INSTR;
    execute_RIS_capture_benchmarks(&OUT[index], r, ADDRESS_VECTOR8, &vet_res8[0][0], NUM_REGISTERS,
    &delta_cyc8, &delta_ins8);
    
    int rs[8][3] = {
        {r[0], r[1], r[2]}, 
        {r[0] + 1, r[1] + 1, r[2] + 1},
        {r[0] + 2, r[1] + 2, r[2] + 2},
        {r[0] + 3, r[1] + 3, r[2] + 3},
        {r[0] + 4, r[1] + 4, r[2] + 4},
        {r[0] + 5, r[1] + 5, r[2] + 5},
        {r[0] + 6, r[1] + 6, r[2] + 6},
        {r[0] + 7, r[1] + 7, r[2] + 7},
    };
    int r4[NUM_REGISTERS * 2] = {r[0], r[0] + 4, r[1], r[1] + 4, r[2], r[2] + 4};
    int r2[NUM_REGISTERS * 4] = {
        r[0], r[0] + 2, r[0] + 4, r[0] + 6,
        r[1], r[1] + 2, r[1] + 4, r[1] + 6, 
        r[2], r[2] + 2, r[2] + 4, r[2] + 6
    };
    int r1[NUM_REGISTERS * 8] = {
        r[0], r[0] + 1, r[0] + 2, r[0] + 3, r[0] + 4, r[0] + 5, r[0] + 6, r[0] + 7,
        r[1], r[1] + 1, r[1] + 2, r[1] + 3, r[1] + 4, r[1] + 5, r[1] + 6, r[1] + 7,
        r[2], r[2] + 1, r[2] + 2, r[2] + 3, r[2] + 4, r[2] + 5, r[2] + 6, r[2] + 7,
    };
    update_LMUL(4);
    load_init_values_scalar(&OUT[index], r4, NUM_REGISTERS * 2);
    for(int i = 0; i < NUM_RANDOM_OPS; i++){
        for(int j = 0; j < 2; j++)
            ADDRESS_VECTOR4[2 * i + j] = add_instruction(ops[i], rx[i], rs[4 * j]);
    }
    ADDRESS_VECTOR4[2 * NUM_RANDOM_OPS] = RET_INSTR;
    execute_RIS_capture_benchmarks(&OUT[index], r4, ADDRESS_VECTOR4, &vet_res4[0][0], NUM_REGISTERS * 2,
    &delta_cyc4, &delta_ins4);
    
    update_LMUL(2);
    load_init_values_scalar(&OUT[index], r2, NUM_REGISTERS * 4);
    for(int i = 0; i < NUM_RANDOM_OPS; i++){
        for(int j = 0; j < 4; j++)
            ADDRESS_VECTOR2[4 * i + j] = add_instruction(ops[i], rx[i], rs[2 * j]);
    }
    ADDRESS_VECTOR2[4 * NUM_RANDOM_OPS] = RET_INSTR;
    execute_RIS_capture_benchmarks(&OUT[index], r2, ADDRESS_VECTOR2, &vet_res2[0][0], NUM_REGISTERS * 4,
    &delta_cyc2, &delta_ins2);
    
    update_LMUL(1);
    load_init_values_scalar(&OUT[index], r1, NUM_REGISTERS * 8);
    for(int i = 0; i < NUM_RANDOM_OPS; i++){
        for(int j = 0; j < 8; j++)
        ADDRESS_VECTOR1[8 * i + j] = add_instruction(ops[i], rx[i], rs[j]); 
    }
    ADDRESS_VECTOR1[8 * NUM_RANDOM_OPS] = RET_INSTR;
    execute_RIS_capture_benchmarks(&OUT[index], r1, ADDRESS_VECTOR1, &vet_res1[0][0], NUM_REGISTERS * 8,
    &delta_cyc1, &delta_ins1);

    printf("DC1 %d %d\n", delta_cyc1, delta_ins1);
    printf("DC2 %d %d\n", delta_cyc2, delta_ins2);
    printf("DC4 %d %d\n", delta_cyc4, delta_ins4);
    printf("DC8 %d %d\n", delta_cyc8, delta_ins8);
    
}

void generate_and_execute_RIS(int index){
    int prev_error = error_count;

    update_LMUL(8);
    shuffle_registers(r, NUM_REGISTERS, 8);
    randomize_instructions();

    if(PRINTS >= 2)printf("STEP BY STEP RESULTS: \n");
    add_all_instructions(index);
    
    update_LMUL(8);
    if(
        compare_solutions(prev_error, r, &vet_res8[0][0]) == 2 && 
        compare_solutions(prev_error, r, &vet_res4[0][0]) == 2 &&
        compare_solutions(prev_error, r, &vet_res2[0][0]) == 2 &&
        compare_solutions(prev_error, r, &vet_res1[0][0]) == 2
){
        printf("Convergence %d-%d\n", index, index + VLEN / SEW * 8 * NUM_REGISTERS);
    }else{
        for(int i = 0; i < NUM_RANDOM_OPS; i++)
            printf("%s; (%d, %d, %d)\n", get_OP_name(ops[i]), r[rx[i][0]], r[rx[i][1]], r[rx[i][2]]);
        printf("===== Scalar =====\n");
        print_vector(&scalar_res[0][0], 32 * VLEN / SEW, VLEN / SEW);
        printf("===== Vector 2 =====\n");
        print_vector(&vet_res2[0][0], 32 * VLEN / SEW, VLEN / SEW);
        printf("===== Vector 4 =====\n");
        print_vector(&vet_res4[0][0], 32 * VLEN / SEW, VLEN / SEW);
        printf("===== Vector 8 =====\n");
        print_vector(&vet_res8[0][0], 32 * VLEN / SEW, VLEN / SEW);
        printf("===== END =====\n");

        printf("Divergence %d-%d\n", index, index + VLEN / SEW * 8 * NUM_REGISTERS);
        
        exit(1);
    }

}

void random_LMUL(int seed){
    set_vet_settings();

    msrand(seed);
    generate_initial_values();
    printf("Done init values\n");
    
    msrand(seed);
    int inc = NUM_REGISTERS * VLEN / SEW * 8;
    for(int z = 0; z + inc <= N; z+= inc){
        printf("==== Begginning test  %d ======\n", z / inc);
        generate_and_execute_RIS(z);
        printf("==== End test  %d        ======\n\n", z / inc);
    }

}

void digest_parameters(){
    if (parameter.argc > 0){
        SEED = parameter.argv[0];
    }

    if(parameter.argc > 1)
        N = parameter.argv[1];

    if(parameter.argc >= 2 + SUPORTED_INSTRUCTIONS){
        fixed_suported_instructions = 0;
        for(int i = 0; i < SUPORTED_INSTRUCTIONS; i++){

            allowed_instructions[i] = parameter.argv[2 + i];

            if(allowed_instructions[i]) fixed_suported_instructions++;
        }
    }else{
        for(int i = 0; i < SUPORTED_INSTRUCTIONS; i++){
            allowed_instructions[i] = 1;
        }
    }
    
}

int main(){
    PRINTS = 0;
    digest_parameters();
    random_LMUL(SEED);    
    
    exit(0);
}