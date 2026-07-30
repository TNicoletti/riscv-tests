#ifndef ASM_FUNCTIONS
#define ASM_FUNCTIONS
// Scalar OPS
#define ADDI_ZERO_T0_INSTR 0x00100293 // addi t0, zero, 1
#define ADDI_ZZZ_INSTR 0x00000013 // addi zero, zero, 0 // NOP
#define RET_INSTR     0x00008067


// --- VECTOR MEMORY ---
// Int 
// VV
#define VADD_VV_INSTR       0x02110057 // vadd.vv v0, v1, v2
#define VSUB_VV_INSTR       0x0a110057 // vsub.vv v0, v1, v2
#define VDIV_VV_INSTR       0x86112057 // vdiv.vv v0, v1, v2
#define VMUL_VV_INSTR       0x96112057 // vmul.vv v0, v1, v2
#define VSLL_VV_INSTR       0x96110057 // vsll.vv v0, v1, v2
#define VSRL_VV_INSTR       0xa2110057 // vsrl.vv v0, v1, v2
#define VAND_VV_INSTR       0x26110057 // vand.vv v0, v1, v2
#define VOR_VV_INSTR        0x2a110057 // vor.vv  v0, v1, v2
#define VXOR_VV_INSTR       0x2e110057 // vxor.vv v0, v1, v2

// VI
#define VADD_VI_INSTR       0x02113057 // vadd.vi v0, v1, 2
#define VSLL_VI_INSTR       0x96113057 // vsll.vi v0, v1, 2
#define VSRL_VI_INSTR       0xa2113057 // vsrl.vi v0, v1, 2
#define VAND_VI_INSTR       0x26113057 // vand.vi v0, v1, 2
#define VOR_VI_INSTR        0x2a113057 // vor.vi  v0, v1, 2
#define VXOR_VI_INSTR       0x2e113057 // vxor.vi v0, v1, 2

// VX
#define VADD_VX_INSTR       0x0212c057 // vadd.vx v0, v1, t0
#define VSUB_VX_INSTR       0x0a12c057 // vsub.vx v0, v1, t0
#define VDIV_VX_INSTR       0x8612e057 // vdiv.vx v0, v1, t0
#define VMUL_VX_INSTR       0x9612e057 // vmul.vx v0, v1, t0
#define VSLL_VX_INSTR       0x9612c057 // vsll.vx v0, v1, t0
#define VSRL_VX_INSTR       0xa212c057 // vsrl.vx v0, v1, t0
#define VAND_VX_INSTR       0x2613c057 // vand.vx v0, v1, t2
#define VOR_VX_INSTR        0x2a13c057 // vor.vx  v0, v1, t2
#define VXOR_VX_INSTR       0x2e13c057 // vxor.vx v0, v1, t2

// Slide
#define VSLIDEUP_VI_INSTR   0x3a113057 // vslideup.vi v0, v1, 2
#define VSLIDEUP_VX_INSTR   0x3a13c057 // vslideup.vx v0, v1, t2
#define VSLIDEDOWN_VI_INSTR 0x3e113057 // vslidedown.vi v0, v1, 2
#define VSLIDEDOWN_VX_INSTR 0x3e13c057 // vslidedown.vx v0, v1, t2

// VECTOR PERMUTATION
#define VMV_V_V_INSTR       0x5e008057 // vmv.v.v v0, v1
#define VMV_V_I_INSTR       0x5e00b057 // vmv.v.i v0, 1
#define VMV_V_X_INSTR       0x5e034057 // vmv.v.x v0, t1

// VECTOR REDUCTION
#define VREDSUM_VS_INSTR    0x0215a557 // vredsum.vs v10, v1, v11
#define VREDMAXU_VS_INSTR   0x1a20a057 // v0,v2,v1
#define VREDMAX_VS_INSTR    0x1e20a057 // v0,v2,v1
#define VREDMINU_VS_INSTR   0x1220a057 // v0,v2,v1
#define VREDMIN_VS_INSTR    0x1620a057 // v0,v2,v1
#define VREDAND_VS_INSTR    0x0620a057 // v0,v2,v1
#define VREDOR_VS_INSTR     0x0a20a057 // v0,v2,v1
#define VREDXOR_VS_INSTR    0x0e20a057 // v0,v2,v1

// Memory
// (la a0, data_array é pseudo-instrução e não possui opcode fixo vetorizado direto)
#define VMV_V_X_T0_INSTR    0x5e02c057 // vmv.v.x v0, t0

