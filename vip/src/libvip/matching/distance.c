/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/distance.c       * TYPE     : Function
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

#include <vip/matching/distance.h>
#include <vip/matching/interpol.h>

extern float VIP_USED_DISTMAP_MULTFACT;

/* Retourne la somme des carres des distances de la liste de points par 
   rapport a la carte de distance dmap, apres un decalage specifie par 
   offset_3d, moyennee par le nombre de points effectivement dans le volume;
   pour chaque point, si le flag out est TRUE, on verifie qu'on reste bien 
   dans le volume, si le flag out est FALSE, utilisation de saut */
double VipGetMoyenneSomSquareDistance( SuperList *list, Volume *dmap, 
				       Vip3DPoint_S16BIT *offset_3d )
{
  double dist=0.0;

  if ((list->interpol!=PLUS_PROCHE_VOISIN) && (list->interpol!=TRILINEAIRE))
    {
      return(-1.0);
    }

  if (list->interpol==PLUS_PROCHE_VOISIN)
      dist = GetMoyenneSomSquareDistancePPVoisin( list, dmap, offset_3d);
  else if (list->interpol==TRILINEAIRE)
      dist = GetMoyenneSomSquareDistanceInterpolTrilin( list, dmap, offset_3d);
	
  return(dist);
}

/* Retourne la somme des carres des distances de la liste de points par 
   rapport a la carte de distance dmap, apres un decalage specifie par 
   offset_3d, moyennee par le nombre de points effectivement dans le volume;
   pour chaque point, si le flag out est TRUE, on verifie qu'on reste bien 
   dans le volume, si le flag out est FALSE, utilisation de saut */
double GetMoyenneSomSquareDistancePPVoisin( SuperList *list, Volume *dmap, 
					    Vip3DPoint_S16BIT *offset_3d )
{
  int npoints=0;
  int i;
  double somme=0.0;
  int x, y, z;
  int saut;
  int xoffset, yoffset, zoffset;
  SuperPoint *lptr;
  Vip_S16BIT *mapptr;
  int xsize, ysize, zsize;
  int dist;
  VipOffsetStruct *vos;

  if (list->interpol!=PLUS_PROCHE_VOISIN)
    {
      return(-1.0);
    }

  xoffset = offset_3d->x;
  yoffset = offset_3d->y;
  zoffset = offset_3d->z;

  xsize = mVipVolSizeX(dmap);
  ysize = mVipVolSizeY(dmap);
  zsize = mVipVolSizeZ(dmap);
  vos = VipGetOffsetStructure(dmap);
  saut = xoffset + yoffset*vos->oLine + zoffset*vos->oSlice;

  lptr = list->data;
  mapptr = VipGetDataPtr_S16BIT(dmap)+vos->oFirstPoint;

  for (i=list->n_points;i>0;i--)
    {
      if (lptr->out == VTRUE)
	{
	  x = lptr->p.x + xoffset;
	  y = lptr->p.y + yoffset;
	  z = lptr->p.z + zoffset;

	  if ((x>=0) && (x<xsize) && 
	      (y>=0) && (y<ysize) && 
	      (z>=0) && (z<zsize) )
	    {
	      dist = *(mapptr+lptr->offset+saut);
	      somme += dist*dist;
	      npoints++;
	    }
	}
      else
	{
	  dist = *(mapptr+lptr->offset+saut);
	  somme += (double)dist*dist;
	  npoints++;
	}
      lptr++;
    }

  /* Il vaudrait mieux definir une constante souple (0.8) */
  if ((npoints < POURCENTAGE_MIN_DANS_CARTE*list->n_points ) && (npoints>0))
    {
      somme = (double)SORT_DE_LA_CARTE;
    }
  else if (npoints>0)
    {
      somme /= (double)npoints;
      somme /= (double)(VIP_USED_DISTMAP_MULTFACT*VIP_USED_DISTMAP_MULTFACT);
      somme = sqrt(somme);	
    }
  else
    {
      somme = INFINI;
    }

  VipFree(vos);

  return(somme);
}

