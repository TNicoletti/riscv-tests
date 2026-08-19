
echo "all_zeros"
python3 ./error_finder.py  ./all_zeros/invalid_outputs
echo "all_ones"
python3 ./error_finder.py   ./all_ones/invalid_outputs
echo "normal"
python3 ./error_finder.py     ./normal/invalid_outputs
echo "log_normal"
python3 ./error_finder.py ./log_normal/invalid_outputs
echo "uniform"
python3 ./error_finder.py    ./uniform/invalid_outputs
