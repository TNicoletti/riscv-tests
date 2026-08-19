#!/bin/bash

[[ "${PWD##*/}" == "test_runs" ]] && cd ..

if [ -z "$1" ]; then
    echo "Error: Missing test name parameter."
    echo "Usage: $0 <name_of_test_to_be_run>"
    exit 1
fi

TEST_NAME="$1"

echo "Running setup for test: ${TEST_NAME}"

# ./gen_files.sh
rm ./benchmarks/$TEST_NAME/test_runs/*/outputs/* -f
./test_runs/faulty_run.sh "$TEST_NAME"
./test_runs/error_finder_faulty.sh "$TEST_NAME"