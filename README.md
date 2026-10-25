# Parallel Computing Project

### Project made by Fábio Semedo, Gabriel Carlos, Gustavo dos Santos

This project was the first assignment for the class of Parallel computing

## Code Structure

Our code is separated in two folders, the [src](src) folder contains the main implementation for this project, using MPI, and other files which contain helper functions like SIMD equivalent implementations of memcpy or memmove and multiplication functions for the fox algorithm.

The folder [srcSec](srcSeq) in turn, contains an adaptation of the code for sequential computing for benchmarking porpuses.

Both of these folders contain a [benchmark.sh](src/benchmark.sh) that benchmark the code by running each pair of input and #processes ten times and then taking the average, and a [tester.sh](src/tester.sh) scripts that assures the code outputs the correct output for every input provided.

Finally the [tests](tests) folder contains the inputs and outputs provided by our professor.

## Build Instructions

Either in [src](src) or the [srcSeq](srcSeq) folders there's a [Makefile](src/Makefile), thus building is as simple as running:

    $ make build # The binary is stored in the bin folder as fox or foxSeq accordingly

## Run Instructions

Running is as simple as doing:

    $ mpirun -n #processes ./bin/fox < ./tests/inputfile // For running the code in parallel with MPI
    $ ./bin/foxSeq < ./tests/inputfile // For running the code sequentially


