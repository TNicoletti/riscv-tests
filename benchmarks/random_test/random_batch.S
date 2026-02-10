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

/* MEMORY HELPERS */ 

vet_load_a0_a1:
    vle32.v v0, (a0)
    vle32.v v1, (a1)
    addi a0, a0, 16
    addi a1, a1, 16
    vle32.v v2, (a0)
    vle32.v v3, (a1)
    addi a0, a0, 16
    addi a1, a1, 16
    vle32.v v4, (a0)
    vle32.v v5, (a1)
    addi a0, a0, 16
    addi a1, a1, 16
    vle32.v v6, (a0)
    vle32.v v7, (a1)
    ret

vet_load_a0:
    vle32.v v0, (a0)
    addi a0, a0, 16
    vle32.v v2, (a0)
    addi a0, a0, 16
    vle32.v v4, (a0)
    addi a0, a0, 16
    vle32.v v6, (a0)
    ret

vet_store_a2:
    vse32.v v0, (a2)
    addi a2, a2, 16
    vse32.v v2, (a2)
    addi a2, a2, 16
    vse32.v v4, (a2)
    addi a2, a2, 16
    vse32.v v6, (a2)
    ret

vet_store_a1:
    vse32.v v0, (a1)
    addi a1, a1, 16
    vse32.v v2, (a1)
    addi a1, a1, 16
    vse32.v v4, (a1)
    addi a1, a1, 16
    vse32.v v6, (a1)
    ret

/* VV operations */

.globl vet_vv_sum
# a0, a1, a2 => a2 = a1 + a0
vet_vv_sum:
    mv t0, ra
    jal vet_load_a0_a1

    vadd.vv v0, v0, v1
    vadd.vv v2, v2, v3
    vadd.vv v4, v4, v5
    vadd.vv v6, v6, v7

    jal vet_store_a2
    mv ra, t0
    ret

.globl vet_vv_sub
# a0, a1, a2 => a2 = a1 - a0
vet_vv_sub:
    mv t0, ra
    jal vet_load_a0_a1

    vsub.vv v0, v0, v1
    vsub.vv v2, v2, v3
    vsub.vv v4, v4, v5
    vsub.vv v6, v6, v7

    jal vet_store_a2
    mv ra, t0
    ret

.globl vet_vv_div
# a0, a1, a2 => a2 = a1 / a0
vet_vv_div:
    mv t0, ra
    jal vet_load_a0_a1

    vdiv.vv v0, v0, v1
    vdiv.vv v2, v2, v3
    vdiv.vv v4, v4, v5
    vdiv.vv v6, v6, v7

    jal vet_store_a2
    mv ra, t0
    ret

.globl vet_vv_mul
# a0, a1, a2 => a2 = a1 * a0
vet_vv_mul:
    mv t0, ra
    jal vet_load_a0_a1

    vmul.vv v0, v0, v1
    vmul.vv v2, v2, v3
    vmul.vv v4, v4, v5
    vmul.vv v6, v6, v7

    jal vet_store_a2
    mv ra, t0
    ret

/* VX OPERATIONS */

.globl vet_vx_sum
vet_vx_sum:
    mv t0, ra
    jal vet_load_a0

    vadd.vx v0, v0, a1
    vadd.vx v2, v2, a1
    vadd.vx v4, v4, a1
    vadd.vx v6, v6, a1

    jal vet_store_a2
    mv ra, t0
    ret

.globl vet_vx_sub
vet_vx_sub:
    mv t0, ra
    jal vet_load_a0

    vsub.vx v0, v0, a1
    vsub.vx v2, v2, a1
    vsub.vx v4, v4, a1
    vsub.vx v6, v6, a1

    jal vet_store_a2
    mv ra, t0
    ret

.globl vet_vx_mul
vet_vx_mul:
    mv t0, ra
    jal vet_load_a0

    vmul.vx v0, v0, a1
    vmul.vx v2, v2, a1
    vmul.vx v4, v4, a1
    vmul.vx v6, v6, a1

    jal vet_store_a2
    mv ra, t0
    ret

.globl vet_vx_div
vet_vx_div:
    mv t0, ra
    jal vet_load_a0

    vdiv.vx v0, v0, a1
    vdiv.vx v2, v2, a1
    vdiv.vx v4, v4, a1
    vdiv.vx v6, v6, a1

    jal vet_store_a2
    mv ra, t0
    ret

/* VI operations */

.globl vet_vi_sum
vet_vi_sum:
    mv t0, ra
    jal vet_load_a0

    vadd.vi v0, v0, 5
    vadd.vi v2, v2, 5
    vadd.vi v4, v4, 5
    vadd.vi v6, v6, 5

    jal vet_store_a1
    mv ra, t0
    ret