.globl load_OUT_t0_vet
load_OUT_t0_vet:
    mv t0, a0
    ret

.globl load_value_ft0
load_value_ft0:
    fmv.s ft0, fa0
    ret

.globl return_t0
return_t0:
    mv a0, t0
    ret
