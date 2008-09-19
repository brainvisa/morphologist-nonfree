/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : voi_resamp.c         * TYPE     : Source
 * AUTHOR      : FROUIN V.            * CREATION : 15/04/1999
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun Ultra
 *****************************************************************************
 *
 * DESCRIPTION : Version maquette d'un systeme de gestion de region d'interet
 *               dans le contexte de Vip.
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *              / /   |              |
 *          17/06/1999| V. Frouin    | Ajout d'un increment de point dans 
 *                    |              |           VipRoiIterateWithDepLinear
 *          08/09/1999| V. Frouin    | Transformation des Roi en Voi
 *                    |              |  Menage divers
 *****************************************************************************/

#include <vip/voi.h>


/* #define VF_DEBUG 1 */

/*---------------------------------------------------------------------------*/
int VipVoiArmWithIdentity(
    VipVoi                    *roi
)
/*---------------------------------------------------------------------------*/
{
  VipIntBucket                *PaquetDoffset;

  Vip3DBucket_S16BIT          *PaquetDePoint;
  Vip3DPoint_S16BIT           *point;

  int                         *offsetdiff;
  int                         i, pixel, coupe, ligne;



  PaquetDoffset       = VipVoiFirstPtrOffset( roi );
  PaquetDePoint       = VipVoiFirstPtrBucket( roi );
  for (i=0; i < roi->NbVoi; i++)
    {
      if ( !(PaquetDePoint->n_points) )
	{
	VipPrintfError("On roi contains 0 point...");
        VipPrintfExit("(roi_utils.c)VipVoiArmResam");
        return(PB);
	}
      offsetdiff   = PaquetDoffset->data; 
      point        = PaquetDePoint->data;

      ligne = coupe = pixel = 0;
      
      while (PaquetDoffset->n_points != PaquetDoffset->size)
	{
	  *offsetdiff =(point->z - coupe)*VipOffsetSlice(roi->ReferenceImage) +
	               (point->y - ligne)*VipOffsetLine(roi->ReferenceImage) +
	               (point->x - pixel);
	  coupe = point->z; ligne = point->y; pixel = point->x;
	  offsetdiff++; point++;(PaquetDoffset->n_points)++;
	  while (point->z == coupe && PaquetDoffset->n_points != PaquetDoffset->size)
	    {
	      *offsetdiff = (point->y - ligne) * VipOffsetLine(roi->ReferenceImage) + (point->x - pixel);
	      ligne = point->y; pixel = point->x;
	      offsetdiff++;point++;(PaquetDoffset->n_points)++;
	      while (point->y == ligne && PaquetDoffset->n_points != PaquetDoffset->size)
		{
		  *offsetdiff = point->x - pixel;
		  pixel = point->x;
		  point++; offsetdiff++;(PaquetDoffset->n_points)++;
		}
	    }
	}
      PaquetDePoint = VipVoiNextPtrBucket( roi );
      PaquetDoffset = VipVoiNextPtrOffset( roi );
    }

  return(OK);
}




/*---------------------------------------------------------------------------*/

