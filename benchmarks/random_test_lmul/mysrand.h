int _SRAND_SEED = 0;


void msrand(int seed){
    _SRAND_SEED = seed;
    _SRAND_SEED += 11239;
    _SRAND_SEED *= 13;
    printf("SRAND_SEED: %d\n", _SRAND_SEED);
}


int mrand(){
    _SRAND_SEED += 11239;
    _SRAND_SEED *= 13;
    return _SRAND_SEED / 7 + 44351;
}