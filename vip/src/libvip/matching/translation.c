/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/translation.c    * TYPE     : Function
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

#include <vip/matching/translation.h>
#include <vip/matching/interpol.h>

/* Retourne la translation initiale obtenue par recalage des centres de 
   gravites: T = echref*gref - echrecal*grecal; le volume est l'objet de 
   reference, la surface l'objet a recaler */
Translation *VipGetInitialTranslation( Surface *surf, Volume *vol,
				       Gravity *volgrav)
{
  Translation *t;
  Echelle *echref, *echrecal;

  VipFillVolumeG( vol, volgrav );

  if (surf->bucket==NULL)
    {
      return((Translation *)NULL);
    }

  VipFillSurfG( surf );

  echref = GetVolumeEchelle( vol );
  echrecal = GetSurfaceEchelle( surf );

  t = CalculInitialTranslation( volgrav, echref, &(surf->g), echrecal);

  VipFree(echref);
  VipFree(echrecal);

  return(t);
}

Echelle *GetVolumeEchelle( Volume *vol )
{
  Echelle *e;

  if (!vol)
    {
      return((Echelle *)NULL);
    }

  e = (Echelle *)VipCalloc(1, sizeof(Echelle), 
			   "Error while allocating Echelle structure");
  if (!e) return((Echelle *)NULL);

  e->x = mVipVolVoxSizeX(vol);
  e->y = mVipVolVoxSizeY(vol);
  e->z = mVipVolVoxSizeZ(vol);

  return(e);
}

Echelle *GetSurfaceEchelle(Surface *surf)
{
  Echelle *e;

  if (!surf)
    {
      return((Echelle *)NULL);
    }

  e = (Echelle *)VipCalloc(1, sizeof(Echelle), 
			   "Error while allocating Echelle structure");
  if (!e) return((Echelle *)NULL);

  e->x = surf->xvoxsize;
  e->y = surf->yvoxsize;
  e->z = surf->zvoxsize;

  return(e);
}

/* Retourne la translation initiale obtenue par recalage des centres de 
   gravites: T = echref*gref - echrecal*grecal; ref sous-entend objet de 
   reference, recal objet a recaler / reference */	
Translation *CalculInitialTranslation( Gravity *gref, Echelle *echref, 
				       Gravity *grecal, Echelle *echrecal )
{
  Translation *t;

  if (!gref || !grecal || !echref || !echrecal)
    {
      return((Translation *)NULL);
    }

  t = (Translation *)VipCalloc(1, sizeof(Translation), 
			       "Error while allocating Translation structure");
  if (!t) return((Translation *)NULL);

  t->x = gref->x * echref->x - grecal->x * echrecal->x;
  t->y = gref->y * echref->y - grecal->y * echrecal->y;
  t->z = gref->z * echref->z - grecal->z * echrecal->z;

  return( t );
}

/* retourne un bucket apres translation reelle,
   xref = inv(echref)*(echrecal*xrecal+t) 
   interpol decide du type d'interpolation: PLUS_PROCHE_VOISIN, TRILINEAIRE*/
SuperList *VipGetTranslatedBucket( VipG3DBucket *buck, Translation *t, 
				   Echelle *echref, int interpol )
{
  SuperList *newl=NULL;

  if (!buck || !buck->data || !t || !echref)
    {
      return((SuperList *)NULL);
    }

  if (interpol == PLUS_PROCHE_VOISIN)
      newl = ApplyTranslationWithPPVoisin( buck, t, echref );
  else if (interpol == TRILINEAIRE)
      newl = ApplyTranslationWithInterpolTrilin( buck, t, echref );

  return(newl);
}

/* retourne un bucket apres translation reelle,
   xref = inv(echref)*(echrecal*xrecal+t)
   echrecal deja effectuee */
SuperList *ApplyTranslationWithPPVoisin( VipG3DBucket *buck, Translation *t, 
					 Echelle *echref )
{
  SuperList *newl;
  SuperPoint *superptr;
  VipG3DPoint *ptr;
  float invxref, invyref, invzref;
  float tx, ty, tz;
  int i;

  if ((echref->x<EPSILON) || (echref->y<EPSILON) || (echref->z<EPSILON))
    {
      return((SuperList *)NULL);
    }

  newl = CreateSuperList( buck->n_points );

  newl->interpol = PLUS_PROCHE_VOISIN;

  i = buck->n_points;
  superptr = newl->data;
  ptr = buck->data;

  invxref = 1.0f/echref->x;
  invyref = 1.0f/echref->y;
  invzref = 1.0f/echref->z;
  tx = t->x;
  ty = t->y;
  tz = t->z;

  /* lourd, mais devrait a priori accelerer un peu?*/
  while (i--)
    {
      superptr->p.x = mVipRint((double)((ptr->p.x + tx) * invxref));
      superptr->p.y = mVipRint((double)((ptr->p.y + ty) * invyref));
      superptr->p.z = mVipRint((double)((ptr->p.z + tz) * invzref)); 
      ptr++;
      superptr++;
    }

  newl->n_points = newl->size;

  return(newl);
}


/* retourne un bucket apres translation reelle,
   xref = inv(echref)*(echrecal*xrecal+t),
   alloue pour chaque point un tableau de 8 coefficients permettant une 
   interpolation lineaire lors du calcul de distance, les coordonnes entieres 
   du point obtenu verifient les proprietes suivantes:
   xentier<= xreel; yentier<= yreel; zentier<= zreel 
   echrecal deja effectue*/
SuperList *ApplyTranslationWithInterpolTrilin( VipG3DBucket *buck, 
					       Translation *t, 
					       Echelle *echref )
{
  SuperList *newl;
  SuperPoint *superptr;
  VipG3DPoint *ptr;
  float invxref, invyref, invzref;
  float tx, ty, tz;
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
  tx = t->x;
  ty = t->y;
  tz = t->z;

  /* lourd, mais devrait a priori accelerer un peu?*/
  i = buck->n_points;

  while (i--)
    {
      /* Coord du point dans l'espace de reference */
      x = (ptr->p.x + tx) * invxref;
      y = (ptr->p.y + ty) * invyref;
      z = (ptr->p.z + tz) * invzref; 

      /* Coord entiere et coef de l'interpolation */
      Ex = (float)floor((double)x);
      Ey = (float)floor((double)y);
      Ez = (float)floor((double)z);

      superptr->p.x = (Vip_S16BIT)Ex;
      superptr->p.y = (Vip_S16BIT)Ey;
      superptr->p.z = (Vip_S16BIT)Ez;

      superptr->out = VTRUE;  /*par default*/

      xcoef = x - Ex;
      xcoefop = 1.0f - xcoef;
      ycoef = y - Ey;
      ycoefop = 1.0f - ycoef;
      zcoef = z - Ez;
      zcoefop = 1.0f - zcoef;

      superptr->coef = (float *)VipCalloc( 8, sizeof(float), "");
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
