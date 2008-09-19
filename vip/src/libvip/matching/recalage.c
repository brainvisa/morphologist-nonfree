/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/recalage.c       * TYPE     : Function
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

#include <vip/matching/recalage.h>
#include <vip/matching/dilatation.h>

/* realise un recalage en translation, la translation du deplacement est 
   corrigee en consequence, la distance minimum est retournee */
double VipGetMinDistTrilinAndFillBestTransForRot( Surface *surf, Volume *ref, 
						  VipDeplacement *dep )
{
  SuperList *sup;
  Echelle *echref;
  double distmin, estimdist, bestdist=INFINI;
  Vip3DPoint_S16BIT maxoffset, bestoffset, offsetnul;
  Vip3DPoint_VFLOAT *interp;
  float tx, ty, tz;
  int flag;
  float increment=0.1f;
  VipDeplacement bestdep;
  int compteur=0;
  Dist6Connexe distvoisins;

  if (!surf || !ref || !dep)
    {
      return(-1.0);
    }

  echref = GetVolumeEchelle(ref);

  maxoffset.x = 1;
  maxoffset.y = 1;
  maxoffset.z = 2;

  offsetnul.x = 0;
  offsetnul.y = 0;
  offsetnul.z = 0;

  /* Premiere minimisation en translation */
  sup = VipGetRotatedBucket( surf->dilated_bucket, dep, echref, TRILINEAIRE );

  distmin = VipFindBestPositionInDistanceMap( sup, ref, &maxoffset, 
					      &bestoffset, &distvoisins );

  interp = VipGetPolynomInterpolBestPosition( sup, ref, &bestoffset, 
					      &distvoisins );

  dep->t.x += interp->x * echref->x;
  dep->t.y += interp->y * echref->y;
  dep->t.z += interp->z * echref->z;

  KillSuperList(sup);

  sup = VipGetRotatedBucket( surf->dilated_bucket, dep, echref, TRILINEAIRE );

  estimdist = VipGetMoyenneSomSquareDistance( sup, ref, &offsetnul );

  if (estimdist < distmin) distmin = estimdist;
  else
    {
      dep->t.x -= interp->x * echref->x;
      dep->t.y -= interp->y * echref->y;
      dep->t.z -= interp->z * echref->z;
    }

  KillSuperList(sup);

  /* Minimisation sub-voxel */
  /* seuil arbitraire pour eviter des calculs inutiles loin de l'optimum*/
  if (distmin < 1.5)
    {
      flag = VTRUE;
      increment = 0.1f; /*step arbitraire raisonable*/
      VipDeplaFirstEgalDeplaSecond( &bestdep, dep );
      compteur = 0;
    }
  else flag = VFALSE;

  while (flag)
    {
      compteur++;
      bestdist = distmin;

      for ( tx = -increment; tx<= increment; tx += increment )
	for ( ty = -increment; ty<= increment; ty += increment )
	  for ( tz = -increment; tz<= increment; tz += increment )
	    {
	      if ((tx*ty==0.) && (tx*tz==0.) && (ty*tz==0.) && ((tx+ty+tz)!=0))
		{
		  dep->t.x += tx * echref->x;
		  dep->t.y += ty * echref->y;
		  dep->t.z += tz * echref->z;
		  sup = VipGetRotatedBucket( surf->dilated_bucket, dep, 
					     echref, TRILINEAIRE );

		  estimdist = VipGetMoyenneSomSquareDistance( sup, ref, 
							      &offsetnul );

		  if (estimdist < bestdist)
		    {
		      bestdist = estimdist;
		      bestdep.t.x = dep->t.x;
		      bestdep.t.y = dep->t.y;
		      bestdep.t.z = dep->t.z;
		    } 

		  dep->t.x -= tx * echref->x;
		  dep->t.y -= ty * echref->y;
		  dep->t.z -= tz * echref->z;

		  KillSuperList(sup);
		}
	    }

      if ((bestdist==distmin) || (compteur>=10))
	{
	  flag = VFALSE;
	}

      VipDeplaFirstEgalDeplaSecond( dep, &bestdep );

      distmin = bestdist;
    }

  VipFree(echref);
  VipFree(interp);

  return(distmin);
}

