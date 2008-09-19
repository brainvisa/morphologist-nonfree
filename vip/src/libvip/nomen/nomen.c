/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : nomen.c              * TYPE     : Package
 * AUTHOR      : Frouin V.            * CREATION : 24/09/1999
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC
 *****************************************************************************
 *
 * DESCRIPTION : Nomenclature management core
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vip/nomen.h>
#include <vip/volume.h>
#include <vip/alloc.h>

static int matchNomenList(int pixel, VipNomenList *list)
{
  int i;
  int *tmp;

  for (i=0, tmp = list->datalabel; i< list->size; i++, tmp++)
    {
      if ( pixel == *tmp ) return(1);
    }
  return(0);
}

static void addEntryNomenList(int pixel, VipNomenList *list)
{
  int *tmp ;
  char **ctmp;

  if (pixel)
    {
      if (list->size == list->capacity)
	{  /*re-alloue un espace double de l'existant */
	  tmp = (int *) VipCalloc(2*list->capacity, sizeof(int), "");
	  memcpy(tmp, list->datalabel, list->capacity * sizeof(int) );
	  VipFree(list->datalabel);
	  list->datalabel = tmp;

	  ctmp = (char **) VipCalloc(2*list->capacity,sizeof(char *),"");
	  memcpy(ctmp, list->dataname, list->capacity * sizeof(char *) );
	  VipFree( list->dataname );
	  list->dataname = ctmp;

	  list->capacity = 2*list->capacity;
	}
      *(list->datalabel + list->size) = pixel;
      list->size++;
    }
}

VipNomenList* VipCreateRefNomenList( )
{
  VipNomenList *list = NULL;
  FILE         *fp   = NULL;
  char         *file_nomen;
  char         buff[VIP_NAME_MAXLEN], labstring[VIP_NAME_MAXLEN];
  char         *n;
  int          i, l;

  if ( (file_nomen = VipGetRTConfigString("VIP_NOMEN_FILE")) == PB )
    {
      VipPrintfWarning("No Reference Name List found\n");
      return( (VipNomenList *) PB );
    }

  if ( ( fp = fopen(file_nomen, "r") ) == NULL)
    {
      sprintf(file_nomen,"VipCreateRefNomenList: Can't open %s\n", file_nomen);
      VipPrintfWarning(file_nomen);
      return((VipNomenList*) PB );
    }

  list = VipCreateNomenList();
  while (fgets(buff, 512, fp) != NULL )
    {
      if (buff[0] != '#')
	{
	  for(i=0; i < (int)strlen(buff); i++)
	    {
	      if (buff[i] == ' ') break;
	      if ((buff[i] <= '9') && (buff[i] >= '0'))
		labstring[i] = buff[i];
	    }
	  labstring[i] = '\0';
	  l = atoi(labstring);
	  buff[strlen(buff)-1] = '\0'; /* Supprime le \n */
	  n = &(buff[i+1]);
	  if (VipAddNomenList(list, l, n) == PB) 
	    {
	      VipPrintfWarning("VipCreateRefNomenList: Can't load reflist\n");
	      return((VipNomenList*) PB );
	    }
	}
      for (i=0;i<512;i++) buff[i] = 0;
    }
  fclose( fp );
  return( list );
}

VipNomenList* VipCreateNomenList( )
{
  VipNomenList *list = VipCalloc(1, sizeof(VipNomenList), "");

  if ( !list ) return( (VipNomenList *) PB);

  list->datalabel = (int *) VipCalloc(VIP_NOMEN_INIT_INCR, sizeof(int), "");
  if ( !list->datalabel ) return( (VipNomenList *) PB);

  list->dataname = (char **) VipCalloc(VIP_NOMEN_INIT_INCR, sizeof(char *),"");
  if ( !list->dataname ) return( (VipNomenList *) PB);


  list->size = 0;
  list->capacity = VIP_NOMEN_INIT_INCR;
  return( list );
}

int VipFreeNomenList(  VipNomenList *list)
{
  int i;

  if (!list)
    {
      return( PB );
    }

  if (list->datalabel) VipFree(list->datalabel);
  if (list->dataname)
    {
      for (i=0; i < list->capacity; i++)
	if (list->dataname[i]) VipFree(list->dataname[i]);
      VipFree( list->dataname );
    }
  VipFree(list);

  return( OK );
}

