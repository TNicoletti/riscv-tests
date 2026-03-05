#define RET_INSTR     0x00008067

// Vetorial OPS
#define VADD_VV_INSTR 0x02110057
#define VSUB_VV_INSTR 0x0a110057
#define VDIV_VV_INSTR 0x86112057
#define VMUL_VV_INSTR 0x96112057

// Vetorial data manipulation
#define VMV_V_X_INSTR 0x5e02c057
#define VLE32_V_INSTR 0x0202e007
#define VSE32_V_INSTR 0x0202e027

// Scalar OPS
#define ADDI_ZERO_T0_INSTR 0x00100293 // addi t0, zero, 1
#define ADDI_ZZZ_INSTR 0x00000013 // addi zero, zero, 0 // NOP



