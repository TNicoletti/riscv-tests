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

int is_nan(uint32_t bits) {
    uint32_t exponent = bits & 0x7F800000;
    uint32_t mantissa = bits & 0x007FFFFF;
    return (exponent == 0x7F800000) && (mantissa != 0);
}