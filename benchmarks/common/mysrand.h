int32_t _SRAND_SEED = 0;

#define MAX_INT32 2147483646


void msrand(int32_t seed){
    _SRAND_SEED = seed % MAX_INT32;
    _SRAND_SEED = (_SRAND_SEED + 11239) % MAX_INT32;
    _SRAND_SEED = (_SRAND_SEED * 13) % MAX_INT32;
}

int32_t mrand_signed(){
    _SRAND_SEED = (_SRAND_SEED + 11239) % MAX_INT32;
    _SRAND_SEED = (_SRAND_SEED * 13) % MAX_INT32;
    return (_SRAND_SEED / 7 + 44351) % MAX_INT32;
}

int32_t mrand(){
    int32_t ret = mrand_signed();
    return ret>0?ret:-ret;
}