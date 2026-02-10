.section .text
.global vetorized_sum_1
.global close_WAW
.global close_RAW
.global close_WAR
.global close_WAW_compile_verify

// Sums 1 in all elements of "vector"
// a0 -> Vector length
// a1 -> LENGTH of vector
// a2 -> VLENGTH vetorization
vetorized_sum_1:
    vsetvli t3, a2, e32, m1, ta, ma
    slli    t1, t3, 2  # t1 = -> cores_used * 4
    slli    t4, a1, 2 # t4 = 4 * vector_length
    add     t0, a0, t4

    v_loop:

        vle32.v v7, (a0)       
        vadd.vi v8, v7, 1
        vse32.v v8, (a0)
        add     a0, a0, t1
        blt     a0, t0, v_loop

    mv a0, t3
    ret

// Sums 1 in all elements of vector, used to test debug registers and if the compiler is doing extra unwanted work. 
// a0 -> Vector length
// a1 -> LENGTH of vector
// a2 -> VLENGTH vetorization
close_WAW_compile_verify:
    vsetvli t3, a2, e32, m1, ta, ma
    slli    t1, t3, 2  # t1 = -> cores_used * 4
    slli    t4, a1, 2 # t4 = 4 * vector_length
    add     t0, a0, t4

    v_loop_WAW_verify:

        vle32.v v7, (a0)       
        vadd.vi v8, v7, 1
        vse32.v v8, (a0)
        add     a0, a0, t1
        blt     a0, t0, v_loop_WAW_verify

    mv a0, t3
    ret

// Sums 4 in all elements of vector, in a way it does a WAW hazard issue.
// a0 -> Vector length
// a1 -> LENGTH of vector
// a2 -> VLENGTH vetorization
close_WAW:
    vsetvli t3, a2, e32, m1, ta, ma
    slli    t1, t3, 2  # t1 = -> cores_used * 4
    slli    t4, a1, 2 # t4 = 4 * vector_length
    add     t0, a0, t4

    v_loop_WAW:

        vle32.v v5, (a0)       
        vadd.vi v8, v5, 1
        vadd.vi v8, v5, 3
        vse32.v v8, (a0)
        add     a0, a0, t1
        blt     a0, t0, v_loop_WAW

    mv a0, t3
    ret

// Sums 1 in all elements of vector, in a way it does a RAW hazard issue.
// a0 -> Vector length
// a1 -> LENGTH of vector
// a2 -> VLENGTH vetorization
close_RAW:
    vsetvli t3, a2, e32, m1, ta, ma
    slli    t1, t3, 2  # t1 = -> cores_used * 4
    slli    t4, a1, 2 # t4 = 4 * vector_length
    add     t0, a0, t4

    v_loop_RAW_verify:
        vle32.v v8, (a0)       
        vadd.vi v7, v8, 1
        vse32.v v7, (a0)
        add     a0, a0, t1
        blt     a0, t0, v_loop_RAW_verify

    mv a0, t3
    ret

// Sums 1 in all elements of vector, in a way it does a WAR hazard issue.
// a0 -> Vector length
// a1 -> LENGTH of vector
// a2 -> VLENGTH vetorization
close_WAR:
    vsetvli t3, a2, e32, m1, ta, ma
    slli    t1, t3, 2  # t1 = -> cores_used * 4
    slli    t4, a1, 2 # t4 = 4 * vector_length
    add     t0, a0, t4

    v_loop_WAR_verify:
        vmv.v.i v7, 0
        vle32.v v9, (a0)       
        vadd.vi v8, v9, 1
        vadd.vi v9, v7, 1 # if write happens before read, v8 will be wrong
        vse32.v v8, (a0)
        add     a0, a0, t1
        blt     a0, t0, v_loop_WAR_verify

    mv a0, t3
    ret