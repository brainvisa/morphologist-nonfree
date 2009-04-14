/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_bucket.c         * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 17/01/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : gestion de bucket de doublon point/vecteur (2D/3D)
 *               jfmangin@cea.fr
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *             7/3/97 | V.Penglaou   | Ajout des buckets de int
 *             2/6/99 | Jeff         | Ajout des buckets De *S16BIT
 *****************************************************************************/


#include <vip/bucket.h>
#if defined( _WIN32 ) || defined( __APPLE__ )
#define _VIP_AVOID_REALLOC
#endif

/*------------------------------------------------------------------*/
VipG2DBucket *VipAllocG2DBucket(int size)
/*------------------------------------------------------------------*/
{
  VipG2DBucket *newb;

  newb = (VipG2DBucket *)VipCalloc(1, sizeof(VipG2DBucket),
				  "Error while allocating  VipG2DBucket");
  if (newb == NULL)
    {
      VipPrintfExit("VipAllocG2DBucket");
      return(PB);
    }

  newb->size = (int)size;
  newb->data = (VipG2DPoint *)VipCalloc(size, sizeof(VipG2DPoint), 
				       "Error while allocating VipG2DPoint vector");
  if (newb->data == NULL)  
  {
    VipFree((void *)newb); 
    VipPrintfExit("VipAllocG2DBucket");
    return(PB);
  }

  return(newb);
}
/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
int VipDwindleG2DBucket(VipG2DBucket *vec)
/*------------------------------------------------------------------*/

{
  VipG2DPoint *newb;

#ifdef _VIP_AVOID_REALLOC
  size_t size, oldsize;
#endif

  if(vec==NULL)
    {
      VipPrintfExit("VipDwindleG2DBucket");
      return(PB);
    }
  else if(vec->data==NULL)
    {
      VipPrintfExit("VipDwindleG2DBucket");
      return(PB);
    }
  else
    {
#ifdef _VIP_AVOID_REALLOC
      size = vec->n_points * sizeof(VipG2DPoint);
      oldsize = vec->size * sizeof(VipG2DPoint);
      if( size < oldsize ) /* shrink: avoid windows bug... */
      {
        newb = (VipG2DPoint *)VipMalloc( size, "");
        if (newb==NULL)
        {
          VipPrintfError("Memory pb in realloc");
          VipPrintfExit("VipDwindleG2DBucket");
          return(PB);
        }
        if( vec->data != NULL )
        {
          memcpy( newb, vec->data, size <= oldsize ? size : oldsize );
          VipFree( vec->data );
        }
      }
      else
      {
        newb = (VipG2DPoint *)VipRealloc((void *)(vec->data),
                vec->n_points * sizeof(VipG2DPoint), "");
      	if (newb==NULL)
        {
          VipPrintfError("Memory pb in realloc");
          VipPrintfExit("VipDwindleG2DBucket");
          return(PB);
        }
      }
#else
	newb = (VipG2DPoint *)VipRealloc((void *)(vec->data),
					vec->n_points * sizeof(VipG2DPoint),
					"");
	if (newb==NULL)
    	{
	  VipPrintfError("Memory pb in realloc");
      	  VipPrintfExit("VipDwindleG2DBucket");
          return(PB);
    	}
#endif
        vec->data = newb;
	vec->size = vec->n_points;
    }
  return(OK);
}
/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
int VipFreeG2DBucket(VipG2DBucket *vec)
/*------------------------------------------------------------------*/
{
  if(vec==NULL)
    {
      VipPrintfExit("VipFreeG2DBucket");
      return(PB);
    }
  else if(vec->data==NULL)
    {
      VipPrintfExit("VipFreeG2DBucket");
      return(PB);
    }
  else
    {
      VipFree(vec->data);
      VipFree(vec);
    }
  return(OK);
}
/*------------------------------------------------------------------*/


/*------------------------------------------------------------------*/
int VipFreeG2DBucketList(VipG2DBucket *list)
/*------------------------------------------------------------------*/
{
  VipG2DBucket *killer;

  while (list!=NULL)
    {
      killer = list;
      list = list->next;
      VipFreeG2DBucket(killer);
    }
  
  return(OK);
}

