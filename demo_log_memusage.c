#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* malloc/free */
#include <unistd.h>     /* gethostname */
#include <assert.h>

#include "log_memusage.h"

struct Books {
  char  title[50];
  char  author[50];
  char  subject[100];
  int   book_id;
  float decimal_number;
};


void touch_mem_ints(int* buf, int cnt)
{
  int i;

  assert (NULL != buf);

  for (i=0; i<cnt; i++)
    assert (i == buf[i]);
}



int* malloc_and_touch_ints(int* buf, int cnt)
{
  int i;

  if (NULL != buf)
    {
      buf = (int*) realloc( buf, cnt * sizeof(int) );
      assert (NULL != buf);
    }
  else
    {
      assert (NULL == buf);
      buf = (int*) malloc( cnt * sizeof(int));
      assert (NULL != buf);
    }

  for (i=0; i<cnt; i++)
    buf[i] = i;

  touch_mem_ints(buf,cnt);

  return buf;
}



void touch_mem_books(struct Books* buf, int cnt)
{
  int i;

  assert (NULL != buf);

  for (i=0; i<cnt; i++)
    assert (i == buf[i].book_id);
}



struct Books* malloc_and_touch_books(struct Books* buf, int cnt)
{
  int i;

  if (NULL != buf)
    {
#ifdef HAVE_REALLOCARRAY
      buf = (struct Books*) reallocarray( buf, cnt, sizeof(struct Books) );
#else
      buf = (struct Books*) calloc( cnt, sizeof(struct Books));
      free(buf);
#endif
      assert (NULL != buf);
    }
  else
    {
      assert (NULL == buf);
      buf = (struct Books*) calloc( cnt, sizeof(struct Books));
      assert (NULL != buf);
    }

  for (i=0; i<cnt; i++)
    buf[i].book_id = i;

  touch_mem_books(buf,cnt);

  return buf;
}



int main (int argc, char **argv)
{
  char hn[256];
  int *ibuf = NULL;
  struct Books *bbuf=NULL;

  gethostname(hn, sizeof(hn) / sizeof(char));

  printf("Hello from %s\n", hn);
  sleep(1);

  printf("allocating memory\n");
  log_memusage_annotate("allocating memory");
  ibuf = malloc_and_touch_ints(ibuf, 1e6);
  sleep(1);

  printf("reallocating more memory\n");
  log_memusage_annotate("allocating more memory");
  ibuf = malloc_and_touch_ints(ibuf, 5e6);
  sleep(1);

  printf("reallocating less memory\n");
  log_memusage_annotate("allocating less memory");
  ibuf = malloc_and_touch_ints(ibuf, 5e5);
  sleep(1);

  printf("allocating memory (struct)\n");
  log_memusage_annotate("allocating memory (struct)");
  bbuf = malloc_and_touch_books(bbuf, 1e4);
  sleep(1);

  printf("reallocating more memory (struct)\n");
  log_memusage_annotate("allocating more memory (struct)");
  bbuf = malloc_and_touch_books(bbuf, 5e5);
  sleep(1);

  printf("reallocating less memory (struct)\n");
  log_memusage_annotate("allocating less memory (struct)");
  bbuf = malloc_and_touch_books(bbuf, 5e4);
  sleep(1);

  printf("freeing memory\n");
  free(ibuf); ibuf = NULL;
  free(bbuf); bbuf = NULL;
  sleep(1);

}
