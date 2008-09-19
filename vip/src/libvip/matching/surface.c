/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/surface.c        * TYPE     : Function
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
 *            03/02/99| POUPON F.    | Passage en ANSI pour VIP
 *****************************************************************************/

#include <string.h>
#include <math.h>
#include <vip/alloc.h>
#include <vip/matching/surface.h>

/* Conversion d'une image sous forme Volume a une structure de liste (Surface) 
   des points non nuls, les coordonnees vont de 0 a size-1 */
Surface *VipVolumeToLittleSurface( Volume *vol, int ratio, char *gradFile,
				   int gradDim )
{
  Surface *surf;
  register int size;
  register Vip_S16BIT *ptr;
  register int x,y,z;
  register VipG3DPoint *point;
  FILE *gradfile=NULL;
  int nlus, dimx, dimy, dimz;
  int flag_boucle;
  int n;
  float proba;
  int saut;
  VipOffsetStruct *vos;
  long off_lbs, off_pbl;
  VipG2DPoint g2D;
  VipG3DPoint g3D;

  surf = (Surface *)VipCalloc(1, sizeof(Surface),
			      "Error while allocating Surface structure.");
  if (!surf) return((Surface *)NULL);

  dimx = surf->xsize = mVipVolSizeX(vol);
  dimy = surf->ysize = mVipVolSizeY(vol);
  dimz = surf->zsize = mVipVolSizeZ(vol);
  surf->xvoxsize = mVipVolVoxSizeX(vol);
  surf->yvoxsize = mVipVolVoxSizeY(vol);
  surf->zvoxsize = mVipVolVoxSizeZ(vol);
  surf->global = NULL;
  surf->gslice = NULL;

  surf->gradDim = gradDim;
  if (gradFile)
    strcpy(surf->grad_file_name, gradFile);

  vos = VipGetOffsetStructure(vol);
  off_pbl = VipOffsetPointBetweenLine(vol);
  off_lbs = VipOffsetLineBetweenSlice(vol);

  ptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
  size = 0;

  for (z=dimz; z--; ptr+=off_lbs)
    for (y=dimy; y--; ptr+=off_pbl)
      for (x=dimx; x--;)
	if ((*ptr++) !=0 ) size++;

  surf->n_buckets=1;
  n = size;
  size /= ratio;

  surf->bucket = VipAllocG3DBucket( size );

  ptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
  point = surf->bucket->data;
  surf->bucket->n_points = size;

  surf->dilated_bucket_allocated = VFALSE;
  surf->dilated_bucket = NULL;
  saut = 1000000;

  if (gradFile)
    {
      if (surf->gradDim == GRAD_2D)
	strcat(surf->grad_file_name, ".g2D\0");
      else
	strcat(surf->grad_file_name, ".g3D\0");
      
      gradfile = fopen(surf->grad_file_name, "r");
    }
 
  if (gradfile)
    {
      for (z=0;z<dimz;z++, ptr+=off_lbs)
	for (y=0;y<dimy;y++, ptr+=off_pbl)
	  for (x=0;x<dimx;x++)
	    {
	      if (saut--==0)
		{
		  if (*ptr != 0)
		    {
				
		      size--;
		      point->p.x = x;
		      point->p.y = y;
		      point->p.z = z;
		      flag_boucle = VTRUE;

		      while(flag_boucle)
			{
			  if (surf->gradDim == GRAD_2D)
			    {
			      nlus = fread((void *)&g2D, sizeof(VipG2DPoint), 
					   1, gradfile);
			      if (nlus == 0)
				{
				  if (surf->bucket) VipFree(surf->bucket);
				  if (surf) VipFree(surf);
				  return((Surface *)NULL);
				}

			      if ((x == g2D.p.x) && (y == g2D.p.y) && (z == g2D.p.z))
				flag_boucle = VFALSE;
			    }
			  else
			    {
			      nlus = fread((void *)&g3D, sizeof(VipG3DPoint),
					   1, gradfile);
			      if (nlus == 0)
				{
				  if (surf->bucket) VipFree(surf->bucket);
				  if (surf) VipFree(surf);
				  return((Surface *)NULL);
				}

			      if ((x == g3D.p.x) && (y == g3D.p.y) && (z == g3D.p.z))
				flag_boucle = VFALSE;
			    }
			}

		      if (surf->gradDim == GRAD_2D)
			{
			  point->g3D.x = g2D.g2D.x;
			  point->g3D.y = g2D.g2D.y;
			}
		      else
			{
			  point->g3D.x = g3D.g3D.x;
			  point->g3D.y = g3D.g3D.y;
			  point->g3D.z = g3D.g3D.z;
			}
		      point++;
		      saut = 1000000;
		    }
		  else saut = 0;
		}

	      if (((*ptr++)!=0) && ((n%ratio)==0) && (size))
		{	
		  proba = (float)((float)rand()/(float)RAND_MAX);
		  saut = (int)(ratio*proba);   
		}

	      if (*ptr!=0) n--;
	    }

      fclose(gradfile);
      surf->gradFilled = VTRUE;
    }
  else
    {
      for (z=0;z<dimz;z++, ptr+=off_lbs)
	for (y=0;y<dimy;y++, ptr+=off_pbl)
	  for (x=0;x<dimx;x++)
	    {
	      if (((*ptr++)!=0) && ((n--%ratio)==0) && (size))
		{	
		  size--;

		  point->p.x = x;
		  point->p.y = y;
		  point->p.z = z;
		  point++;
		}
	    }

      surf->gradFilled = VFALSE;
    }

  VipFree(vos);

  return( surf );
}			

