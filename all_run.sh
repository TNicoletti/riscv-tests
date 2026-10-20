time python3 ./test_runs/multi_run.py random_neural_network 32

time python3 ./test_runs/multi_run.py random_neural_network 32 RAW
time python3 ./test_runs/multi_run.py random_neural_network 32 ndb
time python3 ./test_runs/multi_run.py random_neural_network 32 cold_start
time python3 ./test_runs/multi_run.py random_neural_network 32 good_start

time python3 ./test_runs/multi_run_invalid.py random_neural_network 32 invalid

time python3 ./test_runs/multi_run.py random_neural_network 32 faulty
time python3 ./test_runs/multi_run.py random_neural_network 32 three
time python3 ./test_runs/multi_run.py random_neural_network 32 four
