#include <util.h>
#include "mysrand.h"
#include "myutil.h"

#define N_VECTOR    1024 * 1024
#define SEED 0x12345678
#define SEW  32
#define VLEN 128
#define LMUL 1
#define ELEMENTS_PER_BLOCK 32

/* EXTERNALS */
extern int get_max_vl_8();
extern int get_max_vl_4();
extern int get_max_vl_2();
extern int get_max_vl_1();

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

volatile int A[N_VECTOR];
volatile int B[N_VECTOR];
volatile int OUT[N_VECTOR];
volatile int OUT_SCALAR[N_VECTOR];

int checksum(int *vec, int n) {
  int chk = 0;
  for (int i = 0; i < n; i++) {
    chk ^= vec[i];
  }
  return chk;
}

void random_test(int seed) {
    msrand(seed);
    for (int i = 0; i < N_VECTOR; i++) {
        A[i] = mrand();
        B[i] = mrand();
        OUT[i] = mrand();
    }
    printf("DONE INIT VALUES, BEGINNING EXECUTION \n");
    
    for(int blk = 0; blk < N_VECTOR;blk += ELEMENTS_PER_BLOCK) {
        int el_this_block = 0;
        int setting = abs(mrand() % 4);
        //printf("setting: %d \n", setting);
        switch (setting){
            case 0: el_this_block = get_max_vl_1(); break;
            case 1: el_this_block = get_max_vl_2(); break;
            case 2: el_this_block = get_max_vl_4(); break;
            case 3: el_this_block = get_max_vl_8(); break;
        }
        
        int opt = abs(mrand() % 9);
        int op2 = mrand();
        
        //printf("opt: %d; op2: %d \n", opt, op2);

        // Vetorial
        for(int ver = 0; ver < ELEMENTS_PER_BLOCK; ver += el_this_block)
        {
            //printf("VER:%d\nBLOCK:%d\n", ver, blk);
            switch (opt){
                case  0: vet_vv_sum(&A[blk + ver], &B[blk + ver], &OUT[blk + ver]); break;
                case  1: vet_vv_sub(&A[blk + ver], &B[blk + ver], &OUT[blk + ver]); break;
                case  2: vet_vv_div(&A[blk + ver], &B[blk + ver], &OUT[blk + ver]); break;
                case  3: vet_vv_mul(&A[blk + ver], &B[blk + ver], &OUT[blk + ver]); break;
            
                case  4: vet_vx_sum(&A[blk + ver], op2, &OUT[blk + ver]); break;
                case  5: vet_vx_sub(&A[blk + ver], op2, &OUT[blk + ver]); break;
                case  6: vet_vx_div(&A[blk + ver], op2, &OUT[blk + ver]); break;
                case  7: vet_vx_mul(&A[blk + ver], op2, &OUT[blk + ver]); break;
                
                case  8: vet_vi_sum(&A[blk + ver], &OUT[blk + ver]); break;
            }
        }
        int chksum_vector = checksum(&OUT[blk], ELEMENTS_PER_BLOCK);
        
        // Escalar
        switch (opt){
            case  0: for(int i = blk; i < blk+ELEMENTS_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] + B[i];
                break;
            case  1: for(int i = blk; i < blk+ELEMENTS_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] - B[i];
                break;
            case  2: for(int i = blk; i < blk+ELEMENTS_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] / B[i];
                break;
            case  3: for(int i = blk; i < blk+ELEMENTS_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] * B[i];
                break;
            case  4: for(int i = blk; i < blk+ELEMENTS_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] + op2;
                break;
            case  5: for(int i = blk; i < blk+ELEMENTS_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] - op2;
                break;
            case  6: for(int i = blk; i < blk+ELEMENTS_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] / op2;
                break;
            case  7: for(int i = blk; i < blk+ELEMENTS_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] * op2;
                break;
            case  8: for(int i = blk; i < blk+ELEMENTS_PER_BLOCK; i++) OUT_SCALAR[i] = A[i] + 5;
                break;
        }

        int chksum_scalar = checksum(&OUT_SCALAR[blk], ELEMENTS_PER_BLOCK);
        if (chksum_vector != chksum_scalar) {
        printf("Mismatch at block %d: vector checksum = %08x, scalar checksum = %08x\n",
                blk/el_this_block, chksum_vector, chksum_scalar);
        return;
        }

    }

}

int main(){
    printf("Doing random batch tests with registers v0-v7 with seed %d\n", SEED);
    random_test(SEED);
}