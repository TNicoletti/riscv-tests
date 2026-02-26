/* CONFIGURATION */

.globl set_vet_Xx16
set_vet_Xx16:
    vsetvli a0, a0, e16, m1, ta, ma
    ret

.globl set_vet_Xx32
set_vet_Xx32:
    vsetvli a0, a0, e32, m1, ta, ma
    ret

.globl set_vet_Xx64
set_vet_Xx64:
    vsetvli a0, a0, e64, m1, ta, ma
    ret

.globl clean_vector_scalar
clean_vector_scalar:
    mv t0, a1
    1:
        li t1, 7
        sw t1, (a0)
        addi a0, a0, 4
        addi t0, t0, -1
        bnez t0, 1b
    ret 

/* STALL */
.globl STALL
STALL:
    mv t0, a0

    1:
        addi t0, t0, -1
        bnez t0, 1b
    ret
        
