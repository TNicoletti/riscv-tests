
echo "all_zeros"
python3 ./error_finder.py  ./all_zeros/faulty_outputs
echo "all_ones"
python3 ./error_finder.py   ./all_ones/faulty_outputs
echo "normal"
python3 ./error_finder.py     ./normal/faulty_outputs
echo "log_normal"
python3 ./error_finder.py ./log_normal/faulty_outputs
echo "uniform"
python3 ./error_finder.py    ./uniform/faulty_outputs
