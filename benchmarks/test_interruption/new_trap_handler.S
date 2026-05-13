.section .data
.align 8
trap_reg_save:
    .space 40   # Espaço para salvar 5 registradores de 64 bits (5 * 8 bytes)

.section .text
.globl new_trap_handler
.align 4
new_trap_handler:
    csrw mscratch, t0
    la t0, trap_reg_save
    sd t1, 0(t0)
    sd t3, 8(t0)
    sd t5, 16(t0)
    sd t6, 24(t0)
    csrr t1, mscratch
    sd t1, 32(t0)

    # ========================================================
    # SEU CÓDIGO DE TRAP ORIGINAL COMEÇA AQUI
    # ========================================================
    csrr t6, mcause
    li t5, 8
    bne t5, t6, ignore_exception_handler

    # (Lógica de tratamento do ecall se houver...)
    j trap_entry

ignore_exception_handler:

    /* Conta quantos erros houve */
    la t0, error_count_2
    lw t1, 0(t0)          

    /* Salva os erros */
    addi t1, t1, 1        
    sw t1, 0(t0)

    la t0, last_hw_error
    csrr t1, mcause
    sw t1, 0(t0)
    
    csrr t6, mepc
    lhu t6, 0(t6)          # Usando lhu para não dar erro de alinhamento
    andi t6, t6, 3         
    li t5, 3
    beq t6, t5, is_32bit

is_16bit:
    csrr t6, mepc
    addi t6, t6, 2         
    j update_mepc

is_32bit:
    csrr t6, mepc
    addi t6, t6, 4         

update_mepc:
    csrw mepc, t6          # Atualiza o PC
    # ========================================================
    # FIM DA LÓGICA DO TRAP
    # ========================================================

    la t0, trap_reg_save
    ld t1, 0(t0)
    ld t3, 8(t0)
    ld t5, 16(t0)
    ld t6, 24(t0)
    ld t0, 32(t0)
    
    # 6. Retorna do Trap (agora o C nem sabe que fomos lá)
    mret