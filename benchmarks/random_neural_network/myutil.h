/* STRUCTURE PRINT HELPERS*/

void print_vector(int* vet, int LENGTH){
    for(int i = 0; i < LENGTH; i++){
        printf("%d ", vet[i]);
        if((i % 15) == 14)
            printf("\n");
    }
    printf("\n");
}

void print_matrix(int vet[3][4], int N, int M){
    for(int i = 0; i < N; i++){
      for(int j = 0; j < M; j++){
        printf("v[%d][%d] = %d;", i, j, vet[i][j]);
      }  
      printf("\n");
    }
    printf("\n");
}

/* OUTPUT CONFERENCE HELPERS */
int checksum(int *vec, int n) {
  int chk = 0;
  for (int i = 0; i < n; i++) {
    chk ^= vec[i];
  }
  return chk;
}

int checksum_matrix(int vec[3][4], int n, int m){
  int chk = 0;
  for(int i = 0;i < 3; i++)
    for(int j = 0; j < 4; j++)
      chk ^= vec[i][j];
  return chk;
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