/* Conversion d'une image sous forme Volume a une structure de liste (Surface) 
   des points non nuls, les coordonnees vont de 0 a size-1 */
Surface *VipVolumeToLittleSurfaceGrad2D( Volume *vol, int ratio, 
					 VipG2DBucket *gbuck )
{
  Surface *surf;
  register int size;
  register Vip_S16BIT *ptr;
  register int x,y,z;
  register VipG3DPoint *point;
  int dimx, dimy, dimz;
  int flag_boucle;
  int n;
  float proba;
  int saut;
  VipOffsetStruct *vos;
  long off_lbs, off_pbl;
  VipG2DPoint *gPt;

  surf = (Surface *)VipCalloc(1, sizeof(Surface),
			      "Error while allocating Surface structure.");
  if (!surf) return((Surface *)NULL);

  dimx = surf->xsize = mVipVolSizeX(vol);
  dimy = surf->ysize = mVipVolSizeY(vol);
  dimz = surf->zsize = mVipVolSizeZ(vol);
  surf->xvoxsize = mVipVolVoxSizeX(vol);
  surf->yvoxsize = mVipVolVoxSizeY(vol);
  surf->zvoxsize = mVipVolVoxSizeZ(vol);
  surf->global = NULL;
  surf->gslice = NULL;

  surf->gradDim = GRAD_2D;

  vos = VipGetOffsetStructure(vol);
  off_pbl = VipOffsetPointBetweenLine(vol);
  off_lbs = VipOffsetLineBetweenSlice(vol);

  ptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
  size = 0;

  for (z=dimz; z--; ptr+=off_lbs)
    for (y=dimy; y--; ptr+=off_pbl)
      for (x=dimx; x--;)
	if ((*ptr++) !=0 ) size++;

  surf->n_buckets=1;
  n = size;
  size /= ratio;

  surf->bucket = VipAllocG3DBucket( size );

  ptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
  point = surf->bucket->data;
  surf->bucket->n_points = size;

  surf->dilated_bucket_allocated = VFALSE;
  surf->dilated_bucket = NULL;
  saut = 1000000;

  if ((gbuck != NULL) && (gbuck->data != NULL))
    {
      gPt = gbuck->data;

      for (z=0;z<dimz;z++, ptr+=off_lbs)
	for (y=0;y<dimy;y++, ptr+=off_pbl)
	  for (x=0;x<dimx;x++)
	    {
	      if (saut--==0)
		{
		  if (*ptr != 0)
		    {
		      size--;
		      point->p.x = x;
		      point->p.y = y;
		      point->p.z = z;
		      flag_boucle = VTRUE;

		      while(flag_boucle)
			{
			  if ((x == gPt->p.x) && (y == gPt->p.y) && 
			      (z == gPt->p.z))
			    flag_boucle = VFALSE;
			  else gPt++;
			}

		      point->g3D.x = gPt->g2D.x;
		      point->g3D.y = gPt->g2D.y;
		      point++;
		      gPt++;
		      saut = 1000000;
		    }
		  else saut = 0;
		}

	      if (((*ptr++)!=0) && ((n%ratio)==0) && (size))
		{	
		  proba = (float)((float)rand()/(float)RAND_MAX);
		  saut = (int)(ratio*proba);   
		}

	      if (*ptr!=0) n--;
	    }

      surf->gradFilled = VTRUE;
    }
  else
    {
      for (z=0;z<dimz;z++, ptr+=off_lbs)
	for (y=0;y<dimy;y++, ptr+=off_pbl)
	  for (x=0;x<dimx;x++)
	    {
	      if (((*ptr++)!=0) && ((n--%ratio)==0) && (size))
		{	
		  size--;

		  point->p.x = x;
		  point->p.y = y;
		  point->p.z = z;
		  point++;
		}
	    }

      surf->gradFilled = VFALSE;
    }

  VipFree(vos);

  return( surf );
}			