int VipVoiArmWithDepResampLinear(
    VipVoi                    *roi,
    VipDeplacement            *dep
)
/*---------------------------------------------------------------------------*/
{
  VipIntBucket                *PaquetDoffset;

  Vip3DBucket_S16BIT          *PaquetDePoint;
  Vip3DPoint_S16BIT           *point;

  Vip3DBucket_U16BIT          *PaquetDeDeplaEntier;
  Vip3DPoint_U16BIT           *pointentier;

  int                         *offsetdiff;


  int                          x000, y000, z000;
  int                          xcourant, ycourant, zcourant;
  int                          xu, yu, zu, xv, yv, zv;
  int                          xmax, ymax, zmax;
  int                          lineSizeRef, sliceSizeRef, SEIZE, DEUXPUIS16;
  VipDeplacement               invdep;
  int                          i, coupe;

  /* init des constantes necessaires a l'elaboration des offsets*/
  if(VipInverseDeplacement(dep,&invdep)==PB) return(PB);
  VipScaleDeplacement(dep,roi->ReferenceImage,roi->VoiImage);
  VipScaleDeplacement(&invdep,roi->VoiImage,roi->ReferenceImage);

  SEIZE = 16;
  DEUXPUIS16 = 65536;

  xu = (int)(65536. * invdep.r.xx);
  yu = (int)(65536. * invdep.r.yx);
  zu = (int)(65536. * invdep.r.zx);
  xv = (int)(65536. * invdep.r.xy);
  yv = (int)(65536. * invdep.r.yy);
  zv = (int)(65536. * invdep.r.zy);
  xmax = DEUXPUIS16*(mVipVolSizeX(roi->ReferenceImage) -1);
  ymax = DEUXPUIS16*(mVipVolSizeY(roi->ReferenceImage) -1);
  zmax = DEUXPUIS16*(mVipVolSizeZ(roi->ReferenceImage) -1);
  lineSizeRef = mVipVolSizeX(roi->ReferenceImage);
  sliceSizeRef = mVipVolSizeX(roi->ReferenceImage)*mVipVolSizeY(roi->ReferenceImage);
  

  PaquetDoffset       = VipVoiFirstPtrOffset( roi );
  PaquetDePoint       =  VipVoiFirstPtrBucket( roi );
  PaquetDeDeplaEntier =  VipVoiFirstPtrDepla( roi );
  for (i=0; i < roi->NbVoi; i++)
    {
      if ( !(PaquetDePoint->n_points) )
	{
	VipPrintfError("On roi contains 0 point...");
        return(PB);
	}

      offsetdiff   = PaquetDoffset->data; 
      point        = PaquetDePoint->data;
      pointentier  = PaquetDeDeplaEntier->data;
      while (PaquetDoffset->n_points != PaquetDoffset->size)
	{
	  coupe = point->z;
	  do {
	      /* Gestion d'une coupe */
	      x000 = (int)(65536. * (invdep.t.x + point->z * invdep.r.xz));
	      y000 = (int)(65536. * (invdep.t.y + point->z * invdep.r.yz));
	      z000 = (int)(65536. * (invdep.t.z + point->z * invdep.r.zz));
	      
	      xcourant = x000 + point->y * xv + point->x * xu;
	      ycourant = y000 + point->y * yv + point->x * yu;
	      zcourant = z000 + point->y * zv + point->x * zu;
	      
	      if ( (xcourant>=0) && (xcourant<xmax)
		  && (ycourant>=0) && (ycourant<ymax)
		  && (zcourant>=0) && (zcourant<zmax) )
	         {
		   /* Get du pointeur sur les donnees                     */
		   *offsetdiff = (xcourant >> SEIZE) +
		                 (ycourant >> SEIZE) * lineSizeRef +
		                 (zcourant >> SEIZE) * sliceSizeRef;
		   /* On ecrase les ifons de point pour stocker           */
		   /* des infos de delta a la val entiere la + proche     */
#ifdef VF_DEBUG
		   printf("[debug] (%5d %5d %5d)->",point->x,point->y,point->z);
#endif
		   pointentier->x = xcourant & 0xffff;
		   pointentier->y = ycourant & 0xffff;
		   pointentier->z = zcourant & 0xffff;
#ifdef VF_DEBUG
		   printf("(%5d %5d %5d)\n",pointentier->x,pointentier->y,pointentier->z);
#endif
		   point++;
		   pointentier++; (PaquetDeDeplaEntier->n_points)++;
		   offsetdiff++;(PaquetDoffset->n_points)++;
		 }
	         else
		 {
		   /* Postionnement d'un flag pour traitement dans VipIterate*/
		   *offsetdiff = -1;
		   point++;
		   pointentier++;(PaquetDeDeplaEntier->n_points)++;
		   offsetdiff++;(PaquetDoffset->n_points)++;
		 }

	    } while (point->z == coupe && 
		PaquetDoffset->n_points != PaquetDoffset->size);
	}
      PaquetDoffset = VipVoiNextPtrOffset (roi );
      PaquetDePoint = VipVoiNextPtrBucket( roi );
      PaquetDeDeplaEntier = VipVoiNextPtrDepla( roi );
    }


  return(OK);
}

/*---------------------------------------------------------------------------*/

