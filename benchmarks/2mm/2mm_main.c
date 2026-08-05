/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* 2mm.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
#include "2mm.h"

int32_t ADDRESS_VECTOR[20];

/* Array initialization. */
static
void init_array(int ni, int nj, int nk, int nl,
		DATA_TYPE *alpha,
		DATA_TYPE *beta,
		DATA_TYPE POLYBENCH_2D(tmp,NI,NJ,ni,nj),
    DATA_TYPE POLYBENCH_2D(A,NI,NK,ni,nk),
		DATA_TYPE POLYBENCH_2D(B,NK,NJ,nk,nj),
		DATA_TYPE POLYBENCH_2D(C,NJ,NL,nj,nl),
		DATA_TYPE POLYBENCH_2D(D,NI,NL,ni,nl),
		DATA_TYPE POLYBENCH_2D(VET_D,NI,NL,ni,nl),
		DATA_TYPE POLYBENCH_2D(VET_tmp,NI,NJ,ni,nj))
{
  int i, j;

  *alpha = 1.5;
  *beta = 1.2;
  for (i = 0; i < ni; i++)
    for (j = 0; j < nk; j++)
      A[i][j] = (DATA_TYPE) ((i*j+1) % ni);// / ni;
  for (i = 0; i < nk; i++)
    for (j = 0; j < nj; j++)
      B[i][j] = (DATA_TYPE) (i*(j+1) % nj);// / nj;
  for (i = 0; i < nj; i++)
    for (j = 0; j < nl; j++)
      C[i][j] = (DATA_TYPE) ((i*(j+3)+1) % nl);// / nl;
  for (i = 0; i < ni; i++)
    for (j = 0; j < nl; j++){
      D[i][j] = (DATA_TYPE) (i*(j+2) % nk);// / nk;
      VET_D[i][j] = (DATA_TYPE) (i*(j+2) % nk);// / nk;
    }

  for (i = 0; i < _PB_NI; i++)
    for (j = 0; j < _PB_NJ; j++){
      VET_tmp[i][j] = SCALAR_VAL(0.0);
      tmp[i][j]     = SCALAR_VAL(0.0);
    }
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int ni, int nl,
		 DATA_TYPE POLYBENCH_2D(D,NI,NL,ni,nl))
{
  int i, j;

  POLYBENCH_DUMP_START;
  POLYBENCH_DUMP_BEGIN("D");
  for (i = 0; i < ni; i++)
    for (j = 0; j < nl; j++) {
	if ((i * ni + j) % 20 == 0) printf ("\n");
	printf (DATA_PRINTF_MODIFIER, D[i][j]);
    }
  POLYBENCH_DUMP_END("D");
  POLYBENCH_DUMP_FINISH;
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_2mm(int ni, int nj, int nk, int nl,
		DATA_TYPE alpha,
		DATA_TYPE beta,
		DATA_TYPE POLYBENCH_2D(tmp,NI,NJ,ni,nj),
		DATA_TYPE POLYBENCH_2D(A,NI,NK,ni,nk),
		DATA_TYPE POLYBENCH_2D(B,NK,NJ,nk,nj),
		DATA_TYPE POLYBENCH_2D(C,NJ,NL,nj,nl),
		DATA_TYPE POLYBENCH_2D(D,NI,NL,ni,nl),
		DATA_TYPE POLYBENCH_2D(VET_D,NI,NL,ni,nl),
    DATA_TYPE POLYBENCH_2D(VET_tmp,NI,NJ,ni,nj))
{
  int i, j, k;

  int regs[3]  = {0, 8, 16};
  int rx1[3]   = {0, 1,  2};
  int rxmul[3] = {0, 0, 0};


#pragma scop
  /* D := alpha*A*B*C + beta*D */
  ADDRESS_VECTOR[1] = RET_INSTR;
  for (i = 0; i < _PB_NI; i++) {
    for (k = 0; k < _PB_NK; ++k) {
      for (j = 0; j < _PB_NJ; j+=4) {
        tmp[i][j]   += alpha * A[i][k] * B[k][j];
        tmp[i][j+1] += alpha * A[i][k] * B[k][j+1];
        tmp[i][j+2] += alpha * A[i][k] * B[k][j+2];
        tmp[i][j+3] += alpha * A[i][k] * B[k][j+3];
        
        load_init_values_scalar(&VET_tmp[i][j], &regs[0], 1);
        load_init_values_scalar(&B[k][j], &regs[1], 1);
        t0_VALUE = alpha * A[i][k];

        ADDRESS_VECTOR[0] = add_instruction(VMACC_VX, &rx1[0], regs);
        load_init_values_vector(&B[k][j], &regs[1], 1);
        execute_RIS(&VET_tmp[i][j], regs, ADDRESS_VECTOR, &VET_tmp[i][j], 1);
      }
    }
  }

  ADDRESS_VECTOR[1] = RET_INSTR;
  for (i = 0; i < _PB_NI; i++)
    for (j = 0; j < _PB_NL; j+=4){
      D[i][j]   *= beta;
      D[i][j+1] *= beta;
      D[i][j+2] *= beta;
      D[i][j+3] *= beta;

      load_init_values_scalar(&VET_D[i][j], regs, 1);
      t0_VALUE = beta;
      ADDRESS_VECTOR[0] = add_instruction(VMUL_VX, rxmul, regs);
      execute_RIS(&VET_D[i][j], regs, ADDRESS_VECTOR, &VET_D[i][j], 1);
    }
  
  if(!manual_convergence((int32_t*)&VET_tmp[0][0], (int32_t*)&tmp[0][0], ni, nj)) printf("Divergence TEMP\n");
  if(!manual_convergence((int32_t*)&VET_D[0][0], (int32_t*)&D[0][0], ni, nl)) printf("Divergence D\n");

  ADDRESS_VECTOR[1] = RET_INSTR;
  for (i = 0; i < _PB_NI; i++)
    for (k = 0; k < _PB_NJ; ++k){
      for (j = 0; j < _PB_NL; j+=4) {
        D[i][j]   += tmp[i][k] * C[k][j];
        D[i][j+1] += tmp[i][k] * C[k][j+1];
        D[i][j+2] += tmp[i][k] * C[k][j+2];
        D[i][j+3] += tmp[i][k] * C[k][j+3];

        load_init_values_scalar(&VET_D[i][j], &regs[0], 1);
        load_init_values_scalar(&C[k][j], &regs[1], 1);
        t0_VALUE = VET_tmp[i][k];
        ADDRESS_VECTOR[0] = add_instruction(VMACC_VX, rx1, regs);
        load_init_values_vector(&C[k][j], &regs[1], 1);
        execute_RIS(&VET_D[i][j], regs, ADDRESS_VECTOR, &VET_D[i][j], 1);
      }
    }
#pragma endscop

if(!manual_convergence((int32_t*)&VET_D[0][0], (int32_t*)&D[0][0], ni, nl)) printf("Divergence\n");


}

POLYBENCH_2D_ARRAY_DECL(tmp,DATA_TYPE,NI,NJ,ni,nj);
POLYBENCH_2D_ARRAY_DECL(VET_tmp,DATA_TYPE,NI,NJ,ni,nj);
POLYBENCH_2D_ARRAY_DECL(A,DATA_TYPE,NI,NK,ni,nk);
POLYBENCH_2D_ARRAY_DECL(B,DATA_TYPE,NK,NJ,nk,nj);
POLYBENCH_2D_ARRAY_DECL(C,DATA_TYPE,NJ,NL,nj,nl);
POLYBENCH_2D_ARRAY_DECL(D,DATA_TYPE,NI,NL,ni,nl);
POLYBENCH_2D_ARRAY_DECL(VET_D,DATA_TYPE,NI,NL,ni,nl);


int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int ni = NI;
  int nj = NJ;
  int nk = NK;
  int nl = NL;

  /* Variable declaration/allocation. */
  DATA_TYPE alpha;
  DATA_TYPE beta;

  /* Initialize array(s). */
  init_array (ni, nj, nk, nl, &alpha, &beta,
	      POLYBENCH_ARRAY(tmp),
	      POLYBENCH_ARRAY(A),
	      POLYBENCH_ARRAY(B),
	      POLYBENCH_ARRAY(C),
	      POLYBENCH_ARRAY(D),
        POLYBENCH_ARRAY(VET_D),
        POLYBENCH_ARRAY(VET_tmp));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_2mm (ni, nj, nk, nl,
	      alpha, beta,
	      POLYBENCH_ARRAY(tmp),
	      POLYBENCH_ARRAY(A),
	      POLYBENCH_ARRAY(B),
	      POLYBENCH_ARRAY(C),
	      POLYBENCH_ARRAY(D),
        POLYBENCH_ARRAY(VET_D),
        POLYBENCH_ARRAY(VET_tmp));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(ni, nl,  POLYBENCH_ARRAY(D)));

  /* Be clean. */
  //POLYBENCH_FREE_ARRAY(tmp);
  //POLYBENCH_FREE_ARRAY(A);
  //POLYBENCH_FREE_ARRAY(B);
  //POLYBENCH_FREE_ARRAY(C);
  //POLYBENCH_FREE_ARRAY(D);

  return 0;
}