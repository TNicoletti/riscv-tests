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