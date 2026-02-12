/* CONFIGURATION */
.section .text

.globl get_max_vl_8
get_max_vl_8:    
    vsetvli a0, x0, e32, m8, ta, ma  
    ret

.globl get_max_vl_4
get_max_vl_4:    
    vsetvli a0, x0, e32, m4, ta, ma  
    ret

.globl get_max_vl_2
get_max_vl_2:    
    vsetvli a0, x0, e32, m2, ta, ma  
    ret


.globl get_max_vl_1
get_max_vl_1:    
    vsetvli a0, x0, e32, m1, ta, ma  
    ret

/* MEMORY HELPERS */ 

vet_load_a0_a1:
    vle32.v v0, (a0)
    vle32.v v8, (a1)
    ret

vet_load_a0:
    vle32.v v0, (a0)
    ret

vet_store_a1:
    vse32.v v0, (a1)
    ret

vet_store_a2:
    vse32.v v0, (a2)
    ret

/* VV operations */

.globl vet_vv_sum
# a0, a1, a2 => a2 = a1 + a0
vet_vv_sum:
    mv t0, ra
    jal vet_load_a0_a1

    vadd.vv v0, v0, v8

    jal vet_store_a2
    mv ra, t0
    ret

.globl vet_vv_sub
# a0, a1, a2 => a2 = a1 - a0
vet_vv_sub:
    mv t0, ra
    jal vet_load_a0_a1

    vsub.vv v0, v0, v8

    jal vet_store_a2
    mv ra, t0
    ret

.globl vet_vv_div
# a0, a1, a2 => a2 = a1 / a0
vet_vv_div:
    mv t0, ra
    jal vet_load_a0_a1

    vdiv.vv v0, v0, v8

    jal vet_store_a2
    mv ra, t0
    ret

.globl vet_vv_mul
# a0, a1, a2 => a2 = a1 * a0
vet_vv_mul:
    mv t0, ra
    jal vet_load_a0_a1

    vmul.vv v0, v0, v8

    jal vet_store_a2
    mv ra, t0
    ret

/* VX OPERATIONS */

.globl vet_vx_sum
vet_vx_sum:
    mv t0, ra
    jal vet_load_a0

    vadd.vx v0, v0, a1

    jal vet_store_a2
    mv ra, t0
    ret

.globl vet_vx_sub
vet_vx_sub:
    mv t0, ra
    jal vet_load_a0

    vsub.vx v0, v0, a1

    jal vet_store_a2
    mv ra, t0
    ret

.globl vet_vx_mul
vet_vx_mul:
    mv t0, ra
    jal vet_load_a0

    vmul.vx v0, v0, a1

    jal vet_store_a2
    mv ra, t0
    ret

.globl vet_vx_div
vet_vx_div:
    mv t0, ra
    jal vet_load_a0

    vdiv.vx v0, v0, a1

    jal vet_store_a2
    mv ra, t0
    ret

/* VI operations */

.globl vet_vi_sum
vet_vi_sum:
    mv t0, ra
    jal vet_load_a0

    vadd.vi v0, v0, 5

    jal vet_store_a1
    mv ra, t0
    ret