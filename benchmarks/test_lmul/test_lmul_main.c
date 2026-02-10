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

extern int get_max_vl_8();
extern int sum1_group8(int* vector_test, int LENGTH);

extern int get_max_vl_4();
extern int sum1_group4(int* vector_test, int LENGTH);

extern int get_max_vl_2();
extern int sum1_group2(int* vector_test, int LENGTH);

extern int get_max_vl_1();
extern int sum1_group1(int* vector_test, int LENGTH);

// ================= MAIN =================

#define LENGTH 1024

int main(int argc, char* argv[]) {

    int max_vl_8 = get_max_vl_8();
    int max_vl_4 = get_max_vl_4();
    int max_vl_2 = get_max_vl_2();
    int max_vl_1 = get_max_vl_1();

    printf("MAX LMUL 8 %d\n", max_vl_8);
    printf("MAX LMUL 4 %d\n", max_vl_4);
    printf("MAX LMUL 2 %d\n", max_vl_2);
    printf("MAX LMUL 1 %d\n", max_vl_1);

    int vector_test[1024];

    init_vector_seq(vector_test, 1024, 50);
    if(max_vl_1 != 0)
        BENCH("Sum 1 group 1", sum1_group1, vector_test, 1024, sc_add);
    else
        printf("Unsuported group settings, skippng group with 1");
    
    if(max_vl_2 != 0)
        BENCH("Sum 2 group 2", sum1_group2, vector_test, 1024, sc_add);
    else
        printf("Unsuported group settings, skippng group with 2");

    if(max_vl_4 != 0)
        BENCH("Sum 1 group 4", sum1_group4, vector_test, 1024, sc_add);
    else
        printf("Unsuported group settings, skippng group with 4");

    if(max_vl_8 != 0)
        BENCH("Sum 1 group 8", sum1_group8, vector_test, 1024, sc_add);
    else
        printf("Unsuported group settings, skippng group with 8");

    exit(0);
}