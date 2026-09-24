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
        
        rx[i][0] = mrand() % NUM_REGISTERS; rx[i][1] = mrand() % NUM_REGISTERS; rx[i][2] 
        = mrand() % NUM_REGISTERS;
    }
}

#pragma GCC push_options
#pragma GCC optimize ("no-tree-vectorize")
#pragma GCC optimize ("no-slp-vectorize")
int print_result(int qtd_tests, PASSED_DATATYPE passed[MAX_TESTS_PER_HEURISTIC], 
    char label[MAX_TESTS_PER_HEURISTIC][20], int breakline){
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
#pragma GCC pop_options

int check_cold_start(PASSED_DATATYPE* vet, int N){
    int previous = 0;
    for(int i = 0; i < N; i++){
        if(vet[i] == 0 && previous == 1)
            return 0;
        previous = vet[i];
    }
    return 1;
}

void print_complete_instruction(int op, int reg1, int reg2, int reg3){
    char* str = get_OP_name(op);
    printf("%s v%d, v%d, v%d\n", str, reg1, reg2, reg3);
}

/*===== RANDOM TEST FUNCTIONS =====*/
#pragma GCC optimize ("no-tree-vectorize")
void analyze_results(PASSED_DATATYPE passed[QTD_HEURISTICS][MAX_TESTS_PER_HEURISTIC], 
    int qtd_tests[QTD_HEURISTICS]){    
    
    printf("===== ERROR RESULTS ANALISIS =====\n");
    if(PRINTS >= 0){
        printf("Instructions: \n");
        for(int i = 0; i < NUM_RANDOM_OPS; i++)
            printf("%s; (%d, %d, %d)\n", get_OP_name(ops[i]), rx[i][0], rx[i][1], 
            rx[i][2]);
        printf("r: %d %d %d\n", r[0], r[1], r[2]);
        printf("t0: %d\n", t0_VALUE);
        printf("\n");
    }
    int total_passed = 0;
    printf("%d \n", h0_qtd_repeats);
    printf("0: Repeat %dx\n", h0_qtd_repeats);
    const char labels[MAX_TESTS_PER_HEURISTIC][20];
    
    for (int j = 0; j < MAX_TESTS_PER_HEURISTIC; j++) {
        sprintf(labels[j], "%d", j);
    } 
    total_passed = print_result(qtd_tests[0], passed[0], labels, 10);
    if(compulsory)
        printf("Error is compulsory\n");
    else{
        printf("Error is not compulsory\n");

        if(check_cold_start(passed[0], h0_qtd_repeats))
            printf("Could be cold start problem\n");
        else
            printf("Non deterministic random behavior detected\n");
    }
    
    printf("\n");

    printf("1: NOPS between instructions\n");
    char labels1[MAX_TESTS_PER_HEURISTIC][20] = {"[NOPS]"};
    total_passed = print_result(qtd_tests[1], passed[1], labels1, 4);

    if(total_passed == qtd_tests[1])
        printf("Problem is probably related to data hazards, put nops between \
            instructions solves the issue\n");
    
    printf("\n");
    
    printf("2: Single instruction execution\n");
    char labels2[MAX_TESTS_PER_HEURISTIC][20] = {"[1]", "[2]", "[3]", "[4]"};
    total_passed = print_result(qtd_tests[2], passed[2], labels2, 4);

    if(total_passed == qtd_tests[2])
        printf("Problem is probably related to multiple instructions interference\n");

    int single_instruction_fail = total_passed == 3;
    if(total_passed < 3){
        int previous = wrong_op[0];
        single_instruction_fail = 1;
        for(int i = 1; i < 4 && wrong_op[i] != -1; i++){
            if(previous != wrong_op[i])
                single_instruction_fail = 0;
            break;
        }
    }

    if(single_instruction_fail)
    {
        printf("Problem is probably related to a single instruction\n");
        printf("Probably problematic instruction: %s\n", get_OP_name(ops[wrong_op[0]]));
    }else
        if(total_passed < 3){
            printf("Problem with multiple instructions \nProblematic instructions:\n");
            for(int i = 0; i < 4 && wrong_op[i] != -1; i++){
                printf("Instruction %s;\n", get_OP_name(ops[wrong_op[i]]));
            }
        }

    printf("\n");
    
    printf("3: Delete 1 out of 4 operations\n");
    static const char labels3[MAX_TESTS_PER_HEURISTIC][20] = {"[- 0]", "[- 1]", "[- 2]", 
        "[- 3]"};
    total_passed = print_result(4, passed[3], labels3, 4);
    printf("3.1: Delete 2 out of 4 operations\n");
    static const char labels31[MAX_TESTS_PER_HEURISTIC][20] = {"[- 0 1]", "[- 0 2]", 
        "[- 0 3]", "[- 1 2]", "[- 1 3]", "[- 2 3]"};
    total_passed = print_result(6, &passed[3][4], labels31, 6);


    printf("\n");
    printf("4: Registers change\n");
    static const char labels4[MAX_TESTS_PER_HEURISTIC][20] = {"[r = 0 8 16]"};
    total_passed = print_result(qtd_tests[4], passed[4], labels4, 4);
    if(total_passed)
        printf("Problem is related to registers, as changing them solves the problem\n");
    printf("\n");

    static const char labels5[MAX_TESTS_PER_HEURISTIC][20] = {"[0 1 2 3]", "[0 1 3 2]", 
        "[0 2 1 3]", "[0 2 3 1]", "[0 3 1 2]", "[0 3 2 1]", "[1 0 2 3]", "[1 0 3 2]", 
        "[1 2 0 3]", "[1 2 3 0]", "[1 3 0 2]", "[1 3 2 0]", "[2 0 1 3]", "[2 0 3 1]", 
        "[2 1 0 3]", "[2 1 3 0]", "[2 3 0 1]", "[2 3 1 0]", "[3 0 1 2]", "[3 0 2 1]",
        "[3 1 0 2]", "[3 1 2 0]", "[3 2 0 1]", "[3 2 1 0]"
    };
    printf("5: Change order of instruction execution\n");
    total_passed = print_result(qtd_tests[5], passed[5], labels5, 4);
    printf("\n");

    printf("6: Different signatures\n");
    static const char labels6[MAX_TESTS_PER_HEURISTIC][20] = {
        "[0-{0, 0, 0}]", "[0-{0, 0, 1}]", "[0-{0, 1, 0}]", "[0-{0, 1, 1}]", "[0-{0, 1, 2}]",
        "[1-{0, 0, 0}]", "[1-{0, 0, 1}]", "[1-{0, 1, 0}]", "[1-{0, 1, 1}]", "[1-{0, 1, 2}]",
        "[2-{0, 0, 0}]", "[2-{0, 0, 1}]", "[2-{0, 1, 0}]", "[2-{0, 1, 1}]", "[2-{0, 1, 2}]",
        "[3-{0, 0, 0}]", "[3-{0, 0, 1}]", "[3-{0, 1, 0}]", "[3-{0, 1, 1}]", "[3-{0, 1, 2}]"
    };
    total_passed = print_result(qtd_tests[6], passed[6], labels6, 5);
    printf("\n");

    printf("7: first, first to second, first to third...\n");
    static const char labels7[MAX_TESTS_PER_HEURISTIC][20] = {"[0..0]", "[0..1]", 
        "[0..2]", "[0..3]"};
    total_passed = print_result(qtd_tests[7], passed[7], labels7, 4);
    printf("\n");
    
    printf("Total all instructions:\n");
    print_complete_instruction(ops[0], r[rx[0][0]], r[rx[0][1]], r[rx[0][2]]);
    print_complete_instruction(ops[1], r[rx[1][0]], r[rx[1][1]], r[rx[1][2]]);
    print_complete_instruction(ops[2], r[rx[2][0]], r[rx[2][1]], r[rx[2][2]]);
    print_complete_instruction(ops[3], r[rx[3][0]], r[rx[3][1]], r[rx[3][2]]);
    printf("\n");
    printf("Minimized instruction set with %d instructions: \n", minimum_sequence_length);

    for(int i = 0; i < minimum_sequence_length; i++){
        int idx = minimum_sequence[i];
        print_complete_instruction(ops[idx], r[rx[idx][0]], r[rx[idx][1]], r[rx[idx][2]]);
    }
    //if(RAW )
    if(RAW)
        printf("Possible Read after Write data hazard detected\n");
    if(WAW)
        printf("Possible Write after Write data hazard detected\n");

    const char labels8[MAX_TESTS_PER_HEURISTIC][20];
    
    for (int j = 0; j < h8_qtd_repeats; j++) {
        sprintf(labels8[j], "%d", j);
    }
    total_passed = print_result(qtd_tests[8], passed[8], labels8, 10);
    
    printf("Chance of instruction been wrong: %d\%\n", 100 - 100 * total_passed / h8_qtd_repeats);
    
    /*
    int a1 = minimum_sequence[0], a2 = minimum_sequence[1], a3 = minimum_sequence[2];
    switch (minimum_sequence_length){
        case 1:
            printf("Minimum error sequence is a single instruction: \n");
            print_complete_instruction(ops[a1], r[rx[a1][0]], r[rx[a1][1]], r[rx[a1][2]]);
            break;
        case 2:
            printf("Minimum error sequence is a 2 instruction sequence:\n");
            print_complete_instruction(ops[a1], r[rx[a1][0]], r[rx[a1][1]], r[rx[a1][2]]);
            print_complete_instruction(ops[a2], r[rx[a2][0]], r[rx[a2][1]], r[rx[a2][2]]);

            if(check_for_RAW_2(rx[a1], rx[a2]))
                printf("Possible Read after Write data hazard detected\n");
            if(check_for_WAW_2(rx[a1], rx[a2]))
                printf("Possible Write after Write data hazard detected\n");
            break;
        case 3:
            printf("Minimum error sequence is a 3 instruction sequence: \n");
            print_complete_instruction(ops[a1], r[rx[a1][0]], r[rx[a1][1]], r[rx[a1][2]]);
            print_complete_instruction(ops[a2], r[rx[a2][0]], r[rx[a2][1]], r[rx[a2][2]]);
            print_complete_instruction(ops[a3], r[rx[a3][0]], r[rx[a3][1]], r[rx[a3][2]]);
        
            if(check_for_RAW_3(rx[a1], rx[a2], rx[a3]))
                printf("Possible Read after Write data hazard detected\n");
            if(check_for_WAW_3(rx[a1], rx[a2], rx[a3]))
                printf("Possible Write after Write data hazard detected\n");
            break;
        case 4:
            a1 = ops[0];a2 = ops[1]; a3 = ops[2]; int a4 = ops[3];
            printf("Minimum error sequence is the entire sequence: \n");
            print_complete_instruction(ops[0], r[rx[0][0]], r[rx[0][1]], r[rx[0][2]]);
            print_complete_instruction(ops[1], r[rx[1][0]], r[rx[1][1]], r[rx[1][2]]);
            print_complete_instruction(ops[2], r[rx[2][0]], r[rx[2][1]], r[rx[2][2]]);
            print_complete_instruction(ops[3], r[rx[3][0]], r[rx[3][1]], r[rx[3][2]]);

            if(check_for_RAW_4(rx[a1], rx[a2], rx[a3], rx[a4]))
                printf("Possible Read after Write data hazard detected\n");
            if(check_for_WAW_4(rx[a1], rx[a2], rx[a3], rx[a4]))
                printf("Possible Write after Write data hazard detected\n");
            break;
    }*/
}

#pragma GCC push_options
#pragma GCC optimize ("no-tree-vectorize")
#pragma GCC optimize ("no-slp-vectorize")
void error_discoverer(int index){

    minimum_sequence[0] = 0;
    minimum_sequence[1] = 1;
    minimum_sequence[2] = 2;
    minimum_sequence[3] = 3;
    minimum_sequence_length = 4;

    int prev_error = error_count;

    // HEURISTIC 0
    int qtd_tests[QTD_HEURISTICS] = {0, 0, 0, 0, 0, 0, 0};
    PASSED_DATATYPE passed[QTD_HEURISTICS][MAX_TESTS_PER_HEURISTIC];
    if(PRINTS >= 1) printf("\n===== Heuristic 0 ===== \n");
    for(int i = 0; i < h0_qtd_repeats; i++){
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
            h0_qtd_repeats = 1024;
            compulsory = 0;
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
    // END HEURISTIC 0

    // HEURISTIC 1
    if(PRINTS >= 1) printf("\n===== Heuristic 1 ===== \n");
    qtd_tests[1] = 1;
    int qtd_nops = 32;
    load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);
    for(int i = 0; i < NUM_RANDOM_OPS; i++){
        ADDRESS_VECTOR[i * (qtd_nops + 1)] = add_instruction(ops[i], rx[i], r);
        for(int j = 1; j <= qtd_nops; j++) 
            ADDRESS_VECTOR[j + i * (qtd_nops + 1)] = add_instruction(NOP, rx[0], r);
        
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
    // END HEURISTIC 1
    // HEURISTIC 2
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
            if(PRINTS >= 3) 
            printf("v = %d %s %d\n", rx[i][1], get_OP_name(ops[i]), rx[i][2]);
            int j = 0;
            for(; wrong_op[j] != -1; j++);
            wrong_op[j] = i;
        }
    }

    if(wrong_op[0] != -1){
        minimum_sequence[0] = wrong_op[0];
        minimum_sequence[1] = -1;
        minimum_sequence[2] = -1;
        minimum_sequence[3] = -1;
        minimum_sequence_length = 1;
    }

    // END HEURISTIC 2
    // HEURISTIC 3

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

    if(minimum_sequence_length > 3){
        for(int i = 0; i < 4; i++){
            if(!passed[3][i]){
                printf("3: AQUI %d\n", i);
                minimum_sequence[0] = (i == 0)? 1: 0;
                minimum_sequence[1] = (i > 1)?  1: 2;
                minimum_sequence[2] = (i > 2)?  2: 3;
                minimum_sequence[3] = -1;
                minimum_sequence_length = 3;
                break;
            }
        }
    }

    // END HEURISTIC 3
    // HEURISTIC 3.1

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
    if(minimum_sequence_length >= 2){
        for(int i = 0; i < 6; i++){
            if(!passed[3][4 + i]){
                printf("3.1: AQUI\n");
                minimum_sequence[2] = -1;
                if (i >= 3){
                    minimum_sequence[0] = 0;
                    if(i == 3)
                        minimum_sequence[1] = 3;
                    if(i == 4)
                        minimum_sequence[1] = 2;
                    if(i == 3)
                        minimum_sequence[1] = 1;
                }
                else{
                    if (i == 0){
                        minimum_sequence[0] = 2;
                        minimum_sequence[1] = 3;
                    }else{
                        minimum_sequence[0] = 1;
                        if(i == 1)
                            minimum_sequence[1] = 3;
                        else
                            minimum_sequence[1] = 2;
                    }

                }
                minimum_sequence[2] = -1;
                minimum_sequence[3] = -1;
                minimum_sequence_length = 2;
                break;
            }
        }
    }
        
    // HEURISTIC 3.1
    // HEURISTIC 4

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

    // END HEURISTIC 4
    // HEURISTIC 5

    
    if(PRINTS >= 1) printf("\n===== Heuristic 5 ===== \n\n");

    for(int one = 0; one < 24; one++){
        qtd_tests[5]++;
        int perm[4];
        get_permutation(one, 4, &perm[0]);
        if(PRINTS >= 1) 
            printf("PERMUTATION %d %d %d %d\n", perm[0], perm[1], perm[2], perm[3]);
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

    // END HEURISTIC 5
    // HEURISTIC 6

    if(PRINTS >= 1) printf("\n===== Heuristic 6 ===== \n");
    for(int i = 0; i < NUM_RANDOM_OPS; i++){
        static const int signatures[5][3] = {
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
                if(PRINTS >= 1) {
                    printf("PROBLEMATIC OP: %s\n", get_OP_name(op));  
                    printf("Scalar:\n");
                    print_regs(&scalar_res[0][0], EL_PER_BLOCK, r);
                    printf("Vector:\n");
                    print_regs(&vet_res[0][0], EL_PER_BLOCK, r);
                }
            } 

        }
    }
    printf("\n");

    // END HEURISTIC 6
    // HEURISTIC 7

    if(PRINTS >= 1) printf("\n===== Heuristic 7 ===== \n");
    int order[4] = {0, 1, 2, 3};
    
    for(int num_ops = 0; num_ops < NUM_RANDOM_OPS; num_ops++)
    {
        if(PRINTS >= 1) printf("Test %d\n\n", num_ops);
        int changed = false;
        int break_for = false;
        qtd_tests[7]++;
        while(true){
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
    // END HEURISTIC 7

    // HEURISTIC 8 - Dependency graph

    qtd_tests[8] = 16;
    load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);

    int i1, i2, i3, i4;
    //int xd1, xi1, xj1;
    //int xd2, xi2, xj2;
    //int xd3, xi3, xj3; 
    //int xd4, xi4, xj4;
    switch (minimum_sequence_length){
        case 1:
            i1 = minimum_sequence[0];
            ADDRESS_VECTOR[0] = add_instruction(ops[i1], rx[i1], r);
            ADDRESS_VECTOR[1] = RET_INSTR;

            break;
        case 2:
            i1 = minimum_sequence[0]; i2 = minimum_sequence[1];
        
            ADDRESS_VECTOR[0] = add_instruction(ops[i1], rx[i1], r);
            ADDRESS_VECTOR[1] = add_instruction(ops[i2], rx[i2], r);
            ADDRESS_VECTOR[2] = RET_INSTR;

            //xd1 = rx[i1][0]; xi1 = rx[i1][1]; xj1 = rx[i1][2];
            //xd2 = rx[i2][0]; xi2 = rx[i2][1]; xj2 = rx[i2][2];

            if (check_for_RAW_2(rx[i1], rx[i2]))
                RAW = 1;
            if (check_for_WAW_2(rx[i1], rx[i2]))
                WAW = 1;
            /*if ((xd1 == xd2){
                WAW = 1;
                if(xd1 == xi2 || xd1 == xj2){
                    RAW = 1;
                }
            } else if(xd1 == xi2 || xd1 == xj2){
                RAW = 1;
            })*/

            break;
        case 3:
            i1 = minimum_sequence[0]; i2 = minimum_sequence[1]; i3 = minimum_sequence[2];
            ADDRESS_VECTOR[0] = add_instruction(ops[i1], rx[i1], r);
            ADDRESS_VECTOR[1] = add_instruction(ops[i2], rx[i2], r);
            ADDRESS_VECTOR[2] = add_instruction(ops[i3], rx[i3], r);
            ADDRESS_VECTOR[3] = RET_INSTR;

            //xd1 = rx[i1][0]; xi1 = rx[i1][1]; xj1 = rx[i1][2];
            //xd2 = rx[i2][0]; xi2 = rx[i2][1]; xj2 = rx[i2][2];
            //xd3 = rx[i3][0]; xi3 = rx[i3][1]; xj3 = rx[i3][2];

            if (check_for_RAW_3(rx[i1], rx[i2], rx[i3]))
                RAW = 1;
            if (check_for_WAW_3(rx[i1], rx[i2], rx[i3]))
                WAW = 1;

            break;
        case 4:
            i1 = minimum_sequence[0]; i2 = minimum_sequence[1]; i3 = minimum_sequence[2]; 
            i4 = minimum_sequence[3];
            ADDRESS_VECTOR[0] = add_instruction(ops[i1], rx[i1], r);
            ADDRESS_VECTOR[1] = add_instruction(ops[i2], rx[i2], r);
            ADDRESS_VECTOR[2] = add_instruction(ops[i3], rx[i3], r);
            ADDRESS_VECTOR[3] = add_instruction(ops[i4], rx[i4], r);
            ADDRESS_VECTOR[4] = RET_INSTR;

            //xd1 = rx[i1][0]; xi1 = rx[i1][1]; xj1 = rx[i1][2];
            //xd2 = rx[i2][0]; xi2 = rx[i2][1]; xj2 = rx[i2][2];
            //xd3 = rx[i3][0]; xi3 = rx[i3][1]; xj3 = rx[i3][2];
            //xd4 = rx[i4][0]; xi4 = rx[i4][1]; xj4 = rx[i4][2];

            if (check_for_RAW_4(rx[i1], rx[i2], rx[i3], rx[i4]))
                RAW = 1;
            if (check_for_WAW_4(rx[i1], rx[i2], rx[i3], rx[i4]))
                WAW = 1;

            break;
    }

    for(int i = 0; i < h8_qtd_repeats; i++){
        execute_RIS(&OUT[index], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);
        if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2){
            passed[8][i] = 1;
            h8_qtd_repeats = 1024;
            
            qtd_tests[8] = 1024;
        }
        else{
            passed[8][i] = 0;
            printf("DID NOT CONVERGE\n");
        }
    }
    
    // END HEURISTIC 8

    analyze_results(passed, qtd_tests);
}
#pragma GCC pop_options

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

