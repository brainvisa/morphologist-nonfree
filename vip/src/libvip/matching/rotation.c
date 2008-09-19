/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/rotation.c       * TYPE     : Function
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
 *            09/02/99| POUPON F.    | Passage en ANSI pour VIP
 *****************************************************************************/

#include <vip/matching/rotation.h>
#include <vip/util.h>

/* retourne un bucket apres rotation et translation reelle,
   xref = inv(echref)*(d->r*echrecal*xrecal+d->t) 
   interpol decide du type d'interpolation: PLUS_PROCHE_VOISIN, TRILINEAIRE*/
SuperList *VipGetRotatedBucket( Vip3DBucket_VFLOAT *buck, VipDeplacement *d, 
				Echelle *echref, int interpol )
{
  SuperList *newl=NULL;

  if (!buck || !buck->data || !d || !echref)
    {
      return((SuperList *)NULL);
    }

  if (interpol == PLUS_PROCHE_VOISIN)
      newl = ApplyRotationWithPPVoisin( buck, d, echref );
  else if (interpol == TRILINEAIRE)
      newl = ApplyRotationWithInterpolTrilin( buck, d, echref );

  return(newl);
}

SuperList *ApplyRotationWithInterpolTrilin( Vip3DBucket_VFLOAT *buck, 
					    VipDeplacement *d, 
					    Echelle *echref )
{
  SuperList *newl;
  SuperPoint *superptr;
  Vip3DPoint_VFLOAT *ptr;
  float invxref, invyref, invzref;
  float tx, ty, tz;
  float xreel, yreel, zreel;
  float rxx, rxy, rxz, ryx, ryy, ryz, rzx, rzy, rzz;
  float x, y, z;
  float Ex, Ey, Ez;
  float xcoef, ycoef, zcoef, *coefptr;
  float xcoefop, ycoefop, zcoefop;
  int i, k;

  if ((echref->x<EPSILON) || (echref->y<EPSILON) || (echref->z<EPSILON))
    {
      return((SuperList *)NULL);
    }

  newl = CreateSuperList( buck->n_points );

  newl->interpol = TRILINEAIRE;

  superptr = newl->data;
  ptr = buck->data;

  invxref = 1.0f/echref->x;
  invyref = 1.0f/echref->y;
  invzref = 1.0f/echref->z;
  tx = d->t.x;
  ty = d->t.y;
  tz = d->t.z;
  rxx = (float)d->r.xx;
  rxy = (float)d->r.xy;
  rxz = (float)d->r.xz;
  ryx = (float)d->r.yx;
  ryy = (float)d->r.yy;
  ryz = (float)d->r.yz;
  rzx = (float)d->r.zx;
  rzy = (float)d->r.zy;
  rzz = (float)d->r.zz;
  /* lourd, mais devrait a priori accelerer un peu?*/

  i = buck->n_points;

  while (i--)
    {
      /* Coord du point dans l'espace de reference */
      xreel = ptr->x;
      yreel = ptr->y;
      zreel = ptr->z;
      x = rxx*xreel + rxy*yreel + rxz*zreel + tx;
      y = ryx*xreel + ryy*yreel + ryz*zreel + ty;
      z = rzx*xreel + rzy*yreel + rzz*zreel + tz;
      x = x * invxref;
      y = y * invyref;
      z = z * invzref; 

      /* Coord entiere et coef de l'interpolation */
      Ex = (float)floor((double)x);
      Ey = (float)floor((double)y);
      Ez = (float)floor((double)z);

      superptr->p.x = (int)Ex;
      superptr->p.y = (int)Ey;
      superptr->p.z = (int)Ez;

      xcoef = x - Ex;
      xcoefop = 1.0f - xcoef;
      ycoef = y - Ey;
      ycoefop = 1.0f - ycoef;
      zcoef = z - Ez;
      zcoefop = 1.0f - zcoef;

      superptr->out = VTRUE;  /*par defaut */

      superptr->coef = (float *)VipCalloc(8, sizeof(float), "");
      if (!superptr->coef)
	{
	  for (k=i; k<buck->n_points; k++)
	    {
	      superptr--;
	      if (superptr->coef) VipFree(superptr->coef);
	    }
	  if (newl->data) VipFree(newl->data);
	  if (newl) VipFree(newl);
	  return((SuperList *)NULL);
	}
      coefptr = superptr->coef;

      /* L'ordre de parcours devra etre respecte lors de
	 l'interpolation:
	 x,y,z: 0,0,0 0,0,1 0,1,0 etc... (code binaire) */

      *coefptr++ = xcoefop * ycoefop * zcoefop;
      *coefptr++ = xcoefop * ycoefop * zcoef;
      *coefptr++ = xcoefop * ycoef * zcoefop;
      *coefptr++ = xcoefop * ycoef * zcoef;
      *coefptr++ = xcoef * ycoefop * zcoefop;
      *coefptr++ = xcoef * ycoefop * zcoef;
      *coefptr++ = xcoef * ycoef * zcoefop;
      *coefptr = xcoef * ycoef * zcoef;

      superptr++;
      ptr++;
    }

  newl->n_points = newl->size;

  return(newl);
}

