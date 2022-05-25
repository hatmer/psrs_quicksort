
all:
	gcc -o test -fopenmp -Wall mergesort.cpp && ./test
