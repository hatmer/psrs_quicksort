T=8

all:
	g++ -ggdb -o test -O2 -march=native -fopenmp -Wall quicksort.cpp -lprofiler && bash -c "time ./test $T"

perf:
	env CPUPROFILE=quicksort.prof ./test $T && google-pprof --text --lines test quicksort.prof
