/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : voi_utils.c          * TYPE     : Source
 * AUTHOR      : FROUIN V.            * CREATION : 15/04/1999
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun Ultra
 *****************************************************************************
 *
 * DESCRIPTION : Systeme de gestion de volume d'interet
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
 *****************************************************************************/

#include <vip/voi.h>
#include <vip/nomen.h>
#include <vip/math.h>
#include <stdio.h>

/* #define VF_DEBUG 1*/
extern int VipVoiArmWithIdentity(
    VipVoi                    *roi
);

extern int VipVoiArmWithDepResampLinear(
    VipVoi                    *roi,
    VipDeplacement            *dep
);

extern int VipVoiIterateWithIdentity(
    VipVoi                    *roi
);

extern int VipVoiIterateWithDepResampLinear(
    VipVoi                    *roi,
    VipDeplacement            *dep
);

extern int VipVoiIterateWithDepResampLinearWithMC(
    VipVoi                    *roi,
    VipDeplacement            *dep,
    int                       t
);

/*---------------------------------------------------------------------------*/
static int VipVoiDepIsIdentity(
   VipDeplacement              *dep, 
   Volume                      *depart,
   Volume                      *arrivee)
/*---------------------------------------------------------------------------*/
{
  VipDeplacement               local;

  
  local.r.xx = dep->r.xx; local.r.xy = dep->r.xy; local.r.xz = dep->r.xz;
  local.r.yx = dep->r.yx; local.r.yy = dep->r.yy; local.r.yz = dep->r.yz;
  local.r.zx = dep->r.zx; local.r.zy = dep->r.zy; local.r.zz = dep->r.zz;
  local.t.x  = dep->t.x ; local.t.y  = dep->t.y ; local.t.z  = dep->t.z ;
  VipScaleDeplacement(&local, depart, arrivee);
  
  if (local.r.xx==1.0 && local.r.xy==0.0 && local.r.xz==0.0 &&
      local.r.yx==0.0 && local.r.yy==1.0 && local.r.yz==0.0 &&
      local.r.zx==0.0 && local.r.zy==0.0 && local.r.zz==1.0 &&
      local.t.x==0.0 && local.t.y==0.0 && local.t.z==0.0)
    {
      VipPrintfWarning("Deplacement used is identity");
      return(1);
    }       
  else
    {
      return(0);
    }
}
/*---------------------------------------------------------------------------*/
static   Vip3DBucket_S16BIT  *VipRead3DBucketFromVolume_S16BIT(
   Volume                      *image,
   int                         label
)
/*---------------------------------------------------------------------------*/
{
   int                         i, j, k;
   int                         xsize, ysize, zsize;
   VipOffsetStruct             *vos;
   Vip3DBucket_S16BIT          *bucketcourant;
   Vip3DPoint_S16BIT           *pointcourant;
   Vip_S16BIT                  *imptr;


   


   /* Tests sur les entrees.    */
   if (VipTestExist(image)==PB || VipTestType(image,ANY_TYPE)==PB)
     {
     VipPrintfExit("(voi_utils.c)VipRead3DBucketFromVolume_S16BIT");
     return((Vip3DBucket_S16BIT *)NULL);
     }

   if (mVipVolType(image) != S16BIT)
     {
     VipPrintfWarning("Label image should be S16BIT");
     return((Vip3DBucket_S16BIT *)NULL);
     }


   vos = VipGetOffsetStructure(image);

   xsize = mVipVolSizeX(image);
   ysize = mVipVolSizeY(image);
   zsize = mVipVolSizeZ(image);

   /* Boucles d'obtention proprement dites */
   imptr         = VipGetDataPtr_S16BIT(image)+vos->oFirstPoint;
   bucketcourant = VipAlloc3DBucket_S16BIT(10000);
   if ( !bucketcourant )
     {
       VipPrintfError("(voi_utils.c)VipRead3DBucketFromVolume_S16BIT:alloc error...\n");
       return( PB );
     }


   for (k=0; k<zsize; k++, imptr+=vos->oLineBetweenSlice)
     for (j=0; j<ysize; j++, imptr+=vos->oPointBetweenLine)
       for (i=0; i<xsize; i++, imptr++)
	 {
	   if (*imptr == label)
	     {
	       if (bucketcourant->n_points == bucketcourant->size) 
		 {
		   VipIncrease3DBucket_S16BIT(bucketcourant, 10000);
		 }
	       pointcourant =bucketcourant->data + (bucketcourant->n_points)++;
	       pointcourant->x = i;
	       pointcourant->y = j;
	       pointcourant->z = k;
	     }
	 }	    

   return(bucketcourant);
}

