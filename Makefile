T=8

all:
	gcc -o test -O3 -march=native -I /usr/include/gsl -I /usr/lib/ -lgslcblas -lgsl -fopenmp -Wall quicksort.c && bash -c "./test $T"

perf:
	env CPUPROFILE=quicksort.prof ./test $T && google-pprof --text --lines test quicksort.prof