/* Remplis le champ dist des points (a utiliser avec recalage optimal: offset 
   nul) */
int VipFillSuperPointErreurInterpolTrilin( SuperList *list, Volume *dmap )
{
  int i;
  int x, y, z;
  Vip_S16BIT *ptrbase;
  int saut000, saut001, saut010, saut011;
  int saut100, saut101, saut110, saut111;
  SuperPoint *lptr;
  Vip_S16BIT *mapptr;
  int xsize, ysize, zsize;
  float dist=0.0, *coefptr;
  VipOffsetStruct *vos;

  if (list->interpol!=TRILINEAIRE)
    {
      return(PB);
    }

  if (list->offsetfilled == VFALSE )
	FillSuperPointOffset( list, dmap );

  xsize = mVipVolSizeX(dmap);
  ysize = mVipVolSizeY(dmap);
  zsize = mVipVolSizeZ(dmap);
  vos = VipGetOffsetStructure(dmap);

  saut000 = 0;
  saut001 = saut000 + vos->oSlice;
  saut010 = saut000 + vos->oLine;
  saut011 = saut001 + vos->oLine;
  saut100 = saut000 + 1;
  saut101 = saut001 + 1;
  saut110 = saut010 + 1;
  saut111 = saut011 + 1;

  lptr = list->data;
  mapptr = VipGetDataPtr_S16BIT(dmap)+vos->oFirstPoint;

  for (i=list->n_points;i>0;i--)
    {
      if (lptr->out == VTRUE)
	{
	  x = lptr->p.x;
	  y = lptr->p.y;
	  z = lptr->p.z;

	  if ( (x<0) || (x>=xsize-1) ||
	       (y<0) || (y>=ysize-1) ||
	       (z<0) || (z>=zsize-1) ) 
	    {
	      lptr->dist = -1.0;
	      lptr++;
	      continue;
	    }
	}

      ptrbase = mapptr+lptr->offset;
      coefptr = lptr->coef;

      dist =  *(ptrbase+saut000) * *coefptr++;
      dist += *(ptrbase+saut001) * *coefptr++;
      dist += *(ptrbase+saut010) * *coefptr++;
      dist += *(ptrbase+saut011) * *coefptr++;
      dist += *(ptrbase+saut100) * *coefptr++;
      dist += *(ptrbase+saut101) * *coefptr++;
      dist += *(ptrbase+saut110) * *coefptr++;
      dist += *(ptrbase+saut111) * *coefptr;

      lptr->dist = (float)(dist/VIP_USED_DISTMAP_MULTFACT);
      lptr++;
    }

  VipFree(vos);

  return(OK);
}

/* Retourne la somme des carres des distances de la liste de points par 
   rapport a la carte de distance dmap, apres un decalage specifie par 
   offset_3d, moyennee par le nombre de points effectivement dans le volume;
   pour chaque point, si le flag out est TRUE, on verifie qu'on reste bien 
   dans le volume, si le flag out est FALSE, utilisation de saut */
