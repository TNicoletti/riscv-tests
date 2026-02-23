#include <util.h>

#define N 512
#define SEED 0x12345678
#define SEW 32
#define VLEN 128
#define LMUL 1
#define EL_PER_BLOCK VLEN / SEW
#define REGISTERS_PER_BLOCK 3

#define PRINTS 1

int32_t ADDRESS_VECTOR[20];

/* EXTERNALS */
extern int set_vet_Xx16(int X);
extern int set_vet_Xx32(int X);
extern int set_vet_Xx64(int X);

extern int test(int *vet);

int main(){
    printf("Single test \n");
    
    int v[12] = {1088, 890, 1737, 1249, 
        56, 1623, 355, 1692,
        1625, 335, 1915, 835};
        
    set_vet_Xx32(4);
    test(v);

    for(int i = 0; i < 12; i++){
        printf("v[%d] = %d; ", i, v[i]);
        if(i % 4 == 3)printf("\n");
    }
}