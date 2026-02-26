#include <util.h>

#define true 1
#define false 0

#define N_VECTOR 512
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

int equals(int *vet1, int*vet2){
    for(int i = 0; i < 12; i++)
        if(vet1[i] != vet2[i])
            return false;
    return true;
}

int main(){
    printf("Single test \n");
    
    int v[12] = {1085, 1355, 1235, 176, 
        1753, 1661, 1309, 1890,
        1523, 1818, 898, 595};

    int resp[12] = {1, 1, 1, 1,
        1753, 1661, 1309, 1890,
        1753, 1661, 1309, 1890};
        
    set_vet_Xx32(4);
    test(v);

    if(equals(v, resp)){
        printf("Convergence\n");
        exit(0);
    }

    printf("RESULTS:\n");
    for(int i = 0; i < 12; i++){
        printf("v[%d] = %d; ", i, v[i]);
        if(i % 4 == 3)printf("\n");
    }

    printf("RESP:\n");
    for(int i = 0; i < 12; i++){
        printf("v[%d] = %d; ", i, resp[i]);
        if(i % 4 == 3)printf("\n");
    }



    //int res = 

    exit(0);
}