double GetMoyenneSomSquareDistanceInterpolTrilin(SuperList *list, 
						 Volume *dmap, 
						 Vip3DPoint_S16BIT *offset_3d)
{
  int npoints=0;
  int i;
  double somme=0.0;
  int x, y, z;
  int xoffset, yoffset, zoffset;
  Vip_S16BIT *ptrbase;
  int saut000, saut001, saut010, saut011;
  int saut100, saut101, saut110, saut111;
  SuperPoint *lptr;
  Vip_S16BIT *mapptr;
  int xsize, ysize, zsize;
  float dist=0.0, *coefptr;
  VipOffsetStruct *vos;

  if (list->interpol!=TRILINEAIRE)
    {
      return(-1.0);
    }

  if (list->offsetfilled == VFALSE )
	FillSuperPointOffset( list, dmap );

  xoffset = offset_3d->x;
  yoffset = offset_3d->y;
  zoffset = offset_3d->z;

  xsize = mVipVolSizeX(dmap);
  ysize = mVipVolSizeY(dmap);
  zsize = mVipVolSizeZ(dmap);
  vos = VipGetOffsetStructure(dmap);

  saut000 = xoffset + yoffset*vos->oLine + zoffset*vos->oSlice;
  saut001 = saut000 + vos->oSlice;
  saut010 = saut000 + vos->oLine;
  saut011 = saut001 + vos->oLine;
  saut100 = saut000 + 1;
  saut101 = saut001 + 1;
  saut110 = saut010 + 1;
  saut111 = saut011 + 1;

  lptr = list->data;
  mapptr = VipGetDataPtr_S16BIT(dmap)+vos->oFirstPoint;

  for (i=list->n_points;i>0;i--)
    {
      if (lptr->out == VTRUE)
	{
	  x = lptr->p.x + xoffset;
	  y = lptr->p.y + yoffset;
	  z = lptr->p.z + zoffset;

	  if ( (x<0) || (x>=xsize-1) ||
	       (y<0) || (y>=ysize-1) ||
	       (z<0) || (z>=zsize-1) ) 
	    {
	      lptr++;
	      continue;
	    }
	}

      ptrbase = mapptr+lptr->offset;
      coefptr = lptr->coef;

      dist =  *(ptrbase+saut000) * *coefptr++;
      dist += *(ptrbase+saut001) * *coefptr++;
      dist += *(ptrbase+saut010) * *coefptr++;
      dist += *(ptrbase+saut011) * *coefptr++;
      dist += *(ptrbase+saut100) * *coefptr++;
      dist += *(ptrbase+saut101) * *coefptr++;
      dist += *(ptrbase+saut110) * *coefptr++;
      dist += *(ptrbase+saut111) * *coefptr;

      somme += (double)dist*dist;
      npoints++;
      lptr++;
    }

  if ((npoints < POURCENTAGE_MIN_DANS_CARTE*(list->n_points) ) && (npoints>0))
    {
      somme = (double)SORT_DE_LA_CARTE;
    }
  else if (npoints>0)
    {
      somme /= (double)(VIP_USED_DISTMAP_MULTFACT*VIP_USED_DISTMAP_MULTFACT);
      somme /= (double)npoints;
      somme = sqrt(somme);	
    }
  else
    {
      somme = INFINI;
    }

  VipFree(vos);

  return(somme);
}

/* retourne dans bestoffset, le decalage qui permet d'obtenir la plus faible 
   distance moyenne de la liste dans la carte de distance dans un voisinage 
   dont le centre est donne par initoffset, la taille par maxoffset (relatif),
   la distance minimale est retournee */
