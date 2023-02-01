
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include <stdio.h>	/* print */
#include <stdlib.h>	/* malloc */
#include <unistd.h>     /* optarg */
#include <stdatomic.h>
#include <dlfcn.h>


static void* (*real_malloc)(size_t) = NULL;



__attribute__ ((constructor))
__attribute__ ((visibility ("hidden")))
void init ()
{
  fprintf(stderr, "..(constructor)... %s, line: %d\n", __FILE__, __LINE__);
  return;
}



static void mtrace_init (void)
{
  fprintf(stderr, "..(init)... %s, line: %d\n", __FILE__, __LINE__);
  real_malloc = dlsym(RTLD_NEXT, "malloc");

  if (NULL == real_malloc) {
    fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
  }
}



void *malloc (size_t size)
{
  if(real_malloc==NULL) {
    mtrace_init();
  }

  void *p = NULL;
  fprintf(stderr, "malloc(%ld) = ", size);
  p = real_malloc(size);
  fprintf(stderr, "%p\n", p);
  return p;
}