/*---------------------------------------------------------------------------*/
static int VipVoiArmResamp(
    VipVoi                    *roi,
    VipDeplacement            *dep
)
/*---------------------------------------------------------------------------*/
{
  if ( !(roi->Etat & VROI_RDY_FORARM) )
    {
      VipPrintfError("voi struct not ready to be armed");
      VipPrintfExit("(voi_utils.c)VipVoiArmResamp");
      return(PB);
    } 

  /*Allocation*/
  if ( !(roi->Etat & VROI_ARMED))
    {
      VipVoiAllocateOffsetData(roi);
      if (!VipVoiDepIsIdentity(dep, roi->ReferenceImage, roi->VoiImage))
	{
	  VipVoiAllocateDeplaData(roi);
	}
    }
  else
    {
      VipVoiResetOffsetData( roi );
      VipVoiResetDeplaData( roi );
    }

  /* Initialisation des offsets et pointeurs de data         */
  if (VipVoiDepIsIdentity(dep, roi->ReferenceImage, roi->VoiImage))
    {
      if (VipVoiArmWithIdentity(roi) == PB)
	{
	  VipPrintfError("(voi_utils.c)VipVoiArmResamp");
	  return(PB);
	}
    }
  else
    {
      if (VipVoiArmWithDepResampLinear(roi, dep) == PB)
	{
	  VipPrintfError("(voi_utils.c)VipVoiArmResamp");
	  return(PB);
	}
    }

  roi->Etat |= VROI_ARMED | VROI_RDY_FORITERATE;
  return(OK);
}


/*---------------------------------------------------------------------------*/
static int VipVoiIterateFromVidaWithMC(
   VipVoi                      *roi,
   VipDeplacement              *dep,
   int                         t
)
/*---------------------------------------------------------------------------*/
{

  if ( !(roi->Etat & VROI_RDY_FORITERATE) )
    {
     VipPrintfError("roi struct not ready to iterate.");
     VipPrintfExit("(roi_utils.c)VipVoiIterateFromVida");
     return(PB);
    }

  if (mVipVolType(roi->ReferenceImage) != S16BIT)
     { 
     VipPrintfError("Statistics can only be got from Vida Images.");
     VipPrintfExit("(roi_utils.c)VipVoiIterateFromVida");
     return(PB);
     }
  
  /* Allocations */
  if ( !(roi->Etat & VROI_ITERATED) )
    {
      VipVoiAllocateMesData( roi );
    }


  /* Pour tous les etiquettes decrites dans la structure roi */
  /* faire  le parcours des contenus des adresses et      */

  if (VipVoiIterateWithDepResampLinearWithMC(roi, dep, t) == PB)
    {
      VipPrintfError("(roi_utils.c)VipVoiIterate");
      return(PB);
    }

  roi->Etat |= VROI_ITERATED;
  return(OK);
}

/*---------------------------------------------------------------------------*/
static int VipVoiIterateFromVida(
   VipVoi                      *roi,
   VipDeplacement              *dep
)
/*---------------------------------------------------------------------------*/
{

  if ( !(roi->Etat & VROI_RDY_FORITERATE) )
    {
     VipPrintfError("roi struct not ready to iterate.");
     VipPrintfExit("(roi_utils.c)VipVoiIterateFromVida");
     return(PB);
    }

  if (mVipVolType(roi->ReferenceImage) != S16BIT)
     { 
     VipPrintfError("Statistics can only be got from Vida Images.");
     VipPrintfExit("(roi_utils.c)VipVoiIterateFromVida");
     return(PB);
     }
  
  /* Allocations */
  VipVoiAllocateMesData( roi );

  /* Pour tous les etiquettes decrites dans la structure roi */
  /* faire  le parcours des contenus des adresses et      */
  if (VipVoiDepIsIdentity(dep, roi->ReferenceImage, roi->VoiImage))
      {
	if (VipVoiIterateWithIdentity(roi) == PB)
	  {
	    VipPrintfError("(roi_utils.c)VipVoiIterate");
	    return(PB);
	  }
      }
      else
      {
	if (VipVoiIterateWithDepResampLinear(roi, dep) == PB)
	  {
	    VipPrintfError("(roi_utils.c)VipVoiIterate");
	    return(PB);
	  }
      }

  roi->Etat |= VROI_ITERATED | VROI_RDY_FORSTAT;
  return(OK);
}
/*---------------------------------------------------------------------------*/
int  VipVoiGetStatus(
   VipVoi                      *roi
)
/*---------------------------------------------------------------------------*/
{
   return(roi->Etat);
}
/*---------------------------------------------------------------------------*/
VipVoi  *VipVoiReadFromVolume_S16BIT(
   Volume                      *image,
   int                         *label,
   int                         nb_label
)
/*---------------------------------------------------------------------------*/
{
   VipVoi                      *region;
   Vip3DBucket_S16BIT          *buckptr;
   int l;

   region = VipVoiAllocate( image, label, nb_label );
   
   if (!region)
     {
       return ( (VipVoi *) NULL);
     }

   for (l = 0 ;l < nb_label; l++)
     {
       buckptr = VipRead3DBucketFromVolume_S16BIT(image, label[l]);
       if (buckptr == NULL) 
	 {
	   VipPrintfError("Error allocating Verctor3DBucket...\n");
	   return( PB );
	 }
       *(region->ListeDePaquetDePoint + l) = buckptr;
     }

  region->Etat = VROI_LOADED | VROI_RDY_FORARM;
  return(region);
}

