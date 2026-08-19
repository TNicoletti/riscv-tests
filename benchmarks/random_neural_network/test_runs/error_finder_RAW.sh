
echo "all_zeros"
python3 ./error_finder.py  ./all_zeros/RAW_outputs
echo "all_ones"
python3 ./error_finder.py   ./all_ones/RAW_outputs
echo "normal"
python3 ./error_finder.py     ./normal/RAW_outputs
echo "log_normal"
python3 ./error_finder.py ./log_normal/RAW_outputs
echo "uniform"
python3 ./error_finder.py    ./uniform/RAW_outputs
