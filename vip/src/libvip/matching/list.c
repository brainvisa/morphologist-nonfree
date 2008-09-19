/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/list.c           * TYPE     : Function
 * AUTHOR      : MANGIN J.-F.         * CREATION : 
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : 
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *            08/02/99| POUPON F.    | Passage en ANSI pour VIP
 *****************************************************************************/

#include <vip/matching/list.h>
#include <vip/matching/distance.h>
#include <vip/matching/interpol.h>

SuperList *CreateSuperList( int size )
{
  SuperList *newl;
 
  if (size<0)
    {
      return((SuperList *)NULL);
    }

  newl = (SuperList *)VipCalloc(1, sizeof(SuperList),
			       "Error while allocating SuperList");
  if (!newl) 
    {
      return((SuperList *)NULL);
    }

  newl->size = size;
  newl->n_points = 0;
  newl->offsetfilled = VFALSE;
  newl->ping = EXIST;
  newl->data = NULL;

  if (size>0)
    {
      newl->data = (SuperPoint *)VipCalloc(size, sizeof(SuperPoint),
					  "Error while allocating SuperPoint");
      if (!(newl->data)) 
	{
	  if (newl) VipFree(newl);
	  return((SuperList *)NULL);
	}
    }

  return(newl);
}

int FreeSuperList( SuperList *dead )
{
  if (dead==NULL)
    {
      return(PB);
    }

  if (dead->ping != EXIST)
    {
      return(PB);
    }

  if (dead->data==NULL)
    {
      return(PB);
    }
  else VipFree(dead->data);

  VipFree(dead);

  return(OK);
}

int KillSuperList( SuperList *sup )
{
  SuperPoint *ptr;
  int i;

  if (!sup)
    {
      return(PB);
    }

  if ( sup->data && (sup->interpol == TRILINEAIRE) )
    {
      ptr = sup->data;
      for (i=sup->n_points;i>0;i--)
	{
	  if (ptr->coef) VipFree(ptr->coef);
	  ptr++;
	}
    }

  if (sup->data) VipFree(sup->data);

  VipFree(sup);

  return(OK);
}

Volume *SuperListToVolume( SuperList *list, Volume *volref )
{
  Volume *newv;
  SuperPoint *ptr;
  Vip_S16BIT *newfirst;
  int i;
  int xsize, ysize, zsize;
  int x, y, z;
  VipOffsetStruct *vos;

  newv = VipDuplicateVolumeStructure( volref, "" );
  if (!newv)
    {
      return((Volume *)NULL);
    }

  if ((VipAllocateVolumeData(newv)) == PB)
    {
      if (newv) VipFree(newv);
      return((Volume *)NULL);
    }

  vos = VipGetOffsetStructure(newv);

  if (list->offsetfilled==VFALSE) 
	FillSuperPointOffset(list, volref);

  xsize = mVipVolSizeX(newv);
  ysize = mVipVolSizeY(newv);
  zsize = mVipVolSizeZ(newv);

  ptr = list->data;
  newfirst = VipGetDataPtr_S16BIT(newv)+vos->oFirstPoint;

  for (i=list->n_points; i>0; i--)
    {
      x = ptr->p.x ;
      y = ptr->p.y ;
      z = ptr->p.z ;
      if( (x>=0) && (x<xsize) && (y>=0) && (y<ysize) && (z>=0) && (z<zsize))
	*(newfirst + ptr->offset ) = (Vip_S16BIT)255;
      ptr++;
    }

  VipFree(vos);

  return(newv);
}

int AddSuperListToVolume( SuperList *list, Volume *vol, int value )
{
  SuperPoint *ptr;
  Vip_S16BIT *volfirst;
  int i;
  int xsize, ysize, zsize;
  int x, y, z;
  VipOffsetStruct *vos;

  FillSuperPointOffset(list, vol);

  xsize = mVipVolSizeX(vol);
  ysize = mVipVolSizeY(vol);
  zsize = mVipVolSizeZ(vol);
  vos = VipGetOffsetStructure(vol);

  ptr = list->data;

  volfirst = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;

  for(i=list->n_points; i>0; i--)
    {
      x = ptr->p.x ;
      y = ptr->p.y ;
      z = ptr->p.z ;
      if ((x>=0) && (x<xsize) && (y>=0) && (y<ysize) && (z>=0) && (z<zsize))
	*(volfirst + ptr->offset ) += (Vip_S16BIT)value;
      ptr++;
    }

  VipFree(vos);

  return(OK);
}

