#!/bin/bash

./test 2 10 0
./test 2 10 1
./test 2 10 2
./test 2 10 3
./test 2 10 4

./test 32 100000 0
./test 32 100000 1
./test 32 100000 2
./test 32 100000 3
./test 32 100000 4

./test 64 100000 3
./test 8 1000 2
./test 128 100000 1
./test 8 8000 1
./test 3 8000 2
./test 16 100000 3
./test 64 10001 4

