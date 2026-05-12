#ifndef FLOAT_OPERATOR
#define FLOAT_OPERATOR

#include <string.h>
#include <stdint.h>

// Função auxiliar para reinterpretar Uint32 como Float
float bits_to_float(uint32_t bits);

// Função auxiliar para reinterpretar Float como Uint32 (para guardar o resultado)
uint32_t float_to_bits(float f);

int is_nan(float bits);

#endif