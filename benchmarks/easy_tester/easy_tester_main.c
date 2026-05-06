#include <util.h>
#include "config.h"
#include "myutil.h"

#define PRINTS 1

#define QTD_TESTS 15

int32_t ADDRESS_VECTOR[20];

/* EXTERNALS */
extern int test(int *vet, int *result);

int equals(int *vet1, int*vet2){
    for(int i = 0; i < 12; i++)
        if(vet1[i] != vet2[i])
            return false;
    return true;
}

int main(){
    printf("Single test \n");
    

    
    int init[12] = {
        158653998, 221290364, 191570381, 36659268, 
        136484931, 66907504,  256742140, 270325384, 
        166659206, 288208687, 65823742,  242653232, 
    };
    int result[12] = {
        0, 0, 0, 0, 
        0, 0, 0, 0, 
        0, 0, 0, 0, 
    };

    int resp[12] = {
        36659268, 221290364, 191570381, 36659268,
        572801, 3457661, 2993287, 572801,
        -1508851712, 0, -1302364160, 30238848,
    };
        
    set_vet_settings();

    int con_div[QTD_TESTS];

    printf("RESP:\n");
    for(int i = 0; i < 12; i++){
        printf("v[%d] = %d; ", i, resp[i]);
        if(i % 4 == 3)printf("\n");
    }
    printf("\n");

    for(int i = 0; i < QTD_TESTS; i++){
        test(init, result);
        if(equals(result, resp)){
            printf("Convergence\n");
            con_div[i] = 1;
        }else{
            printf("Divergence\n");
            con_div[i] = 0;
        }
        
        printf("RESULTS:\n");
        for(int i = 0; i < 12; i++){
            printf("v[%d] = %d; ", i, result[i]);
            if(i % 4 == 3)printf("\n");
        }
        printf("\n");
    }

    printf("FINAL RESULTS:\n");
    for(int i = 0; i < QTD_TESTS; i++)
        printf("[%c] ", con_div[i]?'C':'W');

    printf("\n");

    exit(0);
}