int VipVoiIterateWithDepResampLinearWithMC(
    VipVoi                    *roi,
    VipDeplacement            *dep,
    int                       epoch
)
/*---------------------------------------------------------------------------*/
{
  Vip_S16BIT                   *imptr, *ptr, tmp;
  int                          i, j, t;

  VipIntBucket                *paquetoffcourant;
  int                         *offcourant;

  Vip3DPoint_U16BIT           *pointentier;
  Vip3DBucket_U16BIT          *PaquetDeDeplaEntier;

  float                       **tabpaquetmescourant;
  float                       *paquetmescourant;

  VipOffsetStruct             *vos;
  int                         stock1, stock2, stock3;
  int                         SEIZE, DEUXPUIS16;
  int                         lineSizeRef, sliceSizeRef;

  VipDeplacement *bidon = dep; /*pour warning de compil...*/
  bidon = NULL;

   SEIZE = 16;
   DEUXPUIS16 = 65536;

   vos = VipGetOffsetStructure(roi->ReferenceImage);
   lineSizeRef = mVipVolSizeX(roi->ReferenceImage);
   sliceSizeRef = mVipVolSizeX(roi->ReferenceImage)*mVipVolSizeY(roi->ReferenceImage);


   tabpaquetmescourant = VipVoiFirstPtrMes( roi );
   paquetoffcourant    = VipVoiFirstPtrOffset( roi );
   PaquetDeDeplaEntier = VipVoiFirstPtrDepla( roi );
   for (i=0; i < roi->NbVoi; i++)
     {
       paquetmescourant = VipVoiFirstPtrMesVector( roi );
       for (t=0; t < epoch; t++, paquetmescourant=VipVoiNextPtrMesVector( roi ) );
       
       {
	 pointentier   = PaquetDeDeplaEntier->data;
	 offcourant = paquetoffcourant->data;
	 imptr = VipGetDataPtr_S16BIT(roi->ReferenceImage)+vos->oFirstPoint +
	           epoch * VipOffsetVolume(roi->ReferenceImage);
	 for (j=paquetoffcourant->n_points; j>0; j--)
	   {
	     ptr = imptr +(ptrdiff_t)(*offcourant++);
	     
	     stock1 = *ptr * (DEUXPUIS16- pointentier->x);
	     stock1 += *(++ptr) * pointentier->x;
	     stock1 >>= SEIZE;
	     ptr += lineSizeRef;
	     stock2 = *ptr * pointentier->x;
	     stock2 += *(--ptr) * (DEUXPUIS16 - pointentier->x);
	     stock2 >>= SEIZE;

	     stock1 *= (DEUXPUIS16 - pointentier->y);
	     stock1 += stock2 * pointentier->y;
	      stock1 >>= SEIZE;

	     ptr += sliceSizeRef - lineSizeRef;
	     stock2 = *ptr * (DEUXPUIS16 - pointentier->x);
	     stock2 += *(++ptr) * pointentier->x;
	     stock2 >>= SEIZE;
	     ptr += lineSizeRef;
	     stock3 = *ptr * pointentier->x;
	     stock3 += *(--ptr) * (DEUXPUIS16 - pointentier->x);
	     stock3 >>= SEIZE;

	     stock2 *= DEUXPUIS16 - pointentier->y;
	     stock2 += stock3 * pointentier->y;
	     stock2 >>= SEIZE;

	     stock1 *= (DEUXPUIS16 - pointentier->z);
	     stock1 += stock2 * pointentier->z;
	     tmp = (Vip_S16BIT)(stock1 >> SEIZE);
	     *paquetmescourant++ = 
		 mVipVolShfjScale(roi->ReferenceImage)*( (float) tmp ) +
		 mVipVolShfjOffset(roi->ReferenceImage);
	     pointentier++;
	   }

       }

       tabpaquetmescourant = VipVoiNextPtrMes( roi );
       paquetoffcourant    = VipVoiNextPtrOffset( roi );
       PaquetDeDeplaEntier = VipVoiNextPtrDepla( roi );
     }

  return(OK);
}
/*---------------------------------------------------------------------------*/

