#!/bin/bash

BENCH=$1

[[ "${PWD##*/}" == "test_runs" ]] && cd ..
make -j$(nproc) benchmarks
python3 ./mem_setter.py ./benchmarks/$BENCH/params.json

riscv64-unknown-elf-objcopy --update-section \
.PARAMETERS_SECTION=./benchmarks/$BENCH/params.mem ./benchmarks/$BENCH.riscv

input_file=./test_runs/uniform/inputs/$2.int32.in
riscv64-unknown-elf-objcopy --update-section \
    .OUT_SECTION="$input_file" ./benchmarks/$BENCH.riscv


spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb ./benchmarks/$BENCH.riscv