/* Conversion d'une image sous forme Volume a une structure de liste (Surface) 
   des points non nuls, les coordonnees vont de 0 a size-1 */
Surface *VipVolumeToLittleSurfaceGrad3D( Volume *vol, int ratio, 
					 VipG3DBucket *gbuck )
{
  Surface *surf;
  register int size;
  register Vip_S16BIT *ptr;
  register int x,y,z;
  register VipG3DPoint *point;
  int dimx, dimy, dimz;
  int flag_boucle;
  int n;
  float proba;
  int saut;
  VipOffsetStruct *vos;
  long off_lbs, off_pbl;
  VipG3DPoint *gPt;

  surf = (Surface *)VipCalloc(1, sizeof(Surface),
			      "Error while allocating Surface structure.");
  if (!surf) return((Surface *)NULL);

  dimx = surf->xsize = mVipVolSizeX(vol);
  dimy = surf->ysize = mVipVolSizeY(vol);
  dimz = surf->zsize = mVipVolSizeZ(vol);
  surf->xvoxsize = mVipVolVoxSizeX(vol);
  surf->yvoxsize = mVipVolVoxSizeY(vol);
  surf->zvoxsize = mVipVolVoxSizeZ(vol);
  surf->global = NULL;
  surf->gslice = NULL;

  surf->gradDim = GRAD_3D;

  vos = VipGetOffsetStructure(vol);
  off_pbl = VipOffsetPointBetweenLine(vol);
  off_lbs = VipOffsetLineBetweenSlice(vol);

  ptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
  size = 0;

  for (z=dimz; z--; ptr+=off_lbs)
    for (y=dimy; y--; ptr+=off_pbl)
      for (x=dimx; x--;)
	if ((*ptr++) !=0 ) size++;

  surf->n_buckets=1;
  n = size;
  size /= ratio;

  surf->bucket = VipAllocG3DBucket( size );

  ptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
  point = surf->bucket->data;
  surf->bucket->n_points = size;

  surf->dilated_bucket_allocated = VFALSE;
  surf->dilated_bucket = NULL;
  saut = 1000000;

  if ((gbuck != NULL) && (gbuck->data != NULL))
    {
      gPt = gbuck->data;

      for (z=0;z<dimz;z++, ptr+=off_lbs)
	for (y=0;y<dimy;y++, ptr+=off_pbl)
	  for (x=0;x<dimx;x++)
	    {
	      if (saut--==0)
		{
		  if (*ptr != 0)
		    {
		      size--;
		      point->p.x = x;
		      point->p.y = y;
		      point->p.z = z;
		      flag_boucle = VTRUE;

		      while(flag_boucle)
			{
			  if ((x == gPt->p.x) && (y == gPt->p.y) && 
			      (z == gPt->p.z))
			    flag_boucle = VFALSE;
			  else gPt++;
			}

		      point->g3D.x = gPt->g3D.x;
		      point->g3D.y = gPt->g3D.y;
		      point->g3D.z = gPt->g3D.z;
		      point++;
		      gPt++;
		      saut = 1000000;
		    }
		  else saut = 0;
		}

	      if (((*ptr++)!=0) && ((n%ratio)==0) && (size))
		{	
		  proba = (float)((float)rand()/(float)RAND_MAX);
		  saut = (int)(ratio*proba);   
		}

	      if (*ptr!=0) n--;
	    }

      surf->gradFilled = VTRUE;
    }
  else
    {
      for (z=0;z<dimz;z++, ptr+=off_lbs)
	for (y=0;y<dimy;y++, ptr+=off_pbl)
	  for (x=0;x<dimx;x++)
	    {
	      if (((*ptr++)!=0) && ((n--%ratio)==0) && (size))
		{	
		  size--;

		  point->p.x = x;
		  point->p.y = y;
		  point->p.z = z;
		  point++;
		}
	    }

      surf->gradFilled = VFALSE;
    }

  VipFree(vos);

  return( surf );
}

