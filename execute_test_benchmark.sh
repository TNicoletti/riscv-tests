#!/bin/bash

BENCH=$1
DEBUG_FLAG=""
if [ "$2" == "-d" ]; then
    DEBUG_FLAG="-d"
fi

python3 mem_setter2.py ./benchmarks/$BENCH/params.json
riscv64-unknown-elf-objcopy --update-section .PARAMETERS_SECTION=./benchmarks/$BENCH/params.mem ./benchmarks/$BENCH.riscv
spike $DEBUG_FLAG --isa=rv64gcv_zvl128b_zicntr_zba_zbb ./benchmarks/$BENCH.riscv