int VipVoiIterateWithDepResampLinear(
    VipVoi                    *roi,
    VipDeplacement            *dep
)
/*---------------------------------------------------------------------------*/
{
  Vip_S16BIT                   *imptr, *ptr, tmp;
  int                          i, j, t;

  VipIntBucket                *paquetoffcourant;
  int                         *offcourant;

  Vip3DPoint_U16BIT           *pointentier;
  Vip3DBucket_U16BIT          *PaquetDeDeplaEntier;

  float                       **tabpaquetmescourant;
  float                       *paquetmescourant;

  VipOffsetStruct             *vos;
  int                         stock1, stock2, stock3;
  int                         SEIZE, DEUXPUIS16;
  int                         lineSizeRef, sliceSizeRef;

  VipDeplacement *bidon = dep; /*pour warning de compil...*/
  bidon = NULL;

   SEIZE = 16;
   DEUXPUIS16 = 65536;

   vos = VipGetOffsetStructure(roi->ReferenceImage);
   lineSizeRef = mVipVolSizeX(roi->ReferenceImage);
   sliceSizeRef = mVipVolSizeX(roi->ReferenceImage)*mVipVolSizeY(roi->ReferenceImage);


   tabpaquetmescourant = VipVoiFirstPtrMes( roi );
   paquetoffcourant    = VipVoiFirstPtrOffset( roi );
   PaquetDeDeplaEntier = VipVoiFirstPtrDepla( roi );
   for (i=0; i < roi->NbVoi; i++)
     {
       paquetmescourant = VipVoiFirstPtrMesVector( roi );
       for (t = 0 ; t <  mVipVolSizeT(roi->ReferenceImage); t++)
	 {
	   pointentier   = PaquetDeDeplaEntier->data;
	   offcourant = paquetoffcourant->data;
	   imptr = VipGetDataPtr_S16BIT(roi->ReferenceImage)+vos->oFirstPoint +
	           t * VipOffsetVolume(roi->ReferenceImage);
	   for (j=paquetoffcourant->n_points; j>0; j--)
	    {
	      	      ptr = imptr +(ptrdiff_t)(*offcourant++);
	      stock1 = *ptr * (DEUXPUIS16- pointentier->x);
	      stock1 += *(++ptr) * pointentier->x;
	      stock1 >>= SEIZE;
	      ptr += lineSizeRef;
	      stock2 = *ptr * pointentier->x;
	      stock2 += *(--ptr) * (DEUXPUIS16 - pointentier->x);
	      stock2 >>= SEIZE;

	      stock1 *= (DEUXPUIS16 - pointentier->y);
	      stock1 += stock2 * pointentier->y;
	      stock1 >>= SEIZE;

	      ptr += sliceSizeRef - lineSizeRef;
	      stock2 = *ptr * (DEUXPUIS16 - pointentier->x);
	      stock2 += *(++ptr) * pointentier->x;
	      stock2 >>= SEIZE;
	      ptr += lineSizeRef;
	      stock3 = *ptr * pointentier->x;
	      stock3 += *(--ptr) * (DEUXPUIS16 - pointentier->x);
	      stock3 >>= SEIZE;

	      stock2 *= DEUXPUIS16 - pointentier->y;
	      stock2 += stock3 * pointentier->y;
	      stock2 >>= SEIZE;

	      stock1 *= (DEUXPUIS16 - pointentier->z);
	      stock1 += stock2 * pointentier->z;
	      tmp = (Vip_S16BIT)(stock1 >> SEIZE);
	      *paquetmescourant++ = 
		 mVipVolShfjScale(roi->ReferenceImage)*( (float) tmp ) +
		 mVipVolShfjOffset(roi->ReferenceImage);
	      pointentier++;
	    }
	   paquetmescourant = VipVoiNextPtrMesVector(roi );
	 }
       tabpaquetmescourant = VipVoiNextPtrMes( roi );
       paquetoffcourant    = VipVoiNextPtrOffset( roi );
       PaquetDeDeplaEntier = VipVoiNextPtrDepla( roi );
     }

  return(OK);
}


/*---------------------------------------------------------------------------*/

int VipVoiIterateWithIdentity(
    VipVoi                    *roi
)
/*---------------------------------------------------------------------------*/
{
  int                          i, j, t;
  VipIntBucket                *paquetoffcourant;
  int                         *offcourant;
  float                       **tabpaquetmescourant;
  float                       *paquetmescourant;
  Vip_S16BIT                  *imptr;
  VipOffsetStruct             *vos;


   vos = VipGetOffsetStructure(roi->ReferenceImage);
   
   tabpaquetmescourant = VipVoiFirstPtrMes( roi );
   paquetoffcourant    = VipVoiFirstPtrOffset( roi );
   for (i=0; i < roi->NbVoi; i++)
     {
       
       paquetmescourant = VipVoiFirstPtrMesVector(roi);
       for (t = 0 ; t <  mVipVolSizeT(roi->ReferenceImage); t++)
	 {
	   offcourant       = paquetoffcourant->data;
	   imptr = VipGetDataPtr_S16BIT(roi->ReferenceImage)+vos->oFirstPoint +
	        t * VipOffsetVolume(roi->ReferenceImage);
	   for (j=paquetoffcourant->n_points; j>0; j--)
	     {
	       imptr += (ptrdiff_t) (*(offcourant)++);
	       *paquetmescourant = mVipVolShfjScale(roi->ReferenceImage)
		                   *( (float) *imptr ) +
		                   mVipVolShfjOffset(roi->ReferenceImage);
	       paquetmescourant++;
	     }
	   paquetmescourant = VipVoiNextPtrMesVector( roi );
	 }
       paquetoffcourant    = VipVoiNextPtrOffset( roi );
       tabpaquetmescourant = VipVoiNextPtrMes( roi );
     }
   
   return(OK);
}