int VipDumbMainRegistration( Surface *surf, Volume *ref, VipDeplacement *dep, 
			     float incredef, float precidef )
{
  double best;
  VipDeplacement bestdep;
  int flag;
  float increment, precision;

  increment = incredef;
  precision = precidef;

  flag = VFALSE;

  while ((increment>=2*precision) || flag)
    {
      best = VipFindBestRotAroundGForAnAngle( surf, ref, dep,
					      &bestdep, increment );

      VipDeplaFirstEgalDeplaSecond( dep, &bestdep );

      increment /= 2.;

      if (increment<2*precision)
	{
	  if (flag!=VTRUE)
	    {
	      flag=VTRUE;
	      increment = precision;
	    }
	  else flag=VFALSE;
	}
    }

  return(OK);
}

/*seule la rotation autour de l'axe 0z est autorisee*/
int VipDumbMain2DRegistration( Surface *surf, Volume *ref, 
			       VipDeplacement *dep, float incredef, 
			       float precidef )
{
  double best;
  VipDeplacement bestdep;
  int flag;
  float increment, precision;

  increment = incredef;
  precision = precidef;

  flag = VFALSE;

  while ((increment>=2*precision) || flag)
    {
      best = VipFindBest2DRotAroundGForAnAngle( surf, ref, dep,
						&bestdep, increment );

      VipDeplaFirstEgalDeplaSecond( dep, &bestdep );

      increment /= 2.;

      if (increment<2*precision)
	{
	  if (flag!=VTRUE)
	    {
	      flag=VTRUE;
	      increment = precision;
	    }
	  else flag=VFALSE;
	}
    }

  return(OK);
}

int VipDestroyOutliers( Surface *surf, VipDeplacement *dep, Volume *ref )
{
  SuperList *sup;
  double best;
  VipG3DBucket *newbuck;
  VipG3DPoint *newptr, *oldptr;
  Echelle *echref;
  SuperPoint *supptr;
  int i, n;
  float seuil;
  float multseuil;
  Vip3DPoint_S16BIT offset_nul;

  echref = GetVolumeEchelle( ref );

  offset_nul.x = 0;
  offset_nul.y = 0;
  offset_nul.z = 0;

  sup = VipGetRotatedBucket( surf->dilated_bucket, dep, echref, TRILINEAIRE);

  best = VipGetMoyenneSomSquareDistance( sup, ref, &offset_nul );

  VipFillSuperPointErreurInterpolTrilin( sup, ref );
		
  if (sup->n_points != surf->bucket->n_points)
    {
      if (echref) VipFree(echref);
      if (sup) KillSuperList(sup);
      return(PB);
    }

  seuil = 1.5;

  multseuil = (float)(best*seuil);
  supptr = sup->data;
  n = 0;

  for (i=sup->n_points; i>0; i--)
    {
      if ((supptr++)->dist < multseuil ) n++;
    }

  newbuck = VipAllocG3DBucket(n);

  newptr = newbuck->data;
  supptr = sup->data;
  oldptr = surf->bucket->data;
  newbuck->n_points = newbuck->size;

  for (i=sup->size;i>0;i--)
    {
      if ((supptr++)->dist < multseuil )
	{		
	  newptr->p.x = oldptr->p.x;
	  newptr->p.y = oldptr->p.y;
	  newptr->p.z = oldptr->p.z;
	  newptr->g3D.x = oldptr->g3D.x;
	  newptr->g3D.y = oldptr->g3D.y;
	  newptr->g3D.z = oldptr->g3D.z;
	  newptr++;
	}

      oldptr++;
    }

  VipFreeG3DBucket( surf->bucket );
  VipFree3DBucket_VFLOAT( surf->dilated_bucket );

  surf->dilated_bucket_allocated=VFALSE;
  surf->dilated_bucket = NULL; 
  surf->bucket = newbuck;

  VipFree(echref);

  KillSuperList(sup);

  return(OK);
}

