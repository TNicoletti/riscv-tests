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

void print_vector(int* vet, int LENGTH){
    for(int i = 0; i < LENGTH; i++){
        printf("%d ", vet[i]);
        if((i % 15) == 14)
            printf("\n");
    }
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

extern int vector_sum_interrupt(int* vector,int LENGTH);

extern int vector_sum_interrupt(int* vector,int LENGTH);

// ================= MAIN =================

#define LENGTH 1024

int main(int argc, char* argv[]) {

    int vet[LENGTH];
    
    init_vector_seq(vet, LENGTH, 127);
    vector_sum_interrupt(vet, LENGTH);
    printf("%d %d\n",vet[LENGTH], vet[LENGTH + 1024]);

    exit(0);
}