/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/dilatation.c     * TYPE     : Function
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

#include <vip/matching/dilatation.h>

/* dilatation (positive ou negative) dans chaque coupe qui cherche a corriger 
   le fait que la segmentation ne donne pas exactement la meme structure dans 
   les deux modalites:
*/
int VipFillDilatedBucket( Surface *surf, float dilation )
{
  if (!surf)
    {
      return(PB);
    }

  if ( (dilation>20.0) || (dilation<-20.0) )
    {
      return(PB);
    }

  if (!surf->bucket)
    {
      return(PB);
    }

  if (surf->dilated_bucket_allocated==VFALSE)
    {
      surf->dilated_bucket = VipAlloc3DBucket_VFLOAT( (size_t)surf->bucket->n_points );
      surf->dilated_bucket_allocated = VTRUE;
    }

  if ( dilation==0.0 ) FillZeroDilatedBucket( surf );
  else if (surf->gradDim == GRAD_2D)
    {
      if (surf->gradFilled == VTRUE) 
	  FillGrad2DDilatedBucket( surf, dilation );
      else if (surf->gslicefilled == VTRUE)
	  FillGRayDilatedBucket( surf, dilation );
      else  return(PB);
    }
  else if (surf->gradDim == GRAD_3D)
    {
      if (surf->gradFilled == VTRUE)
	  FillGrad3DDilatedBucket( surf, dilation );
      else
	  FillG3DRayDilatedBucket( surf, dilation );
    }

  return(OK);
}

/* simple changement d'echelle */
int FillZeroDilatedBucket( Surface *surf )
{
  VipG3DPoint *ptr;
  Vip3DPoint_VFLOAT *fptr;
  int i;
  Echelle *echrecal;

  echrecal = GetSurfaceEchelle( surf );
  ptr = surf->bucket->data;
  fptr = surf->dilated_bucket->data;

  for (i=surf->bucket->n_points;i>0;i--)
    {		
      fptr->x = ptr->p.x * echrecal->x;
      fptr->y = ptr->p.y * echrecal->y;
      fptr->z = ptr->p.z * echrecal->z;
      fptr++;
      ptr++;
    }

  surf->dilated_bucket->n_points = surf->bucket->size;

  VipFree(echrecal);

  return(OK);
}

int FillGrad2DDilatedBucket( Surface *surf, float dilation )
{
  VipG3DPoint *ptr;
  Vip3DPoint_VFLOAT *fptr;
  int i;
  Echelle *echrecal;

  echrecal = GetSurfaceEchelle( surf );
  ptr = surf->bucket->data;
  fptr = surf->dilated_bucket->data;

  for (i=surf->bucket->n_points;i>0;i--)
    {		
      fptr->x = ptr->p.x * echrecal->x + ptr->g3D.x * dilation;
      fptr->y = ptr->p.y * echrecal->y + ptr->g3D.y * dilation;
      fptr->z = ptr->p.z * echrecal->z;		
      fptr++;
      ptr++;
    }

  surf->dilated_bucket->n_points = surf->bucket->size;

  VipFree(echrecal);

  return(OK);
}

int FillGrad3DDilatedBucket( Surface *surf, float dilation )
{
  VipG3DPoint *ptr;
  Vip3DPoint_VFLOAT *fptr;
  int i;
  Echelle *echrecal;

  echrecal = GetSurfaceEchelle( surf );
  ptr = surf->bucket->data;
  fptr = surf->dilated_bucket->data;

  for (i=surf->bucket->n_points;i>0;i--)
    {		
      fptr->x = ptr->p.x * echrecal->x + ptr->g3D.x * dilation;
      fptr->y = ptr->p.y * echrecal->y + ptr->g3D.y * dilation;
      fptr->z = ptr->p.z * echrecal->z + ptr->g3D.z * dilation;		
      fptr++;
      ptr++;
    }

  surf->dilated_bucket->n_points = surf->bucket->size;

  VipFree(echrecal);

  return(OK);
}

