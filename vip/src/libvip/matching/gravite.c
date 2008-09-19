/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/gravite.c        * TYPE     : Function
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

#include <vip/matching/gravite.h>

/* Calcule les coordonnees du centre de gravite de la surface */
int VipFillSurfG( Surface *surf )
{
  VipG3DBucket *buckptr;
  VipG3DPoint *pointptr;
  double xsom=0.0, ysom=0.0, zsom=0.0;
  int npoints=0;
  int i;

  if (!surf)
    {
      return(PB);
    }

  if (surf->bucket==NULL)
    {
      return(PB);
    }

  buckptr = surf->bucket;

  while (buckptr!=NULL)
    {
      pointptr = buckptr->data;
      for (i=buckptr->n_points;i>0;i--)
	{
	  xsom += pointptr->p.x;
	  ysom += pointptr->p.y;
	  zsom += pointptr->p.z;
	  npoints++;
	  pointptr++;
	}
      buckptr = buckptr->next;
    }

  surf->g.x = (float)(xsom/(float)npoints);
  surf->g.y = (float)(ysom/(float)npoints);
  surf->g.z = (float)(zsom/(float)npoints);

  return(OK);
}     

/* Calcule les coordonnees du centre de gravite du volume */
int VipFillVolumeG( Volume *vol, Gravity *volgrav )
{
  Vip_S16BIT *ptr;
  int x, y, z, dimx, dimy, dimz;
  double xsom=0.0, ysom=0.0, zsom=0.0;
  double npoints=0.0;
  VipOffsetStruct *vos;
  long off_lbs, off_pbl;

  if (!vol || !vol->data || !volgrav)
    {
      return(PB);
    }

  vos = VipGetOffsetStructure(vol);
  off_lbs = VipOffsetLineBetweenSlice(vol);
  off_pbl = VipOffsetPointBetweenLine(vol);

  dimx = mVipVolSizeX(vol);
  dimy = mVipVolSizeY(vol);
  dimz = mVipVolSizeZ(vol);

  ptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;

  for (z=0;z<dimz;z++, ptr+=off_lbs)
    for (y=0;y<dimy;y++, ptr+=off_pbl)
      for (x=0;x<dimx;x++)
	{
	  if (*ptr++)
	    {
	      xsom += (double)x;
	      ysom += (double)y;
	      zsom += (double)z;
	      npoints++;
	    }
	}
	
  volgrav->x = (float)(xsom/npoints);
  volgrav->y = (float)(ysom/npoints);
  volgrav->z = (float)(zsom/npoints);

  VipFree(vos);

  return(OK);
}     

/* Calcule les coordonnees x et y du centre de gravite du bucket */
Gravity *GetGBucketGxy( VipG3DBucket *buck )
{
  Gravity *G;
  VipG3DPoint *pointptr;
  double xsom=0.0, ysom=0.0;
  int npoints=0, i;

  pointptr = buck->data;

  for (i=buck->n_points;i>0;i--)
    {
      xsom += pointptr->p.x;
      ysom += pointptr->p.y;
      npoints++;
      pointptr++;
    }

  G = (Gravity *)VipCalloc(1, sizeof(Gravity), 
			   "Error while allocating Gravity structure");
  if (!G) return((Gravity *)NULL);

  G->x = (float)(xsom/(float)npoints);
  G->y = (float)(ysom/(float)npoints);

  return(G);
}			
	
/* Calcule les coordonnees x,y et z du centre de gravite du bucket */
Gravity *GetGBucketGxyz( VipG3DBucket *buck )
{
  Gravity *G;
  VipG3DPoint *pointptr;
  double xsom=0.0, ysom=0.0, zsom=0.0;
  int npoints=0, i;

  pointptr = buck->data;

  for (i=buck->n_points;i>0;i--)
    {
      xsom += pointptr->p.x;
      ysom += pointptr->p.y;
      zsom += pointptr->p.z;
      npoints++;
      pointptr++;
    }

  G = (Gravity *)VipCalloc(1, sizeof(Gravity), 
			   "Error while allocating Gravity structure");
  if (!G) return((Gravity *)NULL);

  G->x = (float)(xsom/(float)npoints);
  G->y = (float)(ysom/(float)npoints);
  G->z = (float)(zsom/(float)npoints);

  return(G);
}			
