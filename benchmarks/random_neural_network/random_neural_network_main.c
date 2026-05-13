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

int print_result(int qtd_tests, int passed[MAX_TESTS_PER_HEURISTIC], char label[255][MAX_TESTS_PER_HEURISTIC], 
    int breakline){
    int total_passed = 0;
    for(int j = 0; j < qtd_tests; j++){
        switch(passed[j]){
            case  1: printf("%s: [%c]; ", label[j], 'X');break;
            case -1: printf("%s: [%c]; ", label[j], 'I');break;
            case  0: printf("%s: [%c]; ", label[j], '-');break;
            default: printf("PROBLEM: %d\n", passed[j]);break;
        }
        if(passed[j])
            total_passed++;
        if((j + 1) % breakline == 0 && j != (qtd_tests - 1))
            printf("\n");
    }
    printf("\n%d out of %d tests converged\n", total_passed, qtd_tests);
    return total_passed;
}


/*===== RANDOM TEST FUNCTIONS =====*/
void analyze_results(int passed[QTD_HEURISTICS][MAX_TESTS_PER_HEURISTIC], int qtd_tests[QTD_HEURISTICS]){
    printf("===== ERROR RESULTS ANALISIS =====\n");
    if(PRINTS >= 0){
        printf("Instructions: \n");
        for(int i = 0; i < NUM_RANDOM_OPS; i++)
            printf("%s; (%d, %d, %d)\n", get_OP(ops[i]), rx[i][0], rx[i][1], rx[i][2]);
        printf("t0: %d\n", t0_VALUE);
            printf("\n");
    }
    int total_passed = 0;
    printf("0: Repeat 5x\n");
    char labels[255][MAX_TESTS_PER_HEURISTIC] = {"1", "2", "3", "4", "5"}; 
    total_passed = print_result(qtd_tests[0], passed[0], labels, 5);
    if(total_passed == qtd_tests[0])
        printf("Error is not compulsory\n");
    if(total_passed == 0)
        printf("Error is compulsory\n");
    
    printf("\n");

    printf("1: NOPS between instructiosn\n");
    char labels1[255][MAX_TESTS_PER_HEURISTIC] = {"[NOPS]"};
    total_passed = print_result(qtd_tests[1], passed[1], labels1, 4);

    if(total_passed == qtd_tests[1])
        printf("Problem is probably related to data hazards, put nops between instructions solves the issue\n");
    
    printf("\n");
    
    printf("2: Single instruction execution\n");
    char labels2[255][MAX_TESTS_PER_HEURISTIC] = {"[1]", "[2]", "[3]", "[4]"};
    total_passed = print_result(qtd_tests[2], passed[2], labels2, 4);

    if(total_passed == qtd_tests[2])
        printf("Problem is probably related to multiple instructions interference\n");

    if(total_passed == 3)
    {
        printf("Problem is probably related to a single instruction\n");
        if(wrong_op != -1)
            printf("Probably problematic instruction: %s\n", get_OP(wrong_op));
        else
            printf("Problematic instruction could not be identified\n");

    }
    printf("\n");
    
    printf("3: Delete 1 out of 4 operations\n");
    char labels3[255][MAX_TESTS_PER_HEURISTIC] = {"[- 0]", "[- 1]", "[- 2]", "[- 3]"};
    total_passed = print_result(4, passed[3], labels3, 4);
    printf("3.1: Delete 2 out of 4 operations\n");
    char labels31[255][MAX_TESTS_PER_HEURISTIC] = {"[- 0 1]", "[- 0 2]", "[- 0 3]", "[- 1 2]", "[- 1 3]", "[- 2 3]"};
    total_passed = print_result(6, &passed[3][4], labels31, 6);
    

    printf("\n");
    printf("4: Registers change\n");
    char labels4[255][MAX_TESTS_PER_HEURISTIC] = {"[r = 0 8 16]"};
    total_passed = print_result(qtd_tests[4], passed[4], labels4, 4);
    if(total_passed)
        printf("Problem is related to registers, as changing them solves the problem\n");
    printf("\n");

    char labels5[255][MAX_TESTS_PER_HEURISTIC] = {"[0 1 2 3]", "[0 1 3 2]", "[0 2 1 3]", "[0 2 3 1]", "[0 3 1 2]", 
        "[0 3 2 1]", "[1 0 2 3]", "[1 0 3 2]", "[1 2 0 3]", "[1 2 3 0]", "[1 3 0 2]", "[1 3 2 0]", "[2 0 1 3]", 
        "[2 0 3 1]", "[2 1 0 3]", "[2 1 3 0]", "[2 3 0 1]", "[2 3 1 0]", "[3 0 1 2]", "[3 0 2 1]", "[3 1 0 2]", 
        "[3 1 2 0]", "[3 2 0 1]", "[3 2 1 0]"
    };
    printf("5: Change order of instruction execution\n");
    total_passed = print_result(qtd_tests[5], passed[5], labels5, 4);
    printf("\n");

    printf("6: Different signatures\n");
    char labels6[255][MAX_TESTS_PER_HEURISTIC] = {
        "[0-{0, 0, 0}]", "[0-{0, 0, 1}]", "[0-{0, 1, 0}]", "[0-{0, 1, 1}]", "[0-{0, 1, 2}]",
        "[1-{0, 0, 0}]", "[1-{0, 0, 1}]", "[1-{0, 1, 0}]", "[1-{0, 1, 1}]", "[1-{0, 1, 2}]",
        "[2-{0, 0, 0}]", "[2-{0, 0, 1}]", "[2-{0, 1, 0}]", "[2-{0, 1, 1}]", "[2-{0, 1, 2}]",
        "[3-{0, 0, 0}]", "[3-{0, 0, 1}]", "[3-{0, 1, 0}]", "[3-{0, 1, 1}]", "[3-{0, 1, 2}]"
    };
    total_passed = print_result(qtd_tests[6], passed[6], labels6, 5);
    printf("\n");

    printf("7: first, first to second, first to third...\n");
    char labels7[255][MAX_TESTS_PER_HEURISTIC] = {"[0..0]", "[0..1]", "[0..2]", "[0..3]"};
    total_passed = print_result(qtd_tests[7], passed[7], labels7, 4);
    printf("\n");
}

