all:
	gcc -o psrs -O3 -march=native -I /home/haat1611/gsl -L /home/haat1611/gsl/lib -I /home/haat1611/gsl/include -I /home/haat1611/gsl/lib -fopenmp -Wall quicksort.c -lgsl -lgslcblas

test: all
	./run_tests.sh

debug:
	gcc -o psrs -g -I /home/haat1611/gsl/include -I /home/haat1611/gsl/lib -fopenmp -Wall quicksort.c

profile:
	gcc -g -o psrs -O3 -march=native -I /home/haat1611/gsl/include -I /home/haat1611/gsl/lib -fopenmp -Wall -lprofiler quicksort.c && CPUPROFILE=psrs.prof ./psrs --num_threads=4 && pprof --text --lines ./psrs psrs.prof

graphs: all
	./generate_graph_data.sh
	
