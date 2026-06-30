#include "global_configs.h"

int LMUL = 1;
int PRINTS = 0;
int EL_PER_BLOCK = VLEN / SEW * 1;

void update_LMUL(int new_LMUL){
    if(LMUL == new_LMUL)
        return;
    if(PRINTS >=2)
        printf("LMUL: %d -> %d\n", LMUL, new_LMUL);
    LMUL = new_LMUL;
    EL_PER_BLOCK = VLEN / SEW * LMUL;
}

void update_SEW(int new_SEW){
    EL_PER_BLOCK = VLEN / SEW * LMUL;
}