#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAYSIZE 160
#define MASTER 0

float data[ARRAYSIZE];
float do_work(int rank, int offset, int chunksize);

int main(int argc, char *argv[]) {
  int numprocs, rank, tag1, tag2, src, dest, offset, chunksize, i, j;
  float mysum, sum;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (numprocs % 4 != 0) {
    MPI_Finalize();
    if (rank == 0)
      printf("Number of procs must be divisible by 4!\n");
    exit(0);
  }

  printf("Proc %d starting...\n", rank);
  MPI_Barrier(MPI_COMM_WORLD);
  chunksize = (ARRAYSIZE / numprocs);
  tag1 = 1;
  tag2 = 2;
  if (rank == MASTER) {
    sum = 0;
    for (i = 0; i < ARRAYSIZE; i++) {
      data[i] = i * 1.0;
      sum = sum + data[i];
    }
    printf("Initial sum = %f\n", sum);
    offset = chunksize;
    for (dest = 1; dest < numprocs; dest++) {
      MPI_Send(&offset, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD);
      MPI_Send(&data[offset], chunksize, MPI_FLOAT, dest, tag2, MPI_COMM_WORLD);
      printf("Sent %d elements to proc %d offset %d\n", chunksize, dest,
             offset);
      offset = offset + chunksize;
    }
    offset = 0;
    mysum = do_work(rank, offset, chunksize);
    sum = mysum;
    for (src = 1; src < numprocs; src++) {
      MPI_Recv(&offset, 1, MPI_INT, src, tag1, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      MPI_Recv(&data[offset], chunksize, MPI_FLOAT, src, tag2, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      MPI_Recv(&mysum, 1, MPI_FLOAT, src, tag2, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      sum += mysum;
    }
    printf("Final sum = %f\n", sum);
    printf("\nFinal results\n");
    offset = 0;
    for (i = 0; i < numprocs; i++) {
      for (j = 0; j < chunksize; j++)
        printf("%f ", data[offset + j]);
      offset = offset + chunksize;
    }
    printf("\n");
  } else {
    MPI_Recv(&offset, 1, MPI_INT, MASTER, tag1, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    MPI_Recv(&data[offset], chunksize, MPI_FLOAT, MASTER, tag2, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    mysum = do_work(rank, offset, chunksize);
    MPI_Send(&offset, 1, MPI_INT, MASTER, tag1, MPI_COMM_WORLD);
    MPI_Send(&data[offset], chunksize, MPI_FLOAT, MASTER, tag2, MPI_COMM_WORLD);
    MPI_Reduce(&mysum, &sum, 1, MPI_FLOAT, MPI_SUM, MASTER, MPI_COMM_WORLD);
  }

  MPI_Finalize();
}

float do_work(int rank, int offset, int chunksize) {
  float sum;
  int i;

  sum = 0;
  for (i = offset; i < offset + chunksize; i++) {
    data[i] = data[i] * 2;
    sum = sum + data[i];
  }
  printf("Proc %d partial sum = %f\n", rank, sum);
  return sum;
}
