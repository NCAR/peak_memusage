#include "mpi.h"

int main (int argc, char **argv)
{
  int numprocs, rank;
  MPI_Init(&argc, &argv);

  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);

  MPI_Finalize();

  return 0;
}