#define VLE8_V_INSTR        0x02028007 // vle32.v v0, (t0) / (a0)
#define VSE8_V_INSTR        0x02028027 // vse32.v v0, (t0) / (a0)

#define VLE16_V_INSTR       0x0202d007 // vle32.v v0, (t0) / (a0)
#define VSE16_V_INSTR       0x0202d007 // vse32.v v0, (t0) / (a0)

#define VLE32_V_INSTR       0x0202e007 // vle32.v v0, (t0) / (a0)
#define VSE32_V_INSTR       0x0202e027 // vse32.v v0, (t0) / (a0)

#define VLE64_V_INSTR       0x0202f007 // vle64.v v0, (t0) / (a0)
#define VSE64_V_INSTR       0x0202f027 // vse64.v v0, (t0) / (a0)

// STRIDED (Espaçado)
#define LI_T0_4_INSTR       0x00400293 // li t0, 4 (escalar associado)
#define VLSE32_V_INSTR      0x0a556107 // vlse32.v v2, (a0), t0

// INDEXED (Gather/Scatter)
#define VLUXEI32_V_INSTR    0x06456187 // vluxei32.v v3, (a0), v4

// Widening instructions
#define VWADD_VV_INSTR      0xc6112557 // vwadd.vv v10, v1, v2
#define VWADD_VX_INSTR      0xc613e557 // vwadd.vx v10, v1, t2
#define VWSUB_VV_INSTR      0xce112657 // vwsub.vv v12, v1, v2
#define VWSUB_VX_INSTR      0xce13e657 // vwsub.vx v12, v1, t2
#define VWADDU_VV_INSTR     0xc2112757 // vwaddu.vv v14, v1, v2
#define VWADDU_VX_INSTR     0xc213e757 // vwaddu.vx v14, v1, t2
#define VWMUL_VV_INSTR      0xee112857 // vwmul.vv v16, v1, v2
#define VWMUL_VX_INSTR      0xee13e857 // vwmul.vx v16, v1, t2
#define VWMACC_VV_INSTR     0xf620a957 // vwmacc.vv v18, v1, v2

// MASK EXTENSION
// MASK LOGIC
#define VMAND_MM_INSTR      0x66112057 // vmand.mm v0, v1, v2
#define VMOR_MM_INSTR       0x6a112057 // vmor.mm v0, v1, v2
#define VMNAND_MM_INSTR     0x76112057 // vmnand.mm v0, v1, v2
#define VMXOR_MM_INSTR      0x6e112057 // vmxor.mm v0, v1, v2

// COMPARE
#define VMSEQ_VV_INSTR      0x62110057 // vmseq.vv v0, v1, v2
#define VMSEQ_VI_INSTR      0x62113057 // vmseq.vi v0, v1, 2
#define VMSEQ_VX_INSTR      0x6213c057 // vmseq.vx v0, v1, t2

#define VMSNE_VV_INSTR      0x66110057 // vmsne.vv v0, v1, v2
#define VMSNE_VI_INSTR      0x66113057 // vmsne.vi v0, v1, 2
#define VMSNE_VX_INSTR      0x6613c057 // vmsne.vx v0, v1, t2

#define VMSLT_VV_INSTR      0x6e110057 // vmslt.vv v0, v1, v2
#define VMSLT_VX_INSTR      0x6e13c057 // vmslt.vx v0, v1, t2

#define VMSLE_VV_INSTR      0x76110057 // vmsle.vv v0, v1, v2
#define VMSLE_VI_INSTR      0x76113057 // vmsle.vi v0, v1, 2
#define VMSLE_VX_INSTR      0x7613c057 // vmsle.vx v0, v1, t2

// Mapeamentos para pseudo-instruções "Greater Than" geradas pelo compilador
#define VMSGT_VI_INSTR      0x7e113057 // vmsgt.vi v0, v1, 2
#define VMSGT_VX_INSTR      0x7e13c057 // vmsgt.vx v0, v1, t2

// COMPRESS
#define VCOMPRESS_VM_INSTR  0x5e2020d7 // vcompress.vm v1, v2, v0

// MASK POPULATION COUNT
#define VCPOP_M_INSTR       0x420822d7 // vcpop.m t0, v0 (dump usa vcpop ao invés de vpopc)
#define VFIRST_M_INSTR      0x4208a357 // vfirst.m t1, v0

// Float extension
#define VFADD_VV_INSTR      0x02119057 // vfadd.vv v0, v1, v3
#define VFADD_VF_INSTR      0x02115057 // vfadd.vf v0, v1, ft2

