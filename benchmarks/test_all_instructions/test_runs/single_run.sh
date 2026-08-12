#!/bin/bash

BENCH=test_all_instructions

cd ../../../
pwd
make -j$(nproc) benchmarks
python3 ./mem_setter.py ./benchmarks/$BENCH/params.json

riscv64-unknown-elf-objcopy --update-section \
.PARAMETERS_SECTION=./benchmarks/$BENCH/params.mem ./benchmarks/$BENCH.riscv

#riscv64-unknown-elf-objcopy --update-section \
#.OUT_SECTION=./benchmarks/$BENCH/test_runs/uniform/inputs/49.int32.in ./benchmarks/$BENCH.riscv


riscv64-unknown-elf-objcopy --update-section \
.OUT_SECTION=./benchmarks/$BENCH/test_runs/test.int32.in ./benchmarks/$BENCH.riscv


spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb ./benchmarks/$BENCH.riscv

