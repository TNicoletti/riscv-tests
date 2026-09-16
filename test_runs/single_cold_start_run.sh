#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "Uso: $0 <BENCH> <NUMBER_EXECUTION> <INT_FLOAT>"
    exit 1
fi

BENCH=$1
NUMBER_EXECUTION=$2
INT_FLOAT=$3

[[ "${PWD##*/}" == "test_runs" ]] && cd ..

make -j"$(nproc)" benchmarks

cp "./benchmarks/$BENCH.riscv" ./benchmarks/single_execution.riscv

python3 ./mem_setter.py "./benchmarks/$BENCH/params.json"

riscv64-unknown-elf-objcopy --update-section \
    ".PARAMETERS_SECTION=./benchmarks/$BENCH/params.mem" \
    ./benchmarks/single_execution.riscv

input_file="./test_runs/uniform/inputs/${NUMBER_EXECUTION}.${INT_FLOAT}32.in"

riscv64-unknown-elf-objcopy --update-section \
    ".OUT_SECTION=$input_file" \
    ./benchmarks/single_execution.riscv

spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb \
    --cold-start=5,0x02840457 ./benchmarks/single_execution.riscv

rm ./benchmarks/single_execution.riscv