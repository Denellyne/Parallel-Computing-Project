#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MASTER 0

static inline int *alloc(int size) {
  return malloc(sizeof(int) * (size * size));
}
static inline void transpose(int *matrix, const int *arr, const int size) {
  for (int i = 0; i < size; i++) {
    int *mRow = &matrix[i * size];
    for (int j = 0; j < size; j++)
      mRow[j] = arr[j * size + i];
  }
}

static inline int *matrixMultiply(int *matrix, const int *trans, const int *arr,
                                  const int size) {
  for (int i = 0; i < size; i++) {
    int *mRow = &matrix[i * size];
    const int *arrRow = &arr[i * size];

    for (int j = 0; j < size; j++) {
      const int *transRow = &trans[j * size];
      int *m = &mRow[j];
      if (i == j)
        *m = 0;
      else {
        *m = INT32_MAX;
        for (int k = 0; k < size; k++) {
          const int v1 = *m;
          const int v2 = (arrRow[k] != INT32_MAX && transRow[k] != INT32_MAX)
                             ? (arrRow[k] + transRow[k])
                             : INT32_MAX;
          *m = v1 * (v1 < v2) + v2 * (v1 >= v2);
        }
      }
    }
  }

  return matrix;
}

int main(int argc, char *argv[]) {
  int numprocs, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int nodes = 0;
  scanf("%d", &nodes);
  int *graph = alloc(nodes);
  for (int i = 0; i < nodes; i++) {
    int *graphRow = &graph[i * nodes];
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

  int *matrix = alloc(nodes);
  int *trans = alloc(nodes);
  for (int i = 0; i < steps; i++) {
    transpose(trans, graph, nodes);
    matrixMultiply(matrix, trans, graph, nodes);
    int *tmp = graph;
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
    const int *graphRow = &graph[i * nodes];
    for (int k = 0; k < nodes; k++)
      printf("%d ", (graphRow[k] == INT32_MAX) ? 0 : graphRow[k]);

    printf("\n");
  }

  free(matrix);
  free(trans);
  free(graph);

  MPI_Finalize();
}
