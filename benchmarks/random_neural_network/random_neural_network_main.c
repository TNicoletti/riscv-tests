#include <util.h>
#include "mysrand.h"
#include "myutil.h"
#include "asm_functions.h"
#define RET_INSTR 0x00008067

#define N 512
#define SEED 0x12345678
#define SEW 32
#define VLEN 128
#define LMUL 1
#define EL_PER_BLOCK VLEN / SEW
#define REGISTERS_PER_BLOCK 3

#define PRINTS 3

int32_t ADDRESS_VECTOR[20];

/* EXTERNALS */
extern int set_vet_Xx16(int X);
extern int set_vet_Xx32(int X);
extern int set_vet_Xx64(int X);

extern void jump_to_vet();

extern int* load_OUT_t0_vet(int* address);


/* NORMALS */

volatile int A[N];
volatile int B[N];
volatile int32_t OUT[N];
volatile int32_t OUT_SCALAR[N];

void generate_initial_values(){
    for (int i = 0; i < N; i++) {
        A[i] = mrand() % 0x7FF;
        B[i] = mrand() % 0x7FF;
        OUT[i] = mrand() % 0x7FF;
    }
}

int r[3];

shuffle_registers(){
    r[0] = mrand() % 32;

    do
    r[1] = mrand() % 32;
    while(r[1] == r[0]);
    
    do
    r[2] = mrand() % 32;
    while(r[2] == r[0] || r[2] == r[1]);
}

/*
r1 = OUT[index + 0]
r2 = OUT[index + 1 * EL_PER_BLOCK]
r3 = OUT[index + 2 * EL_PER_BLOCK]

rx1 = rx2 op1 rx3
rx4 = rx5 op1 rx6
rx7 = rx8 op1 rx9
rx10 = rx11 op12 rx13

r1 = OUT[index + 0]
r2 = OUT[index + 1 * EL_PER_BLOCK]
r3 = OUT[index + 2 * EL_PER_BLOCK]

1 - picks 3 different registers
2 - picks 4 operations
3 - picks
*/
int generate_RIS(int index){
    
    shuffle_registers();
    if(PRINTS) printf("r[0]= %d; r[1]= %d; r[2]= %d;\n", r[0], r[1], r[2]);

    int ops[4];

    ops[0] = mrand() % 4;
    ops[1] = mrand() % 4;
    ops[2] = mrand() % 4;
    ops[3] = mrand() % 4;

    /*
        Inits scalar res
    */
    int32_t scalar_res[3][4];
    for(int i = 0; i < EL_PER_BLOCK; i++)
    {
        scalar_res[0][i] = OUT[index + 0 * EL_PER_BLOCK + i];
        scalar_res[1][i] = OUT[index + 1 * EL_PER_BLOCK + i];
        scalar_res[2][i] = OUT[index + 2 * EL_PER_BLOCK + i];
    }
    

    int rx[12];
    rx[0] = mrand() % 3; rx[1]  = mrand() % 3; rx[2]  = mrand() % 3;
    rx[3] = mrand() % 3; rx[4]  = mrand() % 3; rx[5]  = mrand() % 3;
    rx[6] = mrand() % 3; rx[7]  = mrand() % 3; rx[8]  = mrand() % 3;
    rx[9] = mrand() % 3; rx[10] = mrand() % 3; rx[11] = mrand() % 3;

    ADDRESS_VECTOR[1] = RET_INSTR;

    /* LOADS VALUES*/
    // v[0] <=
    int32_t instr = VLE32_V_INSTR;
    instr = change_vet_rd(instr, r[0]);
    load_OUT_t0_vet(&OUT[index + 0]);
    ADDRESS_VECTOR[0] = instr;
    jump_to_vet();

    // v[1] <=
    instr = VLE32_V_INSTR;
    instr = change_vet_rd(instr, r[1]);
    load_OUT_t0_vet(&OUT[index + 1 * EL_PER_BLOCK]);
    ADDRESS_VECTOR[0] = instr;
    jump_to_vet();

    // v[2] <=
    instr = VLE32_V_INSTR;
    instr = change_vet_rd(instr, r[2]);
    load_OUT_t0_vet(&OUT[index + 2 * EL_PER_BLOCK]);
    ADDRESS_VECTOR[0] = instr;
    jump_to_vet();

    for(int i = 0; i < 4; i++){
        int instr = 0;
        switch (ops[i]){
            case 0:
                for(int j = 0; j < EL_PER_BLOCK; j++){
                    if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + [%d]%d;\n", rx[0 + i * 3], j, rx[1 + i * 3], scalar_res[rx[1 + i * 3]][j], rx[2 + i * 3], scalar_res[rx[2 + i * 3]][j]);
                    scalar_res[rx[0 + i * 3]][j] = (int32_t)(scalar_res[rx[1 + i * 3]][j] + scalar_res[rx[2 + i * 3]][j]);
                }
                if(PRINTS >= 2) printf("\n");
                instr = VADD_VV_INSTR;
                break;
            case 1:
                for(int j = 0; j < EL_PER_BLOCK; j++){
                    if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d - [%d]%d;\n", rx[0 + i * 3], j, rx[1 + i * 3], scalar_res[rx[1 + i * 3]][j], rx[2 + i * 3], scalar_res[rx[2 + i * 3]][j]);
                    scalar_res[rx[0 + i * 3]][j] = (int32_t)(scalar_res[rx[1 + i * 3]][j] - scalar_res[rx[2 + i * 3]][j]);
                }
                if(PRINTS >= 2) printf("\n");
                instr = VSUB_VV_INSTR;
                break;
            case 2:
                for(int j = 0; j < EL_PER_BLOCK; j++){
                    if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d / [%d]%d;\n", rx[0 + i * 3], j, rx[1 + i * 3], scalar_res[rx[1 + i * 3]][j], rx[2 + i * 3], scalar_res[rx[2 + i * 3]][j]);
                    scalar_res[rx[0 + i * 3]][j] = (int32_t)(scalar_res[rx[1 + i * 3]][j] / scalar_res[rx[2 + i * 3]][j]);
                }
                if(PRINTS >= 2) printf("\n");
                instr = VDIV_VV_INSTR;
                break;
            case 3:
                for(int j = 0; j < EL_PER_BLOCK; j++){
                    if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d * [%d]%d;\n", rx[0 + i * 3], j, rx[1 + i * 3], scalar_res[rx[1 + i * 3]][j], rx[2 + i * 3], scalar_res[rx[2 + i * 3]][j]);
                    scalar_res[rx[0 + i * 3]][j] = (int32_t)(scalar_res[rx[1 + i * 3]][j] * scalar_res[rx[2 + i * 3]][j]);
                }
                if(PRINTS >= 2) printf("\n");
                instr = VMUL_VV_INSTR;
                break;
            default:
                break;
        }
        instr = change_vet_rd(instr, r[rx[0 + i * 3]]);
        instr = change_vet_rs1(instr, r[rx[1 + i * 3]]);
        instr = change_vet_rs2(instr, r[rx[2 + i * 3]]);
        ADDRESS_VECTOR[i] = instr;
    }
    
    ADDRESS_VECTOR[4] = RET_INSTR;

    if(PRINTS){    
        printf("SCALAR MATRIX:\n");
        print_matrix(scalar_res, 3, 4);
    }
    return checksum_matrix(scalar_res, 3, 4);
    
    /* LOAD INSTRUCTIONS */
}

