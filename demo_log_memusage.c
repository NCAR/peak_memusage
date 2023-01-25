#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* malloc/free */
#include <unistd.h>     /* gethostname */
#include <assert.h>

#define ANNOTATE
#ifdef ANNOTATE
# include "log_memusage.h"
#endif



void touch_mem(int* buf, int cnt)
{
  int i;

  assert (NULL != buf);

  for (i=0; i<cnt; i++)
    {
      assert (i == buf[i]);
    }
}



void malloc_and_touch(int* buf, int cnt)
{
  int i;

  assert (NULL == buf);
  buf = (int*) malloc( cnt * sizeof(int));
  assert (NULL != buf);

  for (i=0; i<cnt; i++)
    buf[i] = i;

  touch_mem(buf,cnt);
}



int main (int argc, char **argv)
{
  char hn[256];
  int *buf = NULL;

  gethostname(hn, sizeof(hn) / sizeof(char));

  printf("Hello from %s\n", hn);
  sleep(2);

  printf("allocating memory\n");
#ifdef ANNOTATE
  log_memusage_annotate("allocating memory");
#endif
  malloc_and_touch(buf, 1e7);
  sleep(1);

  printf("freeing memory\n");
#ifdef ANNOTATE
  log_memusage_annotate("freeing memory");
#endif
  free(buf);
  sleep(1);

  printf("allocating more memory\n");
#ifdef ANNOTATE
  log_memusage_annotate("allocating more memory");
#endif
  malloc_and_touch(buf, 5e7);
  sleep(1);

  printf("freeing memory\n");
  free(buf);
  sleep(1);

}
