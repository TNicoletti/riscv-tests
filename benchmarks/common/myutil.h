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

void print_vector(int* vet, int LENGTH, int breakline);

void print_matrix(int* vet, int N, int M);

void print_regs(int* vet, int N, int r[3]);

void clean_matrix(int vet[5][4], int N, int M);

/* OUTPUT CONFERENCE HELPERS */
int checksum(int *vec, int n);

int is_divergent(int *vec, int *vec2, int n);

int checksum_matrix(int32_t* vec, int n, int m);
int is_divergent_matrix(int32_t* vec, int32_t* vec2, int n, int m);

int manual_convergence(int32_t* vec, int32_t* vec2, int n, int m);

/* BIT FLIP JIT CHANGERS HELPERS */
int32_t change_vet_rs1(int32_t inst, int nr2);

int32_t change_vet_rs2(int32_t inst, int nr1);
int32_t get_vet_rs2(int32_t inst);

// Field rd: bits 11 to 7
int32_t change_vet_rd(int32_t inst, int nrd);
int32_t get_vet_rd(int32_t inst);

// Field rd: bits 11 to 7
int32_t change_imm(int32_t inst, int imm);

/*int32_t change_imm5(int32_t inst, int imm){
  uint32_t mask = ~(0x1F << 20); 
  inst &= mask;
  
  inst |= (imm & 0x1F) << 20;
  
  return inst;
}*/

int32_t get_imm(int32_t inst);

// Field rd: bits 11 to 7
int32_t change_vet_vd(int32_t inst, int imm);

int32_t get_vd(int32_t inst);

void get_reg_signature(int a0, int a1, int a2, int ret[3]);

void get_instruction_signature(int32_t inst, int ret[3]);

static inline void sync_caches();

uint32_t maxu(uint32_t a1, uint32_t a2);

int32_t max(int32_t a1, int32_t a2);

uint32_t minu(uint32_t a1, uint32_t a2);

int32_t min(int32_t a1, int32_t a2);

#endif