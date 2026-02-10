.data

wa: .ascii "TEST TEST TEST \n"

.section .text

.globl vector_sum_interrupt
# int vector_agnostic_sum_1(int* vector,int LENGTH);
vector_sum_interrupt:
    addi sp, sp, -32
    sd s0, 0(sp)
    sd s1, 8(sp)
    sd s2, 16(sp)
    sd s4, 24(sp)

    mv s0, a1
    mv s4, a0
    
    #vsetvli s1, s0, e32, m1, tu, ma 

    #li a0, 1
    #la a1, wa
    #li a2, 10
    #li a7, 64
    #ecall

loop:
    

    vsetvli s1, s0, e32, m1, tu, ma 

    vle32.v v3, (s4)   
    vadd.vi v3, v3, 5
    vse32.v v3, (s4)   

    slli s2, s1, 2     
    add s4, s4, s2     
    sub s0, s0, s1     
    bnez s0, loop

    ld s0, 0(sp)
    ld s1, 8(sp)
    ld s2, 16(sp)
    ld s4, 24(sp)
    addi sp, sp, 32
    ret