/*------------------------------------------------------------------*/
VipG3DBucket *VipAllocG3DBucket(int size)
/*------------------------------------------------------------------*/
{
VipG3DBucket *newb;

  newb = (VipG3DBucket *)VipCalloc(1, sizeof(VipG3DBucket),
				         "Error while allocating  VipG3DBucket");
  if (newb == NULL)
    {
      VipPrintfExit("VipAllocG3DBucket");
      return(PB);
    }

  newb->size = (int)size;
  newb->data = (VipG3DPoint *)VipCalloc(size, sizeof(VipG3DPoint), 
				   "Error while allocating VipG3DPoint vector");
  if (newb->data == NULL)  
  {
    VipFree((void *)newb); 
    VipPrintfExit("VipAllocG3DBucket");
    return(PB);
  }

  return(newb);
}
/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
int VipDwindleG3DBucket(VipG3DBucket *vec)
/*------------------------------------------------------------------*/

{
  VipG3DPoint *newb;

#ifdef _VIP_AVOID_REALLOC
  size_t size, oldsize;
#endif

  if(vec==NULL)
    {
      VipPrintfExit("VipDwindleG3DBucket");
      return(PB);
    }
  else if(vec->data==NULL)
    {
      VipPrintfExit("VipDwindleG3DBucket");
      return(PB);
    }
  else
    {
#ifdef _VIP_AVOID_REALLOC
      size = vec->n_points * sizeof(VipG3DPoint);
      oldsize = vec->size * sizeof(VipG3DPoint);
      if( size < oldsize ) /* shrink: avoid windows bug... */
      {
        newb = (VipG3DPoint *)VipMalloc( size, "");
        if (newb==NULL)
        {
          VipPrintfError("Memory pb in realloc");
          VipPrintfExit("VipDwindleG3DBucket");
          return(PB);
        }
        if( vec->data != NULL )
        {
          memcpy( newb, vec->data, size <= oldsize ? size : oldsize );
          VipFree( vec->data );
        }
      }
      else
      {
        newb = (VipG3DPoint *)VipRealloc((void *)(vec->data),
                vec->n_points * sizeof(VipG3DPoint), "");
      	if (newb==NULL)
        {
          VipPrintfError("Memory pb in realloc");
          VipPrintfExit("VipDwindleG3DBucket");
          return(PB);
        }
      }
#else
	newb = (VipG3DPoint *)VipRealloc((void *)(vec->data),
					vec->n_points * sizeof(VipG3DPoint),
					"");
	if (newb==NULL)
    	{
	  VipPrintfError("Memory pb in realloc");
      	  VipPrintfExit("VipDwindleG3DBucket");
          return(PB);
    	}
#endif
        vec->data = newb;
	vec->size = vec->n_points;
    }
  return(OK);
}
/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
int VipFreeG3DBucket(VipG3DBucket *vec)
/*------------------------------------------------------------------*/
{
  if(vec==NULL)
    {
      VipPrintfExit("VipFreeG3DBucket");
      return(PB);
    }
  else if(vec->data==NULL)
    {
      VipPrintfExit("VipFreeG3DBucket");
      return(PB);
    }
  else
    {
      VipFree(vec->data);
      VipFree(vec);
    }
  return(OK);
}
/*------------------------------------------------------------------*/


/*------------------------------------------------------------------*/
int VipFreeG3DBucketList(VipG3DBucket *list)
/*------------------------------------------------------------------*/
{
  VipG3DBucket *killer;

  while (list!=NULL)
    {
      killer = list;
      list = list->next;
      VipFreeG3DBucket(killer);
    }
  
  return(OK);
}

/*------------------------------------------------------------------*/
int VipSaveG3DBucketList(VipG3DBucket *list, char *name)
/*------------------------------------------------------------------*/
{
  VipG3DBucket *writer;
  FILE *f;
  int n;
  char filename[512];

  if(!name)
    {
      VipPrintfError("No name in VipSaveG3DBucketList");
      VipPrintfExit("(bucket)VipSaveG3DBucketList");
      return(PB);
    }
  strcpy(filename,name);
  strcat(filename,".g3D");

  f = VipOpenFile(filename,VIP_WRITE_BIN,"VipSaveG3DBucketList");
  if(f==PB) return(PB);

  while (list!=NULL)
    {
      writer = list;
      list = list->next;
      n = fwrite(writer->data,sizeof(VipG3DPoint),writer->n_points,f);
      if(n!=writer->n_points)
	{
	  VipPrintfError("Problem while writing");
	  VipPrintfExit("(bucket)VipSaveG3DBucketList");
	  fclose(f);
	  return(PB);
	}
    }
  
  fclose(f);
  return(OK);
}

