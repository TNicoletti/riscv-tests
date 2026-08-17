
echo "all_zeros"
python3 ./error_finder.py ./all_zeros/outputs
echo "all_ones"
python3 ./error_finder.py ./all_ones/outputs
echo "normal"
python3 ./error_finder.py ./normal/outputs
echo "log_normal"
python3 ./error_finder.py ./log_normal/outputs
echo "uniform"
python3 ./error_finder.py ./uniform/outputs
