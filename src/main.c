#include "./Misc/Misc.h"
#include "MPIHelper/MPIHelper.h"
#include "Math/Math.h"
#include <math.h>
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MASTER 0

int main(int argc, char *argv[]) {
  int numProcs, rank, subSize, nodes;
  long double startTime, endTime, totalTime;
  MPI_Request requests[2];

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);

  startTime = MPI_Wtime();
  const int sqrtNumProcs = (int)sqrt(numProcs);

  const int group = rank % sqrtNumProcs;
  MPI_Group worldGroup;
  MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);

  MPI_Group rowGroup;
  int rowGroupRanks[sqrtNumProcs];
  int columnGroupRanks[sqrtNumProcs];
  size_t initialIdx = -1, initialColumnIdx = rank;

  for (size_t i = 0; i < sqrtNumProcs; i++) {
    rowGroupRanks[i] = rank + i - group;
    columnGroupRanks[i] = (rank % sqrtNumProcs + i * sqrtNumProcs) % numProcs;
    if (rowGroupRanks[i] % sqrtNumProcs == rowGroupRanks[i] / sqrtNumProcs)
      initialIdx = i;
    if (columnGroupRanks[i] == rank)
      initialColumnIdx = i;
  }

  MPI_Group_incl(worldGroup, sqrtNumProcs, rowGroupRanks, &rowGroup);
  MPI_Comm rowCommunicator;
  MPI_Comm_create_group(MPI_COMM_WORLD, rowGroup, 0, &rowCommunicator);

  FILE *f = NULL;
  if (rank == 0) {
    f = fopen(argv[1], "r");
    fscanf(f, "%d", &nodes);
    if (nodes % sqrtNumProcs != 0) {
      printf(
          "Number of processes must evenly divide the number of nodes in the "
          "matrix\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
      return 1;
    }

    subSize = nodes / sqrtNumProcs;
  }
  MPI_Bcast(&subSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
  int *restrict subMatrix = alloc(subSize);
  int *restrict graph = NULL;

  if (rank == 0) {
    graph = alloc(nodes);
    int count = 0;
    for (size_t i = 0; i < nodes; i++) {
      int *restrict graphRow = &graph[i * nodes];
      for (size_t k = 0; k < nodes; k++) {
        if (fscanf(f, "%d", &graphRow[k]) != 1) {
          fprintf(stderr, "ERROR: scanf failed at [%d][%d]!\n", i, k);
          MPI_Abort(MPI_COMM_WORLD, 1);
          return 1;
        }
        count++;
        if (graphRow[k] == 0 && i != k)
          graphRow[k] = INT16_MAX;
        // printf("%d ", graph[i*size +k]);
        // printf("\n");
      }
      if (count < nodes) {
        fprintf(stderr, "Scan failed %d\n", count);
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
      }
    }
    fclose(f);

    int row = 0;
    int rowIdx = 1;
    for (size_t i = 1; i < numProcs; i++) {
      if (i % sqrtNumProcs == 0) {
        row += subSize;
        rowIdx = 0;
      }

      const int *const restrict graphRow =
          &graph[rowIdx * subSize + (row * nodes)];
      sendArray(graphRow, subSize, nodes, i, &requests[0]);
      rowIdx++;
    }

    for (size_t k = 0; k < subSize; k++)
      simdMemcpy(&subMatrix[k * subSize], &graph[k * nodes], subSize);

  } else
    receiveArray(subSize, subMatrix, 0, &requests[1]);
  MPI_Barrier(MPI_COMM_WORLD);

  int steps = 0;
  if (rank == 0) {
    int power = 1;
    while (power < nodes) {
      steps++;
      power *= 2;
    }
  }

  MPI_Bcast(&steps, 1, MPI_INT, 0, MPI_COMM_WORLD);

  int *const restrict resultMatrix = alloc(subSize);
  int *const restrict result = alloc(subSize);
  int *const restrict local = alloc(subSize);
  int *const restrict transposed = alloc(subSize);
  int *const restrict received = alloc(subSize);
  const int sendTo = (initialColumnIdx + sqrtNumProcs - 1) % sqrtNumProcs;
  const int receiveFrom = (initialColumnIdx + sqrtNumProcs + 1) % sqrtNumProcs;
  for (size_t i = 0; i < steps; i++) {
    simdMemcpy(local, subMatrix, subSize * subSize);
    simdMemset(result, -1, subSize * subSize);
    for (size_t foxSteps = 0; foxSteps < sqrtNumProcs; foxSteps++) {
      if (rank == rowGroupRanks[initialIdx])
        simdMemcpy(received, local, subSize * subSize);

      MPI_Bcast(received, subSize * subSize, MPI_INT, initialIdx,
                rowCommunicator);

      transpose(transposed, subMatrix, subSize);
      matrixMultiply(resultMatrix, received, transposed, subSize);
      MPI_Sendrecv_replace(subMatrix, subSize * subSize, MPI_INT,
                           columnGroupRanks[sendTo], 0,
                           columnGroupRanks[receiveFrom], 0, MPI_COMM_WORLD, 0);
#pragma GCC ivdep
      for (size_t idx = 0; idx < subSize * subSize; idx++)
        if (result[idx] == -1 || (long)(resultMatrix[idx]) < result[idx])
          result[idx] = resultMatrix[idx];

      initialIdx = (initialIdx + 1) % sqrtNumProcs;
    }
    simdMemcpy(subMatrix, result, subSize * subSize);
  }
  free(transposed);
  free(received);
  free(local);
  free(result);
  if (rank == 0) {
    int *restrict graphRow = graph;
    for (size_t k = 0; k < subSize; k++)
      simdMemcpy(&graphRow[k * nodes], &subMatrix[k * subSize], subSize);

    int rowIdx = 1;
    int row = 0;
    for (size_t i = 1; i < numProcs; i++) {
      if (i % sqrtNumProcs == 0) {
        row += subSize;
        rowIdx = 0;
      }
      graphRow = &graph[rowIdx * subSize + (row * nodes)];
      receiveArray(subSize, resultMatrix, i, &requests[1]);
      for (size_t k = 0; k < subSize; k++)
        simdMemcpy(&graphRow[k * nodes], &resultMatrix[k * subSize], subSize);
      rowIdx++;
    }

    for (size_t i = 0; i < nodes * nodes; i++) {
      if (i != 0 && i % nodes == 0)
        printf("\n");
      printf("%d ", (graph[i] == INT16_MAX) ? 0 : graph[i]);
    }
    printf("\n");

    free(graph);
  } else
    sendArrayBasic(subMatrix, subSize, 0, &requests[0]);

  MPI_Barrier(MPI_COMM_WORLD);
  endTime = MPI_Wtime();
  totalTime = endTime - startTime;
  free(subMatrix);
  free(resultMatrix);
  MPI_Comm_free(&rowCommunicator);
  MPI_Group_free(&worldGroup);
  MPI_Group_free(&rowGroup);

  if (rank == 0)
    printf("Execution time: %Lf seconds\n", totalTime);

  MPI_Finalize();

  return 0;
}