/*------------------------------------------------------------------*/
int VipSaveG2DBucketList(VipG2DBucket *list, char *name)
/*------------------------------------------------------------------*/
{
  VipG2DBucket *writer;
  FILE *f;
  int n;
  char filename[512];

  if(!name)
    {
      VipPrintfError("No filename in VipSaveG2DBucketList");
      VipPrintfExit("(bucket)VipSaveG2DBucketList");
      return(PB);
    }
  strcpy(filename,name);
  strcat(filename,".g2D");
  f = VipOpenFile(filename,VIP_WRITE_BIN,"VipSaveG2DBucketList");
  if(f==PB) return(PB);

  while (list!=NULL)
    {
      writer = list;
      list = list->next;
      n = fwrite(writer->data,sizeof(VipG2DPoint),writer->n_points,f);
      if(n!=writer->n_points)
	{
	  VipPrintfError("Problem while writing");
	  VipPrintfExit("(bucket)VipSaveG2DBucketList");
	  fclose(f);
	  return(PB);
	}
    }
  
  fclose(f);
  return(OK);
}

/*------------------------------------------------------------------*/
VipG2DBucket *VipReadG2DFile(char *name)
/*------------------------------------------------------------------*/
{
  VipG2DBucket *buck;
  FILE *f;
  int n;
  char filename[512];
  long flength;
  int ntheory;

  if(!name)
    {
      VipPrintfError("No name in VipReadG2DFile");
      VipPrintfExit("(bucket)VipReadG2DFile");
      return(PB);
    }
  strcpy(filename,name);
  strcat(filename,".g2D");

  f = VipOpenFile(filename,VIP_READ_BIN,"VipReadG2DFile");
  if(f==PB) return(PB);

  fseek(f,0,SEEK_END);
  flength = ftell(f);
  fseek(f,0,SEEK_SET);

  if(flength % sizeof(VipG2DPoint))
    {
      VipPrintfError("Strange length for a G2D file...");
      VipPrintfExit("(bucket)VipReadG2DFile");
      return(PB);
    }
  else ntheory = flength / sizeof(VipG2DPoint);

  buck = VipAllocG2DBucket(ntheory);
  if(!buck) return(PB);

  n = fread( (void *)buck->data,sizeof(VipG2DPoint),ntheory,f);
  if(n!=ntheory)
    {
      VipPrintfError("Reading problem in a G2D file...");
      VipPrintfExit("(bucket)VipReadG2DFile");
      return(PB);
    }
  buck->n_points = n;
  fclose(f);
  return(buck);
}

/*------------------------------------------------------------------*/
VipG3DBucket *VipReadG3DFile(char *name)
/*------------------------------------------------------------------*/
{
  VipG3DBucket *buck;
  FILE *f;
  int n;
  char filename[512];
  long flength;
  int ntheory;

  if(!name)
    {
      VipPrintfError("No name in VipReadG3DFile");
      VipPrintfExit("(bucket)VipReadG3DFile");
      return(PB);
    }
  strcpy(filename,name);
  strcat(filename,".g3D");

  f = VipOpenFile(filename,VIP_READ_BIN,"VipReadG3DFile");
  if(f==PB) return(PB);

  fseek(f,0,SEEK_END);
  flength = ftell(f);
  fseek(f,0,SEEK_SET);

  if(flength % sizeof(VipG3DPoint))
    {
      VipPrintfError("Strange length for a G3D file...");
      VipPrintfExit("(bucket)VipReadG3DFile");
      return(PB);
    }
  else ntheory = flength / sizeof(VipG3DPoint);

  buck = VipAllocG3DBucket(ntheory);
  if(!buck) return(PB);

  n = fread( (void *)buck->data,sizeof(VipG3DPoint),ntheory,f);
  if(n!=ntheory)
    {
      VipPrintfError("Reading problem in a G3D file...");
      VipPrintfExit("(bucket)VipReadG3DFile");
      return(PB);
    }
  buck->n_points = n;

  fclose(f);
  return(buck);
}


int VipPermuteTwoIntBucket( VipIntBucket **b1, VipIntBucket **b2 )
{
  VipIntBucket *temp;

  if((b1==NULL) || (*b1==NULL) || (b2==NULL) || (*b2==NULL))
    {
      VipPrintfError("One NULL bucket in PermuteTwoIntBucket");
      VipPrintfExit("PermuteTwoIntBucket");
      return(PB);
    }
  temp = *b1;
  *b1 = *b2;
  *b2 = temp;
  return(OK);
}

/*---------------------------------------------------------------------------*/
VipIntBucket *VipAllocIntBucket(int size)

