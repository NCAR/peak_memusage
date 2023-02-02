#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#define _GNU_SOURCE
#include <stdio.h>	/* print */
#include <stdlib.h>	/* malloc */
#include <unistd.h>     /* optarg */
#include <stdatomic.h>
#include <dlfcn.h>


static void* (*real_malloc) (size_t) = NULL;
static void  (*real_free)   (void*)  = NULL;



static void mtrace_init (void)
{
  fprintf(stderr, "..(init)... %s, line: %d\n", __FILE__, __LINE__);

  real_malloc = dlsym(RTLD_NEXT, "malloc");
  real_free   = dlsym(RTLD_NEXT, "free");

  if (NULL == real_malloc)
    {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
}



__attribute__ ((constructor))
__attribute__ ((visibility ("hidden")))
void init ()
{
  fprintf(stderr, "..(constructor)... %s, line: %d\n", __FILE__, __LINE__);

  mtrace_init();
  return;
}



void *malloc (size_t size)
{
  if (NULL == real_malloc)
    {
      mtrace_init();
    }

  void *p = NULL;
  fprintf(stderr, "malloc(%ld) = ", size);
  p = real_malloc(size);
  fprintf(stderr, "%p\n", p);
  return p;
}



void free (void *buf)
{
  if (NULL == real_free)
    mtrace_init();

  fprintf(stderr, "free(%p)\n", buf);
  real_free(buf);
}