/* retourne un bucket apres rotation et translation reelle */
SuperList *ApplyRotationWithPPVoisin( Vip3DBucket_VFLOAT *buck, 
				      VipDeplacement *d, 
				      Echelle *echref )
{
  SuperList *newl;
  SuperPoint *superptr;
  Vip3DPoint_VFLOAT *ptr;
  float invxref, invyref, invzref;
  float rxx, rxy, rxz, ryx, ryy, ryz, rzx, rzy, rzz;
  float tx, ty, tz;
  float xreel, yreel, zreel;
  int i;

  if ((echref->x<EPSILON) || (echref->y<EPSILON) || (echref->z<EPSILON))
    {
      return((SuperList *)NULL);
    }

  newl = CreateSuperList( buck->n_points );

  newl->interpol = PLUS_PROCHE_VOISIN;

  superptr = newl->data;
  ptr = buck->data;

  invxref = 1.0f/echref->x;
  invyref = 1.0f/echref->y;
  invzref = 1.0f/echref->z;
  tx = d->t.x;
  ty = d->t.y;
  tz = d->t.z;
  rxx = (float)d->r.xx;
  rxy = (float)d->r.xy;
  rxz = (float)d->r.xz;
  ryx = (float)d->r.yx;
  ryy = (float)d->r.yy;
  ryz = (float)d->r.yz;
  rzx = (float)d->r.zx;
  rzy = (float)d->r.zy;
  rzz = (float)d->r.zz;

  /* lourd, mais devrait a priori accelerer un peu?*/

  i = buck->n_points;

  while (i--)
    {
      xreel = ptr->x;
      yreel = ptr->y;
      zreel = ptr->z;
      superptr->p.x = mVipRint((double)((rxx*xreel +
					 rxy*yreel + 
					 rxz*zreel + tx) * invxref));
      superptr->p.y = mVipRint((double)((ryx*xreel + 
					 ryy*yreel + 
					 ryz*zreel + ty) * invyref));
      superptr->p.z = mVipRint((double)((rzx*xreel +
					 rzy*yreel + 
					 rzz*zreel + tz) * invzref));
      ptr++;
      superptr++;
    }

  newl->n_points = newl->size;

  return(newl);
}

int ConvertRotationTabDouble22( VipRotation *rot, double trot[3][3] )
{
  if (!rot || !trot)
    {
      return(PB);
    }

  trot[0][0] = (double)rot->xx;
  trot[0][1] = (double)rot->xy;
  trot[0][2] = (double)rot->xz;
  trot[1][0] = (double)rot->yx;
  trot[1][1] = (double)rot->yy;
  trot[1][2] = (double)rot->yz;
  trot[2][0] = (double)rot->zx;
  trot[2][1] = (double)rot->zy;
  trot[2][2] = (double)rot->zz;

  return(OK);
}

int ConvertTabDouble22Rotation( double trot[3][3], VipRotation *rot)
{
  if (!rot || !trot)
    {
      return(PB);
    }

  rot->xx=(float)trot[0][0];
  rot->xy=(float)trot[0][1];	
  rot->xz=(float)trot[0][2];
  rot->yx=(float)trot[1][0];
  rot->yy=(float)trot[1][1];
  rot->yz=(float)trot[1][2];
  rot->zx=(float)trot[2][0];
  rot->zy=(float)trot[2][1];
  rot->zz=(float)trot[2][2];

  return(OK);
}
