#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// ================= HELPER FUNCTIONS =================

static inline uint64_t read_cycles() {
    uint64_t cycles;
    asm volatile ("csrr %0, cycle" : "=r" (cycles));
    return cycles;
}

static inline uint64_t read_instret() {
    uint64_t instret;
    asm volatile ("csrr %0, instret" : "=r" (instret));
    return instret;
}

long read_vlenb() {
    long vlenb;
    asm volatile("csrr %0, vlenb" : "=r"(vlenb));
    return vlenb;
}

void print_fixed_point(uint64_t n, uint64_t d) {
    if (d == 0) {
        printf("inf");
        return;
    }
    uint64_t integer_part = n / d;
    uint64_t fractional_part = ((n % d) * 1000) / d;
    printf("%llu.%03llu", integer_part, fractional_part);
}

void print_results(uint64_t cycles, uint64_t insts) {
    if (cycles == 0) return;
    printf("  > Cycles: %llu, Insts: %llu, IPC: ", cycles, insts);
    print_fixed_point(insts, cycles);
    printf("\n");
}


// ================= MACROS =================

#define BENCH(NAME, FUNC, VEC, LEN, EXPECTED_FUNC) do { \
    \
    printf("[TEST] %s (Immediate: %d)\n", NAME, 1); \
    /* 1. Generate Golden Model */ \
    int golden[LEN]; \
    memcpy(golden, VEC, LEN * sizeof(int)); \
    for(int i=0; i<LEN; i++) golden[i] = EXPECTED_FUNC(golden[i], 1); \
    \
    /* 2. Run ASM */ \
    uint64_t start_cyc = read_cycles(); \
    uint64_t start_inst = read_instret(); \
    FUNC(VEC, LEN); \
    uint64_t end_cyc = read_cycles(); \
    uint64_t end_inst = read_instret(); \
    \
    /* 3. Verify */ \
    int err = compare_vectors(VEC, golden, LEN); \
    if(err) { printf("  [FAIL] Verification failed.\n"); } \
    else { print_results(end_cyc - start_cyc, end_inst - start_inst); } \
} while(0)

// ================= SCALAR HELPERS =================

int sc_add(int a, int b) { return a + b; }

// ================= HELPERS =================

void init_vector_seq(int* vet, int len, int start) {
    for(int i = 0; i < len; i++) vet[i] = start + i;
}

void init_vector_val(int* vet, int len, int val) {
    for(int i = 0; i < len; i++) vet[i] = val;
}

int compare_vectors(int* received, int* expected, int len){
    for(int i = 0; i < len; i++){
        if(expected[i] != received[i]){
            printf("  Mismatch at [%d]: Expected %d, Got %d\n", i, expected[i], received[i]);
            return 1;
        }
    }
    return 0;
}

// ================= EXTERNAL =================

extern int vector_agnostic_sum_intercalate(int* vector,int LENGTH);

extern int vector_unchanged_sum_intercalate(int* vector,int LENGTH);


// ================= MAIN =================

#define LENGTH 1024

int agnostic_trend_test()
{
    int TRUE_LENGTH = 1024;
    int COMPUTED_LENGTH = 1022;

    int vector_test[TRUE_LENGTH], vector_compare[TRUE_LENGTH];
    init_vector_seq(vector_test, TRUE_LENGTH, 50);
    init_vector_seq(vector_compare, TRUE_LENGTH, 51);

    vector_agnostic_sum_intercalate(vector_test, TRUE_LENGTH);

    int aux_result = 0;

    for(int i = 0; i < TRUE_LENGTH; i++)
    {
        int va = vector_test[i];
        if (i % 2 == 1){
            if(va != vector_compare[i]){
                printf("MASK AGNOSTIC error on should compute values, expected %d, received %d \n", vector_compare[i], va);
            }
           //printf("[%d] EXPECTED %d, RECEIVED %d \n", i, vector_compare[i], vector_test[i]);
        }else{
           //printf("[%d] EXPECTED %d, RECEIVED %d \n", i, vector_compare[i] - 1, vector_test[i]);
            if (va == vector_compare[i] - 1)
                aux_result -= 1;
            else if (va != vector_compare[i])
                aux_result += 1;
        }

    }

    if(aux_result == 0)
        printf("MASK AGNOSTIC computed\n");
    else {
        if(aux_result < 0)
            printf("MASK AGNOSTIC unchanged\n");
        else
            printf("MASK AGNOSTIC changed to random valued\n");
    }
    return 0;
    
}

int unchanged_test()
{
    int TRUE_LENGTH = 1024;
    int COMPUTED_LENGTH = 1022;

    int vector_test[TRUE_LENGTH], vector_compare[TRUE_LENGTH];
    init_vector_seq(vector_test, TRUE_LENGTH, 50);
    init_vector_seq(vector_compare, TRUE_LENGTH, 51);

    vector_unchanged_sum_intercalate(vector_test, TRUE_LENGTH);

    int aux_result = 0;

    for(int i = 0; i < TRUE_LENGTH; i++)
    {
        int va = vector_test[i];
        if (i % 2 == 1){
            if(va != vector_compare[i]){
                printf("MASK UNCHANGED error on should compute values, expected %d, received %d \n", vector_compare[i], va);
            }
           //printf("[%d] EXPECTED %d, RECEIVED %d \n", i, vector_compare[i], vector_test[i]);
        }else{
           //printf("[%d] EXPECTED %d, RECEIVED %d \n", i, vector_compare[i] - 1, vector_test[i]);
            if (va == vector_compare[i] - 1)
                aux_result -= 1;
            else if (va != vector_compare[i])
                aux_result += 1;
        }

    }

    if(aux_result == 0)
        printf("ERROR: MASK UNCHANGED computed\n");
    else {
        if(aux_result < 0)
            printf("MASK UNCHANGED correctly did not change\n");
        else
            printf("ERROR: MASK UNCHANGED changed to random value \n");
    }
    return 0;
    
}

int main(int argc, char* argv[]) {
    agnostic_trend_test();

    unchanged_test();

    exit(0);
}