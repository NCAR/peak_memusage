/*

$Id$

*/

#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>     

int spare_data2[1000*1000*10];

void process(struct rusage *p, char *when)
{
	printf("%s\n", when);
	printf(" /* user time used */                   %8d  %8d\n",  p->ru_utime.tv_sec,p->ru_utime.tv_usec   );
	printf(" /* system time used */                 %8d  %8d\n",  p->ru_stime.tv_sec,p->ru_stime.tv_usec   );
	printf(" /* integral max memory size */      %8d\n",  p->ru_maxrss           );
	printf(" /* integral shared memory size */      %8d\n",  p->ru_ixrss           );
	printf(" /* integral unshared data  */          %8d\n",  p->ru_idrss           );
	printf(" /* integral unshared stack  */         %8d\n",  p->ru_isrss           );
	printf(" /* page reclaims */                    %8d\n",  p->ru_minflt          );
	printf(" /* page faults */                      %8d\n",  p->ru_majflt          );
	printf(" /* swaps */                            %8d\n",  p->ru_nswap           );
	printf(" /* block input operations */           %8d\n",  p->ru_inblock         );
	printf(" /* block output operations */          %8d\n",  p->ru_oublock         );
	printf(" /* messages sent */                    %8d\n",  p->ru_msgsnd          );
	printf(" /* messages received */                %8d\n",  p->ru_msgrcv          );
	printf(" /* signals received */                 %8d\n",  p->ru_nsignals        );
	printf(" /* voluntary context switches */       %8d\n",  p->ru_nvcsw           );
	printf(" /* involuntary  */                     %8d\n",  p->ru_nivcsw          );

}


int main()
{
	  int ret;
	  char *buf;
	  int i=0;
	  int who= RUSAGE_SELF;
	  struct rusage usage;
	  struct rusage *p=&usage;
	  int last = 1000*1000*100;
	  int *spare_data;
	  

	  ret=getrusage(who,p);
	  process(p, "-------------before");
	  
	  spare_data = malloc(last*sizeof(int));
	  spare_data[last-1] = 24;
	  /* last *= 10; */
	  
          ret=getrusage(who,p);
	  process(p, "\n\n-------------after we run foo1");    

	return 0;
}
