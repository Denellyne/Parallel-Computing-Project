# Parallel Computing Project

### Project made by Fábio Semedo, Gabriel Carlos, Gustavo dos Santos

This project was the first assignment for the class of Parallel Computing.

## Code Structure

Our code is separated in two directories, the [src](src) directory contains the main implementation for this project, using MPI, and other files which contain helper functions like SIMD, equivalent implementations of memcpy and memmove, and multiplication functions for the Fox's algorithm.

In turn, the [srcSec](srcSeq) directory contains a reformulation of the code for sequential computing for benchmarking purposes.

Both of these folders contain a [benchmark.sh](src/benchmark.sh) that benchmarks the code by running each pair of input and then number of processes ten times and then taking the average. There is also a script [tester.sh](src/tester.sh) that assures the code outputs the correct output for every input provided.

Finally the [tests](tests) directory contains the inputs and outputs provided by our professor.

## Build Instructions

Either in [src](src) or the [srcSeq](srcSeq) directory there's a [Makefile](src/Makefile), which means that building is as simple as running:

    $ make build # The binary is stored in the bin folder as fox or foxSeq accordingly

## Run Instructions

Running is as simple as doing:

    $ mpirun -n #processes ./bin/fox < ./tests/inputfile // For running the code in parallel with MPI
    $ ./bin/foxSeq < ./tests/inputfile // For running the code sequentially


