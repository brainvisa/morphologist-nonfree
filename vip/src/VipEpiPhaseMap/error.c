#include "error.h"

#include <stdio.h>
#include <stdlib.h>

void error(char *message)
/* -----------------------------------------------------------------------
   prints message and quits
   ----------------------------------------------------------------------*/
{
  fprintf(stderr,"%s\nexiting\n",message);
  exit(EXIT_FAILURE);
}
