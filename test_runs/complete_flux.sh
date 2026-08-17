#!/bin/bash

if [ -z "$1" ]; then
    echo "Error: Missing test name parameter."
    echo "Usage: $0 <name_of_test_to_be_run>"
    exit 1
fi

TEST_NAME="$1"

echo "Running setup for test: ${TEST_NAME}"

./gen_files.sh
./multi_run.sh "$TEST_NAME"
./error_finder.sh "$TEST_NAME"