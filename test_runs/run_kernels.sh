[[ "${PWD##*/}" == "test_runs" ]] && cd ..



# Test data hazards
spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb \
    ./benchmarks/test_mask_agnostic.riscv

spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb \
    ./benchmarks/test_tail_agnostic.riscv

spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb \
    ./benchmarks/test_vector_compare.riscv

spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb \
    ./benchmarks/test_vlen.riscv

spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb \
    ./benchmarks/test_lmul.riscv

spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb \
    ./benchmarks/test_data_hazards.riscv

spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb \
    ./benchmarks/test_interruption.riscv
