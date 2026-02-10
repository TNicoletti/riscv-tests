#include "util.h"

//V1.0

//asm(".include \"critical_vetorization.S\"");

void print_fixed_point(uint64_t n, uint64_t d) {
    if (d == 0) {
        printf("inf");
        return;
    }
    uint64_t integer_part = n / d;
    // Multiplicamos o resto por 1000 antes de dividir para obter 3 casas decimais
    uint64_t fractional_part = ((n % d) * 1000) / d;
    
    // %03llu garante que 1.005 não vire 1.5
    printf("%llu.%03llu", integer_part, fractional_part);
}

// Declare os protótipos para o C não reclamar que a função não existe
extern int vetorized(int num_elements, int vlen);
extern int non_vetorized(int num_elements, int vlen);

// Função inline para ler o contador de ciclos
static inline int read_cycles() {
    unsigned long int cycles_lo;
    asm volatile ("csrr %0, cycle" : "=r" (cycles_lo));
    int ret = (unsigned long int)cycles_lo;
    return ret;
}

// Função inline para ler instruções retiradas
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
    if (cycles == 0) return; // Evita divisão por zero
    
    printf("Ciclos: %llu, Instruções: %llu, IPC: ", 
           cycles, insts);
    print_fixed_point(insts, cycles);
    printf("\n");
}

void run_test_vetorized(int LENGTH, int nvet_cyc){
    for(int i = 2; ; i*=2)
    {
        int actual_length = LENGTH + LENGTH % i;
        printf("%d elements to calculate / %d requested elements in vector \n", actual_length, i);
        uint64_t start_cyc = read_cycles();
        uint64_t start_inst = read_instret();

        int cores_used = vetorized(actual_length, i);

        uint64_t end_cyc = read_cycles();
        uint64_t end_inst = read_instret();

        print_results(end_cyc - start_cyc, end_inst - start_inst);
        printf("Elements effectively used: %d\n", cores_used);
        printf("Speedup vetorizado: ");
        print_fixed_point(nvet_cyc, end_cyc -  start_cyc);
        printf("\n");
        printf("===============\n");
        if(cores_used < i){
            printf("Elements used < requested elements, ending program\n");
            printf("VLEN defined as %d; VMAX for 32bits defined as %d\n", 32 * cores_used, cores_used);
            return;
        }
    }
}

int run_test(int (*func_ptr)(), int num_elements, int vlen) {
    uint64_t start_cyc = read_cycles();
    uint64_t start_inst = read_instret();

    int cores_used = func_ptr(num_elements, vlen);

    uint64_t end_cyc = read_cycles();
    uint64_t end_inst = read_instret();

    print_results(end_cyc - start_cyc, end_inst - start_inst);
    printf("Cores effectively used: %d\n", cores_used);
    return end_cyc -  start_cyc;
}

int main( int argc, char* argv[] )
{
    int vet_cyc, nvet_cyc;
    const int LENGTH = 1024;
    const int UNTIL = 256;
    
    printf("===== Não vetorizado =====\n");
    nvet_cyc = run_test(non_vetorized, LENGTH, 1);
    printf("\n");
    
    printf("===== Vetorizado =====\n");
    
    run_test_vetorized(LENGTH, nvet_cyc);
    
    printf("Hardware VLEN read: %ld bits\n", read_vlenb() * 8);

    return 0;
}

