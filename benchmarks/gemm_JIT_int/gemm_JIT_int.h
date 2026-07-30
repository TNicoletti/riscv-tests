/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
#include "global_configs.h"  // Global configs like LMUL, SEW, etc
#include "add_instruction.h" // Add instruction and related

 #ifndef _GEMM_H
# define _GEMM_H

/* Default to LARGE_DATASET. */
# if !defined(MINI_DATASET) && !defined(SMALL_DATASET) && !defined(MEDIUM_DATASET) && !defined(LARGE_DATASET) && !defined(EXTRALARGE_DATASET)
#  define SMALL_DATASET
# endif

# if !defined(NI) && !defined(NJ) && !defined(NK)
/* Define sample dataset sizes. */
#  ifdef MINI_DATASET
#   define NI 20
#   define NJ 25
#   define NK 30
#  endif

#  ifdef SMALL_DATASET
#   define NI 60
#   define NJ 72
#   define NK 80
#  endif

#  ifdef CUSTOM_DATASET
#   define NI 160
#   define NJ 224
#   define NK 256
#  endif

#  ifdef MEDIUM_DATASET /*~ 39s */
#   define NI 200
#   define NJ 220
#   define NK 240
#  endif

#  ifdef LARGE_DATASET /*~ 4875s ~1.5h*/
#   define NI 1000
#   define NJ 1100
#   define NK 1200
#  endif

#  ifdef EXTRALARGE_DATASET /*~ 44170s 12.25h */
#   define NI 2000
#   define NJ 2300
#   define NK 2600
#  endif


#endif /* !(NI NJ NK) */

# define _PB_NI POLYBENCH_LOOP_BOUND(NI,ni)
# define _PB_NJ POLYBENCH_LOOP_BOUND(NJ,nj)
# define _PB_NK POLYBENCH_LOOP_BOUND(NK,nk)


/* Default data type */
# if !defined(DATA_TYPE_IS_INT) && !defined(DATA_TYPE_IS_FLOAT) && !defined(DATA_TYPE_IS_DOUBLE)
#  define DATA_TYPE_IS_INT
# endif

#ifdef DATA_TYPE_IS_INT
#  define DATA_TYPE int
#  define DATA_PRINTF_MODIFIER "%d "
#endif

#ifdef DATA_TYPE_IS_FLOAT
#  define DATA_TYPE float
#  define DATA_PRINTF_MODIFIER "%0.2f "
#  define SCALAR_VAL(x) x##f
#  define SQRT_FUN(x) sqrtf(x)
#  define EXP_FUN(x) expf(x)
#  define POW_FUN(x,y) powf(x,y)
# endif

#ifdef DATA_TYPE_IS_DOUBLE
#  define DATA_TYPE double
#  define DATA_PRINTF_MODIFIER "%0.2lf "
#  define SCALAR_VAL(x) x
#  define SQRT_FUN(x) sqrt(x)
#  define EXP_FUN(x) exp(x)
#  define POW_FUN(x,y) pow(x,y)
# endif

#endif /* !_GEMM_H */
