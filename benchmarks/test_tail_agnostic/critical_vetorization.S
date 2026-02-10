.section .text

.globl vector_agnostic_sum_1
# int vector_agnostic_sum_1(int* vector,int LENGTH);
vector_agnostic_sum_1:
    mv t0, a1

loop:
    vsetvli t1, t0, e32, m1, ta, ma 

    vle32.v v1, (a0)   
    vadd.vi v2, v1, 1
    vse32.v v2, (a0)   

    slli t2, t1, 2     
    add a0, a0, t2     
    sub t0, t0, t1     
    bnez t0, loop       
    ret

.globl vector_unchanged_sum_1
# int vector_unchanged_sum_1(int* vector,int LENGTH);
vector_unchanged_sum_1:
    mv t0, a1

loop_2:

    vsetvli t1, t0, e32, m1, tu, mu 

    vle32.v v1, (a0)   
    vadd.vi v2, v1, 1
    vse32.v v2, (a0)   

    slli t2, t1, 2     
    add a0, a0, t2     
    sub t0, t0, t1     
    bnez t0, loop_2      
    ret