double VipFindMinDistInVoisinage( SuperList *list, Volume *dmap, 
				  Vip3DPoint_S16BIT *initoffset, 
				  Vip3DPoint_S16BIT *maxoffset, 
				  Vip3DPoint_S16BIT *bestoffset, 
				  Dist6Connexe *distvoisins)
{
  int x, y, z;
  double dist, distmin;
  Vip3DPoint_S16BIT offset;

  if (!list || !dmap || !list->data || !dmap->data || !initoffset || 
	!bestoffset || !maxoffset || !distvoisins )
    {
      return(-1.0);
    }

  if (list->offsetfilled == VFALSE ) 
	FillSuperPointOffset( list, dmap );

  FillSuperPointExternalFlag( list, dmap, initoffset, maxoffset );

  distmin = INFINI;

  for (x= -maxoffset->x;x<=maxoffset->x;x++)
    for (y= -maxoffset->y;y<=maxoffset->y;y++)
      for (z= -maxoffset->z;z<=maxoffset->z;z++)
	{
	  /* test ne conservant que les six voisins (si maxoffset=1),
	     si on prouve que la fonction distance est convexe
	     par rapport a la translation, il n'est meme
	     plus necessaire de proceder ainsi,
	     on peut optimiser par rapport a chaque direction
	     NB: ceci est a creuser */

	  if (((x*y)!=0) || ((x*z)!=0) || ((y*z)!=0)) continue; 

	  offset.x = initoffset->x + x;
	  offset.y = initoffset->y + y;
	  offset.z = initoffset->z + z;
	  dist = VipGetMoyenneSomSquareDistance( list, dmap, &offset );

	  if ((x == 0) && (y == 0) && (z == 0)) distvoisins->central = dist;
	  else if ((x == -1) && (y == 0) && (z == 0)) distvoisins->xmoins = dist;
	  else if ((x == 1) && (y == 0) && (z == 0)) distvoisins->xplus = dist;
	  else if ((x == 0) && (y == -1) && (z == 0)) distvoisins->ymoins = dist;
	  else if ((x == 0) && (y == 1) && (z == 0)) distvoisins->yplus = dist;
	  else if ((x == 0) && (y == 0) && (z == -1)) distvoisins->zmoins = dist;
	  else if ((x == 0) && (y == 0) && (z == 1)) distvoisins->zplus = dist;

	  if (dist<distmin)
	    {
	      distmin = dist;
	      bestoffset->x = offset.x;
	      bestoffset->y = offset.y;
	      bestoffset->z = offset.z;
	    }
	}

  return(distmin);
}

/* retourne dans bestoffset le decalage qui minimise la distance de la liste 
   dans la carte de distance, la recherche du minimum se deroulant comme suit:
   - position initiale: offset nul (origine),
   - recherche du minimum dans un voisinage  de l'origine dont la taille est 
     fixee par maxoffset,
   - on deplace le voisinage pour le centrer sur le minimum,
   - on determine un nouveau minimum:
   - si les deux min sont identiques, on a termine,
   - sinon on recommence;
   NB: Cette technique frustre amene une certaine redondance dans les calculs
*/
double VipFindBestPositionInDistanceMap( SuperList *list, Volume *dmap, 
					 Vip3DPoint_S16BIT *maxoffset, 
					 Vip3DPoint_S16BIT *bestoffset, 
					 Dist6Connexe *distvoisins )
{
  Vip3DPoint_S16BIT initoffset, min1, min2;
  double dist1, dist2;
  int oncontinue;

  FillSuperPointOffset( list, dmap );

  initoffset.x = 0;
  initoffset.y = 0;
  initoffset.z = 0;

  dist1 = VipFindMinDistInVoisinage( list, dmap, &initoffset, maxoffset, &min1,
				     distvoisins );

  /* si min1=initoffset on s'arrete */
  if ((min1.x==initoffset.x) && (min1.y==initoffset.y) &&
     (min1.z==initoffset.z))
    {
      oncontinue=VFALSE;
    }
  else oncontinue = VTRUE;

  while ( oncontinue )
    {
      initoffset.x = min1.x;
      initoffset.y = min1.y;
      initoffset.z = min1.z;

      dist2 = VipFindMinDistInVoisinage( list, dmap, &initoffset, maxoffset, 
					 &min2, distvoisins );

      if (((min1.x==min2.x) && (min1.y==min2.y) && (min1.z==min2.z))
	  || (dist2>10000) )
	{
	  oncontinue = VFALSE;
	}
      else 
	{
	  min1.x = min2.x;
	  min1.y = min2.y;
	  min1.z = min2.z;
	  dist1 = dist2;
	}
    }

  bestoffset->x = min1.x;
  bestoffset->y = min1.y;
  bestoffset->z = min1.z;

  return(dist1);
}

