#ifndef __MPIHelper__HEADER__
#define __MPIHelper__HEADER__
#include <mpi.h>

static inline void receiveArray(const int subSize, int *restrict subMatrix,
                                const int source) {
  int position = 0;
  MPI_Status status;
  MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
  const int size = status._ucount;
  int buf[size];
  MPI_Recv(buf, size, MPI_PACKED, source, 0, MPI_COMM_WORLD, 0);
  for (int i = 0; i < subSize; i++)
    MPI_Unpack(buf, size, &position, &subMatrix[i * subSize], subSize, MPI_INT,
               MPI_COMM_WORLD);
}

static inline void sendArray(const int *restrict graphRow, const int subSize,
                             const int size, const int nodes,
                             const int destination) {
  int pos = 0;
  int buf[size];
  for (int k = 0; k < subSize; k++) {
    MPI_Pack(graphRow, subSize, MPI_INT, buf, size, &pos, MPI_COMM_WORLD);
    graphRow += nodes;
  }
  MPI_Send(buf, pos, MPI_PACKED, destination, 0, MPI_COMM_WORLD);
}

#endif // !MPIHelper.H
