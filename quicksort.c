#include <float.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PERF 0

// print out an array
void show(double *arr, int length) {
  for (int i = 0; i < length; i++) printf("%.4f,", arr[i]);
  printf("\n");
}

// partition the array around pivot
int partition(double *arr, int lo, int hi) {
  // choose pivot
  double pivot = arr[(hi + lo) / 2];
  // pointer to each end
  int i = lo - 1;
  int j = hi + 1;

  // sort O(n// )
  while (1) {
    do {
      i++;
    } while (arr[i] < pivot);
    do {
      j--;
    } while (arr[j] > pivot);
    if (i >= j) return j;

    // swap
    double tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
  }
}

// sort the array
void quicksort(double *arr, int lo, int hi) {
  if (lo < hi) {
    double pivot = partition(arr, lo, hi);
    // spawn a task for right branch
    if (pivot + 1 != hi) {
      quicksort(arr, pivot + 1, hi);
    }
    // main thread processes left branch
    if (lo != pivot) {
      quicksort(arr, lo, pivot);
    }
  }
}

// verify correct sorting
void verify(double *orig, double *arr, int length) {
  quicksort(orig, 0, length - 1);
  for (int i = 0; i < length; i++) {
    if (orig[i] != arr[i]) {
      printf("invalid sorting for length %d\nexpected:\n", length);
      show(orig, length);
      printf("actual:\n");
      show(arr, length);
      printf("%f != %f\n", orig[i], arr[i]);
      return;
    }
  }
}

void sample(double *arr, int lo, int hi, int interval, double *samples,
            int threadID) {
  int P = omp_get_max_threads();
  int samples_index_zero = threadID * P;
  for (int i = 0; i < P; i++) {
    samples[samples_index_zero + i] = arr[lo + i * interval];
  }
}

int binarySearch(double *arr, int start, int end, double value) {
  int closestLower = -1;
  while (start <= end) {
    int mid = (start + end) / 2;
    if (arr[mid] == value) {
      return mid;
    } else if (arr[mid] < value) {
      closestLower = mid;
      start = mid + 1;
    } else {
      end = mid - 1;
    }
  }
  return closestLower;
}

int unrolledIndex(int P, int row, int col, int ind) {
  return (row * P + col) * 2 + ind;
}

void partition_segment(double *arr, int *partitions, double *pivots,
                       int segment_size, int hi, int P) {
  int myid = omp_get_thread_num();
  int segment_start = myid * segment_size;
  int segment_end = segment_start + segment_size - 1;
  if (myid == P - 1) {
    segment_end = hi;
  }

  int next_start = segment_start;

  for (int slot = 0; slot <= P - 1; slot++) {  // for each pivot
    int index = binarySearch(arr, segment_start, segment_end,
                             pivots[slot]);  // either index of pivot or index
                                             // of closest smaller element
    if (index != -1) {
      partitions[unrolledIndex(P, myid, slot, 0)] = next_start;
      partitions[unrolledIndex(P, myid, slot, 1)] = index;
      next_start = index + 1;
    }
    if (slot == P - 2) {
      // check if any elements remain after last pivot
      if (next_start <= segment_end) {
        partitions[unrolledIndex(P, myid, slot + 1, 0)] = next_start;
        partitions[unrolledIndex(P, myid, slot + 1, 1)] = segment_end;
      }
      return;
    }
  }
}

