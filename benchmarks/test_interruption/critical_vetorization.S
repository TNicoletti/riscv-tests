.data

wa: .ascii "TEST TEST TEST \n"

.section .text

.globl vector_sum_interrupt
# int vector_agnostic_sum_1(int* vector,int LENGTH);
vector_sum_interrupt:
    mv t0, a1
    mv t4, a0
loop:
    vsetvli t1, t0, e32, m1, tu, ma 
    slli t2, t1, 2

    vle32.v v3, (t4)   
    vadd.vi v3, v3, 5
    vse32.v v3, (t4)   

    add t4, t4, t2     
    sub t0, t0, t1     
    bnez t0, loop

    ret

.globl vector_limit
vector_limit:
    # --- RULE 1: The Guard Page (Forbidden Zone) ---
    la a0, vet
    li t0, 4096
    add a0, a0, t0       # Start of forbidden zone (vet + 4096)
    li t0, 8
    add a1, a0, t0       # End of forbidden zone (vet + 4096 + 8)

    srli t2, a0, 2
    csrw pmpaddr0, t2    # PMP0: Start of forbidden zone

    srli t2, a1, 2
    csrw pmpaddr1, t2    # PMP1: End of forbidden zone (TOR)

    # --- RULE 2: Allow everything else (PMP2) ---
    li t2, -1            
    csrw pmpaddr2, t2

    # --- Configuration (pmpcfg0) ---
    # PMP0: 0x00 (OFF - base for TOR)
    # PMP1: 0x08 (TOR | NO permissions) -> Denies access to [vet+4096, vet+8192)
    # PMP2: 0x1F (NAPOT | R | W | X) -> Allows the rest of memory (stack, code, vet itself)
    
    li t3, (0x08 << 8) | (0x1F << 16)
    csrw pmpcfg0, t3

    # --- Transition to User Mode ---
    li t0, ~(3 << 11)    
    csrr t1, mstatus
    and t1, t1, t0
    csrw mstatus, t1

    csrw mepc, ra        
    mret