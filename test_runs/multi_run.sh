#!/bin/bash

BENCH=$1

cd ..
make -j$(nproc) benchmarks
python3 ./mem_setter.py ./benchmarks/$BENCH/params.json

riscv64-unknown-elf-objcopy --update-section \
.PARAMETERS_SECTION=./benchmarks/$BENCH/params.mem ./benchmarks/$BENCH.riscv

# Função auxiliar para iterar dinamicamente sobre todos os arquivos .in da pasta
run_distribution() {
    local folder=$1
    echo "$folder"
    
    local input_dir="./test_runs/$folder/inputs"
    local output_dir="./benchmarks/$BENCH/test_runs/$folder/outputs"

    if [ ! -d "$input_dir" ]; then
        echo "Aviso: Diretório $input_dir não encontrado. Pulando..."
        return
    fi

    # Busca todos os arquivos .in dentro de inputs/ de forma dinâmica
    for input_file in "$input_dir"/*.in; do
        # Verifica se o arquivo realmente existe (evita falha se a pasta estiver vazia)
        [ -e "$input_file" ] || continue

        local filename=$(basename "$input_file")       # Ex: 1.int32.in
        local output_file="$output_dir/${filename%.in}.out" # Ex: 1.int32.out

        riscv64-unknown-elf-objcopy --update-section \
        .OUT_SECTION="$input_file" ./benchmarks/$BENCH.riscv

        spike --isa=rv64gcv_zvl128b_zicntr_zba_zbb ./benchmarks/$BENCH.riscv > "$output_file"
    done
}

# Executa para todas as pastas de teste
run_distribution "uniform"
run_distribution "normal"
run_distribution "log_normal"
run_distribution "all_ones"
run_distribution "all_zeros"