#include "util.h"
#include "benchmarks.h"

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
extern void vetorized();
extern void non_vetorized();

void print_results(uint64_t cycles, uint64_t insts) {
    if (cycles == 0) return; // Evita divisão por zero
    
    printf("Ciclos: %llu, Instruções: %llu, IPC: ", 
           cycles, insts);
    print_fixed_point(insts, cycles);
    printf("\n");
}

int run_test(void (*func_ptr)()) {
    uint64_t start_cyc = read_cycles();
    uint64_t start_inst = read_instret();

    func_ptr(); // Executa a função (vetorizada ou não)

    uint64_t end_cyc = read_cycles();
    uint64_t end_inst = read_instret();

    print_results(end_cyc - start_cyc, end_inst - start_inst);
    return end_cyc -  start_cyc;
}


int main( int argc, char* argv[] )
{
    printf("===== Vetorizado =====\n");
    int vet_cyc = run_test(vetorized);

    printf("===== Não vetorizado =====\n");
    int nvet_cyc = run_test(non_vetorized);

    printf("Speedup vetorizado: \n");
    print_fixed_point(nvet_cyc, vet_cyc);
    printf("\n");

    printf("Hardware VLEN read: %ld bits\n", read_vlenb() * 8);
}

