#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

/*
struct list {
  int* elems;
  int length;
} list;
*/
/*
parallel merge: two threads find and merge top and bottom half of lists
void merge() {

}
*/
/*
Receive pointer to list and length
Partition in half if have 2 or more elements, if have less than 2 elements return
Merge the two lists, i.e. sort them
*/
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

void mergesort(int* arr, int* working_buffer, int length, int depth) {
  if (depth == 0) // TODO improve
    for (int i = 0; i < length; i++)
      working_buffer[i] = arr[i];


  // return if 1 or 0 elements
  if (length < 2)
    return;

  // sort the two halves 
  int N = length / 2;
  mergesort(arr, working_buffer, N, depth+1);
  mergesort(&arr[N], &working_buffer[N], length-N, depth+1);
/*
  // If the direct merge is already sorted
    if (arr[mid] <= arr[start2]) {
        return;
    }
 */

  int i = 0;
  int j = N;
  int *source, *dest;
  if (depth%2) { // // write to wb on steps ...,5,3,1. Write to arr on steps ...,4,2,0
    dest = working_buffer;
    source = arr;
  } else {
    dest = arr;
    source = working_buffer;
  }

  /*printf("-------\nmerging on depth %d\n", depth);
  printf("source: \n");
  show(source, length);
  printf("dest: \n");
  show(dest, length);*/

  // merge the two halves (do it parallel)
  for (int index = 0; index < length; index++) {
		if (i == N) {
      dest[index] = source[j];
      j++;
    } else if (j == length) {
      dest[index] = source[i];
      i++;
    } else if (source[i] <= source[j]) {
      dest[index] = source[i];
      i++;
    } else if (source[j] < source[i]) { // TODO remove if
      dest[index] = source[j];
      j++;
    }
  }
  /*printf("source: \n");
  show(source, length);
  printf("dest: \n");
  show(dest, length);
  printf("-------\n");*/

}


int main(int argc, char *argv[]) {
  // create test input
  for (int N = 0; N < 100; N++) {
		int *orig = (int*)malloc(N*sizeof(int));
		int *arr = (int*)malloc(N*sizeof(int));
		int *working_buffer = (int*)malloc(N*sizeof(int));
		arr[0] = 1;
    orig[0] = 1;
		for (int i = 1; i < N; i++) {
			orig[i] = ((arr[i-1] + 50) - 12) % N;
			arr[i] = ((arr[i-1] + 50) - 12) % N;
		}

    mergesort(arr, working_buffer, N, 0);
    verify(orig, arr, N);
 
    // cleanup
    free(arr);
  }
  printf("all tests completed\n");
}
