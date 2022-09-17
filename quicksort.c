#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
//#include <algorithm>
#include <gsl/gsl_rng.h>
#include <time.h>

// print out an array
void show(double *arr, int length) {
  for (int i = 0; i < length; i++)
    printf("%.4f,", arr[i]);
  printf("\n");
}

/*
// verify correct sorting
void verify(int *orig, int *arr, int length) {
  std::sort(orig, orig+length);
  for (int i = 0; i < length; i++) {
    if (orig[i] != arr[i]) {
      printf("invalid sorting for length %d\nexpected:\n", length);
      show(orig, length);
      printf("actual:\n");
      show(arr, length);
      printf("%d != %d\n", orig[i], arr[i]);
      return;
    }
  }
}
*/

// partition the array around pivot
int partition(double* arr, int lo, int hi) {
  // choose pivot
  double pivot = arr[(hi+lo)/2];
  // pointer to each end
  int i = lo-1;
  int j = hi+1;

  //sort O(n// )
  while (1) {
    do { i++; } while (arr[i] < pivot);
    do { j--; } while (arr[j] > pivot);
    if (i >= j)
      return j;

    // swap
    double tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
  }
}

// sort the array
void quicksort(double* arr, int lo, int hi) {
	if (lo < hi) {
		double pivot = partition(arr,lo,hi);

    // spawn a task for right branch
    if (pivot+1 != hi) {
      //#pragma omp task
      quicksort(arr,pivot+1,hi);
    }

    // main thread processes left branch
    if (lo != pivot) {
		  quicksort(arr,lo,pivot);
    }
  }
}

void sample(double* arr, int lo, int hi, int interval, double* samples, int threadID) {
  int P = omp_get_max_threads();
  int samples_index_zero = threadID * P;

  for (int i = 0; i < P; i++) {
    samples[samples_index_zero+i] = arr[lo + i*interval]; // TODO check for segfault at end of array
  }
}

// https://www.uio.no/studier/emner/matnat/ifi/INF3380/v10/undervisningsmateriale/inf3380-week12.pdf
void PSRS(double* arr, int lo, int hi) {
  int P = omp_get_max_threads();
  //printf("thread: %d\n", omp_get_thread_num());
  // 0. partition list into P segments
  int segment_size = (hi+1) / P;
  //printf("segment_size: %d\n", segment_size);

  // each process will take P samples
  int Psquared = P*P;
  double *samples = (double*)malloc(Psquared*sizeof(double));
  int interval = (hi+1) / Psquared;

  // 1. P processes each do sequential quicksort on local segment
#pragma omp parallel
  {
    int myid = omp_get_thread_num();
    int segment_start = myid*segment_size;
    int segment_end = segment_start+segment_size;
    if (myid == P-1)
      segment_end = hi;

    //printf("%d - %d\n", segment_start, segment_end);

    quicksort(arr, segment_start, segment_end);

    // 2. each process samples its segment
    // => store in array of pointers to arrays
    sample(arr, segment_start, segment_end, interval, samples, myid);
  }

  // 3. one process gathers and sorts samples
  quicksort(samples,0,Psquared);
  show(samples, Psquared);

  // then selects P-1 pivots TODO consider random choice of pivots?
  double pivots[P-1];
  for (int i = 0; i < P-1; i++) {
    pivots[i] = samples[i+P];
  }

  int partitions[P][P][2];
  double *result = (double*)malloc((hi+1)*sizeof(double));
  int starting_result_indices[P];

  // then broadcasts pivots to processes
  #pragma omp parallel
  {
    // 4. each process partitions its sorted segment into P disjoint pieces
    int myid = omp_get_thread_num();
    int segment_start = myid*segment_size;
    int segment_end = segment_start+segment_size;
    if (myid == P-1)
      segment_end = hi+1;
    int pivot_idx = 0;
    int start = segment_start;
    for (int i = segment_start; i<segment_end; i++) {
      if (arr[i+1] > pivots[pivot_idx]) {
        partitions[myid][pivot_idx][0] = start;
        partitions[myid][pivot_idx][1] = i;  // TODO what happens if a partition is empty?
        start = i;
        pivot_idx++;
        if (pivot_idx == P) {
          partitions[myid][P][0] = start;
          partitions[myid][P][1] = segment_end;
          break;
        }
      }
    }
    #pragma omp barrier

    // 5. process i gets all partitions #i and merges its partitions into a single list
    #pragma omp single // to ensure previous threads have completed, to avoid recalculating sums
    {
      // for each process, sum all assigned intervals
      for (int p = 0; p < P; p++) {
        int psum = 0;
        for (int i = 0; i < P; i++) {
          psum += partitions[i][p][1] - partitions[i][p][0];
        }
        starting_result_indices[p] = psum;
        if (p > 0) {
          starting_result_indices[p] += starting_result_indices[p-1];
        }
      }
    }

    // shortcut aliases for source slots
    int pointers[P]; // indices to next potential source slots
    int stops[P];    // index to stop at for each source pointer
    for (int i = 0; i < P; i++) {
      pointers[i] = partitions[i][myid][0];
      stops[i] = partitions[i][myid][1];
    }

    int idx = starting_result_indices[myid]; // index of next destination slot
    int end_index;
    if (myid = P-1) // is last process
      end_index = segment_end; // reuse
    else
      end_index = starting_result_indices[myid+1];

    // for each slot, find smallest element from partitions
    while (idx < end_index) {
      // find max of all valid candidate sources
      double minVal = 0.0;
      int minInd = 0;
      // for each of pointers, if is less than stop and more than maxVal, use arr[pointers[x]] and store x
      for (int i = 0; i < P; i++) {
        if (pointers[i] < stops[i] && arr[pointers[i]] < minVal) {
          minVal = arr[pointers[i]];
          minInd = i;
        }
      }

      result[idx] = minVal;
      // increment source and dest pointers
      pointers[minInd]++;
      idx++;


    }
  }
  free(samples);
}


int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: ./%s <number of threads>\n", argv[0]);
    return -1;
  }
  int threads = atoi(argv[1]);
  printf("using %d threads\n", threads);
  omp_set_num_threads(threads);
  omp_set_nested(1);

  // run tests
  for (int N = 10000; N < 10001; N++) {
		double *orig = (double*)malloc(N*sizeof(double));
		double *arr = (double*)malloc(N*sizeof(double));

    // create random data for testing [http://gnu.ist.utl.pt/software/gsl/manual/html_node/Random-Number-Generator-Examples.html]
    const gsl_rng_type * T;
    gsl_rng * r;
    gsl_rng_env_setup();
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    for (int i = 0; i < N; i++) {
		 	orig[i] = gsl_rng_uniform(r);
		 	arr[i] = orig[i];
		}
    gsl_rng_free(r);
    show(arr, N);

    // time execution [https://stackoverflow.com/questions/5248915/execution-time-of-c-program]
    clock_t begin = clock();
    PSRS(arr, 0, N-1);
    clock_t end = clock();
    double time_spent = (float)(end-begin) / CLOCKS_PER_SEC;
    printf("%.8f\n", time_spent * 1000); // time in milliseconds

    // verify
    //   verify(orig, arr, N);
    show(arr, N);

    // cleanup
    free(arr);
    free(orig);
  }
  //printf("all tests completed\n");
}
