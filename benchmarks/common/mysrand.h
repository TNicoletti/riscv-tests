#ifndef MYSRAND
#define MYSRAND
#include <stdint.h>
#include "global_configs.h"

extern int32_t _SRAND_SEED;

#define MAX_INT8  127
#define MAX_INT16 32768
#define MAX_INT32 2147483646
#define MAX_INT64 9223372036854775807LL

#if SEW == 8
#define MAX_INTSEW MAX_INT8
#endif

#if SEW == 16
#define MAX_INTSEW MAX_INT16
#endif

#if SEW == 32
#define MAX_INTSEW MAX_INT32
#endif
#if SEW == 64
#define MAX_INTSEW MAX_INT32
#endif


#define MAX_VECTOR_REGS 32


void msrand(int32_t seed);
void mrand_set_vector(intSEW* i);

intSEW mrand_signed();

intSEW mrand();

void randomize_vector(intSEW vet[], int N);

int get_random_reg();

void shuffle_registers(INTXLEN* r, int num_registers, int lmul);
#endif