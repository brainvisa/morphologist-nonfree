/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/interpol.c       * TYPE     : Function
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

#include <vip/matching/interpol.h>

/* Interpole les resultats dans chaque direction par un polynome du deuxieme 
   degre pour trouver un offset reel (dist = a*x*x+b*x+c) */
Vip3DPoint_VFLOAT *VipGetPolynomInterpolBestPosition( SuperList *list, 
						      Volume *dmap, 
						      Vip3DPoint_S16BIT *best_offset,
						      Dist6Connexe *distvoisins)
{
  Vip3DPoint_VFLOAT *interp_offset;
  double gauche, droite, best;
  double a,b,c;
  Vip3DPoint_S16BIT offset, maxoffset;

  interp_offset = (Vip3DPoint_VFLOAT *)VipCalloc(1, sizeof(Vip3DPoint_VFLOAT),
						 "Error during allocation");
  if (!interp_offset) 
    {
      return((Vip3DPoint_VFLOAT *)NULL);
    }

  maxoffset.x = 1;
  maxoffset.y = 1;
  maxoffset.z = 1;

  offset.x = best_offset->x;
  offset.y = best_offset->y;
  offset.z = best_offset->z;

  FillSuperPointExternalFlag(list, dmap, &offset, &maxoffset);

  interp_offset->x = best_offset->x;
  interp_offset->y = best_offset->y;
  interp_offset->z = best_offset->z;

  best = distvoisins->central;
  c = best;

  /* Interpolation dans la direction des x */
  gauche = distvoisins->xmoins;
  droite = distvoisins->xplus;
  if ( (gauche<=best) || (droite<=best) )
    {
      /* do nothing */
    }
  else
    {
      a = (gauche + droite)/2.0 - c;
      b = (droite - gauche)/2.0;
      if (a>1E-3) interp_offset->x += (float)(-b/(2.*a));
    }

  /* Interpolation dans la direction des y */
  gauche = distvoisins->ymoins;
  droite = distvoisins->yplus;
  if ( (gauche<=best) || (droite<=best) )
    {
      /* do nothing */
    }
  else
    {
      a = (gauche + droite)/2. - c;
      b = (droite - gauche)/2.;
      if (a>1E-3) interp_offset->y += (float)(-b/(2.*a));
    }

  /* Interpolation dans la direction des z */
  gauche = distvoisins->zmoins;
  droite = distvoisins->zplus;
  if ( (gauche<=best) || (droite<=best) )
    {
      /* do nothing */
    }
  else
    {
      a = (gauche + droite)/2. - c;
      b = (droite - gauche)/2.;
      if (a>1E-3) interp_offset->z += (float)(-b/(2.*a));
    }

  return(interp_offset);
}