int VipDestroyOutliersParam( Surface *surf, VipDeplacement *dep, 
			     Volume *ref, float param )
{
  SuperList *sup;
  double best;
  VipG3DBucket *newbuck;
  VipG3DPoint *newptr, *oldptr;
  Echelle *echref;
  SuperPoint *supptr;
  int i, n;
  float seuil;
  float multseuil;
  Vip3DPoint_S16BIT offset_nul;

  echref = GetVolumeEchelle( ref );

  offset_nul.x = 0;
  offset_nul.y = 0;
  offset_nul.z = 0;

  sup = VipGetRotatedBucket( surf->dilated_bucket, dep, echref, TRILINEAIRE );

  best = VipGetMoyenneSomSquareDistance( sup, ref, &offset_nul );

  VipFillSuperPointErreurInterpolTrilin( sup, ref );
		
  if (sup->n_points != surf->bucket->n_points)
    {
      if (echref) VipFree(echref);
      if (sup) KillSuperList(sup);
      return(PB);
    }

  seuil = param;

  multseuil = (float)(best*seuil);
  supptr = sup->data;
  n = 0;

  for (i=sup->n_points; i>0; i--)
    {
      if ((supptr++)->dist < multseuil ) n++;
    }

  newbuck = VipAllocG3DBucket(n);

  newptr = newbuck->data;
  supptr = sup->data;
  oldptr = surf->bucket->data;
  newbuck->n_points = newbuck->size;

  for (i=sup->size;i>0;i--)
    {
      if ((supptr++)->dist < multseuil )
	{		
	  newptr->p.x = oldptr->p.x;
	  newptr->p.y = oldptr->p.y;
	  newptr->p.z = oldptr->p.z;
	  newptr->g3D.x = oldptr->g3D.x;
	  newptr->g3D.y = oldptr->g3D.y;
	  newptr->g3D.z = oldptr->g3D.z;
	  newptr++;
	}

      oldptr++;
    }

  VipFreeG3DBucket( surf->bucket );
  VipFree3DBucket_VFLOAT( surf->dilated_bucket );

  surf->dilated_bucket_allocated=VFALSE;
  surf->dilated_bucket = NULL; 
  surf->bucket = newbuck;

  VipFree(echref);

  KillSuperList(sup);

  return(OK);
}

/* depini: deplacement initial,
   newdep: deplacement retourne (doit etre prealablement alloue),
   g: Centre d'inertie de la surface a recaler (dans son repere),
   ech: echelle de la surface a recaler;
   newrot = incrrot * oldrot,
   newt = oldt + (I-incrrot)*oldrot*(g*ech);
*/
int VipIncrementeRotWithRotAroundG( VipDeplacement *depini, 
				    VipDeplacement *newdep, 
				    VipRotation *incrrot, Gravity *g, 
				    Echelle *ech )
{
  Vip3DPoint_VFLOAT realg, correction;
  VipDeplacement depinter;

  if (!depini || !newdep || !incrrot || !g || !ech)
    {
      return(PB);
    }

  realg.x = g->x * ech->x;
  realg.y = g->y * ech->y;
  realg.z = g->z * ech->z;

  /* Rotation autour du centre de gravite:
     ceci accelere enormement la minimisation en translation */

  VipDeplaFirstEgalDeplaSecond( &depinter, depini );

  VipApplyRotation( &(depinter.r), &realg, &correction );

  depinter.t.x += correction.x;
  depinter.t.y += correction.y;
  depinter.t.z += correction.z;

  VipApplyRotation( incrrot, &correction, &correction );

  depinter.t.x -= correction.x;
  depinter.t.y -= correction.y;
  depinter.t.z -= correction.z;

  VipMultiplyRotations( incrrot, &(depinter.r), &(depinter.r) );

  VipDeplaFirstEgalDeplaSecond(newdep, &depinter);

  return(OK);
}

