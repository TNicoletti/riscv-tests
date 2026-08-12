#include "add_instruction.h"

intSEW t0_VALUE = 16;
floaSEW f_vf = 0.15;
int imm = 7;
int compare_registers = -1;  //
intSEW actual_t1 = 0;
int error_count = 0;
int last_hw_error = 0;

INTXLEN SL_A_VECTOR[2];
volatile intSEW scalar_res[32][VLEN / SEW];

/* ===== VECTOR LOADERS =====*/

void load_to_vet(intSEW* vet, int reg){
    SL_A_VECTOR[1] = RET_INSTR;
    INT_INST instr = instLSEW;
    instr = change_vet_rd(instr, reg);
    load_OUT_t0_vet(vet);
    SL_A_VECTOR[0] = instr;
    jump_to_vet(&SL_A_VECTOR[0]);
}

void store_to_vet(intSEW* vet, int reg){
    SL_A_VECTOR[1] = RET_INSTR;
    INT_INST instr = instSSEW;
    instr = change_vet_rd(instr, reg);
    load_OUT_t0_vet(vet);
    SL_A_VECTOR[0] = instr;
    jump_to_vet(&SL_A_VECTOR[0]);
}

void load_init_values_vector(intSEW* vet, INTXLEN* regs, int num_registers){    
    set_vet_settings();
    for(int i = 0; i < num_registers; i++){
        load_to_vet(&vet[i * VLEN / SEW * LMUL], regs[i]);
    }
}

void store_vet_values(INTXLEN* r, intSEW* vet_res, int num_registers){
    clean_vector_scalar(&vet_res[0], VLEN / SEW * num_registers);

    set_vet_settings();
    for(int i = 0; i < num_registers; i++){
        store_to_vet(&vet_res[r[i] * VLEN / SEW], r[i]);
    }
}


void load_init_values_scalar(intSEW* vet, int* r, int num_registers){
    //clean_vector_scalar(&scalar_res[0][0], VLEN / SEW * 32);
    imm      = (intSEW)vet[2 * VLEN / SEW] & 0x1F;
    if(imm >= 16)
        imm = imm - 32;
    t0_VALUE =  vet[2 * VLEN / SEW];
    f_vf     =  vet[2 * VLEN / SEW];
    
    for(int i = 0; i < EL_PER_BLOCK; i++)
    {
        for(int j = 0; j < num_registers; j++)
            scalar_res[r[j]][i] = vet[j * EL_PER_BLOCK + i];
    }

}

// r[0] = rd; r[1] = rs1; r[2] = rs2
int widening_forbid(int rx[3]){
    if(SEW * 2 > ELEN)
        return 1;

    if(LMUL == 8)
        return 1;
    if(((rx[1] - rx[0]) <= LMUL && rx[0] <= rx[1]) || (rx[2] - rx[0]
     <= LMUL && rx[0] <= rx[2]) || rx[1] == rx[2])
        return 1;
    return (rx[0] % (LMUL * 2)) != 0;
}

// r[0] = rd; r[1] = rs1; r[2] = rs2
int slideup_forbid(int vd, int vs2, int lmul) {
    if (lmul <= 1) {
        return vd == vs2;
    }
    
    if (abs(vd - vs2) < lmul) {
        return 1;
    }
    
    return 0;
}

int compress_forbid(int vd, int vs1, int vs2) {
    if(vd == vs1)
        return true;
    if(vd == vs2)
        return true;

    return false;
}

void require_imm_positive(){
    imm = (imm>=0)?imm:-imm;
}
void require_imm_normal(){
    if(imm >= 16)
        imm = imm - 32;
}

