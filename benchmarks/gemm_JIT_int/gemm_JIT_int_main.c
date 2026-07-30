/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* gemm.c: this file is part of PolyBench/C */


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
#include "gemm_JIT_int.h"

int32_t ADDRESS_VECTOR[20];

/* Array initialization. */
static
void init_array(int ni, int nj, int nk,
		DATA_TYPE *alpha,
		DATA_TYPE *beta,
		DATA_TYPE POLYBENCH_2D(C,NI,NJ,ni,nj),
		DATA_TYPE POLYBENCH_2D(A,NI,NK,ni,nk),
		DATA_TYPE POLYBENCH_2D(B,NK,NJ,nk,nj),
        DATA_TYPE POLYBENCH_2D(VET_C,NI,NJ,ni,nj),
		DATA_TYPE POLYBENCH_2D(VET_A,NI,NK,ni,nk),
		DATA_TYPE POLYBENCH_2D(VET_B,NK,NJ,nk,nj))
{
  int i, j;

  ADDRESS_VECTOR[1] = RET_INSTR;
  *alpha = 2;
  *beta = 3;
  for (i = 0; i < ni; i++){
    for (j = 0; j < nj; j+=4){
        C[i][j]   = (DATA_TYPE) ((i* j   +1) % ni);// / ni;
        C[i][j+1] = (DATA_TYPE) ((i*(j+1)+1) % ni);// / ni;
        C[i][j+2] = (DATA_TYPE) ((i*(j+2)+1) % ni);// / ni;
        C[i][j+3] = (DATA_TYPE) ((i*(j+3)+1) % ni);// / ni;

        VET_C[i][j]   = (DATA_TYPE) ((i* j   +1) % ni);// / ni;
        VET_C[i][j+1] = (DATA_TYPE) ((i*(j+1)+1) % ni);// / ni;
        VET_C[i][j+2] = (DATA_TYPE) ((i*(j+2)+1) % ni);// / ni;
        VET_C[i][j+3] = (DATA_TYPE) ((i*(j+3)+1) % ni);// / ni;
        //ADDRESS_VECTOR[1] = add_instruction(DIV);
    }
  }

  for (i = 0; i < ni; i++)
    for (j = 0; j < nk; j+=4){
        A[i][j]   = (DATA_TYPE) (i*(j+1) % nk);// / nk;
        A[i][j+1] = (DATA_TYPE) (i*(j+2) % nk);// / nk;
        A[i][j+2] = (DATA_TYPE) (i*(j+3) % nk);// / nk;
        A[i][j+3] = (DATA_TYPE) (i*(j+4) % nk);// / nk;

        VET_A[i][j]   = (DATA_TYPE) (i*(j+1) % nk);// / nk;
        VET_A[i][j+1] = (DATA_TYPE) (i*(j+2) % nk);// / nk;
        VET_A[i][j+2] = (DATA_TYPE) (i*(j+3) % nk);// / nk;
        VET_A[i][j+3] = (DATA_TYPE) (i*(j+4) % nk);// / nk;
    }
  for (i = 0; i < nk; i++)
    for (j = 0; j < nj; j+=4){
        B[i][j]   = (DATA_TYPE) (i*(j+2) % nj);// / nj;
        B[i][j+1] = (DATA_TYPE) (i*(j+3) % nj);// / nj;
        B[i][j+2] = (DATA_TYPE) (i*(j+4) % nj);// / nj;
        B[i][j+3] = (DATA_TYPE) (i*(j+5) % nj);// / nj;

        VET_B[i][j]   = (DATA_TYPE) (i*(j+2) % nj);// / nj;
        VET_B[i][j+1] = (DATA_TYPE) (i*(j+3) % nj);// / nj;
        VET_B[i][j+2] = (DATA_TYPE) (i*(j+4) % nj);// / nj;
        VET_B[i][j+3] = (DATA_TYPE) (i*(j+5) % nj);// / nj;
    }
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int ni, int nj,
		 DATA_TYPE POLYBENCH_2D(C,NI,NJ,ni,nj))
{
  int i, j;

  POLYBENCH_DUMP_START;
  POLYBENCH_DUMP_BEGIN("C");
  for (i = 0; i < ni; i++)
    for (j = 0; j < nj; j++) {
	if ((i * ni + j) % 20 == 0) printf ("\n");
	printf (DATA_PRINTF_MODIFIER, C[i][j]);
    }
  POLYBENCH_DUMP_END("C");
  POLYBENCH_DUMP_FINISH;
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_gemm(int ni, int nj, int nk,
		 DATA_TYPE alpha,
		 DATA_TYPE beta,
		 DATA_TYPE POLYBENCH_2D(C,NI,NJ,ni,nj),
		 DATA_TYPE POLYBENCH_2D(A,NI,NK,ni,nk),
		 DATA_TYPE POLYBENCH_2D(B,NK,NJ,nk,nj),
         DATA_TYPE POLYBENCH_2D(VET_C,NI,NJ,ni,nj),
		 DATA_TYPE POLYBENCH_2D(VET_A,NI,NK,ni,nk),
		 DATA_TYPE POLYBENCH_2D(VET_B,NK,NJ,nk,nj)
        )
{

  int i, j, k;

//BLAS PARAMS
//TRANSA = 'N'
//TRANSB = 'N'
// => Form C := alpha*A*B + beta*C,
//A is NIxNK
//B is NKxNJ
//C is NIxNJ
#pragma scop
int regs[3] = {0, 8, 16};
int rx1[3]  = {0, 0,  0};
int rx2[3]  = {1, 1,  1};
int rx3[3]   = {0, 1, 2};

for (i = 0; i < _PB_NI; i++) {
    ADDRESS_VECTOR[1] = RET_INSTR;
    for (j = 0; j < _PB_NJ; j+=4){
        load_init_values_scalar((int32_t*)&C[i][j], regs, 3);
        
        C[i][j]   *= beta;
        C[i][j+1] *= beta;
        C[i][j+2] *= beta;
        C[i][j+3] *= beta;

        t0_VALUE = beta;
        ADDRESS_VECTOR[0] = add_instruction(VMUL_VX, rx1, regs);
        execute_RIS((int32_t*)&VET_C[i][j], regs, ADDRESS_VECTOR, (int32_t*)&VET_C[i][j], 1);
        
        if(!manual_convergence((int32_t*)&scalar_res[0][0], (int32_t*)&VET_C[i][j], 1, VLEN / SEW))
            printf("Divergence\n");
        
        if(!manual_convergence((int32_t*)&C[i][j], (int32_t*)&VET_C[i][j], 1, VLEN / SEW)){
            printf("Divergence\n");
        }
    }

    ADDRESS_VECTOR[2] = RET_INSTR;
    for (k = 0; k < _PB_NK; k++) {
        for (j = 0; j < _PB_NJ; j+= 4){
            int32_t VA[4] = {A[i][k], A[i][k], A[i][k], A[i][k]};
            load_init_values_scalar((int32_t*)&C[i][j], &regs[0], 1);
            load_init_values_scalar((int32_t*)&VA[0],   &regs[1], 1);
            load_init_values_scalar((int32_t*)&B[k][j], &regs[2], 1);
            C[i][j]   = ((alpha * A[i][k]) * B[k][j]  ) + C[i][j]    ;
            C[i][j+1] = ((alpha * A[i][k]) * B[k][j+1]) + C[i][j + 1];
            C[i][j+2] = ((alpha * A[i][k]) * B[k][j+2]) + C[i][j + 2];
            C[i][j+3] = ((alpha * A[i][k]) * B[k][j+3]) + C[i][j + 3];
            t0_VALUE = alpha;
            ADDRESS_VECTOR[0] = add_instruction(VMUL_VX,  rx2, regs);// A' = A * alpha
            ADDRESS_VECTOR[1] = add_instruction(VMACC_VV, rx3, regs);// C += A' * B 

            set_vet_settings();
            load_init_values_vector((int32_t*)&VA[0],       &regs[1], 1);
            load_init_values_vector((int32_t*)&VET_B[k][j], &regs[2], 1);
            execute_RIS((int32_t*)&VET_C[i][j], regs, ADDRESS_VECTOR, (int32_t*)&VET_C[i][j], 1);
            if(!manual_convergence((int32_t*)&scalar_res[0][0], (int32_t*)&VET_C[i][j], 1, VLEN / SEW * LMUL)){
                printf("Divergence\n");
                exit(0);
            }
            if(!manual_convergence((int32_t*)&C[i][j], (int32_t*)&VET_C[i][j], 1, VLEN / SEW * LMUL)){
                printf("Divergence\n");
                print_matrix((int32_t*)&VET_C[i][j], 4, 1);
                print_matrix((int32_t*)&C[i][j], 4, 1);
                exit(0);
            }
        }

    }
  }
#pragma endscop

}

POLYBENCH_2D_ARRAY_DECL(C,DATA_TYPE,NI,NJ,NI,NJ);
POLYBENCH_2D_ARRAY_DECL(A,DATA_TYPE,NI,NK,NI,NK);
POLYBENCH_2D_ARRAY_DECL(B,DATA_TYPE,NK,NJ,NK,NJ);

POLYBENCH_2D_ARRAY_DECL(VET_C,DATA_TYPE,NI,NJ,NI,NJ);
POLYBENCH_2D_ARRAY_DECL(VET_A,DATA_TYPE,NI,NK,NI,NK);
POLYBENCH_2D_ARRAY_DECL(VET_B,DATA_TYPE,NK,NJ,NK,NJ);

int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int ni = NI;
  int nj = NJ;
  int nk = NK;

  /* Variable declaration/allocation. */
  DATA_TYPE alpha;
  DATA_TYPE beta;

  /* Initialize array(s). */
  init_array (ni, nj, nk, &alpha, &beta,
	      POLYBENCH_ARRAY(C),
	      POLYBENCH_ARRAY(A),
	      POLYBENCH_ARRAY(B),
          POLYBENCH_ARRAY(VET_C),
          POLYBENCH_ARRAY(VET_A),
          POLYBENCH_ARRAY(VET_B));
  //print_array(ni, nj, POLYBENCH_ARRAY(C));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_gemm (ni, nj, nk,
	       alpha, beta,
	       POLYBENCH_ARRAY(C),
	       POLYBENCH_ARRAY(A),
	       POLYBENCH_ARRAY(B),
           POLYBENCH_ARRAY(VET_C),
           POLYBENCH_ARRAY(VET_A),
           POLYBENCH_ARRAY(VET_B));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(ni, nj,  POLYBENCH_ARRAY(C)));

  /* Be clean. */
  //POLYBENCH_FREE_ARRAY(C);
  //POLYBENCH_FREE_ARRAY(A);
  //POLYBENCH_FREE_ARRAY(B);

  return 0;
}