double VipFindBestRotAroundGForAnAngle( Surface *surf, Volume *ref, 
					VipDeplacement *olddep, 
					VipDeplacement *bestdep, 
					float increment)
{
  float degx, degy, degz;
  VipDeplacement newdep, tempbest;
  double dist, bestdist, lastbest;
  VipRotation rot;
  Echelle *echrecal;
  float bestdegx=0.0, bestdegy=0.0, bestdegz=0.0;
  float lastbestdegx, lastbestdegy, lastbestdegz;
  int flag;

  if (!surf || !ref || !olddep || !bestdep || (increment==0.))
    {
      return(-1.0);
    }

  VipDeplaFirstEgalDeplaSecond( bestdep, olddep );

  echrecal = GetSurfaceEchelle( surf );

  flag = NB_ITERATION_MAX;
  bestdist = lastbest = INFINI;
  lastbestdegx = 1000.;
  lastbestdegy = 1000.;
  lastbestdegz = 1000.;

  while (flag)
    {
      bestdist = INFINI;

      for ( degx = -increment; degx<= increment; degx += increment )
	for ( degy = -increment; degy<= increment; degy += increment )
	  for ( degz = -increment; degz<= increment; degz += increment )
	    {
	      if ((degx*degy==0.) && (degx*degz==0.) && (degy*degz==0.))
		{
		  VipFillRotation( degx, degy, degz, &rot );

		  VipIncrementeRotWithRotAroundG( bestdep, &newdep, &rot,
						     &(surf->g),echrecal);

		  dist = VipGetMinDistTrilinAndFillBestTransForRot( surf, ref, 
								    &newdep );

		  if ( bestdist > dist )
		    {
		      bestdist = dist;
		      VipDeplaFirstEgalDeplaSecond( &tempbest, &newdep );

		      bestdegx = degx;
		      bestdegy = degy;
		      bestdegz = degz;
		    } 
		}
	    }

      VipDeplaFirstEgalDeplaSecond( bestdep, &tempbest );

      if (((bestdegx==0.) && (bestdegy==0.) && (bestdegz==0.))
	  || (fabs(bestdist-lastbest)<(bestdist*GAIN_RELATIF))
	  || ((lastbestdegx == -bestdegx) && (lastbestdegy== -bestdegy)
	      && (lastbestdegz==-bestdegz)) )
	{
	  flag = VFALSE;
	}
      else flag--;

      lastbest = bestdist;
      lastbestdegx = bestdegx;
      lastbestdegy = bestdegy;
      lastbestdegz = bestdegz;
    }

  VipFree(echrecal);

  return(bestdist);
}

