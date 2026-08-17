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
n_elements=16384

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
    generate_data "$folder/uniform/inputs/$seed.float32.in" -n $n_elements -s $seed -d uniform -t float32 \
     --low -16 --high 16
    generate_data "$folder/uniform/inputs/$seed.int32.in"   -n $n_elements -s $seed -d uniform -t int32   \
     --low -16 --high 16
done
for seed in {101..200}
do
    generate_data "$folder/uniform/inputs/$seed.float32.in" -n $n_elements -s $seed -d uniform -t float32 \
     --low -32 --high 32
    generate_data "$folder/uniform/inputs/$seed.int32.in"   -n $n_elements -s $seed -d uniform -t int32   \
     --low -32 --high 32
done
for seed in {201..1000}
do
    generate_data "$folder/uniform/inputs/$seed.float32.in" -n $n_elements -s $seed -d uniform -t float32 \
     --low -256 --high 256
    generate_data "$folder/uniform/inputs/$seed.int32.in"   -n $n_elements -s $seed -d uniform -t int32   \
     --low -256 --high 256
done
for seed in {1001..1100}
do
    generate_data "$folder/uniform/inputs/$seed.float32.in" -n $n_elements -s $seed -d uniform -t float32 \
     --low -32768 --high 32768
    generate_data "$folder/uniform/inputs/$seed.int32.in"   -n $n_elements -s $seed -d uniform -t int32   \
     --low -32768 --high 32768
done
for seed in {1101..1200}
do
    generate_data "$folder/uniform/inputs/$seed.float32.in" -n $n_elements -s $seed -d uniform -t float32 \
     --low -1048576 --high 1048576
    generate_data "$folder/uniform/inputs/$seed.int32.in"   -n $n_elements -s $seed -d uniform -t int32   \
     --low -1048576 --high 1048576
done

# normal
for seed in {1..100}
do
    generate_data "$folder/normal/inputs/$seed.float32.in" -n $n_elements -s $seed -d normal -t float32 \
     --mean 1024 --std 16
    generate_data "$folder/normal/inputs/$seed.int32.in"   -n $n_elements -s $seed -d normal -t int32   \
     --mean 1024 --std 16
done

# lognormal
for seed in {1..100}
do
    generate_data "$folder/log_normal/inputs/$seed.float32.in" -n $n_elements -s $seed -d lognormal -t float32 \
     --mean 1024 --std 16
    generate_data "$folder/log_normal/inputs/$seed.int32.in"   -n $n_elements -s $seed -d lognormal -t int32   \
     --mean 1024 --std 16
done

# all zeros
generate_data "$folder/all_zeros/inputs/1.float32.in" -n $n_elements -s 1 -d zeros -t float32
generate_data "$folder/all_zeros/inputs/1.int32.in"   -n $n_elements -s 1 -d zeros -t int32

# all ones
generate_data "$folder/all_ones/inputs/1.float32.in" -n $n_elements -s 1 -d ones -t float32
generate_data "$folder/all_ones/inputs/1.int32.in"   -n $n_elements -s 1 -d ones -t int32

echo "Done processing all input files!"