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


/* #if defined(HAVE_MALLINFO2) || defined(HAVE_MALLINFO) */

/* struct mallinfo my_mallinfo () */
/* { */
/* #ifdef HAVE_MALLINFO2 */

/*   return mallinfo2(); */

/* #elif HAVE_MALLINFO */

/*   return mallinfo(); */

/* #else */
/* #  error "No suitable mallinfo()!!" */
/* #endif */
/* } */
/* #endif */



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


static void* (*real_malloc) (size_t) = NULL;
//static void* (*real_calloc) (size_t nitems, size_t size) = NULL;
static void  (*real_free)   (void*)  = NULL;


static atomic_int_least64_t malloced_bytes=0, malloc_calls=0, calloc_calls=0, freed_bytes=0, free_calls=0, current_bytes=0, max_bytes=0;

//static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;


/* static void display_mallinfo(void) */
/* { */
/*   struct mallinfo mi; */

/*   mi = my_mallinfo(); */

/*   fprintf(stderr, "\n"); */
/*   fprintf(stderr, "# of free chunks (ordblks):               %d\n", mi.ordblks); */
/*   fprintf(stderr, "# of free fastbin blocks (smblks):        %d\n", mi.smblks); */
/*   fprintf(stderr, "# of mapped regions (hblks):              %d\n", mi.hblks); */
/*   fprintf(stderr, "Bytes allocated by means other than mmap: %d\n", mi.arena); */
/*   fprintf(stderr, "Bytes in mapped regions (hblkhd):         %d\n", mi.hblkhd); */
/*   /\* fprintf(stderr, "Max. total allocated space (usmblks):  %d\n", mi.usmblks); *\/ */
/*   fprintf(stderr, "Free bytes held in fastbins (fsmblks):    %d\n", mi.fsmblks); */
/*   fprintf(stderr, "Total allocated space (uordblks):         %d\n", mi.uordblks); */
/*   fprintf(stderr, "Total free space (fordblks):              %d\n", mi.fordblks); */
/*   fprintf(stderr, "Topmost releasable block (keepcost):      %d\n", mi.keepcost); */
/*   fprintf(stderr, "\n"); */
/* } */


/* https://stackoverflow.com/questions/24509509/how-to-get-the-size-of-memory-pointed-by-a-pointer */
/* Form a prefix data type that can hold the `size` and preserves alignment. */
/* It is not specified which is type is wider, so use a union to allocate the widest. */
union my2_size
{
  size_t size;
  max_align_t a;
};



 /* Return how many bytes are pointed to by a pointer allocated with my2_alloc() */
__attribute__ ((visibility ("hidden")))
size_t my2_size(void *buf)
{
  if (buf)
    {
      union my2_size *ptr = buf;
      ptr--;
      return ptr->size;
    }
  return 0;
}



static void mtrace_init (void)
{
  //fprintf(stderr, "..(init)... %s, line: %d\n", __FILE__, __LINE__);

  real_malloc = dlsym(RTLD_NEXT, "malloc");

  if (NULL == real_malloc)
    {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
      abort();
    }

  /* real_calloc = dlsym(RTLD_NEXT, "calloc"); */

  /* if (NULL == real_calloc) */
  /*   { */
  /*     fprintf(stderr, "Error in `dlsym`: %s\n", dlerror()); */
  /*     abort(); */
  /*   } */

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
    {
      mtrace_init();
    }


  if (current_bytes > max_bytes) max_bytes = current_bytes;


  void *ptr = NULL;
  ptr = real_malloc(size);

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



/* void *calloc(size_t nitems, size_t size) */
/* { */
/*   if (NULL == real_calloc) */
/*     { */
/*       /\* https://stackoverflow.com/questions/7910666/problems-with-ld-preload-and-calloc-interposition-for-certain-executables *\/ */
/*       return buffer; */
/*       //mtrace_init(); */
/*     } */

/*   malloced_bytes += nitems*size; */
/*   current_bytes  += nitems*size; */
/*   calloc_calls++; */

/*   if (current_bytes > max_bytes) max_bytes = current_bytes; */

/*   /\* union my2_size *ptr = NULL; *\/ */
/*   /\* ptr = real_malloc(sizeof *ptr + nitems*size); *\/ */
/*   /\* if (ptr) *\/ */
/*   /\*   { *\/ */
/*   /\*     ptr->size = nitems*size; *\/ */
/*   /\*     ptr++; *\/ */
/*   /\*     memset(ptr, 0, nitems*size); *\/ */
/*   /\*   } *\/ */

/*   void *ptr = NULL; */
/*   ptr = real_calloc(nitems, size); */

/*   return ptr; */
/* } */



void free (void *buf)
{
  size_t freed_size=0;

  if (NULL == buf) return;

  if (NULL == real_free)
    mtrace_init();

  freed_size = my_malloc_size(buf);

  /* update atomics */
  free_calls++;
  freed_bytes   += freed_size;
  current_bytes -= freed_size;

  //fprintf(stderr, "free(%p)\n", ptr);
  real_free(buf);
}



/* void *realloc(void *ptr, size_t size) */
/* { */
/*   free(ptr); */
/*   return malloc(size); */
/* } */



void log_memusage_malloc_report (const char* prefix)
{
  fprintf(stderr, "%smalloc: %lld bytes, %lld calls\n", prefix, malloced_bytes, malloc_calls);
  fprintf(stderr, "%sfree:   %lld bytes, %lld calls\n", prefix, freed_bytes,    free_calls);
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
