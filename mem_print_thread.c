/* Includes */
/* #include <unistd.h>     / * Symbolic Constants * /
#include <sys/types.h>  / * Primitive System Data Types * / 
#include <errno.h>      / * Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */

#include <sys/wait.h>   /* Wait for Process Termination */
/* #include <time.h>	/ * time and difftime */
#include <sys/resource.h> /* getrusage */
#include <pthread.h>

int exit_status, thread_running=1;
char *nomefilerun = NULL;
time_t start_time;

void printWarning() {
	printf("\nWARNING: this software runs arbitrary code.\nUnsafe for web usage\n\n");
}

void printUsage(char **argv) {
	printf("\nUsage:\n%s [-d delay (milliseconds) ] [-f nomefile-to-save] -r nomefile-to-run \n\n", argv[0]);
	printWarning();
	exit(1);
}

void *runInThread(void *arg) {
	exit_status=system(nomefilerun);
	thread_running=0;
	return NULL;
}

int main(int argc, char **argv) {
	pthread_t *thread;
	pthread_attr_t pthread_attr;
	char *nomefilesave;
	FILE *myOutputFile;
	struct rusage64 us;	/* resource us struct */
	char *chdelay = NULL;
	int c, delay = 1000;	/* default = 1000ms */
	nomefilesave = "memory.status";

	while ((c = getopt (argc, argv, "d:r:f:")) != -1) {
		switch(c) {
		    case 'd':
			chdelay = optarg;
			break;
		    case 'r':
			nomefilerun = optarg;
			break;
		    case 'f':
			nomefilesave = optarg;
			break;
		    default:
			printUsage(argv);
		}
	}
	
	if (nomefilerun == NULL)
		printUsage(argv);
	
	if (chdelay != NULL) {
		delay = atoi(chdelay);
		if (delay == 0)
			printUsage(argv);
	}

	printWarning();

/*
	int status;     	// parent process: child's exit status
	pid_t childpid; 	// variable to store the child's pid 
	childpid = fork(); // now create new process
	if (childpid >= 0) { // fork succeeded
		if (childpid == 0) { // fork() returns 0 to the child process 
			exit(system(nomefilerun));
			// child exits with "nomefilerun" return code 
		} else { // fork() returns new pid to the parent process
			wait3(&status, WNOHANG, &us); // check if child exited, with usuage
			if WIFEXITED(status) {
				printf("Process %s terminated with exit code: %d\n", nomefilerun, WEXITSTATUS(status));
				break;
			}
		
			/ * this exit always, but it should not! 
			if WIFSIGNALED(status) {
				printf("Process %s abnormally terminated\n", nomefilerun);
				break;
			}
			* /
		}
	} else { // fork returns -1 on failure 
		perror("fork"); // display error message 
		exit(1); 
	}
		
*/

/*	the following does not work
	pthread_attr_init(&pthread_attr);
	pthread_create(thread, &pthread_attr, runInThread, NULL);
*/	pthread_create(thread, NULL, runInThread, NULL);
/*	
	if ((myOutputFile=fopen(nomefilesave, "w+")) == NULL) { 
		printf("Cannot open file %s for writing.\n", nomefilesave);
		return 1;
	}
*/
	myOutputFile = stdout;
	fprintf(myOutputFile, "\nProcess %s started!\n", nomefilerun);
	fprintf(myOutputFile, "Memory usage for:\n%12s %12s %12s %12s %12s\n",
		"Time(s)", "Resident", "Code", "Allocated", "Stack");
	
	start_time = time(NULL);
	while(thread_running)
	{
		getrusage(RUSAGE_SELF, &us);
		fprintf(myOutputFile, "%e %12d %12d %12d %12d\n",
			difftime(time(NULL), start_time), 
			us.ru_maxrss, us.ru_ixrss, us.ru_idrss, us.ru_isrss);
		usleep(delay*1000); /* usleep is in microseconds */
	}
	
	printf("Process terminated (see %s for details)\n", nomefilesave);
/*	the following does not work (or the next must be removed)
	printf("Process %s terminated (see %s for details)\n", nomefilerun, nomefilesave); */
	fprintf(myOutputFile, "\nProcess %s terminated with exit code: %d\n", 
			nomefilerun, WEXITSTATUS(exit_status));
	fclose(myOutputFile);
	
	return 0;
}
