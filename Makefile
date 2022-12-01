all:
	gcc -o psrs -O3 -march=native -fopenmp quicksort.c -lm

test: all
	./run_tests.sh

debug:
	gcc -o psrs -g -fopenmp -Wall quicksort.c -lm

profile:
	gcc -g -o psrs -O3 -march=native -fopenmp -Wall -lprofiler quicksort.c -lm && CPUPROFILE=psrs.prof ./psrs --num_threads=4 && pprof --text --lines ./psrs psrs.prof

graphs: all
	./generate_graph_data.sh
