int _SRAND_SEED = 0;


void msrand(int seed){
    _SRAND_SEED = seed;
    _SRAND_SEED += 11239;
    _SRAND_SEED *= 13;
}

int mrand_signed(){
    _SRAND_SEED += 11239;
    _SRAND_SEED *= 13;
    return _SRAND_SEED / 7 + 44351;
}

int mrand(){
    return abs(mrand_signed());
}