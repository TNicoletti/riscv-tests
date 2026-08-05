/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* 3mm.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
#include "3mm.h"

int32_t ADDRESS_VECTOR[20];

/* Array initialization. */
static
void init_array(int ni, int nj, int nk, int nl, int nm,
		DATA_TYPE POLYBENCH_2D(A,NI,NK,ni,nk),
		DATA_TYPE POLYBENCH_2D(B,NK,NJ,nk,nj),
		DATA_TYPE POLYBENCH_2D(C,NJ,NM,nj,nm),
		DATA_TYPE POLYBENCH_2D(D,NM,NL,nm,nl),
		DATA_TYPE POLYBENCH_2D(E,NI,NJ,ni,nj),
		DATA_TYPE POLYBENCH_2D(VET_E,NI,NJ,ni,nj),
    DATA_TYPE POLYBENCH_2D(F,NJ,NL,nj,nl),
    DATA_TYPE POLYBENCH_2D(VET_F,NJ,NL,nj,nl),
		DATA_TYPE POLYBENCH_2D(G,NI,NL,ni,nl),
		DATA_TYPE POLYBENCH_2D(VET_G,NI,NL,ni,nl)
  )
{
  int i, j;

  for (i = 0; i < ni; i++)
    for (j = 0; j < nk; j++)
      A[i][j] = (DATA_TYPE) ((i*j+1) % ni) ;/// (5*ni);
  for (i = 0; i < nk; i++)
    for (j = 0; j < nj; j++)
      B[i][j] = (DATA_TYPE) ((i*(j+1)+2) % nj);// / (5*nj);
  for (i = 0; i < nj; i++)
    for (j = 0; j < nm; j++)
      C[i][j] = (DATA_TYPE) (i*(j+3) % nl);// / (5*nl);
  for (i = 0; i < nm; i++)
    for (j = 0; j < nl; j++)
      D[i][j] = (DATA_TYPE) ((i*(j+2)+2) % nk);// / (5*nk);

  for(i = 0; i < NI; i++)
    for(j = 0; j < NJ; j++){
      E[i][j] = 0;
      VET_E[i][j] = 0;
    }

  for(i = 0; i < NJ; i++)
    for(j = 0; j < NL; j++){
      F[i][j] = 0;
      VET_F[i][j] = 0;
    }

  for(i = 0; i < NI; i++)
    for(j = 0; j < NL; j++){
      G[i][j] = 0;
      VET_G[i][j] = 0;
    }
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int ni, int nl,
		 DATA_TYPE POLYBENCH_2D(G,NI,NL,ni,nl))
{
  int i, j;

  POLYBENCH_DUMP_START;
  POLYBENCH_DUMP_BEGIN("G");
  for (i = 0; i < ni; i++)
    for (j = 0; j < nl; j++) {
	if ((i * ni + j) % 20 == 0) printf ("\n");
	printf (DATA_PRINTF_MODIFIER, G[i][j]);
    }
  POLYBENCH_DUMP_END("G");
  POLYBENCH_DUMP_FINISH;
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_3mm(int ni, int nj, int nk, int nl, int nm,
		DATA_TYPE POLYBENCH_2D(E,NI,NJ,ni,nj),
		DATA_TYPE POLYBENCH_2D(A,NI,NK,ni,nk),  
		DATA_TYPE POLYBENCH_2D(B,NK,NJ,nk,nj),
		DATA_TYPE POLYBENCH_2D(F,NJ,NL,nj,nl),
		DATA_TYPE POLYBENCH_2D(C,NJ,NM,nj,nm),
		DATA_TYPE POLYBENCH_2D(D,NM,NL,nm,nl),
		DATA_TYPE POLYBENCH_2D(G,NI,NL,ni,nl),
		DATA_TYPE POLYBENCH_2D(VET_E,NI,NJ,ni,nj),
    DATA_TYPE POLYBENCH_2D(VET_F,NJ,NL,nj,nl),
		DATA_TYPE POLYBENCH_2D(VET_G,NI,NL,ni,nl)
  )
{
  int i, j, k;

int32_t regs[3] = {0, 8, 16};
int32_t rx[3] = {0, 1, 2}; 

#pragma scop

  ADDRESS_VECTOR[1] = RET_INSTR;
  for (i = 0; i < _PB_NI; i++) {
    for (k = 0; k < _PB_NK; k++) {
        for (j = 0; j < _PB_NJ; j += 4) {
            load_init_values_scalar((int32_t*)&VET_E[i][j], &regs[0], 1);
            //load_init_values_scalar((int32_t*)&A[i][k], &regs[1], 1);
            load_init_values_scalar((int32_t*)&B[k][j], &regs[1], 1);
            t0_VALUE = A[i][k];
            
            E[i][j]   += A[i][k] * B[k][j];
            E[i][j+1] += A[i][k] * B[k][j+1];
            E[i][j+2] += A[i][k] * B[k][j+2];
            E[i][j+3] += A[i][k] * B[k][j+3];

            ADDRESS_VECTOR[0] = add_instruction(VMACC_VX, &rx[0], regs);
            load_init_values_vector((int32_t*)&B[k][j], &regs[1], 1);

            execute_RIS((int32_t*)&VET_E[i][j], regs, ADDRESS_VECTOR, (int32_t*)&VET_E[i][j], 1);
        }
    }
}

  ADDRESS_VECTOR[1] = RET_INSTR;
  for (i = 0; i < _PB_NJ; i++) {
    for (k = 0; k < _PB_NM; k++) {
        for (j = 0; j < _PB_NL; j+=4) {
            load_init_values_scalar((int32_t*)&VET_F[i][j], &regs[0], 1);
            load_init_values_scalar((int32_t*)&D[k][j], &regs[1], 1);
            t0_VALUE = C[i][k];
            
            F[i][j]   += C[i][k] * D[k][j];
            F[i][j+1] += C[i][k] * D[k][j+1];
            F[i][j+2] += C[i][k] * D[k][j+2];
            F[i][j+3] += C[i][k] * D[k][j+3];

            ADDRESS_VECTOR[0] = add_instruction(VMACC_VX, &rx[0], regs);
            load_init_values_vector((int32_t*)&D[k][j], &regs[1], 1);
            execute_RIS((int32_t*)&VET_F[i][j], regs, ADDRESS_VECTOR, (int32_t*)&VET_F[i][j], 1);
        }
    }
  }

  ADDRESS_VECTOR[1] = RET_INSTR;
  for (i = 0; i < _PB_NI; i++) {
    for (k = 0; k < _PB_NJ; k++) {
        for (j = 0; j < _PB_NL; j += 4) {
            load_init_values_scalar((int32_t*)&VET_G[i][j], &regs[0], 1);
            load_init_values_scalar((int32_t*)&F[k][j], &regs[1], 1);
            t0_VALUE = E[i][k];

            G[i][j]   += E[i][k] * F[k][j];
            G[i][j+1] += E[i][k] * F[k][j+1];
            G[i][j+2] += E[i][k] * F[k][j+2];
            G[i][j+3] += E[i][k] * F[k][j+3];

            ADDRESS_VECTOR[0] = add_instruction(VMACC_VX, &rx[0], regs);
            
            load_init_values_vector((int32_t*)&F[k][j], &regs[1], 1);            
            execute_RIS((int32_t*)&VET_G[i][j], regs, ADDRESS_VECTOR, (int32_t*)&VET_G[i][j], 1);
        }
    }
  }

#pragma endscop

    //print_matrix((int32_t*)&E[0][0], ni, nl);
    //printf("\n========= SEPARATION =========\n");
    //print_matrix((int32_t*)&VET_E[0][0], ni, nl);
    if(!manual_convergence((int32_t*)&VET_E[0][0], (int32_t*)&E[0][0], ni, nj)) printf("Divergence E\n");
    if(!manual_convergence((int32_t*)&VET_F[0][0], (int32_t*)&F[0][0], nj, nl)) printf("Divergence F\n");
    if(!manual_convergence((int32_t*)&VET_G[0][0], (int32_t*)&G[0][0], ni, nl)) printf("Divergence G\n");

}

/* Variable declaration/allocation. */
  POLYBENCH_2D_ARRAY_DECL(E, DATA_TYPE, NI, NJ, NI, NJ);
  POLYBENCH_2D_ARRAY_DECL(A, DATA_TYPE, NI, NK, NI, NK);
  POLYBENCH_2D_ARRAY_DECL(B, DATA_TYPE, NK, NJ, NK, NJ);
  POLYBENCH_2D_ARRAY_DECL(F, DATA_TYPE, NJ, NL, NJ, NL);
  POLYBENCH_2D_ARRAY_DECL(C, DATA_TYPE, NJ, NM, NJ, NM);
  POLYBENCH_2D_ARRAY_DECL(D, DATA_TYPE, NM, NL, NM, NL);
  POLYBENCH_2D_ARRAY_DECL(G, DATA_TYPE, NI, NL, NI, NL);

  POLYBENCH_2D_ARRAY_DECL(VET_E, DATA_TYPE, NI, NJ, NI, NJ);
  POLYBENCH_2D_ARRAY_DECL(VET_F, DATA_TYPE, NJ, NL, NJ, NL);
  POLYBENCH_2D_ARRAY_DECL(VET_G, DATA_TYPE, NI, NL, NI, NL);

int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int ni = NI;
  int nj = NJ;
  int nk = NK;
  int nl = NL;
  int nm = NM;

  

  /* Initialize array(s). */
  init_array (ni, nj, nk, nl, nm,
	      POLYBENCH_ARRAY(A),
	      POLYBENCH_ARRAY(B),
	      POLYBENCH_ARRAY(C),
	      POLYBENCH_ARRAY(D),
        POLYBENCH_ARRAY(E),
        POLYBENCH_ARRAY(VET_E),
        POLYBENCH_ARRAY(F),
        POLYBENCH_ARRAY(VET_F),
        POLYBENCH_ARRAY(G),
        POLYBENCH_ARRAY(VET_G)
);

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_3mm (ni, nj, nk, nl, nm,
	      POLYBENCH_ARRAY(E),
	      POLYBENCH_ARRAY(A),
	      POLYBENCH_ARRAY(B),
	      POLYBENCH_ARRAY(F),
	      POLYBENCH_ARRAY(C),
	      POLYBENCH_ARRAY(D),
	      POLYBENCH_ARRAY(G),
	      POLYBENCH_ARRAY(VET_E),
	      POLYBENCH_ARRAY(VET_F),
	      POLYBENCH_ARRAY(VET_G)
      );

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  //polybench_prevent_dce(print_array(ni, nl,  POLYBENCH_ARRAY(G)));

  /* Be clean. */
  //POLYBENCH_FREE_ARRAY(E);
  //POLYBENCH_FREE_ARRAY(A);
  //POLYBENCH_FREE_ARRAY(B);
  //POLYBENCH_FREE_ARRAY(F);
  //POLYBENCH_FREE_ARRAY(C);
  //POLYBENCH_FREE_ARRAY(D);
  //POLYBENCH_FREE_ARRAY(G);

  return 0;
}