#define VFSUB_VV_INSTR      0x0a119057 // vfsub.vv v0, v1, v3
#define VFSUB_VF_INSTR      0x0a115057 // vfsub.vf v0, v1, ft2

#define VFMUL_VV_INSTR      0x92141057 // vfmul.vv v0, v1, v8
#define VFMUL_VF_INSTR      0x92115057 // vfmul.vf v0, v1, ft2

#define VFDIV_VV_INSTR      0x82111057 // vfdiv.vv v0, v1, v2
#define VFDIV_VF_INSTR      0x82115057 // vfdiv.vf v0, v1, ft2

#define VFSQRT_V_INSTR      0x4e101057 // vfsqrt.v v0, v1

#define VFMACC_VV_INSTR     0xb2209057 // vfmacc.vv v0, v1, v2
#define VMACC_VV_INSTR      0xb620a057 // vfmacc.vv v0, v1, v2

// Float compare
#define VMFEQ_VV_INSTR      0x62111057 // vmfeq.vv v0, v1, v2
#define VMFNE_VV_INSTR      0x72111057 // vmfne.vv v0, v1, v2

#define VMFLT_VV_INSTR      0x6e111057 // vmflt.vv v0, v1, v2
#define VMFLE_VV_INSTR      0x66111057 // vmfle.vv v0, v1, v2

// Vector mask merge
#define VMERGE_VVM_INSTR 0x5c2180d7          	//vmerge.vvm	v0,v1,v2,v0
#define VMERGE_VXM_INSTR 0x5c2e40d7          	//vmerge.vxm	v0,v1,t2,v0
#define VMERGE_VIM_INSTR 0x5c21b0d7          	//vmerge.vim	v0,v1,2,v0

// Vector mask shift u (Comparisons)
#define VMSLTU_VV_INSTR     0x6a110057 // vmsltu.vv v0, v1, v2
#define VMSLTU_VX_INSTR     0x6a13c057 // vmsltu.vx v0, v1, t2
#define VMSLEU_VV_INSTR     0x72110057 // vmsleu.vv v0, v1, v2
#define VMSLEU_VX_INSTR     0x7213c057 // vmsleu.vx v0, v1, t2

// Div unsigned
#define VDIVU_VV_INSTR      0x82112057 // vdivu.vv v0, v1, v2
#define VDIVU_VX_INSTR      0x8213e057 // vdivu.vx v0, v1, t2

// Remainder
#define VREM_VV_INSTR       0x8e112057 // vrem.vv v0, v1, v2
#define VREM_VX_INSTR       0x8e13e057 // vrem.vx v0, v1, t2
#define VREMU_VV_INSTR      0x8a112057 // vremu.vv v0, v1, v2
#define VREMU_VX_INSTR      0x8a13e057 // vremu.vx v0, v1, t2

// Mul High
#define VMULH_VV_INSTR      0x9e112057
#define VMULHU_VV_INSTR     0x92112057 // vmulhu.vv v0, v1, v2
#define VMULHU_VX_INSTR     0x9213e057 // vmulhu.vx v0, v1, t2
#define VMULHSU_VV_INSTR    0x9a112057 // vmulhsu.vv v0, v1, v2
#define VMULHSU_VX_INSTR    0x9a13e057 // vmulhsu.vx v0, v1, t2

// Min and max values
#define VMIN_VV_INSTR       0x16110057 // vmin.vv v0, v1, v2
#define VMIN_VX_INSTR       0x1613c057 // vmin.vx v0, v1, t2
#define VMAX_VV_INSTR       0x1e110057 // vmax.vv v0, v1, v2
#define VMAX_VX_INSTR       0x1e13c057 // vmax.vx v0, v1, t2
#define VMINU_VV_INSTR      0x12110057 // vminu.vv v0, v1, v2
#define VMINU_VX_INSTR      0x1213c057 // vminu.vx v0, v1, t2
#define VMAXU_VV_INSTR      0x1a110057 // vmaxu.vv v0, v1, v2
#define VMAXU_VX_INSTR      0x1a13c057 // vmaxu.vx v0, v1, t2

// Arithmetic shift right
#define VSRA_VV_INSTR       0xa6110057 // vsra.vv v0, v1, v2
#define VSRA_VI_INSTR       0xa6113057 // vsra.vi v0, v1, 2
#define VSRA_VX_INSTR       0xa613c057 // vsra.vx v0, v1, t2

#endif