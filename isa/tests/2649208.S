#include "riscv_test.h"
#include "test_macros.h"
RVTEST_RV64UV
RVTEST_CODE_BEGIN
    # --- Inicialização ---
    # Configura VLEN e SEW (e32, m1)

    li TESTNUM, 1

    vsetvli t0, x0, e32, m1, ta, ma
    # Inicializa escalares para testes .vx
    li t1, 90
    li t2, 35
    li t3, 32
    li t4, 35
    li a0, 35
    li a1, 93

    # --- Início do Teste Aleatório ---
    vor.vx v5, v22, t3            
    vsub.vx v16, v20, t1          
    vxor.vi v29, v16, 3            # Hazard: RAW (vs2)
    vsll.vi v18, v29, 8            # Hazard: RAW (vs2)
    vadd.vx v14, v20, t1          
    vmul.vx v14, v20, t2          
    vsll.vv v6, v14, v2            # Hazard: RAW
    vadd.vi v13, v6, 1             # Hazard: RAW (vs2)
    vrsub.vx v5, v13, t2           # Hazard: RAW (vs2)
    vadd.vi v4, v5, 11             # Hazard: RAW (vs2)
    vmul.vv v21, v8, v21          
    vadd.vi v12, v21, 11           # Hazard: RAW (vs2)
    vxor.vi v16, v12, 12           # Hazard: RAW (vs2)
    vor.vv v6, v20, v31           
    vrsub.vx v12, v6, t1           # Hazard: RAW (vs2)
    vxor.vi v12, v12, 12           # Hazard: RAW (vs2)
    vsll.vv v16, v15, v8          
    vxor.vx v17, v16, t4           # Hazard: RAW (vs2)
    vxor.vi v24, v1, 6            
    vand.vx v12, v24, t1           # Hazard: RAW (vs2)
    vsub.vv v10, v4, v17          
    vadd.vx v18, v26, t1          
    vxor.vx v16, v18, t3           # Hazard: RAW (vs2)
    vadd.vv v2, v16, v29           # Hazard: RAW
    vsll.vv v29, v25, v18         
    vsub.vx v28, v29, a1           # Hazard: RAW (vs2)
    vsub.vv v27, v28, v29          # Hazard: RAW
    vor.vx v20, v14, t4           
    vmul.vx v5, v20, a0            # Hazard: RAW (vs2)
    vand.vi v23, v5, 13            # Hazard: RAW (vs2)
    vxor.vx v14, v23, t1           # Hazard: RAW (vs2)
    vxor.vv v12, v23, v12         
    vsll.vx v27, v12, t2           # Hazard: RAW (vs2)
    vxor.vv v19, v27, v1           # Hazard: RAW
    vand.vx v3, v19, t4            # Hazard: RAW (vs2)
    vxor.vv v24, v5, v24          
    vsll.vv v19, v24, v1           # Hazard: RAW
    vor.vv v23, v19, v18           # Hazard: RAW
    vor.vx v7, v26, a0            
    vor.vv v1, v7, v4              # Hazard: RAW
    vadd.vv v9, v2, v28           
    vsll.vv v13, v9, v8            # Hazard: RAW
    vsll.vv v5, v7, v2            
    vxor.vi v27, v2, 4            
    vand.vv v13, v27, v25          # Hazard: RAW
    vor.vv v13, v25, v1           
    vand.vi v8, v13, 8             # Hazard: RAW (vs2)
    vsll.vv v1, v8, v28            # Hazard: RAW
    vadd.vv v28, v30, v4          
    vand.vx v9, v11, t1           
    vsub.vv v7, v19, v1           
    vand.vx v7, v30, t1           
    vmul.vv v24, v1, v24          
    vsll.vx v6, v24, t4            # Hazard: RAW (vs2)
    vand.vx v7, v6, t2             # Hazard: RAW (vs2)
    vand.vx v2, v30, t3           
    vmul.vx v10, v25, t2          
    vmul.vx v22, v16, t4          
    vsll.vv v23, v22, v16          # Hazard: RAW
    vsll.vv v18, v23, v13          # Hazard: RAW
    vrsub.vi v6, v18, 13           # Hazard: RAW (vs2)
    vsub.vx v9, v6, a0             # Hazard: RAW (vs2)
    vsll.vx v11, v25, t1          
    vrsub.vx v31, v11, a0         
    vadd.vi v28, v31, 15           # Hazard: RAW (vs2)
    vand.vv v30, v28, v18          # Hazard: RAW
    vor.vv v23, v7, v27           
    vxor.vx v31, v23, a1           # Hazard: RAW (vs2)
    vand.vx v5, v22, t3           
    vadd.vi v27, v5, 10            # Hazard: RAW (vs2)
    vand.vv v12, v11, v15         
    vor.vx v26, v12, t4            # Hazard: RAW (vs2)
    vor.vv v29, v26, v9            # Hazard: RAW
    vmul.vv v6, v16, v16          
    vsll.vi v29, v6, 8             # Hazard: RAW (vs2)
    vxor.vx v24, v29, t3           # Hazard: RAW (vs2)
    vadd.vx v23, v25, t4          
    vmul.vx v28, v23, a0           # Hazard: RAW (vs2)
    vsll.vv v16, v1, v13          
    vxor.vv v2, v3, v23           
    vmul.vv v31, v29, v8          
    vxor.vi v8, v10, 3            
    vadd.vi v30, v8, 7             # Hazard: RAW (vs2)
    vadd.vi v10, v29, 0           
    vadd.vx v1, v25, a0           
    vrsub.vx v30, v2, t1          
    vor.vx v20, v29, t3           
    vor.vi v29, v20, 9             # Hazard: RAW (vs2)
    vor.vv v14, v30, v14          
    vsll.vi v9, v13, 6            
    vsll.vi v20, v26, 6           
    vmul.vv v27, v20, v2           # Hazard: RAW
    vxor.vx v8, v27, t3            # Hazard: RAW (vs2)
    vxor.vv v27, v8, v31           # Hazard: RAW
    vadd.vv v31, v27, v5           # Hazard: RAW
    vrsub.vx v20, v31, a0          # Hazard: RAW (vs2)
    vand.vx v23, v20, t3           # Hazard: RAW (vs2)
    vsub.vx v29, v14, t1          
    vand.vi v17, v29, 7            # Hazard: RAW (vs2)
    vand.vi v30, v17, 13           # Hazard: RAW (vs2)

    # --- Fim ---

TEST_PASSFAIL
RVTEST_CODE_END
.data
RVTEST_DATA_BEGIN
TEST_DATA
RVTEST_DATA_END