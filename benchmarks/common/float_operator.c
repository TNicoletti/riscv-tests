#include "float_operator.h"

// Função auxiliar para reinterpretar Uint32 como Float
float bits_to_float(uint32_t bits) {
    float f;
    memcpy(&f, &bits, sizeof(float));
    return f;
}

// Função auxiliar para reinterpretar Float como Uint32 (para guardar o resultado)
uint32_t float_to_bits(float f) {
    uint32_t bits;
    memcpy(&bits, &f, sizeof(uint32_t));
    return bits;
}

int is_nan(float f) {
    uint32_t bits;
    memcpy(&bits, &f, sizeof(uint32_t));

    // A máscara 0x7F800000 isola os 8 bits do expoente.
    // A máscara 0x007FFFFF isola os 23 bits da mantissa.
    
    int is_exponent_all_ones = ((bits & 0x7F800000) == 0x7F800000);
    int is_mantissa_not_zero = ((bits & 0x007FFFFF) != 0);
    
    return is_exponent_all_ones && is_mantissa_not_zero;
}