#ifndef __MPIHelper__HEADER__
#define __MPIHelper__HEADER__
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

static inline void receiveArray(const int subSize, int *restrict subMatrix,
                                const int source, MPI_Request *request) {
  int position = 0;
  int size;
  MPI_Pack_size(subSize * subSize, MPI_INT, MPI_COMM_WORLD, &size);
  int *restrict buf = malloc(size);
  MPI_Irecv(buf, size, MPI_PACKED, source, 0, MPI_COMM_WORLD, request);
  MPI_Wait(request, MPI_STATUS_IGNORE);
  MPI_Unpack(buf, size, &position, subMatrix, subSize * subSize, MPI_INT,
             MPI_COMM_WORLD);
  free(buf);
}

static inline void sendArray(const int *const restrict graphRow,
                             const int subSize, const int nodes,
                             const int destination, MPI_Request *request) {
  int pos = 0;
  int size = 0;
  MPI_Pack_size(subSize * subSize, MPI_INT, MPI_COMM_WORLD, &size);
  int *restrict buf = malloc(size);

  for (size_t k = 0; k < subSize; k++)
    MPI_Pack(&graphRow[k * nodes], subSize, MPI_INT, buf, size, &pos,
             MPI_COMM_WORLD);

  // #pragma GCC ivdep
  //   for (size_t k = 0; k < subSize; k++)
  //     MPI_Pack(&graphRow[k * nodes], subSize, MPI_INT, buf, size, &pos,
  //              MPI_COMM_WORLD);

  MPI_Isend(buf, pos, MPI_PACKED, destination, 0, MPI_COMM_WORLD, request);
  MPI_Wait(request, MPI_STATUS_IGNORE);
  free(buf);
}

static inline void sendArrayBasic(const int *const restrict graphRow,
                                  const int subSize, const int destination,
                                  MPI_Request *request) {
  int pos = 0;

  int size;
  MPI_Pack_size(subSize * subSize, MPI_INT, MPI_COMM_WORLD, &size);
  int *restrict buf = malloc(size);
  MPI_Pack(graphRow, subSize * subSize, MPI_INT, buf, size, &pos,
           MPI_COMM_WORLD);
  MPI_Isend(buf, pos, MPI_PACKED, destination, 0, MPI_COMM_WORLD, request);
  MPI_Wait(request, MPI_STATUS_IGNORE);
  free(buf);
}
#endif // !MPIHelper.H
