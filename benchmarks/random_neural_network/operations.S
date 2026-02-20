.globl jump_to_vet
jump_to_vet:
    addi sp, sp, -16
    sd ra, 0(sp)
    
    fence.i
    jal ADDRESS_VECTOR

    ld ra, 0(sp)
    addi sp, sp, 16
    ret

.globl load_OUT_t0_vet
load_OUT_t0_vet:
    mv t0, a0
    ret