double VipFindBest2DRotAroundGForAnAngle( Surface *surf, Volume *ref, 
					  VipDeplacement *olddep, 
					  VipDeplacement *bestdep, 
					  float increment )
{
  float degx, degy, degz;
  VipDeplacement newdep, tempbest;
  double dist, bestdist, lastbest;
  VipRotation rot;
  Echelle *echrecal;
  float bestdegx=0.0, bestdegy=0.0, bestdegz=0.0;
  float lastbestdegx, lastbestdegy, lastbestdegz;
  int flag;

  if (!surf || !ref || !olddep || !bestdep || (increment==0.))
    {
      return(-1.0);
    }

  VipDeplaFirstEgalDeplaSecond( bestdep, olddep );

  echrecal = GetSurfaceEchelle( surf );

  flag = NB_ITERATION_MAX;
  bestdist = lastbest = INFINI;
  lastbestdegx = 1000.;
  lastbestdegy = 1000.;
  lastbestdegz = 1000.;

  while (flag)
    {
      bestdist = INFINI;
      degx = 0.;
      degy = 0.;

      for ( degz = -increment; degz<= increment; degz += increment )
	{
	  if ((degx*degy==0.) && (degx*degz==0.) && (degy*degz==0.))
	    {
	      VipFillRotation( degx, degy, degz, &rot );

	      VipIncrementeRotWithRotAroundG( bestdep, &newdep, &rot,
						   &(surf->g), echrecal );

	      dist = VipGetMinDistTrilinAndFillBestTransForRot( surf, ref, 
								&newdep );

	      if ( bestdist > dist )
		{
		  bestdist = dist;
		  VipDeplaFirstEgalDeplaSecond( &tempbest, &newdep );

		  bestdegx = degx;
		  bestdegy = degy;
		  bestdegz = degz;
		} 
	    }
	}

      VipDeplaFirstEgalDeplaSecond( bestdep, &tempbest );

      if (((bestdegx==0.) && (bestdegy==0.) && (bestdegz==0.))
	  || (fabs(bestdist-lastbest)<(bestdist*GAIN_RELATIF))
	  || ((lastbestdegx == -bestdegx) && (lastbestdegy== -bestdegy)
	      && (lastbestdegz==-bestdegz)) )
	{
	  flag = VFALSE;
	}
      else flag--;

      lastbest = bestdist;
      lastbestdegx = bestdegx;
      lastbestdegy = bestdegy;
      lastbestdegz = bestdegz;
    }

  VipFree(echrecal);

  return(bestdist);
}

float VipFindBestDilationForDep( Surface *surf, Volume *ref, 
				 VipDeplacement *dep, float dilatini, 
				 float dilatincrement )
{
  double dist, bestdist=INFINI;
  float incr, bestincr, bestdilat, dilatincr;
  int flag, flagpas;
  VipDeplacement bestdep, newdep, initdep;
  int premiere_passe;

  if (!surf || !ref || !dep)
    {
      return((float)NO_BEST_DILATION);
    }

  dilatincr = dilatincrement;
  bestdilat = dilatini;
  flag = VTRUE;
  flagpas = VTRUE;

  VipDeplaFirstEgalDeplaSecond( &bestdep, dep );

  premiere_passe = VTRUE;

  while (flag)
    {
      VipDeplaFirstEgalDeplaSecond(&initdep, &bestdep);
      flagpas = VTRUE;
      if (premiere_passe==VTRUE) bestdist = INFINI;
      bestincr = 0.0;

      for ( incr = -dilatincr; incr<=dilatincr; incr+=dilatincr )
	{
	  if ((incr!=0.) || (premiere_passe==VTRUE))
	    {
	      VipDeplaFirstEgalDeplaSecond( &newdep, &initdep );
	      VipFillDilatedBucket( surf, bestdilat + incr );
	      dist = VipGetMinDistTrilinAndFillBestTransForRot( surf, ref,
								&newdep );

	      if (dist < bestdist)
		{
		  bestdist = dist;
		  bestincr = incr;

		  VipDeplaFirstEgalDeplaSecond( &bestdep, &newdep );
		}
	    }
	}

      premiere_passe = VFALSE;
      bestdilat += bestincr;

      if (bestincr == 0.0) flagpas=VFALSE;

      while (flagpas)
	{
	  VipDeplaFirstEgalDeplaSecond( &newdep, &initdep );
	  VipFillDilatedBucket( surf, bestdilat + bestincr );
	  dist = VipGetMinDistTrilinAndFillBestTransForRot( surf, ref,
							    &newdep );

	  if ( dist < bestdist )
	    {
	      bestdist = dist;
	      bestdilat += bestincr;

	      VipDeplaFirstEgalDeplaSecond( &bestdep, &newdep );
	    }
	  else flagpas = VFALSE;
	}

      dilatincr /= 2.;
      if (dilatincr<0.1) flag=VFALSE;
    }

  VipDeplaFirstEgalDeplaSecond( dep, &bestdep );
  VipFillDilatedBucket( surf, bestdilat );

  return(bestdilat);
}
