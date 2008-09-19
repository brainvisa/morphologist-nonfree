/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : gtm_method.c         * TYPE     : library
 * AUTHOR      : FROUIN V.            * CREATION : 20/09/1999
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun
 ****************************************************************************
 *
 * DESCRIPTION : Mesures 
 *
 ****************************************************************************
 *
 * USED MODULES : 
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vip/pvec.h>
#include <vip/histo.h>
#include <vip/brain.h>
#include <vip/connex.h>
#include <vip/morpho.h>

#include <vip/distmap.h>
#include <vip/volume.h>
#include <vip/util.h>

#define MAX_DYN                 255
#define RATIO_TO_GET_99_SUPPORT 3.0

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*- - - - - - - - - - STATIC  Volume *getReferenceVoi(Volume *anatomy) - - - */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static Volume *getReferenceVoi(Volume *anatomy)
{
  VipT1HistoAnalysis *ana = NULL;   /* Pour la segmentation du fond */



  ana = VipGetT1HistoAnalysis(anatomy);
  if(ana==PB)
    {
      VipPrintfWarning("Histogram analysis problem, sorry...\n");
      return( (Volume *)PB );
    }
  if(ana->gray==PB) 
    {
      VipPrintfWarning("No gray/white distinction, \
                        switching to normal segmentation mode");
      return( (Volume *)PB );
    }

  VipGetBrainForRenderingSimple( anatomy, ana, NO, VFALSE);

   return( anatomy );
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*- - - - - -  STATIC  int getNumberAtLevelPoints(Volume vol, int label) - - */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static int getNumberAtLevelPoints(Volume *vol, int label)
{
  long number;
  VipOffsetStruct *vos;
  int ix, iy, iz, it;
  Vip_S16BIT *ptr;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(util.gen)VipGetNumberNonZeroPoints_S16BIT");
      return(PB);
    }

  vos = VipGetOffsetStructure(vol);
  ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;

  number = 0;
 
  for ( it = mVipVolSizeT(vol); it-- ; )            /* loop on volumes */
    {
      for ( iz = mVipVolSizeZ(vol); iz-- ; )             /* loop on slices */
        {
          for ( iy = mVipVolSizeY(vol); iy-- ; )          /* loop on lines */
            {            
              for ( ix = mVipVolSizeX(vol); ix-- ; )
                {
                  if(*ptr++ == label) number++;
                }
              ptr += vos->oPointBetweenLine;        /*skip border points*/
            }
          ptr += vos->oLineBetweenSlice; /*skip border lines*/
        }
      ptr += vos->oSliceBetweenVolume; /*skip border slices*/
    }

  VipFree(vos);

  return((int)number);
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*- -  STATIC int binarizeVolume2U8BIT(Volume *in, Volume *out, int label) - */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static int binarizeVolume2U8BIT(Volume *in, Volume *out, int label)
{
  
  VipOffsetStruct *vos, *vis;
  int             ix, iy, iz;
  Vip_S16BIT      *iptr;
  Vip_U8BIT       *optr;

  if (mVipVolType(out) != U8BIT)
    {
      return( PB );
    }

  VipSetImageLevel(out, 0);

  vis = VipGetOffsetStructure(in);
  iptr = VipGetDataPtr_S16BIT( in ) + vis->oFirstPoint;
  vos = VipGetOffsetStructure(out);
  optr = VipGetDataPtr_U8BIT( out ) + vos->oFirstPoint;

  for ( iz = mVipVolSizeZ(in); iz-- ; )               /* loop on slices */
    {
      for ( iy = mVipVolSizeY(in); iy-- ; )            /* loop on lines */
	{
         for ( ix = mVipVolSizeX(in); ix-- ; )       /* loop on points */
	   {
	     if (*iptr == label) *optr = 255;
	     iptr++;optr++;
	   }
         iptr += vis->oPointBetweenLine;  /*skip border points*/
         optr += vos->oPointBetweenLine;  /*skip border points*/
	}
      iptr += vis->oLineBetweenSlice;  /*skip border points*/
      optr += vos->oLineBetweenSlice;  /*skip border points*/
   }
  return( OK );
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*- - - - - - - - - - STATIC  float VolIntegre(Volume *vol) - - - -  - - - - */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static float VolIntegre(Volume *vol)
{
  int                i, j, k;
  VipOffsetStruct    *vos;
  Vip_FLOAT         *ptr;
  float              sum; 

  sum= 0;
  vos = VipGetOffsetStructure(vol);
  ptr = VipGetDataPtr_VFLOAT( vol ) + vos->oFirstPoint;
  for(k = mVipVolSizeZ(vol); k--;)
    {
      for(j = mVipVolSizeY(vol); j--;)
	{
	  for(i = mVipVolSizeX(vol); i--;)
	    {
	      sum += *ptr++; 
	    }
	  ptr +=vos->oPointBetweenLine;
	}
      ptr +=vos->oLineBetweenSlice;
    }
  return(sum);
}





VipMatrix_VDOUBLE *VipStriataGtm(
                   Volume *label_vol, int *label, int nb_label, 
		   float sigmaTrans, float sigmaAxi,
                   Volume *anatomy, int mode, int dump)
{ 


  /* Variables de travail                                            */
  VIP_DEC_VOLUME(    poolvol);       /* Pour le lissage              */
  VIP_DEC_VOLUME(    volsmooth);
  VIP_DEC_VOLUME(    tempvol);
  VIP_DEC_VOLUME(    label_vol_tmp);
  Volume	     *volregion;
  int                *cardinal;

  /* Variables de sortie                                            */
  VipMatrix_VDOUBLE  *gtm = NULL;  


  /* temporaires           compteurs                                 */
  int                i,j;
  float              integrale;
  char               tmpmess[VIP_NAME_MAXLEN];
  Vip_DOUBLE         **gtmcell = NULL;


					 
  /* 1- Allocation diverses  ------------------------------------------------*/
  /* ------------------------------------------------------------------------*/
  /*....................................................Matrice gtm a rendre */
  gtm = VipAllocationMatrix_VDOUBLE(nb_label+1, nb_label+1, 
				    "Can't alloc gtmatrix");
  if ( !gtm ) 
    {
      VipPrintfError("VipStriataGtm:malloc error");
      return( (VipMatrix_VDOUBLE *)PB );
    }
  /*.....................................................Table des cardinaux */
  cardinal = (int *) VipCalloc(nb_label+1, sizeof(int), "Can't malloc int");
  if (!cardinal)  
    {
      VipPrintfError("VipStriataGtm:malloc error");
      return( (VipMatrix_VDOUBLE *)PB );
    }
  /*................... Matrice de travail contenant les regions une par une */
  volregion = VipDuplicateVolumeStructure(label_vol,"_label_tmp");
  VipSetType(volregion, U8BIT);
  VipSetBorderWidth(volregion,0);
  if( VipAllocateVolumeData( volregion )  == PB )
    {
      VipPrintfWarning("VipStriataGtm");
      return( (VipMatrix_VDOUBLE *)PB ); 
    }

  /* 2- Segmentation de la partie gris blanc pour obtenir le fond. ----------*/
  /* ------------------------------------------------------------------------*/
  /*......................... Obtention de l'encephale + cervelet + spine ECS*/
  if (mode == GRAY_LEVEL_ANATOMY)
    {
      VipPrintfWarning("Calcul du masque du matiere grise...\n");
      poolvol = getReferenceVoi(anatomy);
    }
  else if (mode == BINARY_BRAIN)
    {
      VipPrintfWarning("Using --brain value...\n");
      poolvol = anatomy;
    }
  else
    {
      VipPrintfWarning("Incorrect mode for VipGTM sub");
      return( (VipMatrix_VDOUBLE *)PB ); 
    }
  sprintf( tmpmess, "%s_label_fond", mVipVolName(label_vol) );
  VipSetVolumeName ( poolvol, tmpmess );
  /*....................................Dilatation de l'ensemble des regions */
  label_vol_tmp = VipCopyVolume(label_vol, "");
  VipDilation(label_vol_tmp, CHAMFER_BALL_3D, 
	      (sigmaTrans > sigmaAxi ? sigmaTrans : sigmaAxi) * RATIO_TO_GET_99_SUPPORT);
  if (dump) VipWriteTivoliVolume(label_vol_tmp, "dilate");
 
  /*.................. Masquage par le ECS et chaque regions passee a l'algo */
  VipMaskVolumeLevel(label_vol_tmp,poolvol, 255);
  VipFreeVolume( poolvol ); VipFree( poolvol ); poolvol = NULL;
  poolvol = label_vol_tmp;
  for (i=0; i < nb_label; i++)
    {
      VipMerge(poolvol,label_vol,VIP_MERGE_ONE_TO_ONE,label[i],0); 
    }
  if (dump) VipWriteTivoliVolume(poolvol, "masque");


  /*3- Obtenir les nombres de points non nuls -------------------------------*/
  /* ------------------------------------------------------------------------*/
  printf("Getting cardinal of Rois...\n");
  for (i=0; i < nb_label+1; i++)
    {
      cardinal[i] = getNumberAtLevelPoints( (i==nb_label ? poolvol:label_vol),
					    (i==nb_label ? 255 : label[i]) );
      if (! cardinal[ i ] )
	{
	  sprintf(tmpmess, "gtm_method: no points for label %d", 
		  (i==nb_label ? 200 : label[i]) );
	  VipPrintfError(tmpmess);
	  return( (VipMatrix_VDOUBLE *)PB );
	}
      printf("cardinal[%d] = %d\n", i, cardinal[i]);
    }



  /* 4- Corps du calcul (lissage + masquage) --------------------------------*/
  printf("Smoothing roi masks...\n");
  for (i=0; i < nb_label+1; i++)
    {
      printf(" \t\tSmoothing %d...\n", i);
      /*........................... Recopie dans le vol temp 8bits d'un label*/
      if (i < nb_label)                    /* Pour poolvol trait particulier */
	{
	if ( binarizeVolume2U8BIT(label_vol, volregion, label[i]) == PB )
	  {
	    VipPrintfWarning("VipStriataGtm:can't convert to 8bits");
	    return( (VipMatrix_VDOUBLE *)PB );
	  }
	}
      else
	{
	  VipFreeVolume(volregion); VipFree(volregion); volregion = NULL;
	  volregion = poolvol;
	}
      if (dump)
	{
	  sprintf(tmpmess, "eti%d", (i==nb_label ? 200 : label[i]) );
	  if (VipWriteTivoliVolume(volregion, tmpmess) == PB)
	    {
	      VipPrintfError("gtm_method:Can't write image");
	      return( (VipMatrix_VDOUBLE *)PB );
	    }
	}
      /*........................................... Calcul de l'image lissee */
      if (volsmooth) 
	{
	  VipFreeVolume(volsmooth);
	  VipFree(volsmooth);
	  volsmooth = NULL;
	}
      volsmooth = VipDeriche2DAxiGaussian(volregion, 
					  sigmaTrans, sigmaAxi,
					  NEW_FLOAT_VOLUME);
      if (volsmooth == NULL) 
	{
	  VipPrintfError("gtm_method:Error while smoothing.");
	  return( (VipMatrix_VDOUBLE *)PB );
	}
      if (dump)
	{
	  sprintf(tmpmess, "eti_smooth%d", (i==nb_label ? 200 : label[i]) );
	  if (VipWriteTivoliVolume(volsmooth, tmpmess) == PB)
          {
            VipPrintfError("gtm_method:Can't write image");
	    return( (VipMatrix_VDOUBLE *)PB );
          }
	}
      /*....................... Calcul des poids correspondant a la lissee i */


      for (j=0; j< nb_label+1; j++)
	{
	  printf(" \t\tMasking smooth %d with roi %d...\n", i,j);
	  /*.......... Recopie de smoothvol dans tempvol qui va etre masquee */
	  if (tempvol) 
	    {
	      VipFreeVolume(tempvol);
	      VipFree(tempvol);
	      tempvol = NULL;
	    }
          tempvol = VipCopyVolume(volsmooth,"");
	  /*.......... ...............Gestion particuliere du masque du fond */
	  if ((i == nb_label-1) && (j == nb_label-1)) 
	    {
	    }
	  /*....................................................... Masquage */
	  if (VipMaskVolumeLevel(tempvol,
			    (j==nb_label ? poolvol:label_vol),
			    (j==nb_label ? 255 : label[j]) ) == PB)
	    {
	      return( (VipMatrix_VDOUBLE *)PB );
	    }
	  /*.......................................Integration/Normalisation */
          integrale =  VolIntegre(tempvol);
	  gtmcell = VipGetMatrixPtr_VDOUBLE(gtm);
	  gtmcell[j][i] = integrale / cardinal[j] / (float) (MAX_DYN);
	}
    }

  /* 5-Create a composite im with origi label AND label 200 (Pool striatal---*/
  VipMerge(label_vol, poolvol, VIP_MERGE_ONE_TO_ONE, 255, 200);
                          

  /* 6-Liberation des ressources */

    if (volsmooth)
      {
	VipFreeVolume(volsmooth);
	VipFree(volsmooth);
      }
    if (tempvol)
      {
	VipFreeVolume(tempvol);
	VipFree(tempvol);
      }
   if (volregion) 
       {
	 VipFreeVolume(volregion);
	 VipFree(volregion);
       }
    if (poolvol) 
       {
	 VipFreeVolume(poolvol);
	 VipFree(poolvol);
       }
  VipFree(cardinal);

  return ( gtm );

}
