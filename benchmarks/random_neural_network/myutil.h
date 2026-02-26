#define true 1
#define false 0

/* STRUCTURE PRINT HELPERS*/

void print_vector(int* vet, int LENGTH, int breakline){
    for(int i = 0; i < LENGTH; i++){
        printf("%d ", vet[i]);
        if((i % breakline) == breakline - 1)
            printf("\n");
    }
    printf("\n");
}

void print_matrix(int vet[3][4], int N_VECTOR, int M){
    for(int i = 0; i < N_VECTOR; i++){
      for(int j = 0; j < M; j++){
        printf("v[%d][%d] = %d;", i, j, vet[i][j]);
      }  
      printf("\n");
    }
    printf("\n");
}

#pragma GCC optimize ("no-tree-vectorize")
void clean_matrix(int vet[3][4], int N, int M){
  for(int i = 0; i < 3; i++)
    for(int j = 0; j < 4; j++)
      vet[i][j] = 7;
}

/* OUTPUT CONFERENCE HELPERS */
int checksum(int *vec, int n) {
  int chk = 0;
  for (int i = 0; i < n; i++) {
    chk ^= vec[i];
  }
  return chk;
}

int is_divergent(int *vec, int *vec2, int n){
  return checksum(vec, n) == checksum(vec2, n);
}

int checksum_matrix(int vec[3][4], int n, int m){
  int chk = 0;
  for(int i = 0;i < 3; i++)
    for(int j = 0; j < 4; j++)
      chk ^= vec[i][j];
  return chk;
}
int is_divergent_matrix(int vec[3][4], int vec2[3][4], int n, int m){
  return checksum_matrix(vec, n, m) == checksum_matrix(vec2, n, m);
}

/* BIT FLIP JIT CHANGERS HELPERS */
int32_t change_vet_rs1(int32_t inst, int nr2) {
    uint32_t mask = ~(0x1F << 20);
    inst &= mask;
    nr2 = (nr2 & 0x1F) << 20; 
    return inst | nr2;
}

int32_t change_vet_rs2(int32_t inst, int nr1){
  uint32_t mask = ~(0x1F << 15);
  inst &= mask;
  inst |= (nr1 & 0x1F) << 15;
  return inst;
}

// Field rd: bits 11 to 7
int32_t change_vet_rd(int32_t inst, int nrd) {
    uint32_t mask = ~(0x1F << 7);
    inst &= mask;
    inst |= (nrd & 0x1F) << 7;
    return inst;
}

// Field rd: bits 11 to 7
int32_t change_imm(int32_t inst, int imm) {
    uint32_t mask = ~(0x7FF << 20);
    inst &= mask;
    inst |= (imm & 0x7FF) << 20;
    return inst;
}

// Field rd: bits 11 to 7
int32_t change_vet_vd(int32_t inst, int imm) {
    uint32_t mask = ~(0x1F << 20);
    inst &= mask;
    inst |= (imm & 0x1F) << 20;
    return inst;
}

/* CACHE SYNCHRONIZER HELPER */
static inline void sync_caches() {
    // Tells the RISC-V core to synchronize the I-Cache with memory/D-Cache
    __asm__ volatile ("fence.i" ::: "memory");
}