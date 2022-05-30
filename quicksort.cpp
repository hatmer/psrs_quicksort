#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <chrono>
#include <unistd.h>
#include <random>

const int page_table_size = 11268 * 1024; 
const int page_size = getpagesize();
const int page_table_int_capacity = page_table_size / sizeof(int*) * page_size / sizeof(int);

// print out an array
void show(int *arr, int length) {
  for (int i = 0; i < length; i++)
    printf("%d,", arr[i]);
  printf("\n");
}

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

// partition the array around pivot
int partition(int* arr, int lo, int hi) {
  // choose pivot
  int pivot = arr[(hi+lo)/2];
  // pointer to each end
  int i = lo-1;
  int j = hi+1;

  // sort O(n)
  while (1) {
    do { i++; } while (arr[i] < pivot);
    do { j--; } while (arr[j] > pivot);
    if (i >= j)
      return j;
    
    // swap
    std::swap(arr[i], arr[j]);
  }
}

// sort the array
void quicksort(int* arr, int lo, int hi) {
	if (lo < hi) {
		int pivot = partition(arr,lo,hi);
    int lower = hi-lo <= page_table_int_capacity;

    // use all threads if entire array fits in memory
    if (lower) {
      // spawn a task for right branch
			if (pivot+1 != hi) {
				#pragma omp task
				quicksort(arr,pivot+1,hi);
			}
    }
    
    // main thread processes left branch
    if (lo != pivot) { 
		  quicksort(arr,lo,pivot);
    }

    // single-thread processing when using multiple threads would thrash page table
    if (!lower) {
    //   printf("upper\n");
       quicksort(arr,pivot+1,hi);
    }

  }

}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: ./%s <number of threads>\n", argv[0]);
    return -1;
  }
  int threads = atoi(argv[1]);
  //printf("using %d threads\n", threads);
  omp_set_num_threads(threads);
  omp_set_nested(1);

  // run tests
  for (int N = 10000; N < 10001; N++) {
		int *orig = (int*)malloc(N*sizeof(int));
		int *arr = (int*)malloc(N*sizeof(int));

    // create random data for testing
    // https://stackoverflow.com/questions/13445688/how-to-generate-a-random-number-in-c
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0,10000000);
		for (int i = 0; i < N; i++) {
			orig[i] = dist(rng);
			arr[i] = orig[i];
		}

    // https://stackoverflow.com/questions/22387586/measuring-execution-time-of-a-function-in-c
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    auto t1 = high_resolution_clock::now();

    #pragma omp parallel
    {
      #pragma omp single nowait
      quicksort(arr,0, N-1);
    }

    auto t2 = high_resolution_clock::now();
    duration<double, std::milli> ms_double = t2 - t1;
    printf("%.2f\n", ms_double.count()/1000.0);
    
    verify(orig, arr, N);
    
    // cleanup
    free(arr);
    free(orig);
  }
  //printf("all tests completed\n");
}