/* points dilates le long d'un rayon issu du centre de gravite de la coupe il 
   s'agit de deplacer chaque point de la distance dilation le long d'un rayon 
   definit par ce point et le g de la slice (oriente vers l'exterieur).
   on notera que ceci sous entend que g est grossierement le centre du cercle 
   approchant le contour contenu dans la slice, si ceci n'est pas verifie pour 
   certaines slices, il est necessaire de faire intervenir de l'info 3d pour 
   definir les centres de dilatation
*/
int FillGRayDilatedBucket( Surface *surf, float dilation )
{
  VipG3DPoint *ptr;
  Vip3DPoint_VFLOAT *fptr;
  int i;
  float rx, ry, norme;
  Echelle *echrecal;
  Vip2DPoint_VFLOAT *glist;

  if( (dilation>20.0) || (dilation<-20.0) )
    {
      return(PB);
    }

  echrecal = GetSurfaceEchelle( surf );
  glist = surf->gslice;
  ptr = surf->bucket->data;
  fptr = surf->dilated_bucket->data;

  for (i=surf->bucket->n_points;i>0;i--)
    {		
      fptr->x = ptr->p.x * echrecal->x;
      fptr->y = ptr->p.y * echrecal->y;
      fptr->z = ptr->p.z * echrecal->z;

      rx = fptr->x - glist[ptr->p.z].x;
      ry = fptr->y - glist[ptr->p.z].y;
      norme = (float)sqrt((double)(rx*rx+ry*ry));
      rx /= norme;
      ry /= norme;

      fptr->x += rx*dilation;
      fptr->y += ry*dilation;
	
      fptr++;
      ptr++;
    }

  surf->dilated_bucket->n_points = surf->bucket->size;

  VipFree(echrecal);

  return(OK);
}

/* points dilates le long d'un rayon issu du centre de gravite de la coupe il 
   s'agit de deplacer chaque point de la distance dilation le long d'un rayon 
   definit par ce point et le g de la slice (oriente vers l'exterieur).
   on notera que ceci sous entend que g est grossierement le centre du cercle 
   approchant le contour contenu dans la slice, si ceci n'est pas verifie pour 
   certaines slices, il est necessaire de faire intervenir de l'info 3d pour 
   definir les centres de dilatation
*/
int FillG3DRayDilatedBucket( Surface *surf, float dilation )
{
  VipG3DPoint *ptr;
  Vip3DPoint_VFLOAT *fptr;
  int i;
  float rx, ry, rz, norme;
  Echelle *echrecal;

  if( (dilation>20.0) || (dilation<-20.0) )
    {
      return(PB);
    }

  echrecal = GetSurfaceEchelle( surf );
  ptr = surf->bucket->data;
  fptr = surf->dilated_bucket->data;

  for (i=surf->bucket->n_points;i>0;i--)
    {		
      fptr->x = ptr->p.x * echrecal->x;
      fptr->y = ptr->p.y * echrecal->y;
      fptr->z = ptr->p.z * echrecal->z;

      rx = fptr->x - surf->g.x;
      ry = fptr->y - surf->g.y;
      rz = fptr->z - surf->g.z;
      norme = (float)sqrt((double)(rx*rx+ry*ry+rz*rz));
      rx /= norme;
      ry /= norme;
      rz /= norme;

      fptr->x += rx*dilation;
      fptr->y += ry*dilation;
      fptr->z += rz*dilation;
	
      fptr++;
      ptr++;
    }

  surf->dilated_bucket->n_points = surf->bucket->size;

  VipFree(echrecal);

  return(OK);
}

/* Calcule les coordonnees des g de chaques slices en tailles reelles, les g 
   sont ordonnes de la slice z=0 a la derniere */
int VipFillSurfaceGSlice( Surface *surf )
{
  Vip2DPoint_VFLOAT *list;
  float *xsommes;
  float *ysommes;
  int *npoints, nptotal;
  int nslices;
  int i;
  VipG3DPoint *ptr;
  int zcourant;

  if (!surf)
    {
      return(PB);
    }

  /*allocations*/
  nslices = surf->zsize;

  list = (Vip2DPoint_VFLOAT *)VipCalloc(nslices, sizeof(Vip2DPoint_VFLOAT),
					"Error while alocating structure");
  xsommes = (float *)VipCalloc(nslices, sizeof(float), "");
  ysommes = (float *)VipCalloc(nslices, sizeof(float), "");
  npoints = (int *)VipCalloc(nslices, sizeof(int), "");
  if (!list || !xsommes || !ysommes || !npoints)
    {
      return(PB);
    }

  ptr = surf->bucket->data;
  nptotal = surf->bucket->n_points;

  for (i=0;i<nslices;i++)
    {
      xsommes[i]=0.0;
      ysommes[i]=0.0;
      npoints[i]=0;
    }

  for (i=nptotal; i>0; i--)
    {
      zcourant = ptr->p.z;
      xsommes[zcourant] += ptr->p.x;
      ysommes[zcourant] += ptr->p.y;
      npoints[zcourant]++;
      ptr++;
    }

  for (i=0;i<nslices;i++)
    {
      list[i].x = xsommes[i] * surf->xvoxsize / (float)npoints[i];
      list[i].y = ysommes[i] * surf->yvoxsize / (float)npoints[i];
    }

  surf->gslice = list;
  surf->gslicefilled = VTRUE;

  VipFree(xsommes);
  VipFree(ysommes);
  VipFree(npoints);

  return(OK);
}
