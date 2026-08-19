[[ "${PWD##*/}" == "test_runs" ]] && cd ..
make clean
make -j$(nproc) benchmarks