/*---------------------------------------------------------------------------*/
int  VipVoiGetFunctionalValueFromVida(
   VipVoi                      *roi,
   VipDeplacement              *dep, 
   Volume                      *image
)
/*---------------------------------------------------------------------------*/
{

  roi->ReferenceImage = image;
  VipVoiSetDeplacement(roi, dep);

  if (VipVoiArmResamp(roi, dep) == PB)
    {
      VipPrintfError("Error while initializing data to resample in roi.");
      VipPrintfExit("(roi_utils.c)VipVoiGetFunctionalValue.");
      return(PB);
    }

  if (VipVoiIterateFromVida(roi, dep) == PB)
    {
      VipPrintfError("Error while iterating to get roi stats.");
      VipPrintfExit("(roi_utils.c)VipVoiGetFunctionalValue.");
      return(PB);
    }
  return(OK);
}

/*---------------------------------------------------------------------------*/
int  VipVoiGetFunctionalValueFromVidaWithMC(
   VipVoi                      *roi,
   VipDeplacementVector        *depVector, 
   Volume                      *image
)
/*---------------------------------------------------------------------------*/
{
  int  t;
  VipDeplacement *dep;

  roi->ReferenceImage = image;
  /*   VipVoiSetDeplacement(roi, dep); */ /* Il faudrait lever ce lievre */


  /* loop on frame at this level */
  for (t = 0,  dep = depVector->dep;
       t <  mVipVolSizeT(roi->ReferenceImage);
       t++, dep++)
    {
      if (VipVoiArmResamp(roi, dep) == PB)
	{
	  VipPrintfError("Error while initializing data to resample in roi.");
	  VipPrintfExit("(roi_utils.c)VipVoiGetFunctionalValueWithMC.");
	  return(PB);
	}

      if (VipVoiIterateFromVidaWithMC(roi, dep, t) == PB)
	{
	  VipPrintfError("Error while iterating to get roi stats.");
	  VipPrintfExit("(roi_utils.c)VipVoiGetFunctionalValueWithMC.");
	  return(PB);
	}
    }
  roi->Etat |=  VROI_RDY_FORSTAT;
  return(OK);
}


/*---------------------------------------------------------------------------*/
int  VipFreeVoi(
   VipVoi                      *roi
)
/*---------------------------------------------------------------------------*/
{
  int                          i, erreur=0;

  for (i=0; i< roi->NbVoi; i++)
    {
      if (VipFreeIntBucket( (roi->ListeDePaquetDoffset)[i] ) == PB) 
	{erreur=1;break;}
      if (VipFree(roi->ListeDePaquetDoffset )== PB) 
	{erreur=1;break;}
      if (VipFree3DBucket_U16BIT( (roi->ListeDePaquetDeDeplaEntier)[i] )== PB) 
	{erreur=1;break;}
      if (VipFree(roi->ListeDePaquetDeDeplaEntier) == PB) 
	{erreur=1;break;}
      if (VipFree3DBucket_S16BIT( (roi->ListeDePaquetDePoint)[i] )== PB) 
	{erreur=1;break;}
      if (VipFree(roi->ListeDePaquetDePoint) == PB) 
	{erreur=1;break;}
      if (VipFree( (roi->ListeDePaquetDeMesure)[i] ) == PB) 
	{erreur=1;break;}
      if (VipFree(roi->ListeDePaquetDeMesure) == PB) 
	{erreur=1;break;}

      if (VipFree(roi) == PB) 
	{erreur=1;break;}
    }

  if (erreur) 
    {
      VipPrintfError("Error while freeing memory.");
      VipPrintfExit("(roi_utils.c)VipFree().");
      return(PB);
    }
  else
    {
      return(OK);
    }
}

