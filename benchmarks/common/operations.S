.globl load_OUT_t0_vet
load_OUT_t0_vet:
    mv t0, a0
    ret

.globl load_value_ft0
load_value_ft0:
    fmv.s ft0, fa0
    ret
