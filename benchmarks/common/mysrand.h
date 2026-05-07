#ifndef MYSRAND
#define MYSRAND
#include <stdint.h>
#include "global_configs.h"

extern int32_t _SRAND_SEED;

#define MAX_INT32 2147483646
#define MAX_VECTOR_REGS 32


void msrand(int32_t seed);

int32_t mrand_signed();

int32_t mrand();

void randomize_vector(int vet[], int N);

int get_random_reg();

void shuffle_registers(int* r, int num_registers, int lmul);
#endif