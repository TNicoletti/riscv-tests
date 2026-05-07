#include "myutil.h"

void set_vet_settings(){
  int el_per_block = VLEN / SEW * LMUL;
    if(SEW == 16){
      if(LMUL == 1)
        set_vet_m1_Xx16(el_per_block);
      else if(LMUL == 2)
        set_vet_m2_Xx16(el_per_block);
      else if(LMUL == 4)
        set_vet_m4_Xx16(el_per_block);
      else if(LMUL == 8)
        set_vet_m8_Xx16(el_per_block);
    }

    else if(SEW == 32){
      if(LMUL == 1)
        set_vet_m1_Xx32(el_per_block);
      else if(LMUL == 2)
        set_vet_m2_Xx32(el_per_block);
      else if(LMUL == 4)
        set_vet_m4_Xx32(el_per_block);
      else if(LMUL == 8)
        set_vet_m8_Xx32(el_per_block);
    }
    else if(SEW == 64)
      if(LMUL == 1)
        set_vet_m1_Xx64(el_per_block);
      else if(LMUL == 2)
        set_vet_m2_Xx64(el_per_block);
      else if(LMUL == 4)
        set_vet_m4_Xx64(el_per_block);
      else if(LMUL == 8)
        set_vet_m8_Xx64(el_per_block);
    else{
      printf("Valor de SEW não suportado!\n");
      exit(1);
    }
}


void print_vector(int* vet, int LENGTH, int breakline){
    for(int i = 0; i < LENGTH; i++){
        printf("%d ", vet[i]);
        if((i % breakline) == breakline - 1)
            printf("\n");
    }
    printf("\n");
}

void print_matrix(int* vet, int N, int M){
    for(int i = 0; i < N; i++){
      for(int j = 0; j < M; j++){
        printf("v[%d][%d] = %d;", i, j, vet[i * M + j]);
      }  
      printf("\n");
    }
    printf("\n");
}

void print_regs(int* vet, int N, int r[3]){

  for(int i = 0; i < 3; i++){
    int idx = r[i];
    for(int j = 0; j < EL_PER_BLOCK; j++){
      printf("v[%d][%d] = %d;", idx, j, vet[idx * EL_PER_BLOCK + j]);
    }  
    printf("\n");
  }
  printf("\n");
}

#pragma GCC optimize ("no-tree-vectorize")
void clean_matrix(int vet[5][4], int N, int M){
  for(int i = 0; i < 3; i++)
    for(int j = 0; j < 4; j++)
      vet[i][j] = 7;
}


int checksum(int *vec, int n) {
  int chk = 0;
  int chk2 = 0;

  for (int i = 0; i < n; i++) {
    chk ^= vec[i];
    chk2 += vec[i];
  }
  return chk ^ chk2;
}

int is_divergent(int *vec, int *vec2, int n){
  return checksum(vec, n) == checksum(vec2, n);
}

int checksum_matrix(int32_t* vec, int n, int m){
  int chk = 0;

  for(int i = 0;i < n; i++)
    for(int j = 0; j < m; j++){
      chk ^= vec[i * m + j];
    }
  return chk;
}
int is_divergent_matrix(int32_t* vec, int32_t* vec2, int n, int m){
  int c1 = checksum_matrix(vec, n, m);
  int c2 = checksum_matrix(vec2, n, m);
  return c1 != c2;
}

int manual_convergence(int32_t* vec, int32_t* vec2, int n, int m){
  for(int i = 0;i < n; i++)
    for(int j = 0; j < m; j++){
      if(vec[i * m + j] != vec2[i * m + j])
        return false;
    }
  return true;
}

/* BIT FLIP JIT CHANGERS HELPERS */
int32_t change_vet_rs1(int32_t inst, int nr2) {
    uint32_t mask = ~(0x1F << 20);
    inst &= mask;
    nr2 = (nr2 & 0x1F) << 20; 
    return inst | nr2;
}
int32_t get_vet_rs1(int32_t inst){
  uint32_t mask = 0x1F << 20;
  return (inst & mask) >> 20;
}

int32_t change_vet_rs2(int32_t inst, int nr1){
  uint32_t mask = ~(0x1F << 15);
  inst &= mask;
  inst |= (nr1 & 0x1F) << 15;
  return inst;
}
int32_t get_vet_rs2(int32_t inst){
  uint32_t mask = 0x1F << 15;
  return (inst & mask) >> 15;
}

// Field rd: bits 11 to 7
int32_t change_vet_rd(int32_t inst, int nrd) {
    uint32_t mask = ~(0x1F << 7);
    inst &= mask;
    inst |= (nrd & 0x1F) << 7;
    return inst;
}
int32_t get_vet_rd(int32_t inst){
  uint32_t mask = 0x1F << 7;
  return (inst & mask) >> 7;
}

// Field rd: bits 11 to 7
int32_t change_imm(int32_t inst, int imm) {
    uint32_t mask = ~(0x7FF << 20);
    inst &= mask;
    inst |= (imm & 0x7FF) << 20;
    return inst;
}



int32_t get_imm(int32_t inst){
  return inst >> 20;
}

// Field rd: bits 11 to 7
int32_t change_vet_vd(int32_t inst, int imm) {
    uint32_t mask = ~(0x1F << 20);
    inst &= mask;
    inst |= (imm & 0x1F) << 20;
    return inst;
}

int32_t get_vd(int32_t inst){
  uint32_t mask = 0x1F << 20;
  return (inst & mask) >> 20;
}

void get_reg_signature(int a0, int a1, int a2, int ret[3]){
  ret[0] = 0;
  if(a0 == a1 && a0 == a2){
    ret[1] = 0;
    ret[2] = 0;
    return;
  }
  if(a0 == a1){
    ret[1] = 0;
    ret[2] = 1;
    return;
  }

  ret[1] = 1;
  if(a0 == a2){
    ret[2] = 0;
    return;
  }

  if(a2 == a1){
    ret[2] = 1;
    return;
  }
  ret[2] = 2;
}

void get_instruction_signature(int32_t inst, int ret[3]){
  int a0 = 0, a1 = 0, a2 = 0;
  a0 = get_vet_rd(inst);
  a1 = get_vet_rs1(inst);
  a2 = get_vet_rs2(inst);

  get_reg_signature(a0, a1, a2, ret);
}


/* CACHE SYNCHRONIZER HELPER */
static inline void sync_caches() {
    // Tells the RISC-V core to synchronize the I-Cache with memory/D-Cache
    __asm__ volatile ("fence.i" ::: "memory");
}

uint32_t maxu(uint32_t a1, uint32_t a2){
  return a1>a2 ? a1 : a2;
}

int32_t max(int32_t a1, int32_t a2){
  return a1>a2 ? a1 : a2;
}

uint32_t minu(uint32_t a1, uint32_t a2){
  return a1<a2 ? a1 : a2;
}

int32_t min(int32_t a1, int32_t a2){
  return a1<a2 ? a1 : a2;
}
