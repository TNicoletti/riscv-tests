[[ "${PWD##*/}" == "test_runs" ]] && cd ..



# Test data hazards
spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb \
    --RAW-register 0x01 ./benchmarks/test_data_hazards.riscv

spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb \
    --faulty-instruction 0x02113057 ./benchmarks/test_lmul.riscv

spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb \
    --faulty-instruction 0x02113057 ./benchmarks/test_lmul.riscv

spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb \
    --faulty-instruction 0x02113057 ./benchmarks/test_lmul.riscv