VipNomenList* VipCreateNomenListFromVolume ( Volume *vol )
{
  VipNomenList *list = NULL;
  VipOffsetStruct  *vos;
  Vip_S16BIT       *sptr;
  Vip_U8BIT        *iptr;
  int               i, j, k;

  if ( (mVipVolType(vol) != U8BIT) && (mVipVolType(vol) != S16BIT) )
    {
      VipPrintfWarning("VipMatchNomenList: U8BIT or S16BIT volume type rquired\n");
      return((VipNomenList*) PB );
    }

  vos = VipGetOffsetStructure(vol);
  list = VipCreateNomenList();


  switch ( mVipVolType(vol) )
    {
    case U8BIT:
      iptr = VipGetDataPtr_U8BIT( vol ) + vos->oFirstPoint;
      for ( k = mVipVolSizeZ(vol); k-- ; )  
	{
	  for ( j = mVipVolSizeY(vol); j-- ; ) 
	    {
	      for ( i = mVipVolSizeX(vol); i-- ; )
		{
		  if ( !matchNomenList( (int) *iptr, list ) )
		    addEntryNomenList((int)*iptr, list);
		  iptr++;
		}
	      iptr += vos->oPointBetweenLine;
	    }
	  iptr += vos->oLineBetweenSlice;
	}
      break;
    case S16BIT:
      sptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
      for ( k = mVipVolSizeZ(vol); k-- ; )  
	{
	  for ( j = mVipVolSizeY(vol); j-- ; ) 
	    {
	      for ( i = mVipVolSizeX(vol); i-- ; )
		{
		  if ( !matchNomenList( (int) *sptr, list ) )
		    addEntryNomenList((int)*sptr, list);
		  sptr++;
		}
	      sptr += vos->oPointBetweenLine;
	    }
	  sptr += vos->oLineBetweenSlice;
	}
      break;
    }
  return ( list );
}


int VipMergeNomenList( VipNomenList *list, VipNomenList *ref)
{
  int i, j;
  
  for (i=0; i < list->size; i++)
    {
      for (j=0; j < ref->size; j++)
	if (list->datalabel[i] == ref->datalabel[j])
	  {
	    if ( (list->dataname)[i] ) VipFree( (list->dataname)[i] );
	    list->dataname[i] = (char *)VipCalloc(strlen( ref->dataname[j])+1,
						  sizeof(char),"");
	     strcpy( (list->dataname)[i],ref->dataname[j]);
	     break;
	  }
      if (j != ref->size) continue;
      if ( (list->dataname)[i] ) VipFree( (list->dataname)[i] );
      list->dataname[i] = (char *)VipCalloc(strlen(NO_NAME)+1,
						  sizeof(char), "");
      strcpy( (list->dataname)[i],NO_NAME);
    }
  return( OK );
}

int        VipGetNumberNomenList( VipNomenList *list )
{
  if ( !list ) return (PB );
  return( list->size );
}


int        VipGetLabelNomenList( VipNomenList *list, int number, int *retval )
{

  if (number < list->size) 
    {
      *retval = list->datalabel[number];
      return( OK );
    }
  else
    {
      *retval = -1;
      return( PB );
    }
}

char *VipGetNameNomenListByLabel( VipNomenList *list, int label)
{

   int i;
   for(i=0; i < list->size;i++)
    {
      if (list->datalabel[i] == label) return(list->dataname[i]);
    }
   return(NULL);
}


int      VipGetNameNomenList( VipNomenList *list, int number, char *retval)
{

  if (number < list->size) 
    {
      strcpy(retval,list->dataname[number]);
      return( OK );
    }
  else
    {
      retval = (char *) NULL;
      return( PB );
    }
}


int        VipAddNomenList( VipNomenList *list, int label, char * name)
{
  int i;

  if  ( (name == NULL) || (label < 0) || (label > 255) )
    {
      VipPrintfWarning("VipAddNomenToken: Tokenname = NULL\n");
      VipPrintfWarning("VipAddNomenToken: or label > 255\n");
      return( PB );
    }
  for(i=0; i < list->size; i++)
    {
    if ( *(list->datalabel + i) == label) return( OK );
    }
  addEntryNomenList(label, list);  /* Najoute que le label*/
  list->dataname[list->size-1] =   /* recopie du name */
    (char *) VipCalloc(strlen(name)+1,sizeof(char),"");
  strcpy(list->dataname[list->size -1], name);


  return( OK );
}


int  VipRemoveNomenListByLabel( VipNomenList *list, int label)
{
  int i,j;

  for(i=0; i < list->size; i++)
    {
      if ( *(list->datalabel + i) == label)
	{
	  if (list->dataname[i]) VipFree(list->dataname[i]);
	  for (j=i+1; j < list->size;j++)
	    {
	      list->datalabel[j - 1] = list->datalabel[j];
	      list->dataname [j - 1] = list->dataname[j]; 
	    }
	  list->dataname[list->size - 1] = 0;
	  list->dataname[list->size - 1] = NULL;
	  list->size -= 1;
	  return( OK );
	}
    }
  return( PB );
}

int VipPrintNomenList( FILE *fp, VipNomenList *list)
{
  int i;

  if (fp == NULL) 
    {
      VipPrintfWarning("VipPrintNomen: Stream fp = NULL\n");
      return( PB );
    }
  if (list == NULL)
    {
      VipPrintfWarning("VipPrintNomen: list = NULL\n");
      return( PB );
    }
  for (i=0; i < list->size; i++)
    {
      fprintf(fp, "% 4d %s\n", list->datalabel[i], list->dataname[i]);
    }

  return ( OK );
}