// [https://www.uio.no/studier/emner/matnat/ifi/INF3380/v10/undervisningsmateriale/inf3380-week12.pdf]
double *PSRS(double *arr, int lo, int hi) {
  int P = omp_get_max_threads();
  // 0. partition list into P segments
  int segment_size = ((hi + 1) / P);  // + 1;

  // each process will take P samples
  int Psquared = P * P;
  double *samples = (double *)malloc(Psquared * sizeof(double));
  int interval = (hi + 1) / Psquared;
  if (interval == 0) interval = 1;

// 1. P processes each do sequential quicksort on local segment
#pragma omp parallel
  {
    int myid = omp_get_thread_num();
    // printf("thread %d doing paralllel quicksort\n", myid);
    int segment_lo = myid * segment_size;
    int segment_hi =
        segment_lo + segment_size - 1;  // last element in this thread's segment
    if (myid == P - 1) segment_hi = hi;

    // printf("thread %d doing paralllel quicksort %d-%d\n", myid, segment_lo,
    // segment_hi);
    quicksort(arr, segment_lo, segment_hi);

    // 2. each process samples its segment
    sample(arr, segment_lo, segment_hi, interval, samples, myid);
  }

  // 3. one process gathers and sorts samples
  quicksort(samples, 0, Psquared - 1);

  // then selects P-1 pivots
  // sample at regular intervals starting at P/2 to avoid lowest and highest
  double *pivots = (double *)malloc((P - 1) * sizeof(double));
  for (int i = 0; i < P - 1; i++) pivots[i] = samples[(P / 2) + i * P];

  int *partitions = (int *)malloc(P * P * 2 * sizeof(int));
  // populate partitions with null entries
  for (int i = 0; i < P * P * 2; i += 2) {
    partitions[i] = -1;
    partitions[i + 1] = -2;
  }

  double *result = (double *)malloc((hi + 1) * sizeof(double));
  int starting_result_indices[P];
  starting_result_indices[0] = 0;

// then broadcasts pivots to processes
#pragma omp parallel
  { partition_segment(arr, partitions, pivots, segment_size, hi, P); }

  // 5. process i gets all partitions #i and merges its partitions into a single
  // list for each process, sum all assigned intervals
  for (int p = 1; p < P; p++) {
    int psum = 0;
    for (int i = 0; i < P; i++) {
      psum += 1 + partitions[unrolledIndex(P, i, p - 1, 1)] -
              partitions[unrolledIndex(P, i, p - 1, 0)];
    }
    starting_result_indices[p] = psum + starting_result_indices[p - 1];
  }

#pragma omp parallel
  {
    // extract elements to avoid recalculations
    int myid = omp_get_thread_num();
    int segment_start = myid * segment_size;
    int segment_end = segment_start + segment_size - 1;
    if (myid == P - 1) {
      segment_end = hi;
    }
    // shortcut aliases for source slots
    int pointers[P];  // indices to next potential source slots
    int stops[P];     // index to stop at for each source pointer
    for (int i = 0; i < P; i++) {
      pointers[i] = partitions[unrolledIndex(P, i, myid, 0)];
      stops[i] = partitions[unrolledIndex(P, i, myid, 1)];
    }

    // merge
    int idx = starting_result_indices[myid];  // index of next destination slot
    int end_index;
    if (myid == P - 1)          // is last process
      end_index = segment_end;  // reuse
    else
      end_index = starting_result_indices[myid + 1] - 1;
    // for each slot, find smallest element from partitions
    while (idx <= end_index) {
      double minVal = DBL_MAX;
      int minInd = 0;
      for (int i = 0; i < P; i++) {
        if (pointers[i] <= stops[i] && arr[pointers[i]] < minVal) {
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
  free(partitions);
  free(pivots);
  free(samples);
  return result;
}

int main(int argc, char *argv[]) {
  int threads;
  int N;
  int distribution;
  srand(time(0));
  if (PERF) {
    threads = 8;
    N = 100000000;
    distribution = 0;
  } else {
    if (argc != 4) {
      printf(
          "usage: ./%s <number of threads> <number of elements> <number "
          "specifying distribution (0-3)>\n",
          argv[0]);
      return -1;
    }
    threads = atoi(argv[1]);
    N = atoi(argv[2]);
    distribution = atoi(argv[3]);
  }
  omp_set_num_threads(threads);
  omp_set_nested(1);

  // run tests
  double *orig = (double *)malloc(N * sizeof(double));
  double *arr = (double *)malloc(N * sizeof(double));
  double lambda = 10;
  double tmp = 0;

  for (int i = 0; i < N; i++) {
    double r = drand48();
    switch (distribution) {
      case 0:  // uniform
        orig[i] = r;
        break;
      case 1:  // exponential
        orig[i] = -lambda * log(1 - drand48());
        break;
      case 2:  // normal
        double x = drand48();
        orig[i] = sqrt(-2 * log(x)) * cos(2 * M_PI * r);
        break;
      case 3:  // sorted
        orig[i] = tmp;
        tmp += 1;
        break;
    }
    arr[i] = orig[i];
  }
  // time execution
  double begin = omp_get_wtime();
  double *result;
  if (threads == 1)
    quicksort(arr, 0, N - 1);
  else
    result = PSRS(arr, 0, N - 1);
  double end = omp_get_wtime();
  double time_spent = (float)(end - begin);
  printf("%.8f\n", time_spent * 1000);  // time in milliseconds

  // verify
  if (threads == 1)
    verify(orig, arr, N);
  else {
    verify(orig, result, N);
    free(result);
  }
  // cleanup
  free(arr);
  free(orig);
}
