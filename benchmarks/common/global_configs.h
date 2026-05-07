#ifndef GLOBAL_CONFIGS
#define GLOBAL_CONFIGS

#define VLEN 128
#define SEW  32
extern int LMUL;

#define MAX_LMUL 8
#define MAX_EL_PER_BLOCK VLEN / SEW * MAX_LMUL

extern int PRINTS;

extern int EL_PER_BLOCK;

void update_LMUL(int new_LMUL);

void update_SEW(int new_SEW);

#endif