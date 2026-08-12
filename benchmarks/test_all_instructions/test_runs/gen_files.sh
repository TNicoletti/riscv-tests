#!/bin/bash

REGENERATE=false

# Parse command line flags
for arg in "$@"; do
    case $arg in
        -r|--regenerate)
            REGENERATE=true
            shift
            ;;
    esac
done

folder=$PWD
n_elements=10000

# Helper function to generate file only if needed
generate_data() {
    local outfile="$1"
    shift
    
    # Ensure target directory exists
    mkdir -p "$(dirname "$outfile")"

    if [ "$REGENERATE" = false ] && [ -f "$outfile" ]; then
        return
    fi

    python3 rng.py "$@" -o "$outfile"
}

# uniform
for seed in {1..100}
do
    generate_data "$folder/uniform/inputs/$seed.float32.in" -n $n_elements -s $seed -d uniform -t float32 --low -16 --high 16
    generate_data "$folder/uniform/inputs/$seed.int32.in"   -n $n_elements -s $seed -d uniform -t int32   --low -16 --high 16
done
for seed in {101..200}
do
    generate_data "$folder/uniform/inputs/$seed.float32.in" -n $n_elements -s $seed -d uniform -t float32 --low -32 --high 32
    generate_data "$folder/uniform/inputs/$seed.int32.in"   -n $n_elements -s $seed -d uniform -t int32   --low -32 --high 32
done
for seed in {201..300}
do
    generate_data "$folder/uniform/inputs/$seed.float32.in" -n $n_elements -s $seed -d uniform -t float32 --low -400 --high 400
    generate_data "$folder/uniform/inputs/$seed.int32.in"   -n $n_elements -s $seed -d uniform -t int32   --low -400 --high 400
done
for seed in {301..400}
do
    generate_data "$folder/uniform/inputs/$seed.float32.in" -n $n_elements -s $seed -d uniform -t float32 --low -5000 --high 5000
    generate_data "$folder/uniform/inputs/$seed.int32.in"   -n $n_elements -s $seed -d uniform -t int32   --low -5000 --high 5000
done
for seed in {401..500}
do
    generate_data "$folder/uniform/inputs/$seed.float32.in" -n $n_elements -s $seed -d uniform -t float32 --low -200000 --high 200000
    generate_data "$folder/uniform/inputs/$seed.int32.in"   -n $n_elements -s $seed -d uniform -t int32   --low -200000 --high 200000
done

# normal
for seed in {1..100}
do
    generate_data "$folder/normal/inputs/$seed.float32.in" -n $n_elements -s $seed -d normal -t float32 --mean 1000 --std 10
    generate_data "$folder/normal/inputs/$seed.int32.in"   -n $n_elements -s $seed -d normal -t int32   --mean 1000 --std 10
done

# lognormal
for seed in {1..100}
do
    generate_data "$folder/log_normal/inputs/$seed.float32.in" -n $n_elements -s $seed -d lognormal -t float32 --mean 1000 --std 10
    generate_data "$folder/log_normal/inputs/$seed.int32.in"   -n $n_elements -s $seed -d lognormal -t int32   --mean 1000 --std 10
done

# all zeros
generate_data "$folder/all_zeros/inputs/1.float32.in" -n $n_elements -s 1 -d zeros -t float32
generate_data "$folder/all_zeros/inputs/1.int32.in"   -n $n_elements -s 1 -d zeros -t int32

# all ones
generate_data "$folder/all_ones/inputs/1.float32.in" -n $n_elements -s 1 -d ones -t float32
generate_data "$folder/all_ones/inputs/1.int32.in"   -n $n_elements -s 1 -d ones -t int32

echo "Done processing all input files!"