#include <stdint.h>

struct parameter {
    int32_t argc;
    int32_t argv[1024];
};

__attribute__((section(".PARAMETERS_SECTION"))) volatile struct parameter parameter;

void print_params(){
    printf("argc: %d\n", parameter.argc);
    for(int i = 0; i < parameter.argc; i++){
        printf("argv[%d] = %d;\n", i, parameter.argv[i]);
    }
    printf("\n");
}