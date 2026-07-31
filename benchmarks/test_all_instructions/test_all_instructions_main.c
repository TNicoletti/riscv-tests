#include "test_all_instructions.h"

/* ===== RANDOMIZERS ===== */
void generate_initial_values(){
    msrand(SEED);
    randomize_vector(OUT, N);
}

int generate_RIS(int op, int index){
    load_init_values_scalar(&OUT[index], r, NUM_REGISTERS);

    int32_t instr = add_instruction(op, rx[0], r);
    if(instr == NOP)
        return 0;
    ADDRESS_VECTOR[0] = instr;    
    
    ADDRESS_VECTOR[1] = RET_INSTR;
    return 1;
}

void eval_results(){
    int qtt_errors = 0;
    if(PRINTS >= 4) help_errors();

    int merge_inst_count = 0;
    int mask_inst_count = 0;
    int float_inst_count = 0;
    int widening_inst_count = 0;


    printf("\nRESULTS\n");
    for(int i = 0; i < SUPORTED_INSTRUCTIONS; i++){
        if(allowed_instructions[i] == 0)
            continue;
        int correct_instruction = 1;
        printf("%d - %s\t ", i, get_OP_name(i));
        for(int j = 0; j < repeat_instructions; j++){
            printf("[%s] ", get_err(res[i][j]));
            if(res[i][j] != 2){
                qtt_errors++;
                correct_instruction = 0;
            }
        }
        if(correct_instruction){
            if(is_mask_instruction(i))     mask_inst_count++;
            if(is_float_instruction(i))    float_inst_count++;
            if(is_widening_instruction(i)) widening_inst_count++;
            if(is_merge_instruction(i)) merge_inst_count++;

            printf("PASS");
            correct_instructions++;
        }
        else{
            printf("ERR");
            wrong_instructions++;
        }
        printf("\n");
    }

    printf("\nErrors: %d\n", qtt_errors);
    printf("Hardware errors: %d\n", error_count);
    printf("Wrong answers: %d\n", qtt_errors - error_count);
    printf("Correct instructions / total: %d / %d\n", correct_instructions, correct_instructions + wrong_instructions);

    if(mask_inst_count == QUANTITY_MASK_INSTRUCTIONS)
        printf("MASK instructions PASSED\n");
    else if(mask_inst_count > 0)
        printf("MASK instructions PARTIAL\n");
    else
        printf("MASK instructions NOT COVERED\n");
    
    if(float_inst_count == QUANTITY_FLOAT_INSTRUCTIONS)
        printf("FLOAT instructions PASSED\n");
    else if(float_inst_count > 0)
        printf("FLOAT instructions PARTIAL\n");
    else
        printf("FLOAT instructions NOT COVERED\n");
    
    if(widening_inst_count == QUANTITY_WIDENING_INSTRUCTIONS)
        printf("WIDENING instructions PASSED\n");
    else if(widening_inst_count > 0)
        printf("WIDENING instructions PARTIAL\n");
    else
        printf("WIDENING instructions NOT COVERED\n");

    if(merge_inst_count == QUANTITY_MERGE_INSTRUCTIONS)
        printf("MERGE instructions PASSED\n");
    else if(merge_inst_count > 0)
        printf("MERGE instructions PARTIAL\n");
    else
        printf("MERGE instructions NOT COVERED\n");
}

int test_for_ls32(){
    if(PRINTS >= 3)printf("==== Testing required instructions ======\n\n");
    int res[repeat_instructions];
    int inc = NUM_REGISTERS * EL_PER_BLOCK;
    for(int j = 0; j < repeat_instructions; j++){
        int prev_error = error_count;   

        if(PRINTS >= 3)printf("Executing instruction VLE32_V VSE32_V %d\n", j);
        load_init_values_scalar(&OUT[j * inc], r, NUM_REGISTERS);
        load_init_values_vector(&OUT[j * inc], r, NUM_REGISTERS);
        store_vet_values(r, &vet_res[0][0], NUM_REGISTERS);

        if(PRINTS >= 3)print_regs(&scalar_res[0][0], NUM_REGISTERS, r);
        if(PRINTS >= 3)print_regs(&vet_res[0][0], NUM_REGISTERS, r);
        
        res[j] = compare_solutions(prev_error, r, &vet_res[0][0]);
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

        if(PRINTS >= 3)printf("Executing instruction %s\n", get_OP_name(op));
        generate_RIS(op, j * inc);
        execute_RIS(&OUT[j * inc], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);

        if(PRINTS >= 3){printf("IN:\n");print_vector(&OUT[0], NUM_REGISTERS * EL_PER_BLOCK, EL_PER_BLOCK);}
        if(PRINTS >= 3){printf("Scalar:\n");  print_regs(&scalar_res[0][0], NUM_REGISTERS, r);}
        if(PRINTS >= 3){printf("Vetorial:\n");print_regs(&vet_res[0][0]   , NUM_REGISTERS, r);}
        
        if(prev_error < error_count){
            if(PRINTS >= 1) printf("Hardware error detected\n");
            res[0][j] = -last_hw_error;
        } else {
            res[0][j] = compare_solutions(prev_error, r, &vet_res[0][0]);   
        } 
    }

    printf("Results: - %s\t ", get_OP_name(op));
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
        if(allowed_instructions[z] == 0)
            continue;
        for(int j = 0; j < repeat_instructions; j++){
            int prev_error = error_count;
            if(PRINTS >= 3)printf("==== Begginning test  %d ======\n\n", z * repeat_instructions + j);

            if(PRINTS >= 3)printf("Executing instruction %s\n", get_OP_name(z));

            if(generate_RIS(z, (z * repeat_instructions + j) * inc) == 0){
                allowed_instructions[z] = 0;
                continue;
            }
            execute_RIS(&OUT[(z * repeat_instructions + j) * inc], r, ADDRESS_VECTOR, &vet_res[0][0], NUM_REGISTERS);
            
            if(PRINTS >= 3){printf("Scalar:\n");   print_regs(&scalar_res[0][0], NUM_REGISTERS, r);}
            if(PRINTS >= 3){printf("Vetorial:\n"); print_regs(&vet_res[0][0],    NUM_REGISTERS, r);}


            if(prev_error < error_count){
                if(PRINTS >= 1) printf("Hardware error detected\n");
                res[z][j] = -last_hw_error;
            } else {
                res[z][j] = compare_solutions(prev_error, r, &vet_res[0][0]);   
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
    if(parameter.argc >= 4 + SUPORTED_INSTRUCTIONS){
        for(int i = 0; i < SUPORTED_INSTRUCTIONS; i++){
            allowed_instructions[i] = parameter.argv[4 + i];
        }
    }else{
        for(int i = 0; i < SUPORTED_INSTRUCTIONS; i++)
            allowed_instructions[i] = 1;
    }
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