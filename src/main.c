
#include "./Misc/Misc.h"
#include "Math/Math.h"
#include <math.h>
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MASTER 0

int main(int argc, char *argv[]) {
  int numProcs, rank, cartRank, subSize, nodes;
  // MPI_Request requests[2];
  double startTime;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  const int sqrtNumProcs = (int)sqrt(numProcs);

  const int sizes[2] = {sqrtNumProcs, sqrtNumProcs};
  const int periodic[2] = {1, 1};
  MPI_Comm gridCommunicator;
  int cart =
      MPI_Cart_create(MPI_COMM_WORLD, 2, sizes, periodic, 1, &gridCommunicator);

  MPI_Group worldGroup;
  MPI_Comm_group(gridCommunicator, &worldGroup);
  const int rootCoords[2] = {0, 0};
  int rootRank;

  // Find the rank of those coordinates in the new communicator
  MPI_Cart_rank(gridCommunicator, rootCoords, &rootRank);
  MPI_Group rowGroup;
  MPI_Group columnGroup;
  int rowGroupRanks[sqrtNumProcs];
  int columnGroupRanks[sqrtNumProcs];
  size_t initialIdx = -1;

  MPI_Comm_rank(gridCommunicator, &cartRank);
  const int group = cartRank % sqrtNumProcs;
  // Here we get the ids for the rows and columns for the fox algorithm
  for (size_t i = 0; i < sqrtNumProcs; i++) {
    rowGroupRanks[i] = cartRank + i - group;
    columnGroupRanks[i] =
        (cartRank % sqrtNumProcs + i * sqrtNumProcs) % numProcs;
    if (rowGroupRanks[i] % sqrtNumProcs == rowGroupRanks[i] / sqrtNumProcs)
      initialIdx = i;
  }

  // Communicator for the fox algorithm row
  MPI_Group_incl(worldGroup, sqrtNumProcs, rowGroupRanks, &rowGroup);
  MPI_Comm rowCommunicator;
  MPI_Comm_create_group(gridCommunicator, rowGroup, 0, &rowCommunicator);
  MPI_Group_incl(worldGroup, sqrtNumProcs, columnGroupRanks, &columnGroup);
  MPI_Comm columnCommunicator;
  MPI_Comm_create_group(gridCommunicator, columnGroup, 0, &columnCommunicator);

  int sendTo, receiveFrom;
  MPI_Cart_shift(columnCommunicator, 0, 1, &sendTo, &receiveFrom);

  // Read the amount of nodes
  if (rank == 0) {
    fscanf(stdin, "%d", &nodes);
    if (nodes % sqrtNumProcs != 0) {
      printf(
          "Number of processes must evenly divide the number of nodes in the "
          "matrix\n");
      MPI_Abort(gridCommunicator, 1);
      return 1;
    }

    subSize = nodes / sqrtNumProcs;
  }
  // Broadcasts the node of the subMatrix to every process
  MPI_Bcast(&subSize, 1, MPI_INT, 0, gridCommunicator);
  int *restrict subMatrix = alloc(subSize);
  int *restrict graph = NULL;

  if (rank == 0) {
    graph = alloc(nodes);
    // Reads the nodes
    for (size_t i = 0; i < nodes; i++) {
      // Here we see the first example of using a variable to point to a
      // specific part of a matrix, because we are using 1D arrays as the
      // representation, not only does this help us work with them by improving
      // readability but also improves compiler auto-vectorization

      int *restrict graphRow = &graph[i * nodes];
      for (size_t k = 0; k < nodes; k++) {
        if (fscanf(stdin, "%d", &graphRow[k]) != 1) {
          fprintf(stderr, "ERROR: scanf failed at [%zu][%zu]!\n", i, k);
          MPI_Abort(gridCommunicator, 1);
          return 1;
        }
        if (graphRow[k] == 0 && i != k)
          graphRow[k] = INF;
      }
    }

    startTime = MPI_Wtime();
    int *restrict buf = alloc(nodes);
    size_t pos = 0;
    size_t row = 0;
    size_t rowIdx = 0;
#pragma GCC ivdep
    for (size_t i = 0; i < numProcs; i++) {
      if (i != 0 && i % sqrtNumProcs == 0) {
        row += subSize;
        rowIdx = 0;
      }

      const int *const restrict graphRow =
          &graph[rowIdx * subSize + (row * nodes)];

      for (size_t k = 0; k < subSize; k++) {
        simdMemcpy(&buf[pos], &graphRow[k * nodes], subSize);
        pos += subSize;
      }
      rowIdx++;
    }

    // Scatters the matrix into sub matrix size subSize*subSize
    // Both the original matrix and subMatrices are 1D arrays for better cache
    // utilization
    MPI_Scatter(buf, subSize * subSize, MPI_INT, subMatrix, subSize * subSize,
                MPI_INT, rootRank, gridCommunicator);
    free(buf);
  } else
    MPI_Scatter(NULL, 0, MPI_INT, subMatrix, subSize * subSize, MPI_INT,
                rootRank, gridCommunicator);

  // Here we get the number of iterations we have to do to get a matrix with the
  // lowest cost for the node traversal
  size_t steps = 0;
  if (rank == 0) {
    size_t power = 1;
    while (power < nodes) {
      steps++;
      power *= 2;
    }
  }

  MPI_Bcast(&steps, 1, MPI_INT, rootRank, gridCommunicator);

  int *const restrict resultMatrix = alloc(subSize);
  int *const restrict localCopy = alloc(subSize);
  int *const restrict transposed = alloc(subSize);
  int *const restrict received = alloc(subSize);
  // The id of the process where we will send our subMatrix
  simdMemset(resultMatrix, INF, subSize * subSize);
  for (size_t i = 0; i < steps; i++) {
    simdMemcpy(localCopy, subMatrix, subSize * subSize);
    for (size_t foxSteps = 0; foxSteps < sqrtNumProcs; foxSteps++) {
      // Checks if we're the ones to send our local copy of the original matrix
      // to the other processes in our row
      if (cartRank == rowGroupRanks[initialIdx])
        simdMemcpy(received, localCopy, subSize * subSize);

      MPI_Bcast(received, subSize * subSize, MPI_INT, initialIdx,
                rowCommunicator);

      // Transposes the subMatrix for better cache locality
      // Now subMatrix is column major instead of row major, so when we loop
      // we can access each element one after the other instead of jumping to
      // subSize elements for the next element, improving cache utilization,
      // ergo code runs faster
      transpose(transposed, subMatrix, subSize);

      // Repeating Squared algorithm, the result is stored in resultMatrix which
      // we set all of its elements to INF every iteration of the outermost loop
      // because this function only changes the value of an element inside
      // resultMatrix if its lower than the current value, allowing for less
      // memsets on it

      matrixMultiply(resultMatrix, received, transposed, subSize);

      // Here we send our subMatrix to other and also receive it all in one
      // function
      MPI_Sendrecv_replace(subMatrix, subSize * subSize, MPI_INT, sendTo, 0,
                           receiveFrom, 0, columnCommunicator,
                           MPI_STATUS_IGNORE);

      // Get the id of the next process to it's subMatrix
      if (++initialIdx >= sqrtNumProcs)
        initialIdx -= sqrtNumProcs;
    }
    // This function copies the resultMatrix into subMatrix and also sets all
    // elements of resultMatrix to INF
    simdMemcpyAndSet(subMatrix, resultMatrix, INF, subSize * subSize);
  }
  free(transposed);
  free(received);
  free(localCopy);

  if (rank == 0) {
    int *restrict buf = alloc(nodes);
    MPI_Gather(subMatrix, subSize * subSize, MPI_INT, buf, subSize * subSize,
               MPI_INT, rootRank, gridCommunicator);
    size_t rowIdx = 0;
    size_t row = 0;
    // Theses loops are for loading the results from all threads to the main
    // thread

#pragma GCC ivdep
    for (size_t i = 0; i < numProcs; i++) {
      if (i != 0 && i % sqrtNumProcs == 0) {
        row += subSize;
        rowIdx = 0;
      }
      int *restrict graphRow = &graph[rowIdx * subSize + (row * nodes)];
      const int *restrict bufPos = &buf[i * subSize * subSize];
#pragma GCC ivdep
      for (size_t k = 0; k < subSize; k++)
        simdMemcpy(&graphRow[k * nodes], &bufPos[k * subSize], subSize);
      rowIdx++;
    }

    // for (size_t i = 0; i < nodes * nodes; i++) {
    //   if (i != 0 && i % nodes == 0)
    //     printf("\n");
    //
    //   printf("%d", (graph[i] == INF) ? 0 : graph[i]);
    //   if (i % nodes < nodes - 1)
    //     printf(" ");
    // }
    // printf("\n");

    free(graph);
    free(buf);
  } else
    MPI_Gather(subMatrix, subSize * subSize, MPI_INT, NULL, 0, MPI_INT,
               rootRank, gridCommunicator);

  const double endTime = MPI_Wtime();
  const double totalTime = endTime - startTime;
  free(subMatrix);
  free(resultMatrix);
  MPI_Comm_free(&rowCommunicator);
  MPI_Comm_free(&columnCommunicator);
  MPI_Comm_free(&gridCommunicator);
  MPI_Group_free(&worldGroup);
  MPI_Group_free(&rowGroup);
  MPI_Group_free(&columnGroup);

  if (rank == 0)
    printf("Execution time: %f seconds\n", totalTime);

  MPI_Finalize();

  return 0;
}
