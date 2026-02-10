.section .text

.globl vector_agnostic_sum_intercalate
# int vector_agnostic_sum_1(int* vector,int LENGTH);
vector_agnostic_sum_intercalate:
    mv t0, a1
    
    vsetvli t1, t0, e32, m1, tu, ma


    vid.v v3                # v3 = [0, 1, 2, 3, 4, 5, ...]
    vand.vi v3, v3, 1       # v3 = [0, 1, 0, 1, 0, 1, ...]
    vmsne.vi v0, v3, 0
    li t5, 1
    vmv.v.x v2, t5

loop:
    vsetvli t1, t0, e32, m1, tu, ma 

    vle32.v v3, (a0)   
    vadd.vv v3, v2, v3, v0.t
    vse32.v v3, (a0)   

    slli t2, t1, 2     
    add a0, a0, t2     
    sub t0, t0, t1     
    bnez t0, loop       
    ret

.globl vector_unchanged_sum_intercalate
# int vector_unchanged_sum_1(int* vector,int LENGTH);
vector_unchanged_sum_intercalate:
    mv t0, a1
    
    vsetvli t1, t0, e32, m1, tu, mu


    vid.v v3                # v3 = [0, 1, 2, 3, 4, 5, ...]
    vand.vi v3, v3, 1       # v3 = [0, 1, 0, 1, 0, 1, ...]
    vmsne.vi v0, v3, 0
    li t5, 1
    vmv.v.x v2, t5

loop_2:
    vsetvli t1, t0, e32, m1, tu, mu 

    vle32.v v3, (a0)   
    vadd.vv v3, v2, v3, v0.t
    vse32.v v3, (a0)   

    slli t2, t1, 2     
    add a0, a0, t2     
    sub t0, t0, t1     
    bnez t0, loop_2       
    ret