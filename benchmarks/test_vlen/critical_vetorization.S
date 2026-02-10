.section .text
.global vetorized
.global non_vetorized

// a0 -> LENGTH of vector
// a1 -> VLENGTH vetorization
vetorized:
    # 1. Configura vetor e pega o 'vl' REAL em t3
    vsetvli t3, a1, e32, m1, ta, ma

    # 2. Calcula o passo (stride) em BYTES para cada iteração
    # t1 = t3 * 4 (Número de elementos reais * 4 bytes)
    slli    t1, t3, 2   # slli x, y, 2 é igual a multiplicar por 4

    # 3. Calcula o tamanho TOTAL do array em BYTES
    # t4 = a0 * 4 
    slli    t4, a0, 2

    # 4. Define o endereço limite (onde o loop deve parar)
    # Limite = SP atual - Tamanho Total em Bytes
    sub     t0, sp, t4

    li      t2, 99

    v_loop:
        vmv.v.x v8, t2          
        vadd.vi v8, v8, 1
        
        # Decrementa SP pelo tamanho do bloco REAL (t1)
        sub     sp, sp, t1
        
        # Guarda o vetor na nova posição do SP
        vse32.v v8, (sp)
        
        # Se o SP atual ainda for maior que o endereço limite (t0), repete
        bgt     sp, t0, v_loop

    # 5. Restaura a pilha usando o tamanho TOTAL em bytes (t4)
    add    sp, sp, t4
    mv a0, t3
    ret

# Função para ler o CSR 'instret'
non_vetorized:
    li t1, 4
    mul a0, a0, t1
    sub t0, sp, a0
    nv_loop:
        li t1, 99
        addi t1, t1, 1
        addi    sp, sp, -4
        sw t1, (sp)
        bne t0, sp, nv_loop


    add sp, sp, a0
    li a0, 1
    ret