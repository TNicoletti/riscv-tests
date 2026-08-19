BENCH=$1

[[ "${PWD##*/}" == "test_runs" ]] && cd ..
cd ./benchmarks/$BENCH/test_runs
./error_finder_faulty.sh