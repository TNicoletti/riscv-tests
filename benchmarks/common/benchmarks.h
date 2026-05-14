#ifndef BENCHMARKS
#define BENCHMARKS
#include <stdint.h>

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
long read_vlenb();
#endif
