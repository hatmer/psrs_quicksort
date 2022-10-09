all:
	gcc -o psrs -O3 -march=native -I /usr/include/gsl -I /usr/lib/ -lgslcblas -lgsl -fopenmp -Wall quicksort.c

test: all
	./run_tests.sh

debug:
	gcc -o psrs -g -I /usr/include/gsl -I /usr/lib/ -lgslcblas -lgsl -fopenmp -Wall quicksort.c

profile:
	gcc -g -o psrs -O3 -march=native -I /usr/include/gsl -I /usr/lib/ -lgslcblas -lgsl -fopenmp -Wall -lprofiler quicksort.c && CPUPROFILE=psrs.prof ./psrs --num_threads=4 && pprof --text --lines ./psrs psrs.prof

graphs: all
	./generate_graph_data.sh
	
