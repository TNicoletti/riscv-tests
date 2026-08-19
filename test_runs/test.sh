#!/bin/bash

BENCH=random_neural_network

[[ "${PWD##*/}" == "test_runs" ]] && cd ..
for i in {1..31};
do
    # Format the decimal integer as a 2-digit uppercase hex value (e.g., 00, 01 ... 1F)
    reg=$(printf "%02X" "$i")
    
    echo "$reg"
    spike --RAW-register "$reg" --isa=rv64gcv_zvl128b_zicntr_zba_zbb ./benchmarks/$BENCH.riscv | grep "Failed on test" || true
done

