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




.globl sum1_group8
sum1_group8:
    mv t1, a1
    vsetvli t0, x0, e32, m8, ta, ma
    slli t3, t0, 2
    j sum_group_generic

.globl sum1_group4
sum1_group4:
    mv t1, a1
    vsetvli t0, x0, e32, m4, ta, ma
    slli t3, t0, 2
    j sum_group_generic

.globl sum1_group2
sum1_group2:
    mv t1, a1
    vsetvli t0, x0, e32, m2, ta, ma
    slli t3, t0, 2
    j sum_group_generic

.globl sum1_group1
sum1_group1:
    mv t1, a1
    vsetvli t0, x0, e32, m1, ta, ma
    slli t3, t0, 2

    j sum_group_generic

sum_group_generic:
    mv t2, zero
s_loop:
    vle32.v v8, (a0)      # Load 8-bit elements into group starting at v0
    vadd.vi v8, v8, 1

    vse32.v v8, (a0)

    add t2, t2, t0
    add a0, a0, t3
    bne t2, a1, s_loop


    mv a0, t0
    ret
