#include "mysrand.h"

int32_t _SRAND_SEED = 0;

typedef enum random_methods{
    RM_SEED = 0,
    RM_SET = 1
};

int random_method = RM_SEED;
int* set_input_variable;

void msrand(int32_t seed){
    _SRAND_SEED = seed % MAX_INTSEW;
    _SRAND_SEED = (_SRAND_SEED + 11239) % MAX_INTSEW;
    _SRAND_SEED = (_SRAND_SEED * 13)    % MAX_INTSEW;
}

void mrand_set_vector(intSEW* i) {
    set_input_variable = i;
    random_method = RM_SET; // Ativa automaticamente o modo de leitura do vetor
}

intSEW _mrand_signed_get(){
    int ret = set_input_variable[0];
    set_input_variable--;
    return ret;
}

intSEW _mrand_signed_seed(){
    _SRAND_SEED = (_SRAND_SEED + 11239) % MAX_INTSEW;
    _SRAND_SEED = (_SRAND_SEED * 13)    % MAX_INTSEW;
    return (_SRAND_SEED / 7 + 44351)    % MAX_INTSEW;
}

intSEW mrand_signed(){
    intSEW ret =  (random_method == RM_SEED)? _mrand_signed_seed(): _mrand_signed_get();
    return ret;
}

intSEW mrand(){
    intSEW ret = mrand_signed();
    return ret>0?ret:-ret;
}

void randomize_vector(intSEW vet[], int N){
    for (int i = 0; i < N; i++) {
        vet[i] = mrand_signed();
    }
}

int get_random_reg(){
    if(LMUL == 1)
        return mrand() % 32;
    else if(LMUL == 2)
        return mrand() % 16 * 2;
    else if(LMUL == 4)
        return mrand() % 8  * 4;
    else if(LMUL == 8)
        return mrand() % 4  * 8;
    return 0;
}

void shuffle_registers(INTXLEN* r, int num_registers, int lmul) {
    if (lmul < 1) lmul = 1;
    
    int num_available = MAX_VECTOR_REGS / lmul;
    
    if (num_registers > num_available) {
        num_registers = num_available; 
    }

    int pool[MAX_VECTOR_REGS];
    for (int i = 0; i < num_available; i++) {
        pool[i] = i * lmul;
    }

    for (int i = 0; i < num_registers; i++) {
        int remaining = num_available - i;
        
        int rand_idx = i + (mrand() % remaining); 
        
        int temp = pool[i];
        pool[i] = pool[rand_idx];
        pool[rand_idx] = temp;
        
        r[i] = pool[i];
    }
}