#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include <stdio.h>      /* Input/Output */
#include <unistd.h>     /* gethostname */



int main (int argc, char **argv)
{
  char hn[256];

  gethostname(hn, sizeof(hn) / sizeof(char));

  printf("Hello from %s\n", hn);
}
