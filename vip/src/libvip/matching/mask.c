/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/mask.c           * TYPE     : Function
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

#include <vip/matching/mask.h>
#include <vip/alloc.h>
#include <vip/util.h>

extern float VIP_USED_DISTMAP_MULTFACT;

/* in: volume constitue de x<>0 pour l'objet et de zero pour l'exterieur
   out: volume contenant une approximation entiere de la distance euclidienne
        a l'objet, a une constante multiplicative pres, obtenue par la 
	technique du chamfrein, float_to_int est un facteur d'echelle,
	dist_voisinage est la distance en deca de laquelle une correction
	est effectuee
*/
int VipCreateDistanceMapWithLut( Volume *vol, int xmask, int ymask, 
				 int zmask, float float_to_int, 
				 float dist_voisinage )
{
  Vip_S16BIT *lut;

  if (float_to_int > EPSILON) VIP_USED_DISTMAP_MULTFACT = float_to_int;

  VipComputeCustomizedChamferDistanceMap(vol, xmask, ymask, zmask, 
					      VIP_USUAL_DISTMAP_MULTFACT);

  lut = VipCreateLutCorrection( vol, xmask, ymask, zmask, dist_voisinage );
  VipApplyLutCorrection( vol, lut, dist_voisinage );
	
  VipFree(lut);

  return(OK);
}

/* Cree la table de conversion entre les valeurs donnees pas la transformation 
   distance du chamfrein et la distance euclidienne dont la distance du 
   chamfrein est issue (dans un voisinage defini par distmax en mm.)*/
Vip_S16BIT *VipCreateLutCorrection( Volume *vol, int xmask, int ymask, 
				    int zmask, float distmax )
{
  VIP_DEC_VOLUME(little);
  Vip_S16BIT *ptr;
  Vip_S16BIT *lut;
  int max;
  int x, y, z, dimx, dimy, dimz;
  float dnew;
  float squareechx, squareechy, squareechz, sx, sy, sz;
  int temp;
  VipOffsetStruct *vos;
  long off_pbl, off_lbs;

  if (!vol)
    {
      return((Vip_S16BIT *)NULL);
    }

  if (distmax<5)
    {
      distmax = 5.;
    }

  little = VipDuplicateVolumeStructure(vol,"");

  sx = mVipVolVoxSizeX(vol);
  sy = mVipVolVoxSizeY(vol);
  sz = mVipVolVoxSizeZ(vol);

  VipSet3DSize(little, (int)(distmax/sx)+1,
	       (int)(distmax/sy)+1, (int)(distmax/sz)+1);

  dimx = mVipVolSizeX(little);
  dimy = mVipVolSizeY(little);
  dimz = mVipVolSizeZ(little);

  if (dimx*dimy*dimz>50000)
    {
      return((Vip_S16BIT *)NULL);
    }

  VipAllocateVolumeData( little );

  vos = VipGetOffsetStructure( little );

  off_lbs = VipOffsetLineBetweenSlice(little);
  off_pbl = VipOffsetPointBetweenLine(little);

  ptr = VipGetDataPtr_S16BIT(little)+vos->oFirstPoint;

  for (z=dimz;z--;ptr+=off_lbs)
    for (y=dimy;y--;ptr+=off_pbl)
      for (x=dimx;x--;) *ptr++ = 0;

  ptr = VipGetDataPtr_S16BIT(little)+vos->oFirstPoint;
  *ptr = 1;

  VipComputeCustomizedChamferDistanceMap(little, xmask, ymask, zmask, 
					      VIP_USUAL_DISTMAP_MULTFACT);

  ptr = VipGetDataPtr_S16BIT(little)+vos->oFirstPoint;
  max = (int)(distmax * VIP_USED_DISTMAP_MULTFACT)+1;

  if(max>5000000)
    {
      if (little)
	{
	  VipFreeVolume(little);
	  VipFree(little);
	}
      return((Vip_S16BIT *)NULL);
    }

  lut = (Vip_S16BIT *)VipCalloc( max+1, sizeof(Vip_S16BIT) ,
				 "Error while allocating lut");

  squareechx = sx * sx;
  squareechy = sy * sy;
  squareechz = sz * sz;

  ptr = VipGetDataPtr_S16BIT(little)+vos->oFirstPoint;
  for(z=0;z<dimz;z++, ptr+=off_lbs)
    for(y=0;y<dimy;y++, ptr+=off_pbl)
      for(x=0;x<dimx;x++, ptr++)
	if (*ptr <= max)
	  {
	    dnew = x*x*squareechx;
	    dnew += y*y*squareechy;
	    dnew += z*z*squareechz;
	    dnew = (float)sqrt((double)dnew);
	    temp = mVipRint(VIP_USED_DISTMAP_MULTFACT*dnew);
	    if (temp > lut[*ptr]) lut[*ptr] = temp;
	  }

  VipFreeVolume(little);
  VipFree(little);
  VipFree(vos);

  return(lut);
}

int VipApplyLutCorrection( Volume *vol, Vip_S16BIT *lut, float dist )
{
  int limite;
  Vip_S16BIT *ptr;
  int z, y, x, dimx, dimy, dimz;
  VipOffsetStruct *vos;
  long off_lbs, off_pbl;

  limite = (int)(dist*VIP_USED_DISTMAP_MULTFACT);

  dimx = mVipVolSizeX(vol);
  dimy = mVipVolSizeY(vol);
  dimz = mVipVolSizeZ(vol);

  vos = VipGetOffsetStructure( vol );

  off_lbs = VipOffsetLineBetweenSlice(vol);
  off_pbl = VipOffsetPointBetweenLine(vol);

  ptr = VipGetDataPtr_S16BIT(vol)+vos->oFirstPoint;

  for(z=1;z<=dimz;z++, ptr+=off_lbs)
    {
      for (y=0;y<dimy;y++, ptr+=off_pbl)
	for (x=0;x<dimx;x++, ptr++)
	  if (*ptr <= limite) *ptr = lut[*ptr];
    }

  VipFree(vos);

  return(OK);
}
