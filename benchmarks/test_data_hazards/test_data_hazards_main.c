#include <stdio.h>
#include <stdint.h>
#include <util.h>

//V1.2 - Com Macro

// ================= HELPER FUNCTIONS =================

void print_fixed_point(uint64_t n, uint64_t d) {
    if (d == 0) {
        printf("inf");
        return;
    }
    uint64_t integer_part = n / d;
    uint64_t fractional_part = ((n % d) * 1000) / d;

    printf("%llu.%03llu", integer_part, fractional_part);
}

static inline int read_cycles() {
    unsigned long int cycles_lo;
    asm volatile ("csrr %0, cycle" : "=r" (cycles_lo));
    int ret = (unsigned long int)cycles_lo;
    return ret;
}

static inline int read_instret() {
    int instret;
    asm volatile ("csrr %0, instret" : "=r" (instret));
    return instret;
}

long read_vlenb() {
    long vlenb;
    asm volatile("csrr %0, vlenb" : "=r"(vlenb));
    return vlenb;
}

void print_results(uint64_t cycles, uint64_t insts) {
    if (cycles == 0) return; 
    
    printf("Ciclos: %llu, Instruções: %llu, IPC: ", cycles, insts);
    print_fixed_point(insts, cycles);
    printf("\n");
}

int compare_vectors(int received[], int expected[], int LENGTH){
    int equal_vectors = 1;
    for(int i = 0; i < LENGTH; i++)
        if(expected[i] != received[i]){
            equal_vectors = 0;
            printf("[%d]: %d != %d \n", i, expected[i], received[i]);
        }
    if(equal_vectors)
        printf("Vectors were equal \n\n"); // Corrigido typos: "where" -> "were"
    else
        printf("Vectors were not equal \n\n");

    return 0;
}

// ================= MACRO DEFINITION =================


int cores_used;
/* * Macro para executar o benchmark de funções vetoriais.
 * FUNC: O nome da função externa a ser chamada
 * VEC: O ponteiro para o vetor
 * LEN: O tamanho a ser calculado
 * MAX_VL: O tamanho requisitado (o 1024 hardcoded no seu exemplo)
 */
#define RUN_TEST_BENCHMARK(FUNC, VEC, LEN, MAX_VL) do { \
    printf("%d elements to calculate / %d requested elements in vector \n", (LEN), (MAX_VL)); \
    uint64_t start_cyc = read_cycles(); \
    uint64_t start_inst = read_instret(); \
    \
    cores_used = FUNC((VEC), (LEN), (MAX_VL)); \
    \
    uint64_t end_cyc = read_cycles(); \
    uint64_t end_inst = read_instret(); \
    \
    print_results(end_cyc - start_cyc, end_inst - start_inst); \
} while(0)


// ================= EXTERNS & WRAPPERS =================

extern int vetorized_sum_1(int* vector, int num_elements, int vlen);
extern int close_WAW(int* vector, int num_elements, int vlen);
extern int close_RAW(int* vector, int num_elements, int vlen);
extern int close_WAR(int* vector, int num_elements, int vlen);

extern int close_WAW_compile_verify(int* vector, int num_elements, int vlen); 

int run_test_vetorized(int* vector, int LENGTH) {
    RUN_TEST_BENCHMARK(vetorized_sum_1, vector, LENGTH, 1024);
}

int run_test_WAW(int* vector, int LENGTH) {
    RUN_TEST_BENCHMARK(close_WAW, vector, LENGTH, 1024);
}

int run_test_RAW(int* vector, int LENGTH) {
    RUN_TEST_BENCHMARK(close_RAW, vector, LENGTH, 1024);
}

int run_test_WAR(int* vector, int LENGTH) {
    RUN_TEST_BENCHMARK(close_WAR, vector, LENGTH, 1024);
}

int run_test_compile_verification(int* vector, int LENGTH) {
    RUN_TEST_BENCHMARK(close_WAW_compile_verify, vector, LENGTH, 1024);
}

// ================= MAIN =================

int* return_consecutive_array(int* vet, int LENGTH, int sum){
    for(int i = 0; i < LENGTH; i++){
        vet[i] = i + 1 + sum;
    }
    
}

void return_consecutive_array_zero(int* vet, int LENGTH){
    return_consecutive_array(vet, LENGTH, 0);
}

void return_equal_array(int* vet, int LENGTH, int num){
    for(int i = 0; i < LENGTH; i++){
        vet[i] = num;
    }
}

int main( int argc, char* argv[] )
{
    const int LENGTH = 1024;
    
    printf("===== Sum 1 in a %d length vector =====\n", LENGTH);
    int real_sum[LENGTH];
    int test_sum[LENGTH];
    for(int i = 0; i < LENGTH; i++) // Corrigido para usar LENGTH constante
    {
        real_sum[i] = i + 1;
        test_sum[i] = i;
    }
    run_test_vetorized(test_sum, LENGTH);
    
    compare_vectors(real_sum, test_sum, LENGTH);

    printf("===== Sum 3 in a vector - WAW =====\n"); // Removido LENGTH extra no printf
    int test_WAW[LENGTH], resp[LENGTH];
    return_consecutive_array_zero(test_WAW, LENGTH);
    return_consecutive_array(resp, LENGTH, 3);
    run_test_WAW(test_WAW, LENGTH);
    compare_vectors(test_WAW, resp, 4);

    printf("===== Sums 1 in a vector - RAW =====\n"); // Removido LENGTH extra no printf
    int test_RAW[LENGTH], resp2[LENGTH];
    return_consecutive_array_zero(test_RAW, LENGTH);
    return_consecutive_array(resp2, LENGTH, 1);
    run_test_RAW(test_RAW, LENGTH);
    compare_vectors(test_RAW, resp2, LENGTH);

    printf("===== Sums 1 in a vector - WAR =====\n"); // Removido LENGTH extra no printf
    int test_WAR[LENGTH];
    return_consecutive_array_zero(test_WAR, LENGTH);
    run_test_WAR(test_WAR, LENGTH);
    compare_vectors(test_WAR, resp2, LENGTH);

    
    printf("===== Verify no compile unwanted optimizations =====\n"); // Removido LENGTH extra no printf
    run_test_compile_verification(test_WAW, 4);
    printf("==========\n");

    printf("Elements effectively used: %d\n", cores_used); \
    printf("Hardware VLEN read: %ld bits\n", read_vlenb() * 8);

    exit(0);
}