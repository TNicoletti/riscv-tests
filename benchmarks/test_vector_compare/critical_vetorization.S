.section .text

# ====================================================================
# MACRO: Unary/Immediate Operation Loop
# Arguments:
#   NAME: Function name
#   INSN: Instruction (e.g., vadd.vi)
# ====================================================================
.macro IMPL_VI_LOOP NAME, INSN
.global \NAME
\NAME:
    # a0 = vector pointer (in/out)
    # a1 = length (N)
    # a2 = immediate value (passed as argument for flexibility)
    
    mv t1, a0           # Save original pointer if needed (not strictly used here)
    mv t2, a1           # Save original length (not strictly used here)

1:  # Loop start
    vsetvli t0, a1, e32, m1, ta, ma   # Set VL based on remaining length
    vle32.v v8, (a0)                  # Load vector
    \INSN   v8, v8, 1                # Execute instruction (v8 = v8 op imm)
    vse32.v v8, (a0)                  # Store vector
    
    sub     a1, a1, t0                # Decrement count
    slli    t3, t0, 2                 # t3 = VL * 4 (bytes)
    add     a0, a0, t3                # Advance pointer
    bnez    a1, 1b                    # Loop if count > 0

    mv a0, t0                         # Return last VL (just to return something)
    ret
.endm

# ====================================================================
# MACRO: Unary/Immediate register Operation Loop
# Arguments:
#   NAME: Function name
#   INSN: Instruction (e.g., vadd.vi)
# ====================================================================
.macro IMPL_VX_LOOP NAME, INSN
.global \NAME
\NAME:
    # a0 = vector pointer (in/out)
    # a1 = length (N)
    # a2 = immediate value (passed as argument for flexibility)
    
    mv t1, a0           # Save original pointer if needed (not strictly used here)
    mv t2, a1           # Save original length (not strictly used here)

1:  # Loop start
    vsetvli t0, a1, e32, m1, ta, ma   # Set VL based on remaining length
    vle32.v v8, (a0)                  # Load vector
    \INSN   v8, v8, a2                # Execute instruction (v8 = v8 op imm)
    vse32.v v8, (a0)                  # Store vector
    
    sub     a1, a1, t0                # Decrement count
    slli    t3, t0, 2                 # t3 = VL * 4 (bytes)
    add     a0, a0, t3                # Advance pointer
    bnez    a1, 1b                    # Loop if count > 0

    mv a0, t0                         # Return last VL (just to return something)
    ret
.endm

# ====================================================================
# MACRO: Binary (Vector-Vector) Operation Loop
# Arguments:
#   NAME: Function name
#   INSN: Instruction (e.g., vadd.vv)
# ====================================================================
.macro IMPL_VV_LOOP NAME, INSN
.global \NAME
\NAME:
    # a0 = dest pointer
    # a1 = src1 pointer
    # a2 = src2 pointer
    # a3 = length (N)
    
1:  # Loop start
    vsetvli t0, a3, e32, m1, ta, ma   # Set VL
    vle32.v v8, (a1)                  # Load src1
    vle32.v v16, (a2)                 # Load src2
    \INSN   v24, v8, v16              # Execute: v24 = v8 op v16
    vse32.v v24, (a0)                 # Store dest
    
    sub     a3, a3, t0                # Decrement count
    slli    t3, t0, 2                 # Offset in bytes
    add     a0, a0, t3                # Advance dest
    add     a1, a1, t3                # Advance src1
    add     a2, a2, t3                # Advance src2
    bnez    a3, 1b                    # Loop

    mv a0, t0                         # Return last VL
    ret
.endm

# ================= IMPLEMENTATIONS =================

# --- Vector-Immediate (In-Place) ---
# Signature: int func(int* vector, int len, int immediate)
IMPL_VI_LOOP VADD_VI_ASM, vadd.vi
IMPL_VI_LOOP VAND_VI_ASM, vand.vi
IMPL_VI_LOOP VOR_VI_ASM,  vor.vi
IMPL_VI_LOOP VXOR_VI_ASM, vxor.vi

# --- Vector-Immediate (In-Place) ---
# Signature: int func(int* vector, int len, int immediate)
IMPL_VX_LOOP VADD_VX_ASM, vadd.vx
IMPL_VX_LOOP VSUB_VX_ASM, vsub.vx  # Note: vsub.vi doesn't exist directly, usually vadd.vi -imm, but for simplicity assuming assembler handles or using vadd
IMPL_VX_LOOP VAND_VX_ASM, vand.vx
IMPL_VX_LOOP VOR_VX_ASM,  vor.vx
IMPL_VX_LOOP VXOR_VX_ASM, vxor.vx

# --- Vector-Vector (Three Pointers) ---
# Signature: int func(int* dst, int* src1, int* src2, int len)
IMPL_VV_LOOP VADD_VV_ASM, vadd.vv
IMPL_VV_LOOP VSUB_VV_ASM, vsub.vv
IMPL_VV_LOOP VMUL_VV_ASM, vmul.vv
IMPL_VV_LOOP VDIV_VV_ASM, vdiv.vv  # Signed divide
IMPL_VV_LOOP VAND_VV_ASM, vand.vv
IMPL_VV_LOOP VOR_VV_ASM,  vor.vv
IMPL_VV_LOOP VXOR_VV_ASM, vxor.vv