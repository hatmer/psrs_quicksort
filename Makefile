
all:
	gcc -o test -fopenmp -Wall quicksort.cpp && ./test 2
