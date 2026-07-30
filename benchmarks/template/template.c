#include "template.h" // Reference for imports

#define QTD_TESTS 15

int32_t ADDRESS_VECTOR[20];

int init[12] = {
    158653998, 221290364, 191570381, 36659268, 
    136484931, 66907504,  256742140, 270325384, 
    166659206, 288208687, 65823742,  242653232, 
};

int result[12] = {
    0, 0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 
};

int registers[3] = {0, 1, 2};
void test(){
    set_vet_settings();
    int register_ops[3][3] = {
        {0, 0, 0},
        {1, 1, 1},
        {2, 2, 2}
    };

    /* Generates the initial values*/
    load_init_values_scalar(&init[0], &registers[0], 3);
    imm = 1;
    t0_VALUE = 0;
    f_vf = 0; /* float t0 */
    ADDRESS_VECTOR[0] = add_instruction(VADD_VI, registers, &register_ops[0][0]);
    ADDRESS_VECTOR[1] = add_instruction(VADD_VI, registers, &register_ops[1][0]);
    ADDRESS_VECTOR[2] = add_instruction(VADD_VI, registers, &register_ops[2][0]);
    ADDRESS_VECTOR[3] = RET_INSTR;

    /* executes values*/
    execute_RIS(init, registers, ADDRESS_VECTOR, result, 3);
}

    

int main(){
    PRINTS = 0;

    int reference[12] = {
        158653999, 221290365, 191570382, 36659269, 
        136484932, 66907505,  256742141, 270325385, 
        166659207, 288208688, 65823743,  242653233, 
    };

    test(init, result);
    printf("DONE TEST\n");
    
    // Compara resultados e referência
    if(manual_convergence(reference, result, 3, VLEN / SEW * LMUL))
        printf("Convergence\n");
    else
        printf("Divergence\n");

    // Compara resultados e resultados escalar
    if(compare_solutions(0, registers, result))
        printf("Convergence\n");
    else
        printf("Divergence\n");
    
    // PRINTS
    /*
    printf("RESULTS:\n");
    print_matrix(result, 3, 4);
    printf("SCALAR:\n");
    print_matrix(&scalar_res[0][0], 3, 4);
    printf("Reference:\n");
    print_matrix(&reference[0], 3, 4);
    */
   
    exit(0);
}