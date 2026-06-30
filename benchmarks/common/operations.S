.globl load_OUT_t0_vet
load_OUT_t0_vet:
    mv t0, a0
    ret

#.globl load_value_ft0_half
#load_value_ft0_single_half:
#    fmv.h ft0, fa0
#    ret

.globl load_value_ft0_single
load_value_ft0_single:
    fmv.s ft0, fa0
    ret

.globl load_value_ft0_double
load_value_ft0_double:
    fmv.d ft0, fa0
    ret

.globl return_t1
return_t1:
    mv a0, t1
    ret
