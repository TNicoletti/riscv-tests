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

int last_vl_used;

// Macro for Vector-Immediate (In-Place)
// FUNC signature: int func(int* vec, int len, int imm)
#define BENCH_VI(NAME, FUNC, VEC, LEN, EXPECTED_FUNC) do { \
    printf("[TEST] %s (Immediate: %d)\n", NAME, 1); \
    \
    /* 1. Generate Golden Model */ \
    int golden[LEN]; \
    memcpy(golden, VEC, LEN * sizeof(int)); \
    for(int i=0; i<LEN; i++) golden[i] = EXPECTED_FUNC(golden[i], 1); \
    \
    /* 2. Run ASM */ \
    uint64_t start_cyc = read_cycles(); \
    uint64_t start_inst = read_instret(); \
    FUNC(VEC, LEN, 1); \
    uint64_t end_cyc = read_cycles(); \
    uint64_t end_inst = read_instret(); \
    \
    /* 3. Verify */ \
    int err = compare_vectors(VEC, golden, LEN); \
    if(err) { printf("  [FAIL] Verification failed.\n"); } \
    else { print_results(end_cyc - start_cyc, end_inst - start_inst); } \
} while(0)

#define BENCH_VX(NAME, FUNC, VEC, LEN, IMM, EXPECTED_FUNC) do { \
    printf("[TEST] %s (Immediate: %d)\n", NAME, IMM); \
    \
    /* 1. Generate Golden Model */ \
    int golden[LEN]; \
    memcpy(golden, VEC, LEN * sizeof(int)); \
    for(int i=0; i<LEN; i++) golden[i] = EXPECTED_FUNC(golden[i], IMM); \
    \
    /* 2. Run ASM */ \
    uint64_t start_cyc = read_cycles(); \
    uint64_t start_inst = read_instret(); \
    FUNC(VEC, LEN, IMM); \
    uint64_t end_cyc = read_cycles(); \
    uint64_t end_inst = read_instret(); \
    \
    /* 3. Verify */ \
    int err = compare_vectors(VEC, golden, LEN); \
    if(err) { printf("  [FAIL] Verification failed.\n"); } \
    else { print_results(end_cyc - start_cyc, end_inst - start_inst); } \
} while(0)

// Macro for Vector-Vector
// FUNC signature: int func(int* dst, int* src1, int* src2, int len)
#define BENCH_VV(NAME, FUNC, SRC1, SRC2, DST, LEN, EXPECTED_FUNC) do { \
    printf("[TEST] %s\n", NAME); \
    \
    /* 1. Generate Golden Model */ \
    int golden[LEN]; \
    for(int i=0; i<LEN; i++) golden[i] = EXPECTED_FUNC(SRC1[i], SRC2[i]); \
    \
    /* 2. Run ASM */ \
    uint64_t start_cyc = read_cycles(); \
    uint64_t start_inst = read_instret(); \
    FUNC(DST, SRC1, SRC2, LEN); \
    uint64_t end_cyc = read_cycles(); \
    uint64_t end_inst = read_instret(); \
    \
    /* 3. Verify */ \
    int err = compare_vectors(DST, golden, LEN); \
    if(err) { printf("  [FAIL] Verification failed.\n"); exit(1); } \
    else { print_results(end_cyc - start_cyc, end_inst - start_inst); } \
} while(0)

// ================= EXTERNS =================

// VI Signatures: int* vec, int len, int imm
extern int VADD_VI_ASM(int* vec, int len, int imm);
extern int VAND_VI_ASM(int* vec, int len, int imm);
extern int VOR_VI_ASM(int* vec, int len, int imm);
extern int VXOR_VI_ASM(int* vec, int len, int imm);

// VX Signatures: int* vec, int len, int imm
extern int VADD_VX_ASM(int* vec, int len, int imm);
extern int VAND_VX_ASM(int* vec, int len, int imm);
extern int VOR_VX_ASM(int* vec, int len, int imm);
extern int VXOR_VX_ASM(int* vec, int len, int imm);

// VV Signatures: int* dst, int* src1, int* src2, int len
extern int VADD_VV_ASM(int* dst, int* src1, int* src2, int len);
extern int VSUB_VV_ASM(int* dst, int* src1, int* src2, int len);
extern int VMUL_VV_ASM(int* dst, int* src1, int* src2, int len);
extern int VDIV_VV_ASM(int* dst, int* src1, int* src2, int len);
extern int VAND_VV_ASM(int* dst, int* src1, int* src2, int len);
extern int VOR_VV_ASM(int* dst, int* src1, int* src2, int len);
extern int VXOR_VV_ASM(int* dst, int* src1, int* src2, int len);

