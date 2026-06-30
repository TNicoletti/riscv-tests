#ifndef FLOAT_OPERATOR
#define FLOAT_OPERATOR

#include <string.h>
#include <stdint.h>
#include "global_configs.h"

#if SEW == 16
#define load_value_ft0 load_value_ft0_half
#endif
#if SEW == 32
#define load_value_ft0 load_value_ft0_single
#endif
#if SEW == 64
#define load_value_ft0 load_value_ft0_double
#endif


extern void load_value_ft0(floaSEW f);

// Função auxiliar para reinterpretar Uint32 como Float
floaSEW bits_to_float(uintSEW bits);

// Função auxiliar para reinterpretar Float como Uint32 (para guardar o resultado)
uintSEW float_to_bits(floaSEW f);

int is_nan(floaSEW bits);

#endif