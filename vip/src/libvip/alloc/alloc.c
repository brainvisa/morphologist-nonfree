/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_alloc.c          * TYPE     : Source
 * AUTHOR      : POUPON F.            * CREATION : 20/11/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Routines d'allocation 1D, 2D, 3D et 4D
 *               fpoupon@cea.fr
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *              / /   |              |
 *****************************************************************************/

#include <vip/alloc.h>

void *VipMalloc(size_t size, char *message)
{ 
  void *ptr;

  ptr = malloc(size);
  if (!ptr) 
    { 
      VipPrintfError("Not enough memory...");
      if (message != NULL)  (void)fprintf(stderr, "%s", message);
      VipPrintfExit("(Vip_alloc.c)VipMalloc");
      return NULL;
    }
  return ptr;
}

void *VipCalloc(size_t nboj, size_t size, char *message)
{ 
  void *ptr;

  ptr = calloc(nboj, size);
  if (!ptr) 
    { 
      VipPrintfError("Not enough memory...");
      if (message != NULL)  (void)fprintf(stderr, "%s", message);
      VipPrintfExit("(Vip_alloc.c)VipCalloc");
      return NULL;
    }
  return ptr;
}

void *VipRealloc(void *p, size_t size, char *message)
{
  void *ptr;

  ptr = realloc(p, size);
  if (!ptr)
    {
      VipPrintfError("Not enough memory...");
      if (message != NULL)  (void)fprintf(stderr, "%s", message);
      VipPrintfExit("(Vip_alloc.c)VipRealloc");
      return NULL;
    }

  return ptr;
}

int VipFree(void *ptr)
{
  if (!ptr)
    {
      VipPrintfWarning("Trying to free NULL pointer");
    }
  free(ptr);
  return OK;
}