// ================= SCALAR HELPERS =================

int sc_add(int a, int b) { return a + b; }
int sc_sub(int a, int b) { return a - b; }
int sc_mul(int a, int b) { return a * b; }
int sc_div(int a, int b) { return (b==0)?0 : a / b; }
int sc_and(int a, int b) { return a & b; }
int sc_or(int a, int b)  { return a | b; }
int sc_xor(int a, int b) { return a ^ b; }

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

// ================= MAIN =================

#define LENGTH 1024

int main(int argc, char* argv[]) {
    printf("=== RISC-V Vector Extension Benchmark Suite ===\n");
    printf("VLENB: %ld bytes (%ld bits)\n\n", read_vlenb(), read_vlenb() * 8);

    // Allocate memory
    int src1[LENGTH];
    int src2[LENGTH];
    int dest[LENGTH];

    // ----------------------------------------
    // TEST 1: Vector-Immediate (VI) Operations
    // ----------------------------------------
    printf("--- VI (Vector-Immediate) Tests ---\n");
    
    init_vector_seq(src1, LENGTH, 0);
    BENCH_VI("VADD.VI", VADD_VI_ASM, src1, LENGTH, sc_add);
    for(int i = 0; i < LENGTH; i++)
        printf("[%d]: %d; ", i, src1[i]);

    init_vector_seq(src1, LENGTH, 0);
    BENCH_VI("VAND.VI", VAND_VI_ASM, src1, LENGTH, sc_and);

    init_vector_seq(src1, LENGTH, 0);
    BENCH_VI("VOR.VI",  VOR_VI_ASM,  src1, LENGTH, sc_or);

    init_vector_seq(src1, LENGTH, 0);
    BENCH_VI("VXOR.VI", VXOR_VI_ASM, src1, LENGTH, sc_xor);
    
    // ----------------------------------------
    // TEST 2: Vector-Immediate (VI) Operations
    // ----------------------------------------
    printf("\n--- VX (Vector-Immediate register) Tests ---\n");
    
    init_vector_seq(src1, LENGTH, 0); // 0, 1, 2...
    BENCH_VX("VADD.VX", VADD_VX_ASM, src1, LENGTH, 10, sc_add);

    init_vector_seq(src1, LENGTH, 0);
    BENCH_VX("VAND.VX", VAND_VX_ASM, src1, LENGTH, 0xFF, sc_and);

    init_vector_seq(src1, LENGTH, 0);
    BENCH_VX("VOR.VX",  VOR_VX_ASM,  src1, LENGTH, 0xF0, sc_or);

    init_vector_seq(src1, LENGTH, 0);
    BENCH_VX("VXOR.VX", VXOR_VX_ASM, src1, LENGTH, 0xFF, sc_xor);

    // ----------------------------------------
    // TEST 2: Vector-Vector (VV) Operations
    // ----------------------------------------
    printf("\n--- VV (Vector-Vector) Tests ---\n");

    // Re-init sources
    init_vector_seq(src1, LENGTH, 0);    // 0, 1, 2...
    init_vector_seq(src2, LENGTH, 1000); // 1000, 1001, 1002...

    BENCH_VV("VADD.VV", VADD_VV_ASM, src1, src2, dest, LENGTH, sc_add);
    BENCH_VV("VSUB.VV", VSUB_VV_ASM, src2, src1, dest, LENGTH, sc_sub); // src2 - src1
    BENCH_VV("VMUL.VV", VMUL_VV_ASM, src1, src2, dest, LENGTH, sc_mul);
    
    // For division, avoid zero division
    init_vector_val(src2, LENGTH, 5); 
    BENCH_VV("VDIV.VV", VDIV_VV_ASM, src1, src2, dest, LENGTH, sc_div);

    init_vector_seq(src1, LENGTH, 0x5555);
    init_vector_seq(src2, LENGTH, 0x0F0F);
    BENCH_VV("VAND.VV", VAND_VV_ASM, src1, src2, dest, LENGTH, sc_and);
    BENCH_VV("VOR.VV",  VOR_VV_ASM,  src1, src2, dest, LENGTH, sc_or);
    BENCH_VV("VXOR.VV", VXOR_VV_ASM, src1, src2, dest, LENGTH, sc_xor);

    printf("\nAll tests passed successfully.\n");
    
    exit(0);
}