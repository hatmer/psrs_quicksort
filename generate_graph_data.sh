#!/bin/bash

echo "speedup per thread"
./psrs 1 1000000 0
./psrs 2 1000000 0
./psrs 4 1000000 0
./psrs 8 1000000 0
./psrs 16 1000000 0
./psrs 32 1000000 0
./psrs 64 1000000 0
./psrs 128 1000000 0






echo "algorithm vs different distributions"

