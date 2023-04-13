#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#define _GNU_SOURCE
#include <stddef.h>
#include <string.h>     /* memset */
#include <stdio.h>	/* print */
#include <stdlib.h>	/* malloc */
#include <unistd.h>     /* optarg */
#include <stdatomic.h>
#include <dlfcn.h>
#include <stdbool.h>
#ifdef HAVE_MALLOC_H
#  include <malloc.h>
#elif HAVE_MALLOC_MALLOC_H
#  include <malloc/malloc.h>
#endif
//#include <pthread.h>

#include "log_memusage.h"
#include "log_memusage_impl.h"



static atomic_int_least64_t
  malloced_bytes=0,       malloc_calls=0,
  calloc_calls=0,         calloced_bytes=0,
  realloced_bytes=0,      realloc_calls=0,
  reallocarrayed_bytes=0, reallocarray_calls=0,
  freed_bytes=0,          free_calls=0,
  current_bytes=0,
  max_bytes=0;

static void * (*real_malloc)       (size_t) = NULL;
static void * (*real_calloc)       (size_t count, size_t size) = NULL;
static void * (*real_realloc)      (void *ptr, size_t size) = NULL;;
static void * (*real_reallocarray) (void *ptr, size_t nmemb, size_t size) = NULL;
static void   (*real_free)         (void*)  = NULL;


//static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;



__attribute__ ((visibility ("hidden")))
size_t my_malloc_size (void *p)
{
#ifdef HAVE_MALLOC_USABLE_SIZE

  return malloc_usable_size(p);

#elif HAVE_MALLOC_SIZE

  return malloc_size(p);

#else
#  error "No suitable malloc_usable_size()!!"
#endif
  return 0;
}



static void mtrace_init ()
{
  //fprintf(stderr, "..(init)... %s, line: %d\n", __FILE__, __LINE__);

  real_malloc = dlsym(RTLD_NEXT, "malloc");

  if (NULL == real_malloc)
    {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
      abort();
    }

  real_calloc = dlsym(RTLD_NEXT, "calloc");

  if (NULL == real_calloc)
    {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
      abort();
    }

  real_realloc = dlsym(RTLD_NEXT, "realloc");

  if (NULL == real_realloc)
    {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
      abort();
    }

  real_reallocarray = dlsym(RTLD_NEXT, "reallocarray");

  if (NULL == real_reallocarray)
    {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
      abort();
    }

  real_free   = dlsym(RTLD_NEXT, "free");

  if (NULL == real_free)
    {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
      abort();
    }
}



void *malloc (size_t size)
{
  size_t alloced_size=0;

  //pthread_mutex_lock(&lock);

  if (NULL == real_malloc)
    mtrace_init();

  void *ptr = real_malloc(size);

  alloced_size = my_malloc_size(ptr);

  /* update atomics */
  malloc_calls++;
  malloced_bytes += alloced_size;
  current_bytes  += alloced_size;

  if (current_bytes > max_bytes) max_bytes = current_bytes;

  //fprintf(stderr, "Requested %p size %ld, used space %ld, overhead %ld, total current=%ld (MB)\n", ptr, size, alloced_size, alloced_size - size, current_bytes/1024/1024);

  //pthread_mutex_unlock(&lock);

  return ptr;
}



void *calloc (size_t nitems, size_t size)
{
  size_t alloced_size=0;

  if (NULL == real_calloc)
    mtrace_init();

  void *ptr = real_calloc(nitems,size);

  alloced_size = my_malloc_size(ptr);

  /* update atomics */
  calloc_calls++;
  calloced_bytes += alloced_size;
  current_bytes  += alloced_size;

  if (current_bytes > max_bytes) max_bytes = current_bytes;

  return ptr;
}



void *realloc (void *ptr, size_t size)
{
  size_t old_size=0, new_size=0;

  if (NULL == real_realloc)
    mtrace_init();

  old_size = my_malloc_size(ptr);

  ptr = real_realloc(ptr,size);

  new_size = my_malloc_size(ptr);

  /* update atomics */
  realloc_calls++;
  if (new_size > old_size)
    {
      realloced_bytes += (new_size - old_size);
      current_bytes   += (new_size - old_size);
    }
  else
    {
      current_bytes   -= (old_size - new_size);
    }

  if (current_bytes > max_bytes) max_bytes = current_bytes;

  return ptr;
}



void *reallocarray (void *ptr, size_t nitems, size_t size)
{
  size_t old_size=0, new_size=0;

  if (NULL == real_reallocarray)
    mtrace_init();

  old_size = my_malloc_size(ptr);

  ptr = real_reallocarray(ptr,nitems,size);

  new_size = my_malloc_size(ptr);

  /* update atomics */
  reallocarray_calls++;
  if (new_size > old_size)
    {
      reallocarrayed_bytes += (new_size - old_size);
      current_bytes        += (new_size - old_size);
    }
  else
    current_bytes          -= (old_size - new_size);

  if (current_bytes > max_bytes) max_bytes = current_bytes;

  return ptr;
}



void free (void *buf)
{
  size_t freed_size=0;

  if (NULL == real_free)
    mtrace_init();

  if (NULL == buf) return;

  freed_size = my_malloc_size(buf);

  /* update atomics */
  free_calls++;
  freed_bytes   += freed_size;
  current_bytes -= freed_size;

  //fprintf(stderr, "free(%p), %d\n", buf, free_calls);
  real_free(buf);
}



void log_memusage_malloc_report (const char* prefix)
{
  fprintf(stderr, "%smalloc: %lld bytes, %lld calls\n", prefix, malloced_bytes, malloc_calls);
  if (calloc_calls)  fprintf(stderr, "%scalloc: %lld bytes, %lld calls\n", prefix, calloced_bytes, calloc_calls);
  if (realloc_calls) fprintf(stderr, "%srealloc: %lld bytes, %lld calls\n", prefix, realloced_bytes, realloc_calls);
  if (reallocarray_calls) fprintf(stderr, "%sreallocarray: %lld bytes, %lld calls\n", prefix, reallocarrayed_bytes, reallocarray_calls);
  fprintf(stderr, "%stotal allocations: %lld bytes, %lld calls\n", prefix,
          malloced_bytes + calloced_bytes + realloced_bytes + reallocarrayed_bytes,
          malloc_calls + calloc_calls + realloc_calls + reallocarray_calls);
  fprintf(stderr, "%sfree: %lld bytes, %lld calls\n", prefix, freed_bytes,    free_calls);
  //fprintf(stderr, "%sremaining: %lld bytes\n", prefix, malloced_bytes - freed_bytes);
  fprintf(stderr, "%shigh water allocation: %lld (MB)\n",   prefix, max_bytes/1024/1024);
}



__attribute__ ((constructor))
__attribute__ ((visibility ("hidden")))
void initialize ()
{
  //fprintf(stderr, "..(constructor)... %s, line: %d\n", __FILE__, __LINE__);
  //pthread_mutex_init(&lock, NULL);
  return;
}



__attribute__ ((destructor))
__attribute__ ((visibility ("hidden")))
void finalize ()
{
  //fprintf(stderr, "..(destructor)... %s, line: %d\n", __FILE__, __LINE__);
  log_memusage_malloc_report(/* prefix = */ LOG_MEMUSAGE_LOGGING_PREFIX);
  //pthread_mutex_destroy(&lock);
}
