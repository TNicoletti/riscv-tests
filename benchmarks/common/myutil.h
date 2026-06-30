#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#ifndef MY_UTIL
#define MY_UTIL
#include "global_configs.h"
#include "stdint.h"

/* VECTOR SETTINGS MACROS*/
extern int set_vet_m1_Xx16(int X);
extern int set_vet_m2_Xx16(int X);
extern int set_vet_m4_Xx16(int X);
extern int set_vet_m8_Xx16(int X);

extern int set_vet_m1_Xx32(int X);
extern int set_vet_m2_Xx32(int X);
extern int set_vet_m4_Xx32(int X);
extern int set_vet_m8_Xx32(int X);

extern int set_vet_m1_Xx64(int X);
extern int set_vet_m2_Xx64(int X);
extern int set_vet_m4_Xx64(int X);
extern int set_vet_m8_Xx64(int X);



void set_vet_settings();

/* STRUCTURE PRINT HELPERS*/

void print_vector(intSEW* vet, int LENGTH, int breakline);

void print_matrix(intSEW* vet, int N, int M);

void print_regs(intSEW* vet, int N, int r[3]);

void clean_matrix(intSEW vet[5][4], int N, int M);

/* OUTPUT CONFERENCE HELPERS */
int checksum(intSEW *vec, int n);

int is_divergent(intSEW *vec, intSEW *vec2, int n);

int checksum_matrix(intSEW* vec, int n, int m);
int is_divergent_matrix(intSEW* vec, intSEW* vec2, int n, int m);

int manual_convergence(intSEW* vec, intSEW* vec2, int n, int m);

/* BIT FLIP JIT CHANGERS HELPERS */
INT_INST change_vet_rs1(INT_INST inst, int nr2);

INT_INST change_vet_rs2(INT_INST inst, int nr1);
INT_INST get_vet_rs2(INT_INST inst);

// Field rd: bits 11 to 7
INT_INST change_vet_rd(INT_INST inst, int nrd);
INT_INST get_vet_rd(INT_INST inst);

// Field rd: bits 11 to 7
INT_INST change_imm(INT_INST inst, int imm);

/*int32_t change_imm5(int32_t inst, int imm){
  uint32_t mask = ~(0x1F << 20); 
  inst &= mask;
  
  inst |= (imm & 0x1F) << 20;
  
  return inst;
}*/

INT_INST get_imm(INT_INST inst);

// Field rd: bits 11 to 7
INT_INST change_vet_vd(INT_INST inst, int imm);

INT_INST get_vd(INT_INST inst);

void get_reg_signature(int a0, int a1, int a2, int ret[3]);

void get_instruction_signature(INT_INST inst, int ret[3]);

static inline void sync_caches();

uintSEW maxu(uintSEW a1, uintSEW a2);

intSEW max(intSEW a1, intSEW a2);

uintSEW minu(uintSEW a1, uintSEW a2);

intSEW min(intSEW a1, intSEW a2);

#endif