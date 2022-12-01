#!/bin/bash

echo "speedup per thread"
./psrs 1 100000000 0
./psrs 2 100000000 0
./psrs 4 100000000 0
./psrs 8 100000000 0
./psrs 16 100000000 0
./psrs 32 100000000 0
./psrs 64 100000000 0
./psrs 128 100000000 0

echo "algorithm vs different distributions"
./psrs 8 100000000 0
./psrs 8 100000000 1
./psrs 8 100000000 2
./psrs 8 100000000 3
