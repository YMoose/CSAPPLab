#!/bin/bsah
make clean
make test-trans tracegen
echo -e "Matrix 32x32: 8 points if m < 300, 0 points if m > 600\n"
./test-trans -M 32 -N 32
echo -e "Matrix 64x64: 8 points if m < 1300, 0 points if m > 2000\n"
./test-trans -M 64 -N 64
echo -e "Matrix 61x67: 10 points if m < 2000, 0 points if m > 3000\n"
./test-trans -M 61 -N 67