{
  VipIntBucket *newb;
  newb = (VipIntBucket *)VipCalloc(1, sizeof(VipIntBucket),
				  "Error while allocating VipIntBucket");
  
  if (newb == NULL)
    {
      VipPrintfExit("VipAllocIntBucket");
      return(PB);
    }

  newb->size = size;
  newb->next = NULL;

  newb->data = (int *)VipCalloc(size, sizeof(int),
				"Error  while allocating VipS16BITPtrBucket");

  

  if (newb->data == NULL)
    {
      VipFree((void *)newb);
      VipPrintfExit("VipAllocIntBucket");
      return(PB);
    }
  
  return(newb);
}

/*---------------------------------------------------------------------------*/
VipS16BITPtrBucket *VipAllocS16BITPtrBucket(int size)

{
  VipS16BITPtrBucket *newb;
  newb = (VipS16BITPtrBucket *)VipCalloc(1, sizeof(VipS16BITPtrBucket),
				  "Error while allocating VipS16BITPtrBucket");
  
  if (newb == NULL)
    {
      VipPrintfExit("VipAllocS16BITPtrBucket");
      return(PB);
    }

  newb->size = size;
  newb->next = NULL;

  newb->data = (Vip_S16BIT **)VipCalloc(size, sizeof(Vip_S16BIT *),
			       "Error  while allocating Vip16BIT * vector");
  

  if (newb->data == NULL)
    {
      VipFree((void *)newb);
      VipPrintfExit("VipAllocS16BITPtrBucket");
      return(PB);
    }
  
  return(newb);
}
/*---------------------------------------------------------------------------*/
VipIntBucket *VipMallocIntBucket(int size)

{
  VipIntBucket *newb;
  newb = (VipIntBucket *)VipCalloc(1, sizeof(VipIntBucket),
				  "Error while allocating VipIntBucket");
  
  if (newb == NULL)
    {
      VipPrintfExit("VipAllocIntBucket");
      return(PB);
    }

  newb->size = size;

  
  newb->data = (int *)VipMalloc(size * sizeof(int),
			       "Error  while allocating int vector");
  

  if (newb->data == NULL)
    {
      VipFree((void *)newb);
      VipPrintfExit("VipAllocIntBucket");
      return(PB);
    }
  
  return(newb);
}

/*---------------------------------------------------------------------------*/
int VipFreeIntBucket(VipIntBucket *vec)

{
  if (vec==NULL)
    {
      VipPrintfWarning("nothing to free in VipFreeIntBucket");
      return(OK);
    }
  else if(vec->data==NULL)
    {
      VipFree(vec); 
      VipPrintfWarning("empty bucket in VipFreeIntBucket");
      return(OK);
    }
  else
    {
      VipFree(vec->data); /* avait ete commente. Pourquoi??? F.P. */
      VipFree(vec);      /*la petite veronique avait pas du tout comprendre...jeff*/
    }

  return(OK);
}

/*---------------------------------------------------------------------------*/
int VipFreeS16BITPtrBucket(VipS16BITPtrBucket *vec)

{
  if (vec==NULL)
    {
      VipPrintfExit("VipFreeS16BITPtrBucket");
      return(PB);
    }
  else if(vec->data==NULL)
    {
      VipPrintfExit("VipFreeS16BITPtrBucket");
      return(PB);
    }
  else
    {
      VipFree(vec->data); /* avait ete commente. Pourquoi??? F.P. */
      VipFree(vec);
    }

  return(OK);
}
/*---------------------------------------------------------------------------*/
int VipFreeIntBucketList(VipIntBucket *vec)

{
  VipIntBucket *next;

  if (vec==NULL)
    {
      VipPrintfWarning("nothing to free in VipFreeIntBucketList");
      return(OK);
    }
  else
    {
      while(vec!=NULL)
	{
	  next = vec->next;
	  VipFreeIntBucket(vec);
	  vec = next;
	}
    }

  return(OK);
}
/*---------------------------------------------------------------------------*/
int VipIncreaseIntBucket(VipIntBucket *vec, int increment)

{
  int *newbuf;

  if(vec==NULL || increment<=0)
    {
      VipPrintfError("args in VipIncreaseIntBucket");
      VipPrintfExit("VipIncreaseIntBucket");
      return(PB);
    }

  newbuf = (int *)VipRealloc((void *)(vec->data),
			     (vec->size+increment)*sizeof(int), "");
  if (newbuf==NULL)
    {
      VipPrintfError("Sorry, no more memory...");
      VipPrintfExit("VipIncreaseIntBucket");
      return(PB);
    }
  else
    {
	vec->data = newbuf;
	vec->size += increment;
    }

  return(OK);
}
