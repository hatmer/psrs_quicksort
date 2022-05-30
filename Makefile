T=64

all:
	g++ -o test -O3 -march=native -fopenmp -Wall quicksort.cpp && bash -c "./test $T"

perf:
	env CPUPROFILE=quicksort.prof ./test $T && google-pprof --text --lines test quicksort.prof
