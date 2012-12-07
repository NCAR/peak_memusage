/*
 * This is just a test, to do the first tests
 * on the actual job_memusage
 * $Id$
 *
 */

#include <stdio.h>	/* print */
#include <stdlib.h>	/* malloc */
#include <unistd.h>     /* optarg */

#ifdef COMPILE_MPI
#include <mpi.h>
#endif

#define MAX_SIZE 2147483647
#define DEFAULT_SIZE 1000000
#define MAX_REPEAT 100

void printUsage(char **argv) {
	printf("\nUsage:\n%s [-s size] [-r repetitions] [-d delay (ms)] \n\n", argv[0]);
	exit(1);
}

void parseInt(int *data, char *param) {
	if (param != NULL) 
		*data = atoi(param);
}

int main(int argc, char **argv) {
	int i, j, last, size = DEFAULT_SIZE, repeat = 1, delay=1000;
	int *spare_data, c, error, rank, poolsize;
	char *ch_size = NULL, *ch_repeat = NULL, *ch_delay=NULL, *ch_rank="thread"; 
	
#ifdef COMPILE_MPI
	if(error = MPI_Init(NULL, NULL)) {
                printf("MPI INIT error: %d", error);
                return 1;
        }
        if(error = MPI_Comm_rank(MPI_COMM_WORLD, &rank)) {
                printf("MPI RANK error: %d", error);
                return 1;
        }
        if(error = MPI_Comm_size(MPI_COMM_WORLD, &poolsize)) {
                printf("MPI SIZE error: %d", error);
                return 1;
        }
        ch_rank="task";
#endif

	while ((c = getopt (argc, argv, "s:r:d:")) != -1) {
		switch(c) {
		    case 's':
			ch_size = optarg;
			break;
		    case 'r':
			ch_repeat = optarg;
			break;
		    case 'd':
			ch_delay = optarg;
			break;
		    default:
			printUsage(argv);
		}
	}
	
	parseInt(&size, ch_size);
	parseInt(&repeat, ch_repeat);
	parseInt(&delay, ch_delay);
	if (size <= 0 || size > MAX_SIZE 
	||  repeat < 1 || repeat > MAX_REPEAT 
	||  delay < 0) {
		printf("size:%d, repeat:%d, delay:%f\n", size, repeat, delay);
		printUsage(argv);
	}

#ifdef COMPILE_OMP
    #pragma omp parallel default(none) private(rank, poolsize, i, j, last, spare_data) shared(size, delay, repeat, ch_rank)
    {
	rank = omp_get_thread_num();
	poolsize = omp_get_num_threads();
#elif !defined COMPILE_MPI
        rank = 0;
        poolsize = 1;
#endif
	for(i=0; i<repeat; i++) {
		last = size *(rank+1); /* every rank will have a different number */
                int amount = last*sizeof(int);
		printf("Allocating and using %d ints (%.2f MiB) in %s %d/%d\n", last, amount/1048576., ch_rank, rank, poolsize);

		spare_data = malloc(amount);
		for (j=0; j<last; j++)
			spare_data[j] = 24;
		usleep(delay * 1000);
	}
	
#ifdef COMPILE_MPI
	if(error = MPI_Finalize()) {
                printf("Finalize error: %d", error);
		return 1;
        }
#else
  #ifdef COMPILE_OMP
  }
  #endif
#endif
	
	return 0;
}
