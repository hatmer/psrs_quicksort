# PSRS QuickSort
Parallel Sorting by Regular Sampling (PSRS) QuickSort algorithm implementation. PSRS achieves better load balancing than other parallel QuickSort algorithms.


PSRS has four phases:
1. Each process uses sequential quicksort on its local segment, and
then selects data items at local indices
0, n/P
2
, 2n/P
2
, . . .,
(
P
− 1)(n/P2) as a regular sample of its
locally sorted block
2. One process gathers and sorts the local regular samples. The
process then selects
P
−
1 pivot values from the sorted list of
regular smaples. The
P
−
1 pivot values are broadcast. Each
process then partitions its sorted sublist into
P disjoint pieces
accordingly.
3. Each process
i keeps its
ith partition and sends the
jth partition
to process
j, for all
j $\neq$ i
4. Each process merges its
P partitions into a single list


![psrs](https://user-images.githubusercontent.com/2366125/194700169-bf13eb1c-5cc3-445c-9a6e-3d7a8e9f5815.png)

<p align="center">Figure 14.5 from Parallel Programming in C with MPI and OpenMP</p>


Source: https://www.uio.no/studier/emner/matnat/ifi/INF3380/v10/undervisningsmateriale/inf3380-week12.pdf
