python3 mem_setter2.py ./benchmarks/$1/params.json
riscv64-unknown-elf-objcopy --update-section .PARAMETERS_SECTION=./benchmarks/$1/params.mem ./benchmarks/$1.riscv
spike --isa=rv64gcv_zvl128b ./benchmarks/$1.riscv
