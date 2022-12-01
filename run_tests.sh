#!/bin/bash

echo "2 threads, 10 elements, all distributions"
./psrs 2 10 0
./psrs 2 10 1
./psrs 2 10 2

echo "Max threads, 1e5 elements, all distributions"
./psrs 32 100000 0
./psrs 32 100000 1
./psrs 32 100000 2

echo "Random tests"
./psrs 64 100000 0
./psrs 8 1000 2
./psrs 128 100000 1
./psrs 8 8000 1
./psrs 3 8000 2
./psrs 16 100000 0
./psrs 64 10001 2

echo "1e7 uniformly distributed elements: 1 vs 8 threads"
./psrs 1 10000000 0
./psrs 8 10000000 0
