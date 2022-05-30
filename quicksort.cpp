#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

void show(int *arr, int length) {
  for (int i = 0; i < length; i++)
    printf("%d,", arr[i]);
  printf("\n");
}

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

void copy(int* src, int* dst, int length) {
  for (int i = 0; i < length; i++)
    dst[i] = src[i];
}


int partition(int* arr, int lo, int hi) {
  // choose pivot
  int pivot = arr[(hi+lo)/2];
  //printf("lo: %d, pivot: %d, hi: %d\n", lo, pivot, hi);
  // pointer to each end
  int i = lo-1;
  int j = hi+1;
  //int tmp;

  // sort O(n)
  while (1) {
    do { i++; } while (arr[i] < pivot);
    do { j--; } while (arr[j] > pivot);
    if (i >= j)
      return j;
    
    // swap
    std::swap(arr[i], arr[j]);
    /*tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;*/
  }
}
#include <unistd.h>
const int page_table_size = 2496 * 1024; // 11268 kb
const int page_size = getpagesize();
const int page_table_int_capacity = page_table_size / sizeof(int*) * page_size / sizeof(int);

void quicksort(int* arr, int lo, int hi) {
	//if (lo < hi) {
		int pivot = partition(arr,lo,hi);
    //printf("(%d/%d): %d %d %d\n", omp_get_thread_num(), omp_get_max_threads(), lo, pivot, hi);
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
       //printf("upper\n");
       quicksort(arr,pivot+1,hi);
    }

  //}

}

int main(int argc, char *argv[]) {
  // create test input
/*
  if (argc != 2) {
    printf("usage: ./%s <number of threads>\n", argv[0]);
    return -1;
  }*/
  int threads = 4; //atoi(argv[1]);
  printf("using %d threads\n", threads);
  omp_set_num_threads(threads);
  omp_set_nested(1);

  for (int N = 100000000; N < 100000001; N++) {
		int *orig = (int*)malloc(N*sizeof(int));
		int *arr = (int*)malloc(N*sizeof(int));
		arr[0] = 1;
    orig[0] = 1;
		for (int i = 1; i < N; i++) {
			orig[i] = ((arr[i-1] + 50) - 12) % N;
			arr[i] = ((arr[i-1] + 50) - 12) % N;
		}
    //show(arr, N);
    #pragma omp parallel
    {
    #pragma omp single nowait
    quicksort(arr,0, N-1);
    }
    //verify(orig, arr, N);
    // cleanup
    free(arr);
    free(orig);
  }
  printf("all tests completed\n");
}