/*---------------------------------------------------------------------------*/
int   VipVoiGTMCorrection(
   VipVoi                      *voi,
   VipMatrix_VDOUBLE           *gtm
)
/*---------------------------------------------------------------------------*/
{
   VipMatrix_VDOUBLE            *statTab, *outstatTab;
   VipMatrix_VDOUBLE            *gtm_1;
   Vip_DOUBLE                   **cell = NULL;

   int                          i, j;
   Statistics                   *curVoiCurEpochStat, *curVoiCurEpochStatCorr;


   /* 1. Test ; verification de l'etat du Nut VipVoi *************************/
   if( !( VipVoiGetStatus(voi) & VROI_RDY_FORCORR) )
    {
      VipPrintfError("Voi not ready for statistics correction.");
      VipPrintfExit("(voi_utils.c)VipVoiGTMCorrection().");
      return(PB);
    }

   if (voi->NbVoi !=  VipGetMatrixSizeY_VDOUBLE(gtm))
     {
      VipPrintfError("gtm dim does not agree with Number of voi\n");
      VipPrintfExit("(voi_utils.c)VipVoiComputeStat().");
      return(PB);
    }

   /* 2. Allocation des espaces **********************************************/
   /*--------------------------------------------Espace interne StatCorrData */
   if (VipVoiAllocateStatCorrData( voi ) == PB)
     {
       return( PB );
     }
   /*-------------------------------------------Tableau de travail temporaire*/
   statTab = VipAllocationMatrix_VDOUBLE( 
		 VipGetMatrixSizeY_VDOUBLE(gtm),
		 mVipVolSizeT(voi->ReferenceImage), 
		 "");
   if (!statTab)
    {
      VipPrintfExit("(voi_utils.c)VipVoiGTMCorrection().");
      return(PB);
    }
   cell =  VipGetMatrixPtr_VDOUBLE(statTab);


   /* 3. Recopie des donnees      ********************************************/
   for (i = 0, (void)VipVoiFirstPtrStat(voi); i < voi->NbVoi; i++)
     {
       for(j=0, curVoiCurEpochStat=VipVoiFirstPtrStatVector(voi);
	   j < mVipVolSizeT(voi->ReferenceImage);
	   j++)
	 {
	   cell[i][j] = curVoiCurEpochStat->mean;
	   curVoiCurEpochStat=VipVoiNextPtrStatVector(voi);
	 }
       (void)VipVoiNextPtrStat(voi);
     }

   
   /* 4. Deconvolution algebrique ********************************************/
   VipPrintStreamMatrix_VDOUBLE(stdout, gtm);

   gtm_1 = VipMatrixInversion_VDOUBLE(gtm);

   VipPrintStreamMatrix_VDOUBLE(stdout, gtm_1);

   outstatTab = VipMatrixProduct_VDOUBLE(gtm_1, statTab);
   if (!outstatTab) 
     {
      VipPrintfExit("(voi_utils.c)VipVoiGTMCorrection().");
      return(PB);
    }

   /* 5. Re-Recopie des donnees  *********************************************/
   cell =  VipGetMatrixPtr_VDOUBLE(outstatTab);
   for (i = 0, (void)VipVoiFirstPtrStatCorr(voi); i < voi->NbVoi; i++)
     {
       for(j=0, curVoiCurEpochStatCorr=VipVoiFirstPtrStatCorrVector(voi);
	   j < mVipVolSizeT(voi->ReferenceImage);
	   j++)
	 {
	   curVoiCurEpochStatCorr->mean = cell[i][j];
	   curVoiCurEpochStatCorr       = VipVoiNextPtrStatCorrVector(voi);
	 }
       (void)VipVoiNextPtrStatCorr(voi);
     }

   return( OK );
}

