#include <iostream>
#include <iomanip>
#include <cassert>
#include <vector>
#include <algorithm>
#include <iterator>
#include <unistd.h>
#include "mpi.h"



int main (int argc, char **argv)
{
  int numranks, rank;
  char hn[256];

  gethostname(hn, sizeof(hn) / sizeof(char));

  MPI_Init(&argc, &argv);

  MPI_Comm_size (MPI_COMM_WORLD, &numranks);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);

  assert (numranks%2 == 0);

  std::cout << "Hello from " << rank
            << " / " << std::string (hn)
	    << ", running " << argv[0] << " on "
	    << numranks << " ranks"
	    << std::endl;

  MPI_Barrier(MPI_COMM_WORLD);

  const std::size_t cnt = (rank < 2) ? 2e7 : 1e6; // ranks 0,1 trade more data

  std::vector<int> buf(cnt);

  if (0 == rank%2) // even sends
    {
      int i=rank;
      std::for_each(buf.begin(), buf.end(), [&i](int &b) { b=i++; });

      std::cout << "Rank " << rank << " sending [";
      auto end = buf.begin();
      std::advance(end,10);
      std::for_each(buf.begin(), end, [](const int &b) { std::cout << b << " "; });
      std::cout << "... ]" << std::endl;

      // First, matched send/recv
      {
        std::cout << "calling MPI_Send";
        MPI_Send (&buf[0], buf.size(), MPI_INT, /* dest = */ rank+1, /* tag = */ 100, MPI_COMM_WORLD);
        std::cout << "...done\n";
      }

      // Next, matched isend/recv
      {
        MPI_Request req;
        std::cout << "calling MPI_Isend";
        MPI_Isend (&buf[0], buf.size(), MPI_INT, /* dest = */ rank+1, /* tag = */ 200, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, MPI_STATUS_IGNORE);
        std::cout << "...done\n";
      }

      // Next, matched ssend/recv
      {
        std::cout << "calling MPI_Ssend";
        MPI_Ssend (&buf[0], buf.size(), MPI_INT, /* dest = */ rank+1, /* tag = */ 300, MPI_COMM_WORLD);
        std::cout << "...done\n";
      }

      // Next, matched isend/recv
      {
        MPI_Request req;
        std::cout << "calling MPI_Issend";
        MPI_Issend (&buf[0], buf.size(), MPI_INT, /* dest = */ rank+1, /* tag = */ 400, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, MPI_STATUS_IGNORE);
        std::cout << "...done\n";
      }
    }

  else // odd recvs
    {
      MPI_Recv (&buf[0], buf.size(), MPI_INT, /* dest = */ rank-1, /* tag = */ 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv (&buf[0], buf.size(), MPI_INT, /* dest = */ rank-1, /* tag = */ 200, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv (&buf[0], buf.size(), MPI_INT, /* dest = */ rank-1, /* tag = */ 300, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv (&buf[0], buf.size(), MPI_INT, /* dest = */ rank-1, /* tag = */ 400, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      int i=(rank-1);
      std::for_each(buf.cbegin(), buf.cend(), [&i](const int &b) { assert(b == i++); });

      std::cout << "Rank " << rank << " recvs [";
      auto end = buf.begin();
      std::advance(end,10);
      std::for_each(buf.begin(), end, [](const int &b) { std::cout << b << " "; });
      std::cout << "... ]" << std::endl;

    }

  MPI_Finalize();

  return 0;
}
