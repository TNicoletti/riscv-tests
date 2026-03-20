#include <stdint.h>

struct parameter {
    int32_t argc;
    int32_t argv[1024];
};

// 2. Assign the single struct to the section
__attribute__((section(".PARAMETERS_SECTION"))) volatile struct parameter parameter;