/* Conversion d'une image sous forme Volume a une structure de liste (Surface) 
   des points non nuls, les coordonnees vont de 0 a size-1 */
Surface *VipVolumeToSurfaceDumb( Volume *vol, char *gradFile, int gradDim )
{
  Surface *surf;
  register int size;	/* Cette variable va servir a calculer le nombre
			   de points non nul dans le volume, il serait 
			   certainement possible de le calculer au cours d'une 
			   etape prealable et de le passer en parametre */
  register Vip_S16BIT *ptr;
  register int x,y,z;
  register VipG3DPoint *point;
  FILE *gradfile=NULL;
  int nlus, dimx, dimy, dimz;
  int flag_boucle;
  VipOffsetStruct *vos;
  long off_lbs, off_pbl;
  VipG2DPoint g2D;
  VipG3DPoint g3D;
	
  surf = (Surface *)VipCalloc(1, sizeof(Surface), 
			      "Error while allocating Surface structure.");
  if (!surf) return((Surface *)NULL);

  dimx = surf->xsize = mVipVolSizeX(vol);
  dimy = surf->ysize = mVipVolSizeY(vol);
  dimz = surf->zsize = mVipVolSizeZ(vol);
  surf->xvoxsize = mVipVolVoxSizeX(vol);
  surf->yvoxsize = mVipVolVoxSizeY(vol);
  surf->zvoxsize = mVipVolVoxSizeZ(vol);
  surf->global = NULL;
  surf->gslice = NULL;

  surf->gradDim = gradDim;

  if (gradFile)
    strcpy(surf->grad_file_name, gradFile);

  vos = VipGetOffsetStructure(vol);
  off_pbl = VipOffsetPointBetweenLine(vol);
  off_lbs = VipOffsetLineBetweenSlice(vol);

  ptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
  size = 0;

  for (z=dimz; z--; ptr+=off_lbs)
    for (y=dimy; y--; ptr+=off_pbl)
      for (x=dimx; x--;)
	if ((*ptr++) !=0 ) size++;

  surf->n_buckets = 1;
  surf->bucket = VipAllocG3DBucket( size );

  ptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
  point = surf->bucket->data;
  surf->bucket->n_points = size;

  surf->dilated_bucket_allocated = VFALSE;
  surf->dilated_bucket = NULL;

  if (gradFile)
    {
      if (surf->gradDim == GRAD_2D)
        strcat(surf->grad_file_name, ".g2D\0");
      else
        strcat(surf->grad_file_name, ".g3D\0");

      gradfile = fopen(surf->grad_file_name, "r");
    }

  if (gradfile)
    {	
      for (z=0;z<dimz;z++, ptr+=off_lbs)
	for (y=0;y<dimy;y++, ptr+=off_pbl)
	  for (x=0;x<dimx;x++)
	    {
	      if ((*ptr++) != 0)
		{	
		  point->p.x = x;
		  point->p.y = y;
		  point->p.z = z;
		  flag_boucle = VTRUE;

		  while(flag_boucle)
		    {
		      if (surf->gradDim == GRAD_2D)
			{
			  nlus = fread((void *)&g2D, sizeof(VipG2DPoint), 
				       1, gradfile);
			  if (nlus == 0)
			    {
			      if (surf->bucket) VipFree(surf->bucket);
			      if (surf) VipFree(surf);
			      return((Surface *)NULL);
			    }

			  if ((x == g2D.p.x) && (y == g2D.p.y) && (z == g2D.p.z))
			    flag_boucle = VFALSE;
			}
		      else
			{
			  nlus = fread((void *)&g3D, sizeof(VipG3DPoint),
				       1, gradfile);
			  if (nlus == 0)
			    {
			      if (surf->bucket) VipFree(surf->bucket);
			      if (surf) VipFree(surf);
			      return((Surface *)NULL);
			    }

			  if ((x == g3D.p.x) && (y == g3D.p.y) && (z == g3D.p.z))
			    flag_boucle = VFALSE;
			}
		    }

		  if (surf->gradDim == GRAD_2D)
		    {
		      point->g3D.x = g2D.g2D.x;
		      point->g3D.y = g2D.g2D.y;
		    }
		  else
		    {
		      point->g3D.x = g3D.g3D.x;
		      point->g3D.y = g3D.g3D.y;
		      point->g3D.z = g3D.g3D.z;
		    }
		  point++;
		}
	    }

      fclose (gradfile);
      surf->gradFilled = VTRUE;
    }
  else
    {
      for (z=0;z<dimz;z++, ptr+=off_lbs)
	for (y=0;y<dimy;y++, ptr+=off_pbl)
	  for (x=0;x<dimx;x++)
	    {
	      if ((*ptr++)!=0)
		{	
		  point->p.x = x;
		  point->p.y = y;
		  point->p.z = z;
		  point++;
		}
	    }

      surf->gradFilled = VFALSE;
    }

  VipFree(vos);

  return( surf );
}			

