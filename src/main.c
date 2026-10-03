#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MASTER 0

int main(int argc, char *argv[]) {
  int numprocs, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int nodes = 0;
  scanf("%d", &nodes);
  int graph[nodes][nodes];
  for (int i = 0; i < nodes; i++)
    for (int k = 0; k < nodes; k++) {
      scanf("%d", &graph[i][k]);
    }

  for (int i = 0; i < nodes; i++) {
    for (int k = 0; k < nodes; k++) {
      printf("%d", graph[i][k]);
    }
    printf("\n");
  }
  MPI_Finalize();
}
