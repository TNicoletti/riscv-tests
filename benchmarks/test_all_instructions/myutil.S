/* CONFIGURATION */

/* SEW = 16 */
.globl set_vet_m1_Xx16
set_vet_m1_Xx16:
    vsetvli a0, a0, e16, m1, tu, mu
    ret

.globl set_vet_m2_Xx16
set_vet_m2_Xx16:
    vsetvli a0, a0, e16, m2, tu, mu
    ret

.globl set_vet_m4_Xx16
set_vet_m4_Xx16:
    vsetvli a0, a0, e16, m2, tu, mu
    ret

.globl set_vet_m8_Xx16
set_vet_m8_Xx16:
    vsetvli a0, a0, e16, m2, tu, mu
    ret

/* SEW = 32 */

.globl set_vet_m1_Xx32
set_vet_m1_Xx32:
    vsetvli a0, a0, e32, m1, tu, mu
    ret

.globl set_vet_m2_Xx32
set_vet_m2_Xx32:
    vsetvli a0, a0, e32, m2, tu, mu
    ret

.globl set_vet_m4_Xx32
set_vet_m4_Xx32:
    vsetvli a0, a0, e32, m4, tu, mu
    ret

.globl set_vet_m8_Xx32
set_vet_m8_Xx32:
    vsetvli a0, a0, e32, m8, tu, mu
    ret

/* SEW = 64 */

.globl set_vet_m1_Xx64
set_vet_m1_Xx64:
    vsetvli a0, a0, e64, m1, tu, mu
    ret

.globl set_vet_m2_Xx64
set_vet_m2_Xx64:
    vsetvli a0, a0, e64, m2, tu, mu
    ret

.globl set_vet_m4_Xx64
set_vet_m4_Xx64:
    vsetvli a0, a0, e64, m4, tu, mu
    ret

.globl set_vet_m8_Xx64
set_vet_m8_Xx64:
    vsetvli a0, a0, e64, m8, tu, mu
    ret

/* JUMPER */
.globl jump_to_vet
jump_to_vet:
    addi sp, sp, -16
    sd ra, 0(sp)
    
    fence.i
    jalr ra, a0, 0

    ld ra, 0(sp)
    addi sp, sp, 16
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

/* StuLL */
.globl StuLL
StuLL:
    mv t0, a0

    1:
        addi t0, t0, -1
        bnez t0, 1b
    ret
        