/*---------------------------------------------------------------------------*/
int   VipVoiComputeStat(
   VipVoi                      *roi
)
/*---------------------------------------------------------------------------*/
{
  int                          i, j, k;
  int                          nb_point;
  float                         mean, std, min, max;
  int                          num;
  float                        **mesCour, *vecMesCour;
  Statistics                   **paquetstatCour;
  Statistics                   *statCour;
  VipIntBucket                 *offsetCour;
 
  
  if( !( VipVoiGetStatus(roi) & VROI_RDY_FORSTAT) )
    {
      VipPrintfError("Voi not ready for statistics computation.");
      VipPrintfExit("(roi_utils.c)VipVoiComputeStat().");
      return(PB);
    }

  /* Allocation de la zone statistique */
  VipVoiAllocateStatData( roi );
  paquetstatCour = VipVoiFirstPtrStat( roi );


  paquetstatCour = VipVoiFirstPtrStat( roi );
  mesCour        = VipVoiFirstPtrMes( roi );
  offsetCour     = VipVoiFirstPtrOffset( roi);
  for (i=0; i< roi->NbVoi; i++)
    {
      nb_point = offsetCour->n_points;
      statCour = VipVoiFirstPtrStatVector( roi );
      vecMesCour = VipVoiFirstPtrMesVector( roi );
      for(j=0; j < mVipVolSizeT(roi->ReferenceImage); j++)
	{
	  num  = 0;
	  mean = 0.0;
	  std  = 0.0;
	  min  = 0.0;
	  max  = 0.0;
	  for (k = nb_point; k > 0; k--)
	    {
	      num++;
	      mean  += *vecMesCour;
	      std   += mVipSq(*vecMesCour);
	      min   = ( (*vecMesCour < min) ? *vecMesCour : min);
	      max   = ( (*vecMesCour > max) ? *vecMesCour : max);
	      vecMesCour++;
	    }
	  statCour->vol  = (float) num * mVipVolVoxSizeX(roi->VoiImage)
	                                  * mVipVolVoxSizeY(roi->VoiImage)
                                          * mVipVolVoxSizeZ(roi->VoiImage);
	  if (num > 0) 
	    {
	      statCour->mean = mean/(float) num;
	      if (num >= 1)
		{
		  statCour->std =sqrt((std- mVipSq(statCour->mean)*num))/
		    ((float) num - 1.0);
		}
	      else
		{
		  statCour->std = 0.0;
		}
	    }
	  else
	    {
	      VipPrintfError("No stats for roi");
	      VipPrintfExit("(roi_utils.c)VipVoiStreamStat().");
	      return(PB);
	    }
	  statCour->min = min;
	  statCour->max = max;

	  vecMesCour = VipVoiNextPtrMesVector( roi );
	  statCour   = VipVoiNextPtrStatVector( roi );
	}
      paquetstatCour = VipVoiNextPtrStat( roi );
      mesCour        = VipVoiNextPtrMes( roi );
      offsetCour     = VipVoiNextPtrOffset( roi );
    }

  roi->Etat |= VROI_STATED | VROI_RDY_FOREDIT | VROI_RDY_FORCORR;
  return(OK);
}



