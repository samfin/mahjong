#!/bin/bash

set -e
rm -f a.exe
g++ main.cpp -Os -Wall -g3 -I. -pg -static -lm -std=c++11
# g++ main.cpp -O3 -std=c++11
./a.exe
gprof ./a.exe > a.txt
