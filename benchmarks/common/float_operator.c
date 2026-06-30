#include "float_operator.h"

// Função auxiliar para reinterpretar Uint32 como Float
floaSEW bits_to_float(uintSEW bits) {
    floaSEW f;
    memcpy(&f, &bits, sizeof(uintSEW));
    return f;
}

// Função auxiliar para reinterpretar Float como Uint32 (para guardar o resultado)
uintSEW float_to_bits(floaSEW f) {
    uintSEW bits;
    memcpy(&bits, &f, sizeof(uintSEW));
    return bits;
}

int is_nan(floaSEW f) {
    uintSEW bits;
    memcpy(&bits, &f, sizeof(uintSEW));

    int total_bits = sizeof(uintSEW) * 8;
    int exp_bits = 0;

    switch (total_bits) {
        case 16:  exp_bits = 5;  break;
        case 32:  exp_bits = 8;  break;
        case 64:  exp_bits = 11; break;
        case 128: exp_bits = 15; break;
        default:  return 0; // Tamanho não suportado
    }

    int mantissa_bits = total_bits - 1 - exp_bits;

    // Cria as máscaras dinamicamente
    uintSEW one = 1; 
    uintSEW mantissa_mask = (one << mantissa_bits) - one;
    uintSEW exp_mask = ((one << exp_bits) - one) << mantissa_bits;

    int is_exponent_all_ones = ((bits & exp_mask) == exp_mask);
    int is_mantissa_not_zero = ((bits & mantissa_mask) != 0);

    return is_exponent_all_ones && is_mantissa_not_zero;
}