/* Met le flag out a TRUE, si les decalage definis par initoffset et maxoffset 
   peuvent faire sortir le point de la carte de distance; dans le cas 
   contraire, le flag est a FALSE et il n'est pas necessaire de faire la 
   batterie de test; en outre, l'offset initial peut etre calcule une fois
   pour toute pour chaque point; dans le cas ou la superliste est destinee a 
   une interpolation trilineaire, les tests sont plus exigeant du fait du 
   decalage supplementaire positif d'une unite dans chaque direction;
*/
int FillSuperPointExternalFlag( SuperList *list, Volume *dmap, 
				Vip3DPoint_S16BIT *initoffset, 
				Vip3DPoint_S16BIT *maxoffset )
{
  int xnegoffset, xposoffset;
  int ynegoffset, yposoffset;
  int znegoffset, zposoffset;
  int xsize, ysize, zsize;
  SuperPoint *ptr;
  int i;
  int x, y, z;

  if ((list->interpol!=PLUS_PROCHE_VOISIN) && (list->interpol!=TRILINEAIRE))
    {
      return(PB);
    }

  xnegoffset = initoffset->x - maxoffset->x;
  ynegoffset = initoffset->y - maxoffset->y;
  znegoffset = initoffset->z - maxoffset->z;

  xposoffset = initoffset->x + maxoffset->x;
  yposoffset = initoffset->y + maxoffset->y;
  zposoffset = initoffset->z + maxoffset->z;

  if (list->interpol==TRILINEAIRE)
    {
      xposoffset+=1;
      yposoffset+=1;
      zposoffset+=1;
    }

  xsize = mVipVolSizeX(dmap);
  ysize = mVipVolSizeY(dmap);
  zsize = mVipVolSizeZ(dmap);

  ptr = list->data;

  for (i=list->n_points; i>0;i--)
    {
      x = ptr->p.x + xnegoffset;
      if ( (x<0) || (x>=xsize) )
	{
	  ptr->out = VTRUE;
	  ptr++;
	  continue;
	}

      x = ptr->p.x + xposoffset;
      if ( (x<0) || (x>=xsize) )
	{
	  ptr->out = VTRUE;
	  ptr++;
	  continue;
	}

      y = ptr->p.y + ynegoffset;
      if ( (y<0) || (y>=ysize) )
	{
	  ptr->out = VTRUE;
	  ptr++;
	  continue;
	}

      y = ptr->p.y + yposoffset;
      if ( (y<0) || (y>=ysize) )
	{
	  ptr->out = VTRUE;
	  ptr++;
	  continue;
	}

      z = ptr->p.z + znegoffset;
      if ( (z<0) || (z>=zsize) )
	{
	  ptr->out = VTRUE;
	  ptr++;
	  continue;
	}

      z = ptr->p.z + zposoffset;
      if ( (z<0) || (z>=zsize) )
	{
	  ptr->out = VTRUE;
	  ptr++;
	  continue;
	}

      ptr->out = VFALSE; /* tout s'est bien passe */
      ptr++;
    }

  return(OK);
}

/* il s'agit de calculer d'avance le saut qui permet d'aller chercher la 
   distance correspondant a un point dans la carte de distance pour un offset 
   nul, ceci permettra d'obtenir les distances pour un offset en xyz 
   quelconque a l'aide d'un saut precalcule on notera que l'offset n'est 
   valable que lorsqu'on est effectivement dans la carte!!!!!!! */
int FillSuperPointOffset( SuperList *list, Volume *dmap )
{
  SuperPoint *ptr;
  int i;
  VipOffsetStruct *vos;

  ptr = list->data;
  vos = VipGetOffsetStructure(dmap);

  for (i=list->n_points; i>0; i--)
    {
      ptr->offset = ptr->p.x + ptr->p.y*vos->oLine + ptr->p.z*vos->oSlice;
      ptr++;
    }

  list->offsetfilled = VTRUE;

  VipFree(vos);

  return(OK);
}
