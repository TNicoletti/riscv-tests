#!/bin/bash

if [ "$#" -lt 3 ] || [ "$#" -gt 4 ]; then
    echo "Uso: $0 <BENCH> <NUMBER_EXECUTION> <INT_FLOAT> [LOG]"
    echo "  LOG: 'log' gera run.log (padrao: sem log)"
    exit 1
fi

BENCH=$1
NUMBER_EXECUTION=$2
INT_FLOAT=$3
LOG=${4:-nolog}

[[ "${PWD##*/}" == "test_runs" ]] && cd ..

make -j"$(nproc)" benchmarks

cp "./benchmarks/$BENCH.riscv" ./benchmarks/single_execution.riscv

python3 ./mem_setter.py "./benchmarks/$BENCH/params.json"

riscv64-unknown-elf-objcopy --update-section \
    ".PARAMETERS_SECTION=./benchmarks/$BENCH/params.mem" \
    ./benchmarks/single_execution.riscv

input_file="./test_runs/uniform/inputs32/${NUMBER_EXECUTION}.${INT_FLOAT}32.in"

riscv64-unknown-elf-objcopy --update-section \
    ".OUT_SECTION=$input_file" \
    ./benchmarks/single_execution.riscv

if [ "$LOG" == "log" ]; then
    LOG_FLAGS=(-l --log-commits --log=run.log)
else
    LOG_FLAGS=()
fi

start=$(date +%s.%N)
spike "${LOG_FLAGS[@]}" --isa=rv64gcv_zvl128b_zicntr_zba_zbb \
    ./benchmarks/single_execution.riscv
end=$(date +%s.%N)

LC_NUMERIC=C printf "Tempo de execucao: %.3f s\n" "$(echo "$end - $start" | bc)"

rm ./benchmarks/single_execution.riscv