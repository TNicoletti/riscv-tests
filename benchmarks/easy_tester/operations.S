.globl test
test:
    mv t0, a0
    vle32.v v0, (t0)
    addi t0, t0, 16
    vle32.v v1, (t0)
    addi t0, t0, 16
    vle32.v v2, (t0)

    vadd.vv v2, v1, v0
    vdiv.vv v0, v2, v1
    vdiv.vv v2, v1, v1
    vmul.vv v2, v1, v0

    mv t0, a0
    vse32.v v0, (t0)
    addi t0, t0, 16
    vse32.v v1, (t0)
    addi t0, t0, 16
    vse32.v v2, (t0)

    ret