#include "random_neural_network.h"

/* ===== RANDOMIZERS ===== */
void generate_initial_values(){
    randomize_vector(OUT, N);
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


/*===== RANDOM TEST FUNCTIONS =====*/
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
            printf("Probably problematic instruction: %s\n", get_OP(wrong_op));
        else
            printf("Problematic instruction could not be identified\n");
    }
}

void error_discoverer(int index){
    int prev_error = error_count;

    int qtd_tests[QTD_HEURISTICS] = {0, 0, 0, 0, 0, 0, 0};
    int passed[QTD_HEURISTICS]    = {0, 0, 0, 0, 0, 0, 0};
    if(PRINTS >= 1) printf("\n===== Heuristic 0 ===== \n");
    for(int i = 0; i < 5; i++){
        qtd_tests[0]++;
        load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);

        for(int j = 0; j < NUM_RANDOM_OPS; j++){
            ADDRESS_VECTOR[j] = add_instruction(ops[j], rx[j], r);
        }
    
        ADDRESS_VECTOR[NUM_RANDOM_OPS] = RET_INSTR;

        execute_RIS(&OUT[index], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);
        if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2){
            if(PRINTS >= 1) printf("Convergence\n");
            passed[0]++;
        }else{
            if(PRINTS >= 1) printf("Divergence\n");
        }
        if(PRINTS >= 3){
            printf("SCALAR:\n");
            print_regs(&scalar_res[0][0], NUM_REGISTERS, r);
            printf("VETORIAL:\n");
            print_regs(&vet_res[0][0], NUM_REGISTERS, r);
        }
    }

    if(PRINTS >= 1) printf("\n===== Heuristic 1 ===== \n");
    qtd_tests[1] = 1;
    int qtd_nops = 32;
    load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);
    for(int i = 0; i < NUM_RANDOM_OPS; i++){
        ADDRESS_VECTOR[i * (qtd_nops + 1)] = add_instruction(ops[i], rx[i], r);
        for(int j = 1; j <= qtd_nops; j++) ADDRESS_VECTOR[j + i * (qtd_nops + 1)] = add_instruction(NOP, rx[0], r);
        
    }
    ADDRESS_VECTOR[NUM_RANDOM_OPS * (qtd_nops + 1)] = RET_INSTR;
    execute_RIS(&OUT[index], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);

    if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2){
        if(PRINTS >= 1) printf("Convergence, probably data hazard problem\n");
        passed[1]++;
    }else{
        if(PRINTS >= 1) printf("Divergence => proceed to next test\n");
        if(PRINTS >= 3) print_regs(&scalar_res[0][0], NUM_REGISTERS, r);
        if(PRINTS >= 3) print_regs(&vet_res[0][0], NUM_REGISTERS, r);

    }

    if(PRINTS >= 1) printf("\n===== Heuristic 2 ===== \n\n");
    for(int i = 0; i < 4; i++){
        qtd_tests[2]++;        
        load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);
        ADDRESS_VECTOR[0] = add_instruction(ops[i], rx[i], r);
        ADDRESS_VECTOR[1] = RET_INSTR;
        execute_RIS(&OUT[index], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);
        if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2){
            if(PRINTS >= 1) printf("Convergence\n");
            passed[2]++;
        }
        else {
            if(PRINTS >= 1) printf("Divergence => problem single with instruction\n");
            if(PRINTS >= 3) printf("v = %d %s %d\n", rx[i][1], get_OP(ops[i]), rx[i][2]);
            wrong_op = ops[i];
        }
    }


    // Excluir 2
    if(PRINTS >= 1) printf("\n===== Heuristic 3 ===== \n\n");
    for(int i = 0; i < 4; i++){
        if(PRINTS >= 1) printf("Removed instruction %d\n", i);
        qtd_tests[3]++;        
        load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);
        for(int j = 0; j < 4; j++){
            if(j == i)
                continue;
            if (j < i)
                ADDRESS_VECTOR[j] = add_instruction(ops[j], rx[j], r);
            else
                ADDRESS_VECTOR[j - 1] = add_instruction(ops[j], rx[j], r);
        }
        ADDRESS_VECTOR[3] = RET_INSTR;
        execute_RIS(&OUT[index], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);
        if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2){
            if(PRINTS >= 1) printf("Convergence\n");
            passed[3]++;
        }
        else {
            if(PRINTS >= 1) printf("Divergence\n");
        }
    }

    // Excluir 3
    if(PRINTS >= 1) printf("\n===== Heuristic 3.1 ===== \n\n");
    for(int i = 0; i < 4; i++)
        for(int j = i + 1; j < 4; j++){
            qtd_tests[3]++;
            if(PRINTS >= 1) printf("Removed instruction %d %d\n", i, j);        
            load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);
            int t = 0;
            for(int z = 0; z < 4; z++){
                if(z == i || z == j)
                    continue;
                ADDRESS_VECTOR[t] = add_instruction(ops[z], rx[z], r);
                t++;
            }
            ADDRESS_VECTOR[2] = RET_INSTR;
            execute_RIS(&OUT[index], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);
            if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2){
                if(PRINTS >= 1) printf("Convergence\n");
                passed[3]++;
            }
            else if(PRINTS >= 1) printf("Divergence\n");
        }
        
    // Registradores
    if(PRINTS >= 1) printf("\n===== Heuristic 4 ===== \n\n");
    prev_error = error_count;
    qtd_tests[4] = 1;
    int other_r[3] = {0, 8, 24};
    load_init_values_scalar(&OUT[index], other_r, NUM_REGISTERS);
    ADDRESS_VECTOR[0] = add_instruction(ops[0], rx[0], other_r);
    ADDRESS_VECTOR[1] = add_instruction(ops[1], rx[1], other_r);
    ADDRESS_VECTOR[2] = add_instruction(ops[2], rx[2], other_r);
    ADDRESS_VECTOR[3] = add_instruction(ops[3], rx[3], other_r);
    ADDRESS_VECTOR[4] = RET_INSTR;
    execute_RIS(&OUT[index], other_r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);
    if(compare_solutions(prev_error, other_r, &vet_res[0][0]) == 2){
        if(PRINTS >= 1) printf("Convergence\n\n");
        passed[4]++;
    }
    else{
        if(PRINTS >= 1) printf("Divergence\n\n");
        if(PRINTS >= 3) print_regs(&scalar_res[0][0], NUM_REGISTERS, other_r);;
        if(PRINTS >= 3) print_regs(&vet_res[0][0], 3, other_r);
    } 
    
    if(PRINTS >= 1) printf("\n===== Heuristic 5 ===== \n\n");

    for(int one = 0; one < 24; one++){
        qtd_tests[5]++;
        int perm[4];
        get_permutation(one, 4, &perm[0]);
        if(PRINTS >= 1) printf("PERMUTATION %d %d %d %d\n", perm[0], perm[1], perm[2], perm[3]);
        load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);
        ADDRESS_VECTOR[0] = add_instruction(ops[perm[0]], rx[perm[0]], r);
        ADDRESS_VECTOR[1] = add_instruction(ops[perm[1]], rx[perm[1]], r);
        ADDRESS_VECTOR[2] = add_instruction(ops[perm[2]], rx[perm[2]], r);
        ADDRESS_VECTOR[3] = add_instruction(ops[perm[3]], rx[perm[3]], r);
        ADDRESS_VECTOR[4] = RET_INSTR;
        execute_RIS(&OUT[index], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);
        if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2){
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
            load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);
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
            execute_RIS(&OUT[index], other_r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);
            if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2){
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
        load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);

        for(int j = 0; j <= i; j++){
            ADDRESS_VECTOR[j] = add_instruction(ops[j], rx[j], r);
        }
    
        ADDRESS_VECTOR[i + 1] = RET_INSTR;

        execute_RIS(&OUT[index], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);
        if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2){
            if(PRINTS >= 1) printf("Convergence\n");
            passed[7]++;
        }else{
            if(PRINTS >= 1) printf("Divergence\n");
        }
        if(PRINTS >= 3){
            printf("SCALAR:\n");
            print_regs(&scalar_res[0][0], NUM_REGISTERS, r);
            printf("VETORIAL:\n");
            print_regs(&vet_res[0][0], NUM_REGISTERS, r);
        }
    }
    printf("\n");

    analyze_results(passed, qtd_tests);
    
}

void generate_RIS(int index){
    shuffle_registers(r, NUM_REGISTERS, LMUL);
    randomize_instructions();
    load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);

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
        int prev_error = error_count;
        printf("==== Begginning test  %d ======\n", z / inc);

        generate_RIS(z);
        execute_RIS(&OUT[z], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);

        if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2){
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
                printf("SCALAR MATRIX:\n");
                print_regs(&scalar_res[0][0], NUM_REGISTERS, r);
                
                printf("OUTPUT from vector:\n");
                print_regs(&vet_res[0][0], NUM_REGISTERS, r);
                printf("\n");
            }
            
            error_discoverer(z);
            exit(0);
        }
        printf("==== End test  %d        ======\n\n", z / inc);
    }

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