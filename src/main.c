#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MASTER 0

void freeArr(int **ptr, const int size) {
  for (int i = 0; i < size; i++) {
    free(ptr[i]);
  }
  free(ptr);
}

int **alloc(int size) {
  int **matrix = malloc(sizeof(int *) * size);
  for (int i = 0; i < size; i++) {
    matrix[i] = malloc(sizeof(int) * size);
  }
  return matrix;
}
int **transpose(int **arr, const int size) {
  int **matrix = alloc(size);
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++) {
      matrix[i][j] = arr[j][i];
    }
  return matrix;
}

int **matrixMultiply(int **arr, const int size) {
  int **matrix = alloc(size);
  int **trans = transpose(arr, size);
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (i == j) {
        matrix[i][j] = 0;
        continue;
      }

      matrix[i][j] = INT32_MAX;
      for (int k = 0; k < size; k++) {
        const int v1 = matrix[i][j];
        const int v2 = (arr[i][k] != INT32_MAX && trans[j][k] != INT32_MAX)
                           ? (arr[i][k] + trans[j][k])
                           : INT32_MAX;
        matrix[i][j] = v1 * (v1 < v2) + v2 * (v1 >= v2);
      }
    }
  }
  freeArr(trans, size);

  return matrix;
}

int main(int argc, char *argv[]) {
  int numprocs, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int nodes = 0;
  scanf("%d", &nodes);
  int **graph = alloc(nodes);
  for (int i = 0; i < nodes; i++) {
    for (int k = 0; k < nodes; k++) {
      scanf("%d", &graph[i][k]);
      if (graph[i][k] == 0 && i != k)
        graph[i][k] = INT32_MAX;
      // printf("%d ", graph[i][k]);
    }
    // printf("\n");
  }
  int steps = 0;
  int power = 1;
  while (power < nodes) {
    steps++;
    power *= 2;
  }
  // printf("\n");

  for (int i = 0; i < steps; i++) {
    int **m = matrixMultiply(graph, nodes);
    freeArr(graph, nodes);
    graph = m;
    // for (int i = 0; i < nodes; i++) {
    //   for (int k = 0; k < nodes; k++) {
    //     printf("%d ", (graph[i][k] == INT32_MAX) ? 0 : graph[i][k]);
    //   }
    //   printf("\n");
    // }
    // printf("\n");
  }
  for (int i = 0; i < nodes; i++) {
    for (int k = 0; k < nodes; k++) {
      printf("%d ", (graph[i][k] == INT32_MAX) ? 0 : graph[i][k]);
    }
    printf("\n");
  }
  freeArr(graph, nodes);

  MPI_Finalize();
}
