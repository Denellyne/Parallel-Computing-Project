#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MASTER 0

static inline int *alloc(const int size) {
  return malloc(sizeof(int) * (size * size));
}
static inline void transpose(int *restrict matrix,
                             const int *const restrict graph, const int size) {
  for (int i = 0; i < size; i++) {
    int *restrict mRow = &matrix[i * size];
    for (int j = 0; j < size; j++)
      mRow[j] = graph[j * size + i];
  }
}

static inline void matrixMultiply(int *restrict matrix,
                                  const int *const restrict trans,
                                  const int *const restrict graph,
                                  const int size) {
  for (int row = 0; row < size; row++) {
    int *restrict mRow = &matrix[row * size];
    const int *restrict graphRow = &graph[row * size];

    for (int col = 0; col < size; col++) {
      const int *restrict transRow = &trans[col * size];
      int minCost = INT32_MAX;

#pragma GCC ivdep
      for (int k = 0; k < size; k++) {
        const int graphCost = graphRow[k];
        const int transCost = transRow[k];
        if ((unsigned long long)(graphCost + transCost) < minCost)
          minCost = (graphCost + transCost);
      }
      mRow[col] = minCost;
    }
  }
}

int main(int argc, char *argv[]) {
  int numprocs, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int nodes = 0;
  scanf("%d", &nodes);
  int *restrict graph = alloc(nodes);
  for (int i = 0; i < nodes; i++) {
    int *restrict graphRow = &graph[i * nodes];
    for (int k = 0; k < nodes; k++) {
      scanf("%d", &graphRow[k]);
      if (graphRow[k] == 0 && i != k)
        graphRow[k] = INT32_MAX;
      // printf("%d ", graph[i*size +k]);

      // printf("\n");
    }
  }
  int steps = 0;
  int power = 1;
  while (power < nodes) {
    steps++;
    power *= 2;
  }
  // printf("\n");

  int *restrict matrix = alloc(nodes);
  int *restrict trans = alloc(nodes);

  for (int i = 0; i < steps; i++) {
    transpose(trans, graph, nodes);
    matrixMultiply(matrix, trans, graph, nodes);
    int *const tmp = graph;
    graph = matrix;
    matrix = tmp;
    // for (int i = 0; i < nodes; i++) {
    //   for (int k = 0; k < nodes; k++) {
    //     printf("%d ", (graph[i*size +k] == INT32_MAX) ? 0 : graph[i*size
    //     +k]);
    //   }
    //   printf("\n");
    // }
    // printf("\n");
  }

  for (int i = 0; i < nodes; i++) {
    const int *const restrict graphRow = &graph[i * nodes];
    for (int k = 0; k < nodes; k++)
      printf("%d ", (graphRow[k] == INT32_MAX) ? 0 : graphRow[k]);

    printf("\n");
  }

  free(matrix);
  free(trans);
  free(graph);

  MPI_Finalize();

  return 0;
}
