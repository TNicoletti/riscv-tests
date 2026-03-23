# RISC-V Assembly Program: Vector Extension & Exception Handling
# Target: RV64IMAFDCV (or RV32 equivalent logic)
# Author: Gemini
#
# Concepts covered:
# 1. Exception Handling (mtvec) with instruction skipping.
# 2. PMP Configuration (allowing User Mode memory access).
# 3. Mode Switching (Machine -> User).
# 4. Vector Configuration (e32, m1, ta, ma).
# 5. Execution of Instruction classes.

.section .data
.align 4
data_array:
    .word 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8
.align 4
result_array:
    .space 32
.section .data
.align 8
error_log:          
    .space 256 # Espaço para salvar até 32 endereços de erro (RV64)

.section .text
# -------------------------------------------------------------------------
# TRAP HANDLER (Machine Mode)
# -------------------------------------------------------------------------
.globl new_trap_handler
.align 4
new_trap_handler:
    # 1. Save temporary register to scratch so we can use it
    csrw mscratch, t6

    # 2. Read the cause of the exception
    csrr t6, mcause
    li t5, 8
    bne t5, t6, ignore_exception_handler

    csrw mscratch, t6
    j trap_entry

ignore_exception_handler:

    /* Conta quantos erros houve */
    la t0, error_count
    lw t1, 0(t0)          # t1 = contador atual

    /* Salva os erros */
    la t2, error_log
    slli t3, t1, 3        # t3 = contador * 8 (offset para RV64)
    add t2, t2, t3        # t2 = posição no array
    
    addi t1, t1, 1        # Incrementa contador
    sw t1, 0(t0)

    la t0, last_hw_error
    csrr t1, mcause
    sw t1, 0(t0)
    
    csrr t3, mepc         # Pega o endereço da instrução que falhou
    sd t3, 0(t2)          # Salva o endereço no log


    csrr t6, mepc

    lw t6, 0(t6)           # Load instruction encoding
    andi t6, t6, 3         # Mask last 2 bits
    li t5, 3
    beq t6, t5, is_32bit

is_16bit:
    csrr t6, mepc
    addi t6, t6, 2         # Skip 2 bytes
    j update_mepc

is_32bit:
    csrr t6, mepc
    addi t6, t6, 4         # Skip 4 bytes

update_mepc:
    csrw mepc, t6          # Write back new return address


    # 5. Restore registers
    csrr t6, mscratch
    
    # 6. Return from Trap
    mret