#include "../src/Misc/Misc.h"
//
#include "../src/Math/Math.h"
#include <bits/time.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MASTER 0

int main(int argc, char *argv[]) {
  int nodes;
  // MPI_Request requests[2];
  struct timespec start, end;
  // Read the amount of nodes
  fscanf(stdin, "%d", &nodes);

  // Broadcasts the node of the subMatrix to every process
  unsigned short *restrict subMatrix = alloc(nodes);
  unsigned short *restrict graph = NULL;

  graph = alloc(nodes);
  // Reads the nodes
  for (size_t i = 0; i < nodes; i++) {
    // Here we see the first example of using a variable to point to a
    // specific part of a matrix, because we are using 1D arrays as the
    // representation, not only does this help us work with them by improving
    // readability but also improves compiler auto-vectorization

    unsigned short *restrict graphRow = &graph[i * nodes];
    for (size_t k = 0; k < nodes; k++) {
      if (fscanf(stdin, "%hd", &graphRow[k]) != 1) {
        fprintf(stderr, "ERROR: scanf failed at [%zu][%zu]!\n", i, k);
        return 1;
      }
      if (graphRow[k] == 0 && i != k)
        graphRow[k] = INF;
    }
  }

  // Here we get the number of iterations we have to do to get a matrix with the
  // lowest cost for the node traversal
  clock_gettime(CLOCK_MONOTONIC, &start);
  size_t steps = 0;
  size_t power = 1;
  while (power < nodes) {
    steps++;
    power *= 2;
  }

  unsigned short *const restrict resultMatrix = alloc(nodes);
  unsigned short *const restrict transposed = alloc(nodes);
  unsigned short *const restrict received = alloc(nodes);
  for (size_t i = 0; i < steps; i++) {
    simdMemset(resultMatrix, INF, nodes * nodes);

    transpose(transposed, graph, nodes);

    matrixMultiply(resultMatrix, graph, transposed, nodes);

    simdMemcpyAndSet(graph, resultMatrix, INF, nodes * nodes);
  }
  free(transposed);
  free(received);

  // for (size_t i = 0; i < nodes * nodes; i++) {
  //   if (i != 0 && i % nodes == 0)
  //     printf("\n");
  //
  //   printf("%hd", (graph[i] == INF) ? 0 : graph[i]);
  //   if (i % nodes < nodes - 1)
  //     printf(" ");
  // }
  // printf("\n");

  free(graph);

  clock_gettime(CLOCK_MONOTONIC, &end);
  const double totalTime = (double)(end.tv_sec - start.tv_sec) +
                           (double)(end.tv_nsec - start.tv_nsec) / 1e9;
  printf("Execution time: %f seconds\n", totalTime);

  return 0;
}
