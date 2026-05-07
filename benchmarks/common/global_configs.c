#include "global_configs.h"

int LMUL = 1;
int PRINTS = 0;
int EL_PER_BLOCK = 4;

void update_LMUL(int new_LMUL){
    LMUL = new_LMUL;
    EL_PER_BLOCK = VLEN / SEW * LMUL;
}

void update_SEW(int new_SEW){
    EL_PER_BLOCK = VLEN / SEW * LMUL;
}