.globl test
test:
    mv t0, a0
    vle32.v v8, (t0)
    addi t0, t0, 16
    vle32.v v6, (t0)
    addi t0, t0, 16
    vle32.v v15, (t0)
    li t0, 166659206

    VMAX.VX v15, v6, t0
    VSLL.VV v15, v6, v8
    VREDMINU.VS v8, v8, v8
    VSRA.VI v6, v8, 6

    mv t0, a1
    vse32.v v8, (t0)
    addi t0, t0, 16
    vse32.v v6, (t0)
    addi t0, t0, 16
    vse32.v v15, (t0)

    ret