int add_instruction(int op, int rxa[3], int r[3]){
    

    intSEW i = 1;

    INTXLEN ut0 = t0_VALUE;

    int rx[3] = {r[rxa[0]], r[rxa[1]], r[rxa[2]]};

    //if(PRINTS >= 2) printf("OP: %s\n", get_OP_name(op));
    switch (op){
        case VADD_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] + scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VSUB_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d - [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] - scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VDIV_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d / [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] / scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMUL_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d * [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] * scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VSLL_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d << [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (intSEW)((uintSEW)scalar_res[rx[1]][j] << (scalar_res[rx[2]][j]));
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VSRL_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d >> [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (intSEW)((uintSEW)scalar_res[rx[1]][j] >> (scalar_res[rx[2]][j]));
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VAND_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d & [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] & scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VOR_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d | [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] | scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VXOR_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d ^ [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] ^ scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VADD_VI:
            require_imm_normal(imm);
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + imm(%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] + imm);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VSLL_VI:
            require_imm_positive();
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d << imm(%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] << imm);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VSRL_VI:
            require_imm_positive();
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d >> imm(%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j] = (intSEW)((uintSEW)scalar_res[rx[1]][j] >> imm);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VAND_VI:
            int ima = imm>=16?-imm:imm;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d & imm(%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j], ima);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] & ima);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VOR_VI:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d | imm(%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] | imm);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VXOR_VI:
            require_imm_normal();
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d ^ imm(%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] ^ imm);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VADD_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] + t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VSUB_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d - %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] - t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VDIV_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d / %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] / t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMUL_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d * %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] * t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VSLL_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d << %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] << t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VSRL_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d >> %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (intSEW)((uintSEW)scalar_res[rx[1]][j] >> t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VAND_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d & %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] & t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VOR_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d | %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] | t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VXOR_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d ^ %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] ^ t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VFADD_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) + bits_to_float(scalar_res[rx[2]][j]));
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VFADD_VF:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + 0.15;\n", rx[0], j, rx[1], scalar_res[rx[1]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) + f_vf);
            }
            break;
        case VFSUB_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d - [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) - bits_to_float(scalar_res[rx[2]][j]));
            }
            break;

        case VFSUB_VF:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d - 0.15;\n", rx[0], j, rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) - f_vf);
            }
            break;

        case VFMUL_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d * [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) * bits_to_float(scalar_res[rx[2]][j]));
            }
            break;

        case VFMUL_VF:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d * 0.15;\n", rx[0], j, rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) * f_vf);
            }
            break;

        case VFDIV_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d / [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) / bits_to_float(scalar_res[rx[2]][j]));
            }
            break;

        case VFDIV_VF:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d / 0.15;\n", rx[0], j, rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) / (double)f_vf);
            }
            break;

        case VFSQRT_V:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = sqrt([%d]%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j]);
                switch(SEW){
                    case 32:
                        scalar_res[rx[0]][j] = float_to_bits(sqrtf(bits_to_float(scalar_res[rx[1]][j])));
                        break;
                    case 64:
                        scalar_res[rx[0]][j] = float_to_bits(sqrt(bits_to_float(scalar_res[rx[1]][j])));
                        break;
                }
            }
            break;
        case VFMACC_VV: // TODO: fix very big numbers issue
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d * [%d]%d) + [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j], rx[0], scalar_res[rx[0]][j]);
                scalar_res[rx[0]][j] = float_to_bits(bits_to_float(scalar_res[rx[1]][j]) * bits_to_float(scalar_res[rx[2]][j]) + bits_to_float(scalar_res[rx[0]][j]));
            }
            break;
        case VMACC_VV: // TODO: fix very big numbers issue
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d * [%d]%d) + [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j], rx[0], scalar_res[rx[0]][j]);
                scalar_res[rx[0]][j] = scalar_res[rx[1]][j] * scalar_res[rx[2]][j] + scalar_res[rx[0]][j];
            }
            break;
        case VMACC_VX: // TODO: fix very big numbers issue
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ((t0)%d * [%d]%d) + [%d]%d;\n", rx[0], t0_VALUE, j, rx[1], rx[0], scalar_res[rx[0]][j]);
                scalar_res[rx[0]][j] = scalar_res[rx[1]][j] * t0_VALUE + scalar_res[rx[0]][j];
            }
            break;
        case VSLIDEUP_VI:
            require_imm_positive();
            if(slideup_forbid(rx[0], rx[1], LMUL)) { printf("FORBIDDEN\n"); return NOP; }
            for(int j = 0; j < EL_PER_BLOCK; j++){
                int dest_idx = imm + j;
    
                int idx  = rx[0] + dest_idx / (VLEN / SEW);
                //printf("%d %d %d\n", rx[0], dest_idx, (VLEN / SEW));
                
                if (dest_idx >= EL_PER_BLOCK) break;

                int idx2 = dest_idx % (VLEN / SEW);
                
                int src_reg = rx[1] + j / (VLEN / SEW);
                int src_idx = j % (VLEN / SEW);
                
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d;\n", idx, idx2, src_reg, scalar_res[idx][idx2]);
                scalar_res[idx][idx2] = (intSEW)(scalar_res[src_reg][src_idx]);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VSLIDEUP_VX:
            if(slideup_forbid(rx[0], rx[1], LMUL)) {printf("FORBIDDEN\n"); return NOP;}
            for(int j = 0; j < EL_PER_BLOCK; j++){
                uint dest_idx = ut0 + j;
                uint idx  = rx[0] + dest_idx / (VLEN / SEW);
                //printf("%d %d %d\n", rx[0], dest_idx, (VLEN / SEW));

    
                if (idx >= EL_PER_BLOCK) break;

                uint idx2 = dest_idx % (VLEN / SEW);
                
                uint src_reg = rx[1] + j / (VLEN / SEW);
                uint src_idx = j % (VLEN / SEW);
                
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d;\n", rx[0], j, rx[1], scalar_res[idx][idx2]);
                scalar_res[idx][idx2] = (intSEW)(scalar_res[src_reg][src_idx]);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VSLIDEDOWN_VI:
            require_imm_positive();
            for(int j = 0; j < EL_PER_BLOCK; j++){
                
                uint src_idx  = j + imm;
                
                uint dest_reg  = rx[0] + j / (VLEN / SEW);
                uint dest_elem = j % (VLEN / SEW);

                if (src_idx >= EL_PER_BLOCK) {
                    scalar_res[dest_reg][dest_elem] = 0;
                } else {
                    uint src_reg  = rx[1] + src_idx / (VLEN / SEW);
                    uint src_elem = src_idx % (VLEN / SEW);

                    //printf("[%d][%d] = [%d][%d]\n", dest_reg, dest_elem, src_reg, src_elem);
                    
                    scalar_res[dest_reg][dest_elem] = (intSEW)(scalar_res[src_reg][src_elem]);
                }
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VSLIDEDOWN_VX:
            if(rx[0] == rx[1]) return NOP;
            int maior = 0; // prevents overflow problems
            for(uint32_t j = 0; j < EL_PER_BLOCK; j++){
                uintSEW dest_idx = j;
                uintSEW src_idx  = j + t0_VALUE;
                
                uintSEW dest_reg  = (uintSEW)rx[0] + dest_idx / (uintSEW)(VLEN / SEW);
                uintSEW dest_elem = dest_idx % (uintSEW)(VLEN / SEW);

                if (src_idx >= (uintDSEW)(VLEN / SEW) * (uintDSEW)LMUL || maior == 1) {
                    maior = 1;
                    
                    scalar_res[dest_reg][dest_elem] = 0;
                } else {
                    uintSEW src_reg  = rx[1] + src_idx / (VLEN / SEW);
                    uintSEW src_elem = src_idx % (VLEN / SEW);
                    
                    scalar_res[dest_reg][dest_elem] = (intSEW)(scalar_res[src_reg][src_elem]);
                }
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMV_V_V:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = %d;\n", rx[0], j, scalar_res[rx[1]][j]);
                scalar_res[rx[0]][j] = scalar_res[rx[1]][j];
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMV_V_I:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = %d;\n", rx[0], j, imm);
                scalar_res[rx[0]][j] = imm;
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMV_V_X:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = %d;\n", rx[0], j, t0_VALUE);
                scalar_res[rx[0]][j] = t0_VALUE;
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VWADD_VV:
            if(widening_forbid(rx)) return NOP;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                intDSEW res = (intDSEW)((intDSEW)scalar_res[rx[1]][j] + (intDSEW)scalar_res[rx[2]][j]);
                scalar_res[rx[0]][(2 * j)]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[0]][(2 * j)+ 1 ] = \
                (res >> (intDSEW)32);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VWADD_VX:
            if(widening_forbid(rx)) return NOP;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                intDSEW res = (intDSEW)((intDSEW)scalar_res[rx[1]][j] + (intDSEW)t0_VALUE);
                scalar_res[rx[0]][(2 * j)]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[0]][(2 * j) + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VWSUB_VV:
            if(widening_forbid(rx)) return NOP;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d - [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                intDSEW res = (intDSEW)((intDSEW)scalar_res[rx[1]][j] - (intDSEW)scalar_res[rx[2]][j]);
                scalar_res[rx[0]][(2 * j)]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[0]][(2 * j) + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VWSUB_VX:
            if(widening_forbid(rx)) return NOP;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d - %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                intDSEW res = (intDSEW)((intDSEW)scalar_res[rx[1]][j] - (intDSEW)t0_VALUE);
                scalar_res[rx[0]][(2 * j)]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[0]][(2 * j) + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VWADDU_VV:
            if(widening_forbid(rx)) return NOP;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                uintDSEW res = ((uintDSEW)(uintSEW)scalar_res[rx[1]][j] + 
                (uintDSEW)(uintSEW)scalar_res[rx[2]][j]);
                scalar_res[rx[0]][(2 * j)]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[0]][(2 * j) + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VWADDU_VX:
            if(widening_forbid(rx)) return NOP;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d + %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                uintDSEW res = ((uintDSEW)(uintSEW)scalar_res[rx[1]][j] + (uintDSEW)(uintSEW)t0_VALUE);
                scalar_res[rx[0]][(2 * j)]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[0]][(2 * j) + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VWMUL_VV:
            if(widening_forbid(rx)) return NOP;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d * [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                intDSEW res = (intDSEW)((intDSEW)scalar_res[rx[1]][j] * (intDSEW)scalar_res[rx[2]][j]);
                scalar_res[rx[0]][(2 * j)]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[0]][(2 * j) + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VWMUL_VX:
            if(widening_forbid(rx)) return NOP;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d * %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                uintDSEW res = (uintDSEW)((uintDSEW)scalar_res[rx[1]][j] * (uintDSEW)t0_VALUE);
                scalar_res[rx[0]][(2 * j)]     = \
                res & 0xFFFFFFFF;
                scalar_res[rx[0]][(2 * j) + 1 ] = \
                (res >> 32);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VWMACC_VV:
            if(widening_forbid(rx)) {printf("FORBIDDEN\n"); return NOP;}
    
            intDSEW res[VLEN / SEW * 8];
            
            for(int j = 0; j < EL_PER_BLOCK; j++) {
                intDSEW product = (intDSEW)scalar_res[rx[1]][j] * (intDSEW)scalar_res[rx[2]][j];

                int flat_idx_lo = 2 * j;
                int reg_lo      = rx[0] + (flat_idx_lo / EL_PER_BLOCK);
                int idx_lo      = flat_idx_lo % EL_PER_BLOCK;

                int flat_idx_hi = 2 * j + 1;
                int reg_hi      = rx[0] + (flat_idx_hi / EL_PER_BLOCK);
                int idx_hi      = flat_idx_hi % EL_PER_BLOCK;

                uintSEW lo = (uintSEW)scalar_res[reg_lo][idx_lo];
                uintSEW hi = (uintSEW)scalar_res[reg_hi][idx_hi];                
                intDSEW existing_acc = (intDSEW)(((uintDSEW)hi << 32) | lo);
                res[j] = product + existing_acc;
            }

            for(int j = 0; j < EL_PER_BLOCK; j++) {
                int flat_idx_lo = 2 * j;
                int reg_lo      = rx[0] + (flat_idx_lo / EL_PER_BLOCK);
                int idx_lo      = flat_idx_lo % EL_PER_BLOCK;

                int flat_idx_hi = 2 * j + 1;
                int reg_hi      = rx[0] + (flat_idx_hi / EL_PER_BLOCK);
                int idx_hi      = flat_idx_hi % EL_PER_BLOCK;

                scalar_res[reg_lo][idx_lo] = res[j] & 0xFFFFFFFF;
                
                scalar_res[reg_hi][idx_hi] = (res[j] >> 32) & 0xFFFFFFFF;
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VREDSUM_VS:
            // VREDSUM.VS: Vector reduction sum. vd[0] = sum(vs2[0..VL-1]) + vs1[0]
            // Maps rx[1] -> vs1 (scalar accumulator) and rx[2] -> vs2 (vector to reduce).
            intSEW sum = scalar_res[rx[2]][0];
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = [%d]%d", rx[0], rx[1], scalar_res[rx[1]][0]);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf(" + [%d]%d", rx[2], scalar_res[rx[2]][j]);
                sum += scalar_res[rx[1]][j];
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = (intSEW)sum;

            break;
        case VREDMAXU_VS:
            // VREDSUM.VS: Vector reduction sum. vd[0] = sum(vs2[0..VL-1]) + vs1[0]
            // Maps rx[1] -> vs1 (scalar accumulator) and rx[2] -> vs2 (vector to reduce).
            uintSEW mx = scalar_res[rx[2]][0];
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = MAX [%d]%d", rx[0], rx[2], scalar_res[rx[2]][0]);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("; [%d]%d", rx[1], scalar_res[rx[1]][j]);
                mx = maxu(mx, scalar_res[rx[1]][j]);
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = (uintSEW)mx;

            break;

        case VREDMAX_VS:
            intSEW mx_s = scalar_res[rx[2]][0]; // vs1[0] is the scalar accumulator
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = MAX_S [%d]%d", rx[0], rx[2], mx_s);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                intSEW val = (intSEW)scalar_res[rx[1]][j];
                if(PRINTS >= 2) printf("; [%d]%d", rx[1], val);
                mx_s = max(mx_s, val);
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = (intSEW)mx_s;
            break;

        case VREDMINU_VS:
            // VREDMINU.VS: Vector reduction minimum (unsigned)
            uintSEW mn_u = (uintSEW)scalar_res[rx[2]][0];
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = MIN_U [%d]%u", rx[0], rx[2], mn_u);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                uintSEW val = (uintSEW)scalar_res[rx[1]][j];
                if(PRINTS >= 2) printf("; [%d]%u", rx[1], val);
                if(val < mn_u) mn_u = val;
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = (uintSEW)mn_u;
            break;

        case VREDMIN_VS:
            // VREDMIN.VS: Vector reduction minimum (signed)
            intSEW mn_s = (intSEW)scalar_res[rx[2]][0];
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = MIN_S [%d]%d", rx[0], rx[2], mn_s);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                intSEW val = (intSEW)scalar_res[rx[1]][j];
                if(PRINTS >= 2) printf("; [%d]%d", rx[1], val);
                if(val < mn_s) mn_s = val;
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = (intSEW)mn_s;
            break;

        case VREDAND_VS:
            // VREDAND.VS: Vector reduction bitwise AND
            uintSEW and_res = (uintSEW)scalar_res[rx[2]][0];
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = AND [%d]%x", rx[0], rx[2], and_res);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                uintSEW val = (uintSEW)scalar_res[rx[1]][j];
                if(PRINTS >= 2) printf(" & [%d]%x", rx[1], val);
                and_res &= val;
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = and_res;
            break;

        case VREDOR_VS:
            // VREDOR.VS: Vector reduction bitwise OR
            uintSEW or_res = (uintSEW)scalar_res[rx[2]][0];
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = OR [%d]%x", rx[0], rx[2], or_res);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                uintSEW val = (uintSEW)scalar_res[rx[1]][j];
                if(PRINTS >= 2) printf(" | [%d]%x", rx[1], val);
                or_res |= val;
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = or_res;
            break;

        case VREDXOR_VS:
            // VREDXOR.VS: Vector reduction bitwise XOR
            uintSEW xor_res = (uintSEW)scalar_res[rx[2]][0];
            if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][0] = XOR [%d]%x", rx[0], rx[2], xor_res);
            
            for(int j = 0; j < EL_PER_BLOCK; j++){
                uintSEW val = (uintSEW)scalar_res[rx[1]][j];
                if(PRINTS >= 2) printf(" ^ [%d]%x", rx[1], val);
                xor_res ^= val;
            }
            if(PRINTS >= 2) printf(";\n\n");
            
            scalar_res[rx[0]][0] = xor_res;
            break;
        case VMAND_MM:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d & [%d]%d;\n", rx[0], j / SEW, rx[1], 
                    scalar_res[rx[1]][j / SEW] & i, rx[2], scalar_res[rx[2]][j / SEW] & i);
                intSEW vs1 = scalar_res[rx[1]][j / SEW];
                intSEW vs2 = scalar_res[rx[2]][j / SEW];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= ((vs1 & i) & (vs2 & i));
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMOR_MM:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d | [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                intSEW vs1 = scalar_res[rx[1]][j / SEW];
                intSEW vs2 = scalar_res[rx[2]][j / SEW];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 & i) | (vs2 & i);
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMNAND_MM:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d NAND [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                intSEW vs1 = scalar_res[rx[1]][j / SEW];
                intSEW vs2 = scalar_res[rx[2]][j / SEW];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (~(vs1 & vs2)) & i;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMXOR_MM:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d ^ [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                intSEW vs1 = scalar_res[rx[1]][j / SEW];
                intSEW vs2 = scalar_res[rx[2]][j / SEW];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= ((vs1 ^ vs2)) & i;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMSEQ_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d == [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                intSEW vs1 = scalar_res[rx[1]][j];
                intSEW vs2 = scalar_res[rx[2]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 == vs2)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMSEQ_VI:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d == imm(%d)) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                intSEW vs1 = scalar_res[rx[1]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 == imm)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMSEQ_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d == %d) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                intSEW vs1 = scalar_res[rx[1]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 == t0_VALUE)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMSNE_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d != [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                intSEW vs1 = scalar_res[rx[1]][j];
                intSEW vs2 = scalar_res[rx[2]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 != vs2)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMSNE_VI:
           for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d != imm(%d)) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                intSEW vs1 = scalar_res[rx[1]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 != imm)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMSNE_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d != %d) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                intSEW vs1 = scalar_res[rx[1]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 != t0_VALUE)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMSLT_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d < [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                intSEW vs1 = scalar_res[rx[1]][j];
                intSEW vs2 = scalar_res[rx[2]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 < vs2)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMSLT_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d < %d) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                intSEW vs1 = scalar_res[rx[1]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 < t0_VALUE)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMSLE_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d <= [%d]%d) ? 1 : 0;\n", rx[0], j,
                    rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                intSEW vs1 = scalar_res[rx[1]][j];
                intSEW vs2 = scalar_res[rx[2]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 <= vs2)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMSLE_VI:
            require_imm_normal();
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d <= %d) ? 1 : 0;\n", rx[0], j,
                    rx[1], scalar_res[rx[1]][j], imm);
                intSEW vs1 = scalar_res[rx[1]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 <= imm)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMSLE_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d < %d) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                intSEW vs1 = scalar_res[rx[1]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 <= t0_VALUE)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMSGT_VI:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d > %d) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                intSEW vs1 = scalar_res[rx[1]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 > imm)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMSGT_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = ([%d]%d > %d) ? 1 : 0;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                intSEW vs1 = scalar_res[rx[1]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 > t0_VALUE)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VCOMPRESS_VM: {
            if(compress_forbid(rx[0], rx[1], rx[2])){
                return NOP;
            }
            // Compressão baseada na máscara vs1 (rx[2]), filtrando elementos de vs2 (rx[1])
            int compress_idx = 0;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if((scalar_res[rx[2]][j / SEW] & i) != 0){ // Se o bit da máscara está ativo
                    scalar_res[rx[0]][compress_idx] = scalar_res[rx[1]][j];
                    compress_idx++;
                }
                i = (i == (1 << SEW * LMUL - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;
        }

        case VCPOP_M: {
            // Conta bits setados na máscara (population count). Resultado é um escalar em vd.
            int popcount = 0;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if((scalar_res[rx[1]][j / SEW] & i) != 0){ 
                    popcount++;
                }
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("SCALAR_RESULT:t1 = POPCOUNT([%d]) = %d;\n\n", rx[0], popcount);
            compare_registers = popcount;
            break;
        }

        case VFIRST_M: {
            // Encontra o primeiro bit setado na máscara. Retorna índice, ou -1 se não encontrar.
            int first_idx = -1;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if((scalar_res[rx[1]][j / SEW] & i) != 0){
                    first_idx = j;
                    break;
                }
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("SCALAR_RESULT:t0 = VFIRST([%d]) = %d;\n\n", rx[1], first_idx);
            compare_registers = first_idx;
            break;
        }
        case VMFEQ_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                floaSEW f1 = bits_to_float(scalar_res[rx[1]][j]);
                floaSEW f2 = bits_to_float(scalar_res[rx[2]][j]);
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = (%d == %d) ? 1 : 0;\n", rx[0], j / SEW, f1, f2);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (f1 == f2) ? i : 0;

                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMFNE_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                floaSEW f1 = bits_to_float(scalar_res[rx[1]][j]);
                floaSEW f2 = bits_to_float(scalar_res[rx[2]][j]);
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = (%f != %f) ? 1 : 0;\n", rx[0], j / SEW, f1, f2);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (f1 != f2) ? i : 0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMFLT_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                floaSEW f1 = bits_to_float(scalar_res[rx[1]][j]);
                floaSEW f2 = bits_to_float(scalar_res[rx[2]][j]);
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = (%f < %f) ? 1 : 0;\n", rx[0], j / SEW, f1, f2);
                scalar_res[rx[0]][j / SEW] &= ~i;
                if (!is_nan(f1) && !is_nan(f2))
                    scalar_res[rx[0]][j / SEW] |= (f1 < f2) ? i : 0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMFLE_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                floaSEW f1 = bits_to_float(scalar_res[rx[1]][j]);
                floaSEW f2 = bits_to_float(scalar_res[rx[2]][j]);
                if(PRINTS >= 2){
                    printf("SCALAR_RESULT:[%d][%d] = ((%d)%f", rx[0], j / SEW, rx[1], f1);
                    printf(" <= (%d)%f)? 1 : 0;\n", rx[2], f2);
                }
                scalar_res[rx[0]][j / SEW] &= ~i;
                if (!is_nan(f1) && !is_nan(f2))
                    scalar_res[rx[0]][j / SEW] |= (f1 <= f2) ? i : 0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

       

        case VMERGE_VVM:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                int mask_active = (scalar_res[0][j / SEW] & i) == 0;

                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = mask(%d) ? [%d]%d : [%d]%d;\n", rx[0], j, mask_active, rx[2], scalar_res[rx[2]][j], rx[1], scalar_res[rx[1]][j]);
                
                scalar_res[1][j] = mask_active ? scalar_res[2][j]: 0/* MASK 3 */;
                
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMERGE_VXM:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                int mask_active = (scalar_res[0][j / SEW] & i) == 0;

                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = mask(%d) ? [%d]%d : %d;\n", rx[0], j, mask_active, rx[2], scalar_res[rx[2]][j], t0_VALUE);
                
                scalar_res[1][j] = !mask_active ? scalar_res[2][j]: t0_VALUE;
                
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;

        case VMERGE_VIM:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                int mask_active = (scalar_res[0][j / SEW] & i) == 0;

                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = mask(%d) ? [%d]%d : %d;\n", rx[0], j, mask_active, rx[2], scalar_res[rx[2]][j], imm);
                
                scalar_res[1][j] = mask_active ? scalar_res[2][j]: imm/* MASK 3 */;
                
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMSLTU_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                uintSEW vs1 = scalar_res[rx[1]][j];
                uintSEW vs2 = scalar_res[rx[2]][j];
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d < [%d]%d = %d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j],
                    rx[2], scalar_res[rx[2]][j], (vs1 < vs2)?1:0);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 < vs2)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMSLTU_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d < [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                uintSEW vs1 = scalar_res[rx[1]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 < (INTXLEN)t0_VALUE)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMSLEU_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d <= [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                uintSEW vs1 = scalar_res[rx[1]][j];
                uintSEW vs2 = scalar_res[rx[2]][j];
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= (vs1 <= vs2)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMSLEU_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d <= [%d]%d;\n", rx[0], j / SEW, rx[1], scalar_res[rx[1]][j / SEW], rx[2], scalar_res[rx[2]][j / SEW]);
                scalar_res[rx[0]][j / SEW] &= ~i;
                scalar_res[rx[0]][j / SEW] |= ((uintSEW)scalar_res[rx[1]][j] <= (uintSEW)t0_VALUE)?i:0;
                i = (i == (1 << SEW - 1))?1:i << 1;
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VDIVU_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d / [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (intSEW)((uintSEW)scalar_res[rx[1]][j] / (uintSEW)scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VDIVU_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d / %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (intSEW)((uintSEW)scalar_res[rx[1]][j] / (uintSEW)t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VREM_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d % [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] % scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VREM_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d % %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] % t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VREMU_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d % [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (intSEW)((uintSEW)scalar_res[rx[1]][j] % (uintSEW)scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VREMU_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d % %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (intSEW)((uintSEW)scalar_res[rx[1]][j] % (uintSEW)t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMULH_VV:
            if(SEW >= 64)
                return NOP;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                intDSEW op1 = (intDSEW)scalar_res[rx[1]][j];
                intDSEW op2 = (intDSEW)scalar_res[rx[2]][j];
                
                scalar_res[rx[0]][j] = (intSEW)((op1 * op2) >> 32);
                
                if(PRINTS >= 2) {
                    printf("VMULH_VV: [%d][%d] = (high)(%d * %d)\n", 
                            rx[0], j, op1, op2);
                }
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMULHU_VV:
            if(SEW >= 64)
                return NOP;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                uintDSEW op1 = (uintDSEW)(uintSEW)scalar_res[rx[1]][j];
                uintDSEW op2 = (uintDSEW)(uintSEW)scalar_res[rx[2]][j];
                
                scalar_res[rx[0]][j] = (uintSEW)((op1 * op2) >> 32);
                
                if(PRINTS >= 2) {
                    printf("VMULHU_VV: [%d][%d] = (high)(%d * %d)\n", 
                            rx[0], j, op1, op2);
                }
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMULHU_VX:
            if(SEW >= 64)
                return NOP;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                uintDSEW op1 = (uintDSEW)(uintSEW)scalar_res[rx[1]][j];
                uintDSEW op2 = (uintDSEW)(uintSEW)t0_VALUE;
                
                scalar_res[rx[0]][j] = (uintSEW)((op1 * op2) >> 32);
                
                if(PRINTS >= 2) {
                    printf("VMULHU_VX: [%d][%d] = (high)(%d * %d)\n", 
                            rx[0], j, op1, op2);
                }
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMULHSU_VV:
            if(SEW >= 64)
                return NOP;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                uintDSEW op1 = (uintDSEW)scalar_res[rx[1]][j];
                uintDSEW op2 = (uintDSEW)(uintSEW)scalar_res[rx[2]][j];
                
                scalar_res[rx[0]][j] = (uintSEW)((op1 * op2) >> 32);
                
                if(PRINTS >= 2) {
                    printf("VMULHU_VV: [%d][%d] = (high)(%d * %d)\n", 
                            rx[0], j, op1, op2);
                }
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMULHSU_VX:
            if(SEW >= 64)
                return NOP;
            for(int j = 0; j < EL_PER_BLOCK; j++){
                uintDSEW op1 = (uintDSEW)scalar_res[rx[1]][j];
                uintDSEW op2 = (uintDSEW)(uintSEW)t0_VALUE;
                
                scalar_res[rx[0]][j] = (uintSEW)((op1 * op2) >> 32);
                
                if(PRINTS >= 2) {
                    printf("VMULHU_VX: [%d][%d] = (high)(%d * %d)\n", 
                            rx[0], j, op1, op2);
                }
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMIN_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) {
                    printf("VMIN_VV: [%d][%d](%d) = (min)(%d, %d)\n",
                            rx[0], j, scalar_res[rx[0]][j], scalar_res[rx[1]][j], scalar_res[rx[2]][j]);
                }

                scalar_res[rx[0]][j] = min(scalar_res[rx[1]][j], scalar_res[rx[2]][j]);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMIN_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                scalar_res[rx[0]][j] = min(scalar_res[rx[1]][j], t0_VALUE);
                
                if(PRINTS >= 2) {
                    printf("VMIN_VX: [%d][%d] = (min)(%d, %d)\n", 
                            rx[0], j, scalar_res[rx[1]][j], t0_VALUE);
                }
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMAX_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                scalar_res[rx[0]][j] = max(scalar_res[rx[1]][j], scalar_res[rx[2]][j]);
                
                if(PRINTS >= 2) {
                    printf("VMAX_VV: [%d][%d] = (max)(%d, %d)\n", 
                            rx[0], j, scalar_res[rx[1]][j], scalar_res[rx[2]][j]);
                }
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMAX_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                scalar_res[rx[0]][j] = max(scalar_res[rx[1]][j], t0_VALUE);
                
                if(PRINTS >= 2) {
                    printf("VMAX_VX: [%d][%d] = (max)(%d, %d)\n", 
                            rx[0], j, scalar_res[rx[1]][j], t0_VALUE);
                }
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMINU_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                scalar_res[rx[0]][j] = minu(scalar_res[rx[1]][j], scalar_res[rx[2]][j]);
                
                if(PRINTS >= 2) {
                    printf("VMINU_VV: [%d][%d] = (min)(%d, %d)\n", 
                            rx[0], j, scalar_res[rx[1]][j], scalar_res[rx[2]][j]);
                }
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMINU_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                scalar_res[rx[0]][j] = minu(scalar_res[rx[1]][j], t0_VALUE);
                
                if(PRINTS >= 2) {
                    printf("VMIN_VX: [%d][%d] = (min)(%d, %d)\n", 
                            rx[0], j, scalar_res[rx[1]][j], t0_VALUE);
                }
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMAXU_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                scalar_res[rx[0]][j] = maxu(scalar_res[rx[1]][j], scalar_res[rx[2]][j]);
                
                if(PRINTS >= 2) {
                    printf("VMAX_VV: [%d][%d] = (maxu)(%d, %d)\n", 
                            rx[0], j, scalar_res[rx[1]][j], scalar_res[rx[2]][j]);
                }
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VMAXU_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                scalar_res[rx[0]][j] = maxu(scalar_res[rx[1]][j], t0_VALUE);
                
                if(PRINTS >= 2) {
                    printf("VMAXU_VX: [%d][%d] = (maxu)(%d, %d)\n", 
                            rx[0], j, scalar_res[rx[1]][j], t0_VALUE);
                }
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VSRA_VV:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d >> [%d]%d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], rx[2], scalar_res[rx[2]][j]);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] >> (scalar_res[rx[2]][j]));
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VSRA_VI:
            require_imm_positive();
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d >> imm(%d);\n", rx[0], j, rx[1], scalar_res[rx[1]][j], imm);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] >> imm);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case VSRA_VX:
            for(int j = 0; j < EL_PER_BLOCK; j++){
                if(PRINTS >= 2) printf("SCALAR_RESULT:[%d][%d] = [%d]%d >> %d;\n", rx[0], j, rx[1], scalar_res[rx[1]][j], t0_VALUE);
                scalar_res[rx[0]][j] = (intSEW)(scalar_res[rx[1]][j] >> t0_VALUE);
            }
            if(PRINTS >= 2) printf("\n");
            break;
        case NOP:
            return ADDI_ZZZ_INSTR;
        default:
            break;
    }
    
    return add_instruction_no_mirror(op, rxa, r);
}

int add_instruction_no_mirror(int op, int rxa[3], int r[3]){
    int instr = 0;
    int instr_type = VV;
    int rx[3] = {r[rxa[0]], r[rxa[1]], r[rxa[2]]};

    switch (op){
        case VADD_VV:
            instr = VADD_VV_INSTR;
            break;
        case VSUB_VV:
            instr = VSUB_VV_INSTR;
            break;
        case VDIV_VV:
            instr = VDIV_VV_INSTR;
            break;
        case VMUL_VV:
            instr = VMUL_VV_INSTR;
            break;
        case VSLL_VV:
            instr = VSLL_VV_INSTR;
            break;
        case VSRL_VV:
            instr = VSRL_VV_INSTR;
            break;
        case VAND_VV:
            instr = VAND_VV_INSTR;
            break;
        case VOR_VV:
            instr = VOR_VV_INSTR;
            break;
        case VXOR_VV:
            instr = VXOR_VV_INSTR;
            break;
        case VADD_VI:
            //require_imm_normal(imm);
            instr = VADD_VI_INSTR;
            instr_type = VI;
            break;
        case VSLL_VI:
            //require_imm_positive();
            instr = VSLL_VI_INSTR;
            instr_type = VI;
            break;
        case VSRL_VI:
            //require_imm_positive();
            instr = VSRL_VI_INSTR;
            instr_type = VI;
            break;
        case VAND_VI:
            int ima = imm>=16?-imm:imm;
            instr = VAND_VI_INSTR;
            instr_type = VI;
            break;
        case VOR_VI:
            instr = VOR_VI_INSTR;
            instr_type = VI;
            break;
        case VXOR_VI:
            //require_imm_normal();
            instr = VXOR_VI_INSTR;
            instr_type = VI;
            break;
        case VADD_VX:
            instr = VADD_VX_INSTR;
            instr_type = VX;
            break;
        case VSUB_VX:
            instr = VSUB_VX_INSTR;
            instr_type = VX;
            break;
        case VDIV_VX:
            instr = VDIV_VX_INSTR;
            instr_type = VX;
            break;
        case VMUL_VX:
            instr = VMUL_VX_INSTR;
            instr_type = VX;
            break;
        case VSLL_VX:
            instr = VSLL_VX_INSTR;
            instr_type = VX;
            break;
        case VSRL_VX:
            instr = VSRL_VX_INSTR;
            instr_type = VX;
            break;
        case VAND_VX:
            instr = VAND_VX_INSTR;
            instr_type = VX;
            break;
        case VOR_VX:
            instr = VOR_VX_INSTR;
            instr_type = VX;
            break;
        case VXOR_VX:
            instr = VXOR_VX_INSTR;
            instr_type = VX;
            break;
        case VFADD_VV:
            instr = VFADD_VV_INSTR;
            break;
        case VFADD_VF:
            instr = VFADD_VF_INSTR;
            instr_type = VF;
            break;
        case VFSUB_VV:
            instr = VFSUB_VV_INSTR;
            break;
        case VFSUB_VF:
            instr = VFSUB_VF_INSTR;
            instr_type = VF;
            break;
        case VFMUL_VV:
            instr = VFMUL_VV_INSTR;
            break;
        case VFMUL_VF:
            instr = VFMUL_VF_INSTR;
            instr_type = VF;
            break;
        case VFDIV_VV:
            instr = VFDIV_VV_INSTR;
            break;
        case VFDIV_VF:
            instr = VFDIV_VF_INSTR;
            instr_type = VF;
            break;
        case VFSQRT_V:
            instr = VFSQRT_V_INSTR;
            instr_type = V;
            break;
        case VFMACC_VV: // TODO: fix very big numbers issue
            instr = VFMACC_VV_INSTR;
            break;
        case VMACC_VV: // TODO: fix very big numbers issue
            instr = VMACC_VV_INSTR;
            break;
        case VMACC_VX: // TODO: fix very big numbers issue
            instr = VMACC_VX_INSTR;
            instr_type = VX;
            break;
        case VSLIDEUP_VI:
            //require_imm_positive();
            if(slideup_forbid(rx[0], rx[1], LMUL)) { printf("FORBIDDEN\n"); return NOP; }
            instr = VSLIDEUP_VI_INSTR;
            instr_type = VI;
            break;
        case VSLIDEUP_VX:
            if(slideup_forbid(rx[0], rx[1], LMUL)) {printf("FORBIDDEN\n"); return NOP;}
            instr = VSLIDEUP_VX_INSTR;
            instr_type = VX;
            break;
        case VSLIDEDOWN_VI:
            //require_imm_positive();
            instr = VSLIDEDOWN_VI_INSTR;
            instr_type = VI;
            break;
        case VSLIDEDOWN_VX:
            if(rx[0] == rx[1]) return NOP;
            instr = VSLIDEDOWN_VX_INSTR;
            instr_type = VX;
            break;
        case VMV_V_V:
            instr = VMV_V_V_INSTR;
            instr = change_vet_rd(instr, rx[0]);
            instr = change_vet_rs1(instr, 0);
            instr = change_vet_rs2(instr, rx[1]);
            instr_type = NO_TYPE;
            break;
        case VMV_V_I:
            instr = VMV_V_I_INSTR;

            instr = change_vet_rd(instr, rx[0]);
            instr = change_vet_rs1(instr, 0);
            instr = change_vet_rs2(instr, imm);

            instr_type = NO_TYPE;
            break;
        case VMV_V_X:
            instr = VMV_V_X_INSTR;
            instr = change_vet_rd(instr, rx[0]);
            instr = change_vet_rs1(instr, 0);
            instr = change_vet_rs2(instr, 5);
            instr_type = NO_TYPE;
            break;
        case VWADD_VV:
            if(widening_forbid(rx)) return NOP;
            instr = VWADD_VV_INSTR;
            break;
        case VWADD_VX:
            if(widening_forbid(rx)) return NOP;
            instr = VWADD_VX_INSTR;
            instr_type = VX;
            break;
        case VWSUB_VV:
            if(widening_forbid(rx)) return NOP;
            instr = VWSUB_VV_INSTR;
            break;
        case VWSUB_VX:
            if(widening_forbid(rx)) return NOP;
            instr = VWSUB_VX_INSTR;
            instr_type = VX;
            break;
        case VWADDU_VV:
            if(widening_forbid(rx)) return NOP;
            instr = VWADDU_VV_INSTR;
            break;
        case VWADDU_VX:
            if(widening_forbid(rx)) return NOP;
            instr = VWADDU_VX_INSTR;
            instr_type = VX;
            break;
        case VWMUL_VV:
            if(widening_forbid(rx)) return NOP;
            instr = VWMUL_VV_INSTR;
            break;
        case VWMUL_VX:
            if(widening_forbid(rx)) return NOP;
            instr = VWMUL_VX_INSTR;
            instr_type = VX;
            break;
        case VWMACC_VV:
            if(widening_forbid(rx)) {printf("FORBIDDEN\n"); return NOP;}
    
            instr = VWMACC_VV_INSTR;
            break;
        case VREDSUM_VS:
            instr = VREDSUM_VS_INSTR;
            instr_type = VV; 
            break;
        case VREDMAXU_VS:
            instr = VREDMAXU_VS_INSTR;
            instr_type = VV; 
            break;

        case VREDMAX_VS:
            instr = VREDMAX_VS_INSTR;
            instr_type = VV; 
            break;

        case VREDMINU_VS:
            instr = VREDMINU_VS_INSTR;
            instr_type = VV; 
            break;

        case VREDMIN_VS:
            instr = VREDMIN_VS_INSTR;
            instr_type = VV; 
            break;

        case VREDAND_VS:
            instr = VREDAND_VS_INSTR;
            instr_type = VV; 
            break;

        case VREDOR_VS:
            instr = VREDOR_VS_INSTR;
            instr_type = VV; 
            break;

        case VREDXOR_VS:
            instr = VREDXOR_VS_INSTR;
            instr_type = VV;
            break;
        case VMAND_MM:
            instr = VMAND_MM_INSTR;
            break;

        case VMOR_MM:
            instr = VMOR_MM_INSTR;
            break;

        case VMNAND_MM:
            instr = VMNAND_MM_INSTR;
            break;

        case VMXOR_MM:
            instr = VMXOR_MM_INSTR;
            break;

        case VMSEQ_VV:
            instr = VMSEQ_VV_INSTR;
            break;

        case VMSEQ_VI:
            instr = VMSEQ_VI_INSTR;
            instr_type = VI;
            break;

        case VMSEQ_VX:
            instr = VMSEQ_VX_INSTR;
            instr_type = VX;
            break;

        case VMSNE_VV:
            instr = VMSNE_VV_INSTR;
            break;

        case VMSNE_VI:
            instr = VMSNE_VI_INSTR;
            instr_type = VI;
            break;

        case VMSNE_VX:
            instr = VMSNE_VX_INSTR;
            instr_type = VX;
            break;

        case VMSLT_VV:
            instr = VMSLT_VV_INSTR;
            break;
        case VMSLT_VX:
            instr = VMSLT_VX_INSTR;
            instr_type = VX;
            break;

        case VMSLE_VV:
            instr = VMSLE_VV_INSTR;
            break;

        case VMSLE_VI:
            //require_imm_normal();
            instr = VMSLE_VI_INSTR;
            instr_type = VI;
            break;

        case VMSLE_VX:
            instr = VMSLE_VX_INSTR;
            instr_type = VX;
            break;
        case VMSGT_VI:
            instr = VMSGT_VI_INSTR;
            instr_type = VI;
            break;

        case VMSGT_VX:
            instr = VMSGT_VX_INSTR;
            instr_type = VX;
            break;

        case VCOMPRESS_VM: {
            if(compress_forbid(rx[0], rx[1], rx[2])) return NOP;
            instr = VCOMPRESS_VM_INSTR;
            break;
        }

        case VCPOP_M: {
            instr = VCPOP_M_INSTR;
            instr = change_vet_rd(instr, 6);
            instr = change_vet_rs1(instr, rx[1]);    
            instr_type = NO_TYPE;
            break;
        }

        case VFIRST_M: {
            instr = VFIRST_M_INSTR;
            instr = change_vet_rd(instr, 6);
            instr = change_vet_rs1(instr, rx[1]);
            instr_type = NO_TYPE;
            break;
        }
        case VMFEQ_VV:
            instr = VMFEQ_VV_INSTR;
            break;

        case VMFNE_VV:
            instr = VMFNE_VV_INSTR;
            break;

        case VMFLT_VV:
            instr = VMFLT_VV_INSTR;
            break;

        case VMFLE_VV:
            instr = VMFLE_VV_INSTR;
            break;

        case VMERGE_VVM:
            instr = VMERGE_VVM_INSTR;
            instr = change_vet_rd(instr,  rx[1]);
            instr = change_vet_rs1(instr, rx[2]);
            instr = change_vet_rs2(instr, rx[1]);
            instr_type = NO_TYPE;
            break;

        case VMERGE_VXM:
            instr = VMERGE_VXM_INSTR;
            instr = change_vet_rd(instr,  rx[1]);
            instr = change_vet_rs1(instr, rx[2]);
            instr = change_vet_rs2(instr, 5);
            instr_type = NO_TYPE;
            break;

        case VMERGE_VIM:
            instr = VMERGE_VIM_INSTR;
            instr = change_vet_rd(instr,  rx[1]);
            instr = change_vet_rs1(instr, rx[2]);
            instr = change_vet_rs2(instr, imm);
            instr_type = NO_TYPE;
            break;
        case VMSLTU_VV:
            instr = VMSLTU_VV_INSTR;
            break;
        case VMSLTU_VX:
            instr = VMSLTU_VX_INSTR;
            instr_type = VX;
            break;
        case VMSLEU_VV:
            instr = VMSLEU_VV_INSTR;
            break;
        case VMSLEU_VX:
            instr = VMSLEU_VX_INSTR;
            instr_type = VX;
            break;
        case VDIVU_VV:
            instr = VDIVU_VV_INSTR;
            break;
        case VDIVU_VX:
            instr = VDIVU_VX_INSTR;
            instr_type = VX;
            break;
        case VREM_VV:
            instr = VREM_VV_INSTR;
            break;
        case VREM_VX:
            instr = VREM_VX_INSTR;
            instr_type = VX;
            break;
        case VREMU_VV:
            instr = VREMU_VV_INSTR;
            break;
        case VREMU_VX:
            instr = VREMU_VX_INSTR;
            instr_type = VX;
            break;
        case VMULH_VV:
            if(SEW >= 64) return NOP;
            instr = VMULH_VV_INSTR;
            break;
        case VMULHU_VV:
            if(SEW >= 64) return NOP;
            instr = VMULHU_VV_INSTR;
            break;
            case VMULHU_VX:
            if(SEW >= 64) return NOP;
            instr = VMULHU_VX_INSTR;
            instr_type = VX;
            break;
        case VMULHSU_VV:
            if(SEW >= 64) return NOP;
            instr = VMULHSU_VV_INSTR;
            break;
        case VMULHSU_VX:
            if(SEW >= 64) return NOP;
            instr = VMULHSU_VX_INSTR;
            instr_type = VX;
            break;
        case VMIN_VV:
            instr = VMIN_VV_INSTR;
            break;
        case VMIN_VX:
            instr = VMIN_VX_INSTR;
            instr_type = VX;
            break;
        case VMAX_VV:
            instr = VMAX_VV_INSTR;
            break;
        case VMAX_VX:
            instr = VMAX_VX_INSTR;
            instr_type = VX;
            break;
        case VMINU_VV:
            instr = VMINU_VV_INSTR;
            break;
        case VMINU_VX:
            instr = VMINU_VX_INSTR;
            instr_type = VX;
            break;
        case VMAXU_VV:
            instr = VMAXU_VV_INSTR;
            break;
        case VMAXU_VX:
            instr = VMAXU_VX_INSTR;
            instr_type = VX;
            break;
        case VSRA_VV:
            instr = VSRA_VV_INSTR;
            break;
        case VSRA_VI:
            //require_imm_positive();
            instr = VSRA_VI_INSTR;
            instr_type = VI;
            break;
        case VSRA_VX:
            instr = VSRA_VX_INSTR;
            instr_type = VX;
            break;
        case NOP:
            return ADDI_ZZZ_INSTR;
        default:
            break;
    }

    if(instr_type == VV){
        instr = change_vet_rd(instr,  rx[0]);
        instr = change_vet_rs1(instr, rx[1]);
        instr = change_vet_rs2(instr, rx[2]);
    }
    if(instr_type == VX){
        instr = change_vet_rd(instr,  rx[0]);
        instr = change_vet_rs1(instr, rx[1]);
        instr = change_vet_rs2(instr, 5);
    }
    if(instr_type == VI){
        instr = change_vet_rd(instr,  rx[0]);
        instr = change_vet_rs1(instr, rx[1]);
        instr = change_vet_rs2(instr, imm);
    }
    if(instr_type == VF){
        instr = change_vet_rd(instr,  rx[0]);
        instr = change_vet_rs1(instr, rx[1]);
        
        instr = change_vet_rs2(instr, 0); 
        load_value_ft0(f_vf);
    }
    if(instr_type == V){
        instr = change_vet_rd(instr,  rx[0]);
        instr = change_vet_rs1(instr, rx[1]);
    }
}

int compare_solutions(int prev_error, int32_t r[3], intSEW* vet_res){
    int cmr = compare_registers;
    compare_registers = -1;
    if(PRINTS >= 3) printf("r: %d %d %d\n", r[0], r[1], r[2]);

    if(cmr != -1){
        if(actual_t1 == cmr){
            if(PRINTS >= 1)printf("Convergence on t1 comparison \n");
        }else{
            if(PRINTS >= 1)printf("Divergence on t1: (actual)%d != %d\n", actual_t1, cmr);
            return 1;
        }
    }
    for(int i = 0; i < 3; i++){
        if(!manual_convergence(&scalar_res[r[i]][0], &vet_res[r[i] * VLEN / SEW], 1, VLEN / SEW * LMUL)){
            if(PRINTS >= 1)printf("Divergence on register %d(%d) \n", r[i], i);
            return 1;
        }
    }

        if(PRINTS >= 1)printf("Convergence \n");
        return 2;
}

void execute_RIS(intSEW* vet_init, INTXLEN* r, INT_INST address_vector[], intSEW* vet_res, int num_registers){
    set_vet_settings();
    load_init_values_vector(vet_init, r, num_registers);
    set_vet_settings();
    
    load_OUT_t0_vet((int32_t*)t0_VALUE);// Gambiarra simples para ter t0 com t0_VALUE
    load_value_ft0(f_vf);
    
    jump_to_vet(&address_vector[0]);
    actual_t1 = return_t1();
    if(PRINTS >= 2 && compare_registers != -1) 
        printf("Compare_registers: %d, actual_t1: %d \n", compare_registers, actual_t1);
    store_vet_values(r, vet_res, num_registers);
}

void execute_RIS_capture_benchmarks(intSEW* vet, INTXLEN* r, INT_INST address_vector[], intSEW* vet_res, int num_registers,
int* delta_cyc, int* delta_inst){
    set_vet_settings();
    load_init_values_vector(vet, r, num_registers);
    set_vet_settings();

    load_OUT_t0_vet((int32_t*)t0_VALUE);// Gambiarra simples para ter t0 com t0_VALUE
    load_value_ft0(f_vf);
    
    uint start_cyc = read_cycles();
    uint start_inst = read_instret();
    jump_to_vet(&address_vector[0]);
    *delta_cyc = read_cycles() - start_cyc;
    *delta_inst = read_instret() - start_inst;

    actual_t1 = return_t1();
    if(PRINTS >= 2 && compare_registers != -1) 
        printf("Compare_registers: %d, actual_t1: %d \n", compare_registers, actual_t1);
    store_vet_values(r, vet_res, num_registers);
}

void help_errors(){
    printf("Error reference:\n");
    printf("'XX'  : correct answer\n\n");
    printf("'WA' : wrong answer (no hardware errors)\n");
    printf("Hardware errors\n");
    printf("'IM': Instruction Address Misaligned\n");
    printf("'IF': Instruction Access Fault\n");
    printf("'II' : Illegal Instruction / Unimplemented Instruction\n");
    printf("'BP' : Breakpoint (ebreak)\n");
    printf("'LM': Load Address Misaligned\n");
    printf("'LF': Load Access Fault\n");
    printf("'SM': Store/AMO Address Misaligned\n");
    printf("'SF': Store/AMO Access Fault\n");
    printf("'EU': Environment Call from U-mode\n");
    printf("'ES': Environment Call from S-mode\n");
    printf("'EM': Environment Call from M-mode\n");
    printf("'IP': Instruction Page Fault\n");
    printf("'LP': Load Page Fault\n");
    printf("'SP': Store/AMO Page Fault\n");
    printf("'HE' : Hardware Error (implementation specific)\n");
    printf("'UK': Unknown / Reserved\n");
}

char* get_err(int err){
    switch(err){
        case   2: return "XX"; // correct answer
        case   1: return "WA"; // wrong answer
        case   0: return "IM";  // Instruction Address Misaligned
        case  -1: return "IF";  // Instruction Access Fault
        case  -2: return "II";   // Illegal Instruction
        case  -3: return "BP";   // Breakpoint (ebreak)
        case  -4: return "LM";  // Load Address Misaligned
        case  -5: return "LF";  // Load Access Fault
        case  -6: return "SM";  // Store/AMO Address Misaligned
        case  -7: return "SF";  // Store/AMO Access Fault
        case  -8: return "EU";  // Environment Call from U-mode
        case  -9: return "ES";  // Environment Call from S-mode
        case -11: return "EM";  // Environment Call from M-mode
        case -12: return "IP";  // Instruction Page Fault
        case -13: return "LP";  // Load Page Fault
        case -15: return "SP";  // Store/AMO Page Fault
        case -16: return "HE";   // Hardware Error (implementation specific)
        default: return "UK";  // Unknown / Reserved
    }
}