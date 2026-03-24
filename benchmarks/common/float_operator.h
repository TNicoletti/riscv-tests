#include <string.h>
#include <stdint.h>

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