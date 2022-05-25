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
  printf("lo: %d, pivot: %d, hi: %d\n", lo, pivot, hi);
  // pointer to each end
  int i = lo-1;
  int j = hi+1;
  int tmp;

  // sort O(n)
  while (1) {
    //printf("%d, %d\n", i, j);
    do { i++; } while (arr[i] < pivot);
    do { j--; } while (arr[j] > pivot);
    if (i >= j)
      return j;
    
    // swap
    tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
  }
}

void quicksortparallel(int* arr, int lo, int hi) {
  if (lo >= 0 && hi >= 0 && lo < hi) {
		int pivot = partition(arr,lo,hi);
		int args[4] = {lo, pivot, pivot+1, hi};
		#pragma omp parallel for
		for (int i = 0; i < 4; i+=2) {
      printf("thread %d\n", omp_get_thread_num());
			quicksortparallel(arr,args[i],args[i+1]);
		}
  }
  
}

void quicksort(int* arr, int lo, int hi) {
	if (lo >= 0 && hi >= 0 && lo < hi) {
		int pivot = partition(arr,lo,hi);
    //printf("pivot is %d\n", pivot);
		quicksortparallel(arr,lo,pivot);
		quicksortparallel(arr,pivot+1,hi);
	}

}

int main(int argc, char *argv[]) {
  // create test input
  if (argc != 2) {
    printf("usage: ./%s <number of threads>\n", argv[0]);
    return -1;
  }

  int threads = atoi(argv[1]);
  omp_set_num_threads(threads);

  for (int N = 10; N < 11; N++) {
		int *orig = (int*)malloc(N*sizeof(int));
		int *arr = (int*)malloc(N*sizeof(int));
		arr[0] = 1;
    orig[0] = 1;
		for (int i = 1; i < N; i++) {
			orig[i] = ((arr[i-1] + 50) - 12) % N;
			arr[i] = ((arr[i-1] + 50) - 12) % N;
		}
    show(arr, N);
    quicksort(arr,0, N-1);
    verify(orig, arr, N);
 
    // cleanup
    free(arr);
    free(orig);
  }
  printf("all tests completed\n");
}