int execute_RIS(int index){
    jump_to_vet();

    /* STORES VALUES */
    ADDRESS_VECTOR[1] = RET_INSTR;
    // v[0] <=
    int32_t instr = VSE32_V_INSTR;
    instr = change_vet_rd(instr, r[0]);
    load_OUT_t0_vet(&OUT[index + 0]);
    ADDRESS_VECTOR[0] = instr;
    jump_to_vet();

    // v[1] <=
    instr = VSE32_V_INSTR;
    instr = change_vet_rd(instr, r[1]);
    load_OUT_t0_vet(&OUT[index + 1 * EL_PER_BLOCK]);
    ADDRESS_VECTOR[0] = instr;
    jump_to_vet();

    // v[2] <=
    instr = VSE32_V_INSTR;
    instr = change_vet_rd(instr, r[2]);
    load_OUT_t0_vet(&OUT[index + 2 * EL_PER_BLOCK]);
    ADDRESS_VECTOR[0] = instr;
    jump_to_vet();

    return checksum(&OUT[index], 3 * 4);
}

void random_test(int seed) {
    #if SEW == 16
        set_vet_Xx16(VLEN / SEW);
    #elif SEW == 32
        set_vet_Xx32(VLEN / SEW);
    #elif SEW == 64
        set_vet_Xx64(VLEN / SEW);
    #else
        #error "Valor de SEW não suportado!"
    #endif

    msrand(seed);
    generate_initial_values();
    printf("Done init values\n");
    
    //msrand(seed);

    int inc = 4 * 3;
    for(int z = 0; z + inc < N; z+= inc){
        printf("Begginning test %d\n", z / inc);

        if(PRINTS >= 3){
            printf("OUT before modifications:\n");
            for(int i = z; i < z + inc; i++)
                printf("v[%d] = %d;", i, OUT[i]);
            printf("\n");
        }

        int checksum_escalar = generate_RIS(z);
    
        int checksum = execute_RIS(z);

        if(PRINTS){    
            printf("OUTPUT from vector:\n");
            for(int i = z; i < z + inc; i++){
                printf("v[%d][%d] = %d;", (i - z) / 4, (i - z) % 4, OUT[i]);
                if(i % 4 == 3)
                    printf("\n");
            }
        }
        if(checksum == checksum_escalar){
            printf("Convergence %d-%d\n\n", z, z + inc);
        }else{
            printf("Divergence %d-%d\n\n", z, z + inc);
            exit(0);
        }
    }

}

int main(){
    printf("Doing random batch tests with registers v0-v7 with seed %d\n", SEED);
    random_test(SEED);
}