/* NB: il doit y avoir correspondance entre les points de list,listpp*/
int MixteErreurVolumeRef( SuperList *list, SuperList *listpp, Volume *ref, 
			  double moy )
{
  SuperPoint *ptr, *ptrpp;
  Vip_S16BIT *reffirst, *refptr;
  int i;
  int xsize, ysize, zsize, x, z, y;
  float max;
  VipOffsetStruct *vos;

  if(!list || !listpp || !ref)
    {
      return(PB);
    }

  xsize = mVipVolSizeX(ref);
  ysize = mVipVolSizeY(ref);
  zsize = mVipVolSizeZ(ref);

  vos = VipGetOffsetStructure(ref);

  ptr = list->data;
  max = 0.0;

  for (i=list->n_points;i>0;i--)
    {
      if (max<ptr->dist) max = ptr->dist;
      ptr++;
    }

  if (max==0.0)
    {
      if (vos) VipFree(vos);
      return(PB);
    }

  ptr = list->data;
  ptrpp = listpp->data;
  reffirst = VipGetDataPtr_S16BIT(ref)+vos->oFirstPoint;	

  for(i=list->n_points;i>0;i--)
    {
      x = ptrpp->p.x ;
      y = ptrpp->p.y ;
      z = ptrpp->p.z ;

      refptr = reffirst+x + y*vos->oLine + z*vos->oSlice;
      if ((x>=0) && (x<xsize) && (y>=0) && (y<ysize) && (z>=0) && (z<zsize))
	{
	  if (ptr->dist == -1.0) *refptr = (Vip_S16BIT)251;
	  else if ((ptr->dist)>(5*moy)) *refptr = (Vip_S16BIT)250;
	  else *refptr = (Vip_S16BIT)(50* (ptr->dist/moy));
	}
      ptr++;
      ptrpp++;
    }

  VipFree(vos);

  return(OK);
}

/* NB: il doit y avoir correspondance entre les points de buck et de list*/
int AddErreurToVolumeRecal( SuperList *list, VipG3DBucket *buck, 
			    Volume *recal, double moy )
{
  SuperPoint *ptr;
  Vip_S16BIT *recalfirst, *recalptr;
  VipG3DPoint *buckptr;
  int i;
  float max;
  VipOffsetStruct *vos;

  if (!list || !buck || !recal)
    {
      return(PB);
    }

  vos = VipGetOffsetStructure(recal);

  ptr = list->data;
  max = 0.0;

  for(i=list->n_points;i>0;i--)
    {
      if(max<ptr->dist) max = ptr->dist;
      ptr++;
    }

  if(max==0.0)
    {
      if (vos) VipFree(vos);
      return(PB);
    }

  ptr = list->data;
  buckptr = buck->data;
  recalfirst = VipGetDataPtr_S16BIT(recal)+vos->oFirstPoint;

  for (i=list->n_points;i>0;i--)
    {
      recalptr = recalfirst+buckptr->p.x + buckptr->p.y*vos->oLine + 
	buckptr->p.z*vos->oSlice;

      if (!*recalptr)
	{
	  if (vos) VipFree(vos);
	  return(PB);
	}
      else
	{
	  if(ptr->dist== -1.0) *recalptr = (Vip_S16BIT)(255);
	  else if((ptr->dist)>(5*moy)) *recalptr = (Vip_S16BIT)250;
	  else *recalptr = (Vip_S16BIT)(50* (ptr->dist/moy));
	}
      ptr++;
      buckptr++;
    }

  VipFree(vos);

  return(OK);
}