/* Conversion d'une image sous forme Volume a une structure de liste (Surface) 
   des points non nuls, les coordonnees vont de 0 a size-1 */
Surface *VipVolumeToSurface( Volume *vol, char *gradFile, int gradDim )
{
  Surface *surf;
  register int size;	
  register Vip_S16BIT *ptr;
  register int x,y,z;
  register VipG3DPoint *point;
  FILE *gradfile=NULL;
  int nlus, dimx, dimy, dimz;
  int flag_boucle;
  VipOffsetStruct *vos;
  long off_lbs, off_pbl;
  VipG2DPoint g2D;
  VipG3DPoint g3D;
	
  surf = (Surface *)VipCalloc(1,sizeof(Surface),
			      "Error while allocating Surface structure");
  if (!surf) return((Surface *)NULL);

  dimx = surf->xsize = mVipVolSizeX(vol);
  dimy = surf->ysize = mVipVolSizeY(vol);
  dimz = surf->zsize = mVipVolSizeZ(vol);
  surf->xvoxsize = mVipVolVoxSizeX(vol);
  surf->yvoxsize = mVipVolVoxSizeY(vol);
  surf->zvoxsize = mVipVolVoxSizeZ(vol);
  surf->global = NULL;
  surf->gslice = NULL;

  surf->gradDim = gradDim;

  if (gradFile)
    strcpy(surf->grad_file_name, gradFile);

  vos = VipGetOffsetStructure(vol);
  off_pbl = VipOffsetPointBetweenLine(vol);
  off_lbs = VipOffsetLineBetweenSlice(vol);

  ptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
  size = 0;

  for (z=dimz; z--; ptr+=off_lbs)
    for (y=dimy; y--; ptr+=off_pbl)
      for (x=dimx; x--;)
	if ((*ptr++) !=0 ) size++;

  surf->n_buckets = 1;
  surf->bucket = VipAllocG3DBucket( size );

  ptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;
  point = surf->bucket->data;
  surf->bucket->n_points = size;

  surf->dilated_bucket_allocated = VFALSE;
  surf->dilated_bucket = NULL;

  if (gradFile)
    {
      if (surf->gradDim == GRAD_2D)
        strcat(surf->grad_file_name, ".g2D\0");
      else
        strcat(surf->grad_file_name, ".g3D\0");

      gradfile = fopen(surf->grad_file_name, "r");
    }

  if (gradfile)
    {	
      for (z=0;z<dimz;z++, ptr+=off_lbs)
	for (y=0;y<dimy;y++, ptr+=off_pbl)
	  for (x=0;x<dimx;x++)
	    {
	      if ((*ptr++) != 0)
		{	
		  point->p.x = x;
		  point->p.y = y;
		  point->p.z = z;
		  flag_boucle = VTRUE;

		  while(flag_boucle)
		    {
		      if (surf->gradDim == GRAD_2D)
			{
			  nlus = fread((void *)&g2D, sizeof(VipG2DPoint), 
				       1, gradfile);
			  if (nlus == 0)
			    {
			      if (surf->bucket) VipFree(surf->bucket);
			      if (surf) VipFree(surf);
			      return((Surface *)NULL);
			    }

			  if ((x == g2D.p.x) && (y == g2D.p.y) && (z == g2D.p.z))
			    flag_boucle = VFALSE;
			}
		      else
			{
			  nlus = fread((void *)&g3D, sizeof(VipG3DPoint),
				       1, gradfile);
			  if (nlus == 0)
			    {
			      if (surf->bucket) VipFree(surf->bucket);
			      if (surf) VipFree(surf);
			      return((Surface *)NULL);
			    }

			  if ((x == g3D.p.x) && (y == g3D.p.y) && (z == g3D.p.z))
			    flag_boucle = VFALSE;
			}
		    }

		  if (surf->gradDim == GRAD_2D)
		    {
		      point->g3D.x = g2D.g2D.x;
		      point->g3D.y = g2D.g2D.y;
		    }
		  else
		    {
		      point->g3D.x = g3D.g3D.x;
		      point->g3D.y = g3D.g3D.y;
		      point->g3D.z = g3D.g3D.z;
		    }
		  point++;
		}
	    }
      fclose (gradfile);
      surf->gradFilled = VTRUE;
    }
  else
    {
      for (z=0;z<dimz;z++, ptr+=off_lbs)
	for (y=0;y<dimy;y++, ptr+=off_pbl)
	  for (x=0;x<dimx;x++)
	    {
	      if ((*ptr++) != 0)
		{	
		  point->p.x = x;
		  point->p.y = y;
		  point->p.z = z;
		  point++;
		}
	    }

      surf->gradFilled = VFALSE;
    }

  VipFree(vos);

  return( surf );
}			

int VipFreeSurface(Surface *surf)
{
  if (surf == NULL) return(OK);

  if (surf->bucket) VipFreeG3DBucket(surf->bucket);
  if (surf->dilated_bucket) VipFree3DBucket_VFLOAT(surf->dilated_bucket);
  if (surf->global) VipFree3DBucket_S16BIT(surf->global);
  if (surf->gslice) VipFree(surf->gslice);
  if (surf) VipFree(surf);

  return(OK);
}