void random_test() {    
    int inc = NUM_REGISTERS * EL_PER_BLOCK;
    for(int z = 0; z + inc <= N; z+= inc){
        int prev_error = error_count;
        if (PRINTS >= 0) printf("==== Begginning test  %d ======\n", z / inc);

        generate_RIS(z);
        execute_RIS(&OUT[z], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);

        if(compare_solutions(prev_error, r, &vet_res[0][0]) == 2){
            if (PRINTS >= 0) printf("Convergence %d-%d\n", z, z + inc);
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
            printf("==== Failed on test %d ====\n", z / inc);
            exit(0);
        }
        if (PRINTS >= 0) printf("%s %s %s %s \n", get_OP_name(ops[0]), get_OP_name(ops[1]),
    get_OP_name(ops[2]), get_OP_name(ops[3]));
        if (PRINTS >= 0) printf("==== End test  %d        ======\n\n", z / inc);
    }

}

void digest_parameters(){
    //print_params();
    //if (parameter.argc > 0){
        //SEED = parameter.argv[0];
    //}
    //if(OUT[0] != 0){
        //SEED = OUT[0];
        //printf("SEED: %d\n", SEED);
        //mrand_set_vector(&OUT[MAX_N - 1]);
    //}
    mrand_set_vector(&OUT[MAX_N - 1]);

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

    /*if(OUT[0] == 0){
        msrand(SEED);
        generate_initial_values();
        printf("Done init values\n");
        msrand(SEED); // Length of the values should not alter significantly the operations
    }*/
    
}

int main(){
    update_LMUL(1);
    digest_parameters();
    asm volatile("csrw mtvec, %0" : : "r" (new_trap_handler));

    //printf("Doing random batch tests with registers v0-v7 with seed %d\n", SEED);
    random_test();
    exit(0);
}