void error_discoverer(int index){
    int prev_error = error_count;

    int qtd_tests[QTD_HEURISTICS] = {0, 0, 0, 0, 0, 0, 0};
    int passed[QTD_HEURISTICS][MAX_TESTS_PER_HEURISTIC];
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
            passed[0][i] = 1;
        }else{
            passed[0][i] = 0;
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
        passed[1][0] = 1;
    }else{
        passed[1][0] = 0;
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
            passed[2][i] = 1;
        }
        else {
            passed[2][i] = 0;
            if(PRINTS >= 1) printf("Divergence => problem single with instruction\n");
            if(PRINTS >= 3) printf("v = %d %s %d\n", rx[i][1], get_OP(ops[i]), rx[i][2]);
            wrong_op = ops[i];
        }
    }


    // Excluir 1
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
            passed[3][i] = 1;
        }
        else {
            passed[3][i] = 0;
            if(PRINTS >= 1) printf("Divergence\n");
        }
    }

    // Excluir 2
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
                passed[3][qtd_tests[3] - 1] = 1;
            }
            else {
                if(PRINTS >= 1) printf("Divergence\n");
                passed[3][qtd_tests[3] - 1] = 0;
            }
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
        passed[4][0] = 1;
    }
    else{
        passed[4][0] = 0;
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
            passed[5][one] = 1;
        }
        else{
            passed[5][one] = 0;
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
            for(int j = 0; j < 1; j++){
                ADDRESS_VECTOR[j] = add_instruction(op, s, r);
                if(ADDRESS_VECTOR[j] == NOP){
                    invalid_instruction = 1;
                    break;
                }
            }
            if(invalid_instruction){
                passed[6][qtd_tests[6] - 1] = 1;
                break;
            }
            ADDRESS_VECTOR[1] = RET_INSTR;
            execute_RIS(&OUT[index], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);
            if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2){
                //if(PRINTS >= 1) printf("Convergence\n");
                passed[6][qtd_tests[6] - 1] = 1;
            }
            else{
                passed[6][qtd_tests[6] - 1] = 0;
                //if(PRINTS >= 1) printf("Divergence\n");
                if(PRINTS >= 1) printf("PROBLEMATIC OP: %s\n", get_OP(op));  
                printf("Scalar:\n");
                print_regs(&scalar_res[0][0], EL_PER_BLOCK, r);
                printf("Vector:\n");
                print_regs(&vet_res[0][0], EL_PER_BLOCK, r);
            } 

        }
    }
    printf("\n");

    if(PRINTS >= 1) printf("\n===== Heuristic 7 ===== \n");
    int order[4] = {0, 1, 2, 3};
    
    for(int num_ops = 0; num_ops < NUM_RANDOM_OPS; num_ops++)
    {
        if(PRINTS >= 1) printf("Test %d\n\n", num_ops);
        int changed = false;
        int break_for = false;
        qtd_tests[7]++;
        while(true){
            printf("ORDER:", order[0], order[1], order[2], order[3]);
            for(int i = 0; i <= num_ops; i++)
                printf("%d ", order[i]);
            printf("\n");
            load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);

            for(int j = 0; j <= num_ops; j++){
                ADDRESS_VECTOR[j] = add_instruction(ops[order[j]], rx[order[j]], r);
            }
        
            ADDRESS_VECTOR[num_ops + 1] = RET_INSTR;

            execute_RIS(&OUT[index], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);
            if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2){
                if(PRINTS >= 1) printf("Convergence\n");
                passed[7][num_ops] = 1;
                break;
            }else{
                passed[7][num_ops] = 0;
                if(PRINTS >= 1) printf("Divergence\n");
                if(num_ops == NUM_RANDOM_OPS - 1)
                    break;
                if(changed){
                    break_for = true;
                    break;
                }
                else{
                    changed = true;
                    int aux = order[num_ops];
                    order[num_ops] = order[num_ops + 1];
                    order[num_ops + 1] = aux;
                }
                if(PRINTS >= 3){
                    printf("SCALAR:\n");
                    print_regs(&scalar_res[0][0], NUM_REGISTERS, r);
                    printf("VETORIAL:\n");
                    print_regs(&vet_res[0][0], NUM_REGISTERS, r);
                }
            }
        }
        if(break_for)
            break;
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
    update_LMUL(1);
    digest_parameters();

    printf("Doing random batch tests with registers v0-v7 with seed %d\n", SEED);
    random_test(SEED);
    exit(0);
}