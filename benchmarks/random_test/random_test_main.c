#include <util.h>
#include "mysrand.h"
#include "myutil.h"

#define N 1024*1024
#define SEED 0x12345678
#define SEW 32
#define VLEN 128
#define LMUL 1
#define EL_PER_BLOCK 4 * VLEN/SEW * LMUL

/* EXTERNALS */
extern int set_vet_Xx16(int X);
extern int set_vet_Xx32(int X);
extern int set_vet_Xx64(int X);

extern void vet_vv_sum(int* vet1, int* vet2, int* vetd);
extern void vet_vv_sub(int* vet1, int* vet2, int* vetd);
extern void vet_vv_div(int* vet1, int* vet2, int* vetd);
extern void vet_vv_mul(int* vet1, int* vet2, int* vetd);

extern void vet_vx_sum(int* vet1, int op2, int* vetd);
extern void vet_vx_sub(int* vet1, int op2, int* vetd);
extern void vet_vx_div(int* vet1, int op2, int* vetd);
extern void vet_vx_mul(int* vet1, int op2, int* vetd);

extern void vet_vi_sum(int* vet1, int* vetd);

/* NORMALS */

volatile int A[N];
volatile int B[N];
volatile int OUT[N];
volatile int OUT_SCALAR[N];

int checksum(int *vec, int n) {
  int chk = 0;
  for (int i = 0; i < n; i++) {
    chk ^= vec[i];
  }
  return chk;
}

void random_test(int seed) {
    #if SEW == 16
        set_vet_Xx16(VLEN / SEW);
    #elif SEW == 32
        set_vet_Xx32(VLEN / SEW);
    #elif SEW == 64
        set_vet_Xx64(VLEN / SEW);
    #else
        #error "Valor de SEW não suportado!"
    #endif

    msrand(seed);
    for (int i = 0; i < N; i++) {
        A[i] = mrand();
        B[i] = mrand();
        OUT[i] = mrand();
    }
    printf("DONE INIT VALUES \n");

    for(int blk = 0; blk < N; blk+=EL_PER_BLOCK) {
        int opt = abs(mrand() % 9);
        int op2 = mrand();
        
        // Vetorial
        switch (opt)
        {
        case  0: vet_vv_sum(&A[blk], &B[blk], &OUT[blk]); break;
        case  1: vet_vv_sub(&A[blk], &B[blk], &OUT[blk]); break;
        case  2: vet_vv_div(&A[blk], &B[blk], &OUT[blk]); break;
        case  3: vet_vv_mul(&A[blk], &B[blk], &OUT[blk]); break;
        
        case  4: vet_vx_sum(&A[blk], op2, &OUT[blk]); break;
        case  5: vet_vx_sub(&A[blk], op2, &OUT[blk]); break;
        case  6: vet_vx_div(&A[blk], op2, &OUT[blk]); break;
        case  7: vet_vx_mul(&A[blk], op2, &OUT[blk]); break;

        case  8: vet_vi_sum(&A[blk], &OUT[blk]); break;
        }
        
        int chksum_vector = checksum(&OUT[blk], EL_PER_BLOCK);
        
        // Escalar
        switch (opt){
            case  0: for(int i = blk; i < blk+EL_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] + B[i];
                break;
            case  1: for(int i = blk; i < blk+EL_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] - B[i];
                break;
            case  2: for(int i = blk; i < blk+EL_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] / B[i];
                break;
            case  3: for(int i = blk; i < blk+EL_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] * B[i];
                break;
            case  4: for(int i = blk; i < blk+EL_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] + op2;
                break;
            case  5: for(int i = blk; i < blk+EL_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] - op2;
                break;
            case  6: for(int i = blk; i < blk+EL_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] / op2;
                break;
            case  7: for(int i = blk; i < blk+EL_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] * op2;
                break;
            case  8: for(int i = blk; i < blk+EL_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] + 5;
                break;
        }

        int chksum_scalar = checksum(&OUT_SCALAR[blk], EL_PER_BLOCK);
        if (chksum_vector != chksum_scalar) {
            char* helper[9] = {"VADD.VV", "VSUB.V", "VDIV.VV", "VMUL.VV", "VADD.VX", "VSUB.VX",
                 "VDIV.VX", "VMUL.VX", "VADD.VI"};
            printf("Mismatch at block at position: %d;\n vector checksum = %08x, scalar checksum = %08x;\n",
            blk/EL_PER_BLOCK, chksum_vector, chksum_scalar);
            printf("Operation %s, Operator2: %d\n", helper[opt], op2);
        return;
        }

    }

}

int main(){
    printf("Doing random batch tests with registers v0-v7 with seed %d\n", SEED);
    random_test(SEED);
}