/*---------------------------------------------------------------------------*/
int   VipVoiStreamStat(
   VipVoi                      *voi,
   FILE                        *stream,
   VipNomenList                *nomen,
   int                         flag
)
/*---------------------------------------------------------------------------*/
{
  int                          i, j, max;
  Statistics                   *curVoiCurEpochStat;
  Statistics                   *curVoiCurEpochStatCorr;
  char                          formatting[256];
  char                          printlabel[256];
  char                         *tmp;

  if ( !( voi->Etat & VROI_RDY_FOREDIT) )
    {
      VipPrintfError("Voi not ready for statistics edition.");
      VipPrintfExit("(roi_utils.c)VipVoiStreamStat().");
      return(PB);
    }


  if (nomen)
    {
      for (i = 0,max=0,(void)VipVoiFirstPtrStatCorr(voi); i < voi->NbVoi; i++)
	{
	  tmp = VipGetNameNomenListByLabel(nomen,voi->label[i]);
	  if (tmp)
	    {
	      if (max < (j= strlen(tmp))) max = j;
	    }
	}
      if (max < 10) max = 10;
    }
  else
    max = 10;


  switch (flag) 
    {
    case PVEC_CORR :
      sprintf(formatting, "%%%ds |%s",
	         max,
	        "%5s| %8s | %8s | %8s | %4s | %8s | %8s | %8s | %7s | %7s\n");
      fprintf(stream,formatting,
	  "Name", "Frame",
	  "Volume", "Mean", "StdDev","%",
	  "Min", "Max", "MeanCorr", "St Time", "Du Time");
      sprintf(formatting, "%%%ds |%s",
	              max,
	            " %3d |% .3e|% .3e|% .3e|%6d|% .3e|% .3e|% .3e|%9d|%9d\n");

      for (i = 0, (void)VipVoiFirstPtrStatCorr(voi),
	          (void)VipVoiFirstPtrStat(voi);
	   i < voi->NbVoi; i++)
	{
	  if (nomen)
	    {
	      if (VipGetNameNomenListByLabel(nomen,voi->label[i]) == NULL)
		sprintf(printlabel, "etiquet%03d", voi->label[i]);
	      else
		sprintf(printlabel, "%10s",
			VipGetNameNomenListByLabel(nomen,voi->label[i]));
	    }
	  else
	    {
	      sprintf(printlabel, "etiquet%03d", voi->label[i]);
	    }
	  for(j=0, curVoiCurEpochStatCorr=VipVoiFirstPtrStatCorrVector(voi),
		   curVoiCurEpochStat=VipVoiFirstPtrStatVector(voi);
	      j < mVipVolSizeT(voi->ReferenceImage);
	      j++)
	    {
	      fprintf(stream,formatting,
		      printlabel,
		      j,
		      curVoiCurEpochStat->vol,
		      curVoiCurEpochStat->mean,
		      curVoiCurEpochStat->std,
		      (int) (curVoiCurEpochStat->std/
			     curVoiCurEpochStat->mean*100),
		      curVoiCurEpochStat->min,
		      curVoiCurEpochStat->max,
		      curVoiCurEpochStatCorr->mean,  /* corrected term added*/
		      VipShfjStartTime(voi->ReferenceImage, j),
		      VipShfjDurationTime(voi->ReferenceImage,j)
		      );
	      curVoiCurEpochStatCorr = VipVoiNextPtrStatCorrVector(voi);
	      curVoiCurEpochStat = VipVoiNextPtrStatVector(voi);
	    }
	  (void)VipVoiNextPtrStatCorr(voi);
	  (void)VipVoiNextPtrStat(voi);
	}
      break;
    case UN_CORR :
    default :
      sprintf(formatting, "%%%ds |%s",
	      max,
	      "%5s| %8s | %8s | %8s | %4s | %8s | %8s | %7s | %7s\n");
      fprintf(stream,
	  "%10s |%5s| %8s | %8s | %8s | %4s | %8s | %8s | %7s | %7s\n",
	  "Name", "Frame",
	  "Volume", "Mean", "StdDev","%",
	  "Min", "Max", "St Time", "Du Time");
      sprintf(formatting, "%%%ds |%s",
	                  max,
	                  " %3d |% .3e|% .3e|% .3e|%6d|% .3e|% .3e|%9d|%9d\n");
      for (i = 0, (void)VipVoiFirstPtrStat(voi); i < voi->NbVoi; i++)
	{
	  if (nomen)
	    {
	      if (VipGetNameNomenListByLabel(nomen,voi->label[i]) == NULL)
		sprintf(printlabel, "etiquet%03d", voi->label[i]);
	      else
		sprintf(printlabel, "%10s",
			VipGetNameNomenListByLabel(nomen,voi->label[i]));
	    }
	  else
	    {
	      sprintf(printlabel, "etiquet%03d", voi->label[i]);
	    }
	  for(j=0, curVoiCurEpochStat=VipVoiFirstPtrStatVector(voi);
	      j < mVipVolSizeT(voi->ReferenceImage);
	      j++)
	    {  
	      fprintf(stream,formatting,
		      printlabel,
		      j,
		      curVoiCurEpochStat->vol,
		      curVoiCurEpochStat->mean,
		      curVoiCurEpochStat->std,
		      (int) (curVoiCurEpochStat->std/
			     curVoiCurEpochStat->mean*100),
		      curVoiCurEpochStat->min,
		      curVoiCurEpochStat->max,
		      VipShfjStartTime(voi->ReferenceImage, j),
		      VipShfjDurationTime(voi->ReferenceImage,j)
		      );
	      curVoiCurEpochStat = VipVoiNextPtrStatVector(voi);
	    }
	  (void)VipVoiNextPtrStat(voi);
	}
      break;
    }

  return(OK);
}
