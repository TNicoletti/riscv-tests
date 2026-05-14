#include "benchmarks.h"

long read_vlenb() {
    long vlenb;
    asm volatile("csrr %0, vlenb" : "=r"(vlenb));
    return vlenb;
}
