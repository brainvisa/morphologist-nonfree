/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_brain.c          * TYPE     : sources
 * AUTHOR      : MANGIN J.F.          * CREATION : 20/12/1997
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : reguarisation de la classification gris/blanc pour la
 *               segmentation du cerveau dans une image T1
 *               jfmangin@cea.fr
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


#include <vip/volume.h>
#include <vip/util.h>
#include <vip/connex.h>
#include <vip/talairach.h>
#include "vip/histo.h"
#include "vip/brain.h"
#include <vip/skeleton.h>
#include <vip/skeleton_static.h>

#define BORDER_LABEL 51
#define VOID_STATIC_LABEL 11
#define GRAY_STATIC_LABEL 101
#define BRAIN_STATIC_LABEL 151
#define WHITE_STATIC_LABEL 201
#define FAT_STATIC_LABEL 221

/*---------------------------------------------------------------------------*/
static double Potentiel(int label, int graylevel, VipT1HistoAnalysis *ana);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static double PotentielForRobust(int label, int graylevel, VipT1HistoAnalysis *ana);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static double PotentielForRobust2005(int label, int graylevel, VipT1HistoAnalysis *ana);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static double PotentielForVoxelBasedAna(int label, int graylevel, VipT1HistoAnalysis *ana);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static double PotentielForVoxelBasedAna2011(int label, int graylevel, double mW, double mG, double sW, double sG);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static double PotentielForVoxelBasedNeighbourhood(Volume *vol, Volume *classif, Vip_S16BIT *vol_ptr, Vip_S16BIT *classif_ptr, int connectivity, int label);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static double NewPotentiel(int label, int graylevel, VipT1HistoAnalysis *ana);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static float deltaGPotentialGtoW(int glevel,float KG,float mG,float sigmaG,
			     float KW,float mW,float sigmaW);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static float deltaGPotentialWtoG(int glevel,float KG,float mG,float sigmaG,
			     float KW,float mW,float sigmaW);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static VipIntBucket *VipCreateFrontIntBucketGWClassifObject( Volume *vol, Volume *classif, int connectivity, int front_value, int domain, int outside);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int VipFillNextFrontFromOldFrontGWClassifObject(
Vip_S16BIT *first_vol_point,
Vip_S16BIT *first_classif_point,
VipIntBucket *buck,
VipIntBucket *nextbuck,
VipConnectivityStruct *vcs,
int front_value,
int domain,
int outside);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int GetSinusBucket(  Volume *vol, Volume *brain,
			    VipTalairach *tal,  
			    VipT1HistoAnalysis *ana, Vip3DBucket_S16BIT **buck);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Volume *VipComputeRawWhiteMask(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float white_erosion_size,
int white_recovering_size,
int T_white_fat_low,
int T_gray_white);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
Volume *VipGrayWhiteClassificationRegularisation(Volume *vol,VipT1HistoAnalysis *ana,
int dumb)
/*---------------------------------------------------------------------------*/
{
  Volume *classif;
  Vip_U8BIT *classif_ptr, *classif_first;
  Vip_S16BIT *data_ptr, *data_first;
  int sigma_gray_left, sigma_gray_right, sigma_white_left, sigma_white_right;
  int T_VOID_GRAY_LOW, T_VOID_GRAY_HIGH, T_GRAY_WHITE_LOW, T_GRAY_WHITE_HIGH, T_WHITE_FAT;
  VipOffsetStruct *data_vos, *classif_vos;
  VipConnectivityStruct *vcs;
  int icon; 
  int ix, iy, iz, n;
  int temp;
  int nb_iterations, nmodif;
  double KPOTTS;
  int currentlabel, neighbor_label, currentlabelsur2, newlabel, newlabelsur2;
  double deltaU;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipGrayWhiteClassificationRegularisation");
      return(PB);
    }
  if(!ana)
    {
      VipPrintfError("Null VipT1HistoAnalysis!");
      VipPrintfExit("(VipGrayWhiteClassificationRegularisation");
      return(PB);
    }

  if(ana->gray!=NULL && ana->white!=NULL)
    {
      sigma_gray_left = ana->gray->left_sigma;
      sigma_gray_right = ana->gray->right_sigma;
      sigma_white_left = ana->white->left_sigma;
      sigma_white_right = ana->white->right_sigma;
      if(ana->sequence==MRI_T1_SPGR && ana->partial_volume_effect==VFALSE)
	{
	  T_VOID_GRAY_LOW = ana->gray->mean - 5*ana->gray->left_sigma;
	  T_VOID_GRAY_HIGH = ana->gray->mean - 2.5*ana->gray->left_sigma;
	}
      else
	{
	  if(ana->sequence==MRI_T1_IR)
	    T_VOID_GRAY_LOW = ana->gray->mean - 3*ana->gray->left_sigma;
	  else
	    T_VOID_GRAY_LOW = ana->gray->mean - 4*ana->gray->left_sigma;
	  T_VOID_GRAY_HIGH = ana->gray->mean - 2*ana->gray->left_sigma;
	}
      T_GRAY_WHITE_LOW = ana->gray->mean + ana->gray->right_sigma;
      T_GRAY_WHITE_HIGH = ana->white->mean - ana->white->left_sigma;
      T_WHITE_FAT = ana->white->mean + 20*ana->white->right_sigma;
    } 
  else
    {
      VipPrintfError("The histogram scale space analysis did not give gray and white object");
      VipPrintfExit("VipGrayWhiteClassificationRegularisation");
      return(PB);
    }

  classif = VipDuplicateVolumeStructure( vol, "classif");
  if(classif == PB) return(PB);
  VipSetType(classif,U8BIT);
  if(mVipVolBorderWidth(classif)<=0) VipSetBorderWidth(classif,1);
  if (VipAllocateVolumeData( classif )==PB) return(PB);

  data_vos = VipGetOffsetStructure(vol);
  data_first = VipGetDataPtr_S16BIT( vol ) + data_vos->oFirstPoint;
  classif_vos = VipGetOffsetStructure(classif);
  classif_first = VipGetDataPtr_U8BIT( classif ) + classif_vos->oFirstPoint;
  data_ptr = data_first;
  classif_ptr = classif_first;

  /*initialisation*/

   for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
   {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
            temp = *data_ptr;
	    if(temp<=T_VOID_GRAY_LOW) *classif_ptr = VOID_STATIC_LABEL;
	    else if(temp<T_VOID_GRAY_HIGH)
	      {
		if(Potentiel(VOID_LABEL,temp,ana)<Potentiel(GRAY_LABEL,temp,ana))
		  *classif_ptr = VOID_LABEL;
		else *classif_ptr = GRAY_LABEL;
	      }
	    else if (temp<=T_GRAY_WHITE_LOW) *classif_ptr = GRAY_STATIC_LABEL;
	    else if (temp<T_GRAY_WHITE_HIGH)
	      {
		if(Potentiel(GRAY_LABEL,temp,ana)<Potentiel(WHITE_LABEL,temp,ana))
		  *classif_ptr = GRAY_LABEL;
		else *classif_ptr = WHITE_LABEL;
	      }
	    else if (temp<T_WHITE_FAT) *classif_ptr = WHITE_STATIC_LABEL;
	    else *classif_ptr = VOID_STATIC_LABEL;
            data_ptr++;
	    classif_ptr++;
         }
         classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
         data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
      }
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
      data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
   }

   VipSetBorderLevel(classif,BORDER_LABEL);
   vcs = VipGetConnectivityStruct( classif, CONNECTIVITY_6 );


 /*Minimisation*/
   KPOTTS = 0.1;

   if(dumb==VFALSE)
     {
       printf("------------------------------\n");
       printf("Classification regularisation:\n");
       printf("------------------------------\n");

       printf("[%d-%d]: CSF,skull / gray matter\n",T_VOID_GRAY_LOW,T_VOID_GRAY_HIGH);
       printf("[%d-%d]: gray matter / white matter\n",T_GRAY_WHITE_LOW,T_GRAY_WHITE_HIGH);
       printf("[%d-%d]: supposed to include white matter during white matter extraction\n",T_GRAY_WHITE_HIGH,T_WHITE_FAT);
     }
   nmodif = -1;
   nb_iterations = 3;
   for(n=1;(n<=nb_iterations)&&(nmodif!=0);n++)
     {
       nmodif = 0;
       data_ptr = data_first;
       classif_ptr = classif_first;
       for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
	 {
	   for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	     {
	       for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
		 {
		   currentlabel = *classif_ptr;
		   if(currentlabel%2==0)
		     {
		       currentlabelsur2 = currentlabel/2;
		       temp = *data_ptr;
		       if(currentlabel==GRAY_LABEL)
			 {
			   if(temp<T_VOID_GRAY_HIGH) newlabel = VOID_LABEL;
			   else newlabel = WHITE_LABEL;
			 }
		       else newlabel = GRAY_LABEL; /*from white or gray*/		     
		       newlabelsur2 = newlabel/2;

		       deltaU = 0.;
		       for(icon=0 ; icon<vcs->nb_neighbors ; icon++)
			 {
			   neighbor_label = *(classif_ptr + vcs->offset[icon]);
			   if(currentlabelsur2== (neighbor_label/2)) deltaU += KPOTTS;
			   else if(newlabelsur2==(neighbor_label/2)) deltaU -= KPOTTS;
			 }
		       /* printf("delta Potts: %f\n",deltaU);*/
		       deltaU += Potentiel(newlabel,temp,ana) - Potentiel(currentlabel,temp,ana);
		       /*printf(" Pot %f -> %f\n",Potentiel(currentlabel,temp,ana),Potentiel(newlabel,temp,ana));
		       scanf("%s",&c);*/
		       if(deltaU<0)
			 {
			   nmodif++;
			   *classif_ptr = newlabel;
			 }
		     }
		   data_ptr++;
		   classif_ptr++;
		 }
	       classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
	       data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
	     }
	   classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
	   data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
	 }
       if(dumb==VFALSE) printf("iteration %d/%d, modif: %d\n",n,nb_iterations,nmodif);
     }
   
  classif_ptr = classif_first;

  /*end of the stuff*/

   for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
   {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
            if(*classif_ptr%2==1) *classif_ptr -=1;
	    classif_ptr++;
         }
         classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
      }
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
   }


   return(classif);
}

/*---------------------------------------------------------------------------*/
Volume *VipGrayWhiteClassificationRegularisationForRobust(Volume *vol,VipT1HistoAnalysis *ana,
int dumb)
/*---------------------------------------------------------------------------*/
{
  Volume *classif;
  Vip_U8BIT *classif_ptr, *classif_first;
  Vip_S16BIT *data_ptr, *data_first;
  int sigma_gray_left, sigma_gray_right, sigma_white_left, sigma_white_right;
  int T_VOID_GRAY_LOW, T_VOID_GRAY_HIGH, T_GRAY_WHITE_LOW, T_GRAY_WHITE_HIGH, T_WHITE_FAT;
  VipOffsetStruct *data_vos, *classif_vos;
  VipConnectivityStruct *vcs;
  int icon; 
  int ix, iy, iz, n;
  int temp;
  int nb_iterations, nmodif;
  double KPOTTS;
  int currentlabel, neighbor_label, currentlabelsur2, newlabel, newlabelsur2;
  double deltaU;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipGrayWhiteClassificationRegularisation");
      return(PB);
    }
  if(!ana)
    {
      VipPrintfError("Null VipT1HistoAnalysis!");
      VipPrintfExit("(VipGrayWhiteClassificationRegularisation");
      return(PB);
    }

  if(ana->gray!=NULL && ana->white!=NULL)
    {
      sigma_gray_left = ana->gray->left_sigma;
      sigma_gray_right = ana->gray->right_sigma;
      sigma_white_left = ana->white->left_sigma;
      sigma_white_right = ana->white->right_sigma;
      if(ana->sequence==MRI_T1_SPGR && ana->partial_volume_effect==VFALSE)
	{
	  T_VOID_GRAY_LOW = ana->gray->mean - 5*ana->gray->left_sigma;
	  T_VOID_GRAY_HIGH = ana->gray->mean - 2.5*ana->gray->left_sigma;
	}
      else
	{
	  if(ana->sequence==MRI_T1_IR)
	    T_VOID_GRAY_LOW = ana->gray->mean - 3*ana->gray->left_sigma;
	  else
	    T_VOID_GRAY_LOW = ana->gray->mean - 4*ana->gray->left_sigma;
	  T_VOID_GRAY_HIGH = ana->gray->mean - 2*ana->gray->left_sigma;
	}
      T_GRAY_WHITE_LOW = ana->gray->mean + ana->gray->right_sigma;
      T_GRAY_WHITE_HIGH = ana->white->mean - ana->white->left_sigma;
      T_WHITE_FAT = ana->white->mean + 20*ana->white->right_sigma;
    } 
  else
    {
      VipPrintfError("The histogram scale space analysis did not give gray and white object");
      VipPrintfExit("VipGrayWhiteClassificationRegularisation");
      return(PB);
    }

  classif = VipDuplicateVolumeStructure( vol, "classif");
  if(classif == PB) return(PB);
  VipSetType(classif,U8BIT);
  if(mVipVolBorderWidth(classif)<=0) VipSetBorderWidth(classif,1);
  if (VipAllocateVolumeData( classif )==PB) return(PB);

  data_vos = VipGetOffsetStructure(vol);
  data_first = VipGetDataPtr_S16BIT( vol ) + data_vos->oFirstPoint;
  classif_vos = VipGetOffsetStructure(classif);
  classif_first = VipGetDataPtr_U8BIT( classif ) + classif_vos->oFirstPoint;
  data_ptr = data_first;
  classif_ptr = classif_first;

  /*initialisation*/

   for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
   {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
            temp = *data_ptr;
	    if(temp<=T_VOID_GRAY_LOW) *classif_ptr = VOID_STATIC_LABEL;
	    else if(temp<T_VOID_GRAY_HIGH)
	      {
		if(PotentielForRobust(VOID_LABEL,temp,ana)<PotentielForRobust(GRAY_LABEL,temp,ana))
		  *classif_ptr = VOID_LABEL;
		else *classif_ptr = GRAY_LABEL;
	      }
	    else if (temp<=T_GRAY_WHITE_LOW) *classif_ptr = GRAY_STATIC_LABEL;
	    else if (temp<T_GRAY_WHITE_HIGH)
	      {
		if(PotentielForRobust(GRAY_LABEL,temp,ana)<PotentielForRobust(WHITE_LABEL,temp,ana))
		  *classif_ptr = GRAY_LABEL;
		else *classif_ptr = WHITE_LABEL;
	      }
	    else if (temp<T_WHITE_FAT) *classif_ptr = WHITE_STATIC_LABEL;
	    else *classif_ptr = VOID_STATIC_LABEL;
            data_ptr++;
	    classif_ptr++;
         }
         classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
         data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
      }
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
      data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
   }

   VipSetBorderLevel(classif,BORDER_LABEL);
   vcs = VipGetConnectivityStruct( classif, CONNECTIVITY_6 );


 /*Minimisation*/
   KPOTTS = 0.1;

   if(dumb==VFALSE)
     {
       printf("------------------------------\n");
       printf("Classification regularisation:\n");
       printf("------------------------------\n");

       printf("[%d-%d]: CSF,skull / gray matter\n",T_VOID_GRAY_LOW,T_VOID_GRAY_HIGH);
       printf("[%d-%d]: gray matter / white matter\n",T_GRAY_WHITE_LOW,T_GRAY_WHITE_HIGH);
       printf("[%d-%d]: supposed to include white matter during white matter extraction\n",T_GRAY_WHITE_HIGH,T_WHITE_FAT);
     }
   nmodif = -1;
   nb_iterations = 3;
   for(n=1;(n<=nb_iterations)&&(nmodif!=0);n++)
     {
       nmodif = 0;
       data_ptr = data_first;
       classif_ptr = classif_first;
       for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
	 {
	   for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	     {
	       for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
		 {
		   currentlabel = *classif_ptr;
		   if(currentlabel%2==0)
		     {
		       currentlabelsur2 = currentlabel/2;
		       temp = *data_ptr;
		       if(currentlabel==GRAY_LABEL)
			 {
			   if(temp<T_VOID_GRAY_HIGH) newlabel = VOID_LABEL;
			   else newlabel = WHITE_LABEL;
			 }
		       else newlabel = GRAY_LABEL; /*from white or gray*/		     
		       newlabelsur2 = newlabel/2;

		       deltaU = 0.;
		       for(icon=0 ; icon<vcs->nb_neighbors ; icon++)
			 {
			   neighbor_label = *(classif_ptr + vcs->offset[icon]);
			   if(currentlabelsur2== (neighbor_label/2)) deltaU += KPOTTS;
			   else if(newlabelsur2==(neighbor_label/2)) deltaU -= KPOTTS;
			 }
		       /* printf("delta Potts: %f\n",deltaU);*/
		       deltaU += PotentielForRobust(newlabel,temp,ana) - PotentielForRobust(currentlabel,temp,ana);
		       /*printf(" Pot %f -> %f\n",Potentiel(currentlabel,temp,ana),Potentiel(newlabel,temp,ana));
		       scanf("%s",&c);*/
		       if(deltaU<0)
			 {
			   nmodif++;
			   *classif_ptr = newlabel;
			 }
		     }
		   data_ptr++;
		   classif_ptr++;
		 }
	       classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
	       data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
	     }
	   classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
	   data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
	 }
       if(dumb==VFALSE) printf("iteration %d/%d, modif: %d\n",n,nb_iterations,nmodif);
     }
   
  classif_ptr = classif_first;

  /*end of the stuff*/

   for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
   {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
            if(*classif_ptr%2==1) *classif_ptr -=1;
	    classif_ptr++;
         }
         classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
      }
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
   }


   return(classif);
}

/*---------------------------------------------------------------------------*/
Volume *VipGrayWhiteClassificationRegularisationForRobust2005(Volume *vol,VipT1HistoAnalysis *ana,
                                                              int T_VOID_GRAY_LOW, int T_VOID_GRAY_HIGH,
                                                              int T_GRAY_WHITE_LOW, int T_GRAY_WHITE_HIGH,
                                                              int T_WHITE_FAT, int dumb)
/*---------------------------------------------------------------------------*/
{
  Volume *classif;
  Vip_U8BIT *classif_ptr, *classif_first;
  Vip_S16BIT *data_ptr, *data_first;
  VipOffsetStruct *data_vos, *classif_vos;
  VipConnectivityStruct *vcs;
  int icon; 
  int ix, iy, iz, n;
  int temp;
  int nb_iterations, nmodif;
  double KPOTTS;
  int currentlabel, neighbor_label, currentlabelsur2, newlabel, newlabelsur2;
  double deltaU;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipGrayWhiteClassificationRegularisation");
      return(PB);
    }
  if(!ana)
    {
      VipPrintfError("Null VipT1HistoAnalysis!");
      VipPrintfExit("(VipGrayWhiteClassificationRegularisation");
      return(PB);
    }

  classif = VipDuplicateVolumeStructure( vol, "classif");
  if(classif == PB) return(PB);
  VipSetType(classif,U8BIT);
  if(mVipVolBorderWidth(classif)<=0) VipSetBorderWidth(classif,1);
  if (VipAllocateVolumeData( classif )==PB) return(PB);

  data_vos = VipGetOffsetStructure(vol);
  data_first = VipGetDataPtr_S16BIT( vol ) + data_vos->oFirstPoint;
  classif_vos = VipGetOffsetStructure(classif);
  classif_first = VipGetDataPtr_U8BIT( classif ) + classif_vos->oFirstPoint;
  data_ptr = data_first;
  classif_ptr = classif_first;

  /*initialisation*/

   for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
   {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
            temp = *data_ptr;
	    if(temp<=T_VOID_GRAY_LOW) *classif_ptr = VOID_STATIC_LABEL;
	    else if(temp<T_VOID_GRAY_HIGH)
	      {
		if(PotentielForRobust2005(VOID_LABEL,temp,ana)<PotentielForRobust2005(GRAY_LABEL,temp,ana))
		  *classif_ptr = VOID_LABEL;
		else *classif_ptr = GRAY_LABEL;
	      }
	    else if (temp<=T_GRAY_WHITE_LOW) *classif_ptr = GRAY_STATIC_LABEL;
	    else if (temp<T_GRAY_WHITE_HIGH)
	      {
		if(PotentielForRobust2005(GRAY_LABEL,temp,ana)<PotentielForRobust2005(WHITE_LABEL,temp,ana))
		  *classif_ptr = GRAY_LABEL;
		else *classif_ptr = WHITE_LABEL;
	      }
	    else if (temp<T_WHITE_FAT) *classif_ptr = WHITE_STATIC_LABEL;
	    else *classif_ptr = VOID_STATIC_LABEL;
            data_ptr++;
	    classif_ptr++;
         }
         classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
         data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
      }
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
      data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
   }

   VipSetBorderLevel(classif,BORDER_LABEL);
   vcs = VipGetConnectivityStruct( classif, CONNECTIVITY_6 );


 /*Minimisation*/
   KPOTTS = 0.1;

   if(dumb==VFALSE)
     {
       printf("------------------------------\n");
       printf("Classification regularisation:\n");
       printf("------------------------------\n");

       printf("[%d-%d]: CSF,skull / gray matter\n",T_VOID_GRAY_LOW,T_VOID_GRAY_HIGH);
       printf("[%d-%d]: gray matter / white matter\n",T_GRAY_WHITE_LOW,T_GRAY_WHITE_HIGH);
       printf("[%d-%d]: supposed to include white matter during white matter extraction\n",T_GRAY_WHITE_HIGH,T_WHITE_FAT);
     }
   nmodif = -1;
   nb_iterations = 3;
   for(n=1;(n<=nb_iterations)&&(nmodif!=0);n++)
     {
       nmodif = 0;
       data_ptr = data_first;
       classif_ptr = classif_first;
       for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
	 {
	   for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	     {
	       for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
		 {
		   currentlabel = *classif_ptr;
		   if(currentlabel%2==0)
		     {
		       currentlabelsur2 = currentlabel/2;
		       temp = *data_ptr;
		       if(currentlabel==GRAY_LABEL)
			 {
			   if(temp<T_VOID_GRAY_HIGH) newlabel = VOID_LABEL;
			   else newlabel = WHITE_LABEL;
			 }
		       else newlabel = GRAY_LABEL; /*from white or gray*/		     
		       newlabelsur2 = newlabel/2;

		       deltaU = 0.;
		       for(icon=0 ; icon<vcs->nb_neighbors ; icon++)
			 {
			   neighbor_label = *(classif_ptr + vcs->offset[icon]);
			   if(currentlabelsur2== (neighbor_label/2)) deltaU += KPOTTS;
			   else if(newlabelsur2==(neighbor_label/2)) deltaU -= KPOTTS;
			 }
		       /* printf("delta Potts: %f\n",deltaU);*/
		       deltaU += PotentielForRobust2005(newlabel,temp,ana) - PotentielForRobust2005(currentlabel,temp,ana);
		       /*printf(" Pot %f -> %f\n",Potentiel(currentlabel,temp,ana),Potentiel(newlabel,temp,ana));
		       scanf("%s",&c);*/
		       if(deltaU<0)
			 {
			   nmodif++;
			   *classif_ptr = newlabel;
			 }
		     }
		   data_ptr++;
		   classif_ptr++;
		 }
	       classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
	       data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
	     }
	   classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
	   data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
	 }
       if(dumb==VFALSE) printf("iteration %d/%d, modif: %d\n",n,nb_iterations,nmodif);
     }
   
  classif_ptr = classif_first;

  /*end of the stuff*/

   for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
   {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
            if(*classif_ptr%2==1) *classif_ptr -=1;
	    classif_ptr++;
         }
         classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
      }
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
   }


   return(classif);
}

/*---------------------------------------------------------------------------*/
static double NewPotentiel(int label, int graylevel, VipT1HistoAnalysis *ana)
/*---------------------------------------------------------------------------*/
  {
    double result, x;

  if(ana->gray!=NULL && ana->white!=NULL)
    {
      switch(label)
	{
	case GRAY_LABEL:
	  if(graylevel<ana->gray->mean)
	    {
	      if(ana->sequence==MRI_T1_SPGR && ana->partial_volume_effect==VFALSE)
		x = (double)(graylevel-ana->gray->mean)/(double)(ana->gray->sigma);
	      else
		x = (double)(graylevel-ana->gray->mean)/(double)(ana->gray->sigma);
	    }
	  else
	    {
	      x = (double)(graylevel-ana->gray->mean)/(double)(ana->gray->sigma);
	    }
	  /*
	  result = x*x / (1+x*x);
	  */
	  result = 1.5*x*x;
	  break;
	case WHITE_LABEL :
	  x = (double)(graylevel-ana->white->mean)/(double)(ana->white->sigma);
	  /*	  result = x*x / (1+x*x) could be better for white matter
		  it's better to prevent holes because the no risk of confusion with the skull
		  like for grey matter?;*/
	  result = 0.5*x*x;
	  break;
	case VOID_LABEL :
	  if(ana->sequence==MRI_T1_SPGR) result=0.5;
	  else
	    {
	      /*
	      x = (double)(graylevel-ana->gray->mean)/(double)(ana->gray->sigma);
	      result = 1. - x*x / (1+x*x);
	      */
	      result = 0.5;
	    }
	  break;
	case FAT_LABEL :
	  result = 0.5;
	  break;
	case BRAIN_LABEL :
	  if(graylevel<=ana->gray->mean)
	    {
	      if(ana->sequence==MRI_T1_SPGR && ana->partial_volume_effect==VFALSE)
		x = (double)(graylevel-ana->gray->mean)/(double)(2.5*ana->gray->sigma);
	      else
		x = (double)(graylevel-ana->gray->mean)/(double)(2*ana->gray->sigma);
	    }
	  else
	    {
	      x = (double)(graylevel-ana->white->mean)/(double)(2*ana->white->sigma);
	    }
	  result = x*x / (1+x*x);
	  break;
	default:
	  VipPrintfError("You can not use this label to get a potential!");
	  fprintf(stderr,"label: %d\n",label);
	  VipPrintfExit("Potentiel (Vip_Potts)");
	  return(PB);
	}
    }
  else
    {
 switch(label)
	{
	case VOID_LABEL :
	  if(ana->sequence==MRI_T1_SPGR) result=0.5;
	  else
	    {
	      x = (double)(graylevel-ana->brain->mean)/(double)(2.5*ana->brain->sigma);
	      result = 1. - x*x / (1+x*x);
	    }
	  break;
	case FAT_LABEL :
	  result = 0.5;
	  break;
	case BRAIN_LABEL :
	  if(graylevel<=ana->brain->mean)
	    {
	      x = (double)(graylevel-ana->brain->mean)/(double)(2.5*ana->brain->sigma);
	    }
	  else
	    {
	      x = (double)(graylevel-ana->brain->mean)/(double)(2*ana->brain->sigma);
	    }
	  result = x*x / (1+x*x);
	  break;
	default:
	  VipPrintfError("You can not use this label to get a potential (no gray/white cascades...)!");
	  fprintf(stderr,"label: %d\n",label);
	  VipPrintfExit("Potentiel (Vip_Potts)");
	  return(PB);
	}
    }
    return(result);
	
  }

  
/*---------------------------------------------------------------------------*/
Volume *VipCSFBrainFatClassificationRegularisation(Volume *vol,VipT1HistoAnalysis *ana,
int dumb, int nb_iterations)
/*---------------------------------------------------------------------------*/
{
  Volume *classif;
  Vip_U8BIT *classif_ptr, *classif_first;
  Vip_S16BIT *data_ptr, *data_first;
  int T_VOID_GRAY_LOW, T_VOID_GRAY_HIGH, T_WHITE_FAT_LOW, T_WHITE_FAT_HIGH;
  VipOffsetStruct *data_vos, *classif_vos;
  VipConnectivityStruct *vcs;
  int icon; 
  int ix, iy, iz, n;
  int temp;
  int nmodif;
  double KPOTTS;
  int currentlabel, neighbor_label, currentlabelsur2, newlabel, newlabelsur2;
  double deltaU;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipCSFBrainFatClassificationRegularisation");
      return(PB);
    }
  if(!ana)
    {
      VipPrintfError("Null VipT1HistoAnalysis!");
      VipPrintfExit("(VipCSFBrainFatClassificationRegularisation");
      return(PB);
    }

  if(ana->gray!=NULL && ana->white!=NULL)
    {  
      if(ana->sequence==MRI_T1_SPGR && ana->partial_volume_effect==VFALSE)
	{
	  T_VOID_GRAY_LOW = ana->gray->mean - 5*ana->gray->left_sigma;
	  T_VOID_GRAY_HIGH = ana->gray->mean - (int)(2.5*ana->gray->left_sigma);
	}
      else
	{
	  if(ana->sequence==MRI_T1_IR)
	    T_VOID_GRAY_LOW = ana->gray->mean - 3*ana->gray->left_sigma;
	  else
	    T_VOID_GRAY_LOW = ana->gray->mean - 4*ana->gray->left_sigma;
	  T_VOID_GRAY_HIGH = ana->gray->mean - 2*ana->gray->left_sigma;
	}
      T_WHITE_FAT_LOW = ana->white->mean + 2*ana->white->right_sigma;
      T_WHITE_FAT_HIGH = ana->white->mean + 6* ana->white->right_sigma;
    } 
  else if(ana->brain!=NULL)
    {
      T_VOID_GRAY_LOW = ana->brain->mean - 4*ana->brain->left_sigma;
      T_VOID_GRAY_HIGH = ana->brain->mean - (int)(2.5*ana->brain->left_sigma);
      T_WHITE_FAT_LOW = ana->brain->mean + 2*ana->brain->right_sigma;
      T_WHITE_FAT_HIGH = ana->brain->mean + 6* ana->brain->right_sigma;    
    }
  else return(PB);
    

  classif = VipDuplicateVolumeStructure( vol, "classif");
  if(classif == PB) return(PB);
  VipSetType(classif,U8BIT);
  if(mVipVolBorderWidth(classif)<=0) VipSetBorderWidth(classif,1);
  if (VipAllocateVolumeData( classif )==PB) return(PB);

  data_vos = VipGetOffsetStructure(vol);
  data_first = VipGetDataPtr_S16BIT( vol ) + data_vos->oFirstPoint;
  classif_vos = VipGetOffsetStructure(classif);
  classif_first = VipGetDataPtr_U8BIT( classif ) + classif_vos->oFirstPoint;
  data_ptr = data_first;
  classif_ptr = classif_first;

  /*initialisation*/

   for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
   {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
            temp = *data_ptr;
	    if(temp<=T_VOID_GRAY_LOW) *classif_ptr = VOID_STATIC_LABEL;
	    else if(temp<T_VOID_GRAY_HIGH)
	      {
		if(Potentiel(VOID_LABEL,temp,ana)<Potentiel(BRAIN_LABEL,temp,ana))
		  *classif_ptr = VOID_LABEL;
		else *classif_ptr = BRAIN_LABEL;
	      }
	    else if (temp<=T_WHITE_FAT_LOW) *classif_ptr = BRAIN_STATIC_LABEL;
	    else if (temp<T_WHITE_FAT_HIGH)
	      {
		if(Potentiel(BRAIN_LABEL,temp,ana)<Potentiel(FAT_LABEL,temp,ana))
		  *classif_ptr = BRAIN_LABEL;
		else *classif_ptr = FAT_LABEL;
	      }
	    else *classif_ptr = FAT_STATIC_LABEL;
            data_ptr++;
	    classif_ptr++;
         }
         classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
         data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
      }
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
      data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
   }

   VipSetBorderLevel(classif,BORDER_LABEL);
   vcs = VipGetConnectivityStruct( classif, CONNECTIVITY_6 );


 if(dumb==VFALSE)
     {
       printf("------------------------------\n");
       printf("Classification regularisation:\n");
       printf("------------------------------\n");
       printf("[%d-%d]: CSF,skull / brain tissues\n",T_VOID_GRAY_LOW,T_VOID_GRAY_HIGH);
       printf("[%d-%d]: brain tissues / fat, vascular\n",T_WHITE_FAT_LOW,T_WHITE_FAT_HIGH);
     }

 /*Minimisation*/
   KPOTTS = 0.1;

   if(dumb==VFALSE) printf("Classification regularisation...\n");
   nmodif = -1;
   for(n=1;(n<=nb_iterations)&&(nmodif!=0);n++)
     {
       nmodif = 0;
       data_ptr = data_first;
       classif_ptr = classif_first;
       for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
	 {
	   for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	     {
	       for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
		 {
		   currentlabel = *classif_ptr;
		   if(currentlabel%2==0)
		     {
		       currentlabelsur2 = currentlabel/2;
		       temp = *data_ptr;
		       if(currentlabel==BRAIN_LABEL)
			 {
			   if(temp<T_VOID_GRAY_HIGH) newlabel = VOID_LABEL;
			   else newlabel = FAT_LABEL;
			 }
		       else newlabel = BRAIN_LABEL; /*from white or gray*/		     
		       newlabelsur2 = newlabel/2;

		       deltaU = 0.;
		       for(icon=0 ; icon<vcs->nb_neighbors ; icon++)
			 {
			   neighbor_label = *(classif_ptr + vcs->offset[icon]);
			   if(currentlabelsur2== (neighbor_label/2)) deltaU += KPOTTS;
			   else if(newlabelsur2==(neighbor_label/2)) deltaU -= KPOTTS;
			 }
		       /* printf("delta Potts: %f\n",deltaU);*/
		       deltaU += Potentiel(newlabel,temp,ana) - Potentiel(currentlabel,temp,ana);
		       /*printf(" Pot %f -> %f\n",Potentiel(currentlabel,temp,ana),Potentiel(newlabel,temp,ana));
		       scanf("%s",&c);*/
		       if(deltaU<0)
			 {
			   nmodif++;
			   *classif_ptr = newlabel;
			 }
		     }
		   data_ptr++;
		   classif_ptr++;
		 }
	       classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
	       data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
	     }
	   classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
	   data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
	 }
       if(dumb==VFALSE) printf("iteration %d/%d, modif: %d\n",n,nb_iterations,nmodif);
     }
   
  classif_ptr = classif_first;

  /*end of the stuff*/

   for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
   {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
            if(*classif_ptr%2==1) *classif_ptr -=1;
	    classif_ptr++;
         }
         classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
      }
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
   }


   return(classif);
}

/*---------------------------------------------------------------------------*/
Volume *VipCSFGrayWhiteFatClassificationRegularisation(Volume *vol,VipT1HistoAnalysis *ana,
int dumb, int nb_iterations)
/*---------------------------------------------------------------------------*/
{
  Volume *classif;
  Vip_S16BIT *classif_ptr, *classif_first;
  Vip_S16BIT *data_ptr, *data_first;
  int T_VOID_GRAY_LOW, T_VOID_GRAY_HIGH, T_WHITE_FAT_LOW, T_WHITE_FAT_HIGH;
  int T_GRAY_WHITE;
  VipOffsetStruct *data_vos, *classif_vos;
  VipConnectivityStruct *vcs;
  int icon; 
  int ix, iy, iz, n;
  int temp;
  int nmodif;
  double KPOTTS;
  int currentlabel, neighbor_label, currentlabelsur2, newlabel, newlabelsur2;
  double deltaU;


  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipCSFBrainFatClassificationRegularisation");
      return(PB);
    }
  if(!ana)
    {
      VipPrintfError("Null VipT1HistoAnalysis!");
      VipPrintfExit("(VipCSFBrainFatClassificationRegularisation");
      return(PB);
    }

  if(ana->gray!=NULL && ana->white!=NULL)
    {  
      if(ana->sequence==MRI_T1_SPGR && ana->partial_volume_effect==VFALSE)
	{
	  T_VOID_GRAY_LOW = ana->gray->mean - 2.5*ana->gray->left_sigma;
	  T_VOID_GRAY_HIGH = ana->gray->mean - (int)(1.7*ana->gray->left_sigma);
	}
      else
	{
	  if(ana->sequence==MRI_T1_IR)
	      {
		  T_VOID_GRAY_LOW = ana->gray->mean - 2.2*ana->gray->sigma;
		  T_VOID_GRAY_HIGH = ana->gray->mean - 1.7*ana->gray->sigma;
	      }
	  else
	      {
		  T_VOID_GRAY_LOW = ana->gray->mean - 2.5*ana->gray->left_sigma;
		  T_VOID_GRAY_HIGH = ana->gray->mean - 1.7*ana->gray->left_sigma;
	      }

	}
      T_GRAY_WHITE = ana->gray->mean + (ana->white->mean - ana->gray->mean)/2;
      T_WHITE_FAT_LOW = ana->white->mean + 3*ana->white->right_sigma;
      T_WHITE_FAT_HIGH = ana->white->mean + 5* ana->white->right_sigma;
    } 
  else if(ana->brain!=NULL)
    {
      T_VOID_GRAY_LOW = ana->brain->mean - 4*ana->brain->left_sigma;
      T_VOID_GRAY_HIGH = ana->brain->mean - (int)(2*ana->brain->left_sigma);
      T_GRAY_WHITE = ana->brain->mean;
      T_WHITE_FAT_LOW = ana->brain->mean + 3*ana->brain->right_sigma;
      T_WHITE_FAT_HIGH = ana->brain->mean + 5* ana->brain->right_sigma;    
    }
  else return(PB);
    
  if(T_VOID_GRAY_HIGH<10)
    {
      VipPrintfWarning("Risky regularization, gray matter to close to 0, skip!");
      nb_iterations = 0;
    }

  classif = VipDuplicateVolumeStructure( vol, "classif");
  if(classif == PB) return(PB);
  if(mVipVolBorderWidth(classif)<=0) VipSetBorderWidth(classif,1);
  if (VipAllocateVolumeData( classif )==PB) return(PB);

  data_vos = VipGetOffsetStructure(vol);
  data_first = VipGetDataPtr_S16BIT( vol ) + data_vos->oFirstPoint;
  classif_vos = VipGetOffsetStructure(classif);
  classif_first = VipGetDataPtr_S16BIT( classif ) + classif_vos->oFirstPoint;
  data_ptr = data_first;
  classif_ptr = classif_first;

  /*initialisation*/

   for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
   {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
            temp = *data_ptr;
	    if(temp<=T_VOID_GRAY_LOW) *classif_ptr = VOID_STATIC_LABEL;
	    else if(temp<T_VOID_GRAY_HIGH)
	      {
		  *classif_ptr = VOID_LABEL;
	      }
	    else if (temp<T_GRAY_WHITE) *classif_ptr = GRAY_STATIC_LABEL;
	    else if (temp<=T_WHITE_FAT_LOW) *classif_ptr = WHITE_STATIC_LABEL;
	    else if (temp<T_WHITE_FAT_HIGH)
	      {
		  *classif_ptr = FAT_LABEL;
	      }
	    else *classif_ptr = FAT_STATIC_LABEL;
            data_ptr++;
	    classif_ptr++;
         }
         classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
         data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
      }
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
      data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
   }

   VipSetBorderLevel(classif,BORDER_LABEL);
   vcs = VipGetConnectivityStruct( classif, CONNECTIVITY_26 );

   /*VipWriteTivoliVolume(classif,"avant");*/

   printf("------------------------------\n");
   printf("Classification regularisation:\n");
   printf("------------------------------\n");
   
   printf("[%d-%d]: CSF,skull / brain tissues\n",T_VOID_GRAY_LOW,T_VOID_GRAY_HIGH);
   printf("[%d-%d]: brain tissues / fat, vascular\n",T_WHITE_FAT_LOW,T_WHITE_FAT_HIGH);

 /*Minimisation*/
   KPOTTS = 0.3;

   if(dumb==VFALSE) printf("Classification regularisation...\n");
   nmodif = -1;
   for(n=1;(n<=nb_iterations)&&(nmodif!=0);n++)
     {
       nmodif = 0;
       data_ptr = data_first;
       classif_ptr = classif_first;
       for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
	 {
	   for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	     {
	       for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
		 {
		   currentlabel = *classif_ptr;
		   if(currentlabel%2==0)
		     {
		       currentlabelsur2 = currentlabel/2;
		       if(currentlabel==GRAY_LABEL) newlabel = VOID_LABEL;
		       else if (currentlabel==VOID_LABEL) newlabel = GRAY_LABEL;
		       else if (currentlabel==WHITE_LABEL) newlabel = FAT_LABEL;
		       else if(currentlabel == FAT_LABEL) newlabel = WHITE_LABEL;
		       else newlabel = currentlabel;

		       if(newlabel!=currentlabel)
			   {
			       newlabelsur2 = newlabel/2;

			       deltaU = 0.;
			       for(icon=0 ; icon<vcs->nb_neighbors ; icon++)
				   {
				       neighbor_label = *(classif_ptr + vcs->offset[icon]);
				       if(currentlabelsur2== (neighbor_label/2)) deltaU += KPOTTS;
				       else if(newlabelsur2==(neighbor_label/2)) deltaU -= KPOTTS;
				   }
			       /* printf("delta Potts: %f\n",deltaU);*/
			       deltaU += NewPotentiel(newlabel,*data_ptr,ana) - NewPotentiel(currentlabel,*data_ptr,ana);
			       /*printf(" Pot %f -> %f\n",NewPotentiel(currentlabel,temp,ana),NewPotentiel(newlabel,temp,ana));
				 scanf("%s",&c);*/
			       if(deltaU<0)
				   {
				       nmodif++;
				       *classif_ptr = newlabel;
				   }
			   }
		     }
		   data_ptr++;
		   classif_ptr++;
		 }
	       classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
	       data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
	     }
	   classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
	   data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
	 }
       if(dumb==VFALSE) printf("iteration %d/%d, modif: %d\n",n,nb_iterations,nmodif);
     }
   
   /*VipWriteTivoliVolume(classif,"apres");*/

  classif_ptr = classif_first;

  /*end of the stuff*/

   for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
   {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
            if(*classif_ptr%2==1) *classif_ptr -=1;
	    if(*classif_ptr==GRAY_LABEL) *classif_ptr = BRAIN_LABEL;
	    else if(*classif_ptr==WHITE_LABEL) *classif_ptr = BRAIN_LABEL;
	    classif_ptr++;
         }
         classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
      }
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
   }


   return(classif);
}

/*---------------------------------------------------------------------------*/
Volume *VipCSFGrayWhiteFatClassificationRegularisationForRobustApproach(Volume *vol,VipT1HistoAnalysis *ana,
int dumb, int nb_iterations,int T_VOID_GRAY_LOW, int T_VOID_GRAY_HIGH, int T_WHITE_FAT_LOW, int T_WHITE_FAT_HIGH, int T_GRAY_WHITE)
/*---------------------------------------------------------------------------*/
{
  Volume *classif;
  Vip_S16BIT *classif_ptr, *classif_first;
  Vip_S16BIT *data_ptr, *data_first;
  VipOffsetStruct *data_vos, *classif_vos;
  VipConnectivityStruct *vcs;
  int icon; 
  int ix, iy, iz, n;
  int temp;
  int nmodif;
  double KPOTTS;
  int currentlabel, neighbor_label, currentlabelsur2, newlabel, newlabelsur2;
  double deltaU;


  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipCSFBrainFatClassificationRegularisation");
      return(PB);
    }
  if(!ana)
    {
      VipPrintfError("Null VipT1HistoAnalysis!");
      VipPrintfExit("(VipCSFBrainFatClassificationRegularisation");
      return(PB);
    }
    
  if(T_VOID_GRAY_HIGH<10)
    {
      VipPrintfWarning("Risky regularization, gray matter to close to 0, skip!");
      nb_iterations = 0;
    }

  classif = VipDuplicateVolumeStructure( vol, "classif");
  if(classif == PB) return(PB);
  if(mVipVolBorderWidth(classif)<=0) VipSetBorderWidth(classif,1);
  if (VipAllocateVolumeData( classif )==PB) return(PB);

  data_vos = VipGetOffsetStructure(vol);
  data_first = VipGetDataPtr_S16BIT( vol ) + data_vos->oFirstPoint;
  classif_vos = VipGetOffsetStructure(classif);
  classif_first = VipGetDataPtr_S16BIT( classif ) + classif_vos->oFirstPoint;
  data_ptr = data_first;
  classif_ptr = classif_first;

  /*initialisation*/

   for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
   {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
            temp = *data_ptr;
	    if(temp<=T_VOID_GRAY_LOW) *classif_ptr = VOID_STATIC_LABEL;
	    else if(temp<T_VOID_GRAY_HIGH)
	      {
		  *classif_ptr = VOID_LABEL;
	      }
	    else if (temp<T_GRAY_WHITE) *classif_ptr = GRAY_STATIC_LABEL;
	    else if (temp<=T_WHITE_FAT_LOW) *classif_ptr = WHITE_STATIC_LABEL;
	    else if (temp<T_WHITE_FAT_HIGH)
	      {
		  *classif_ptr = FAT_LABEL;
	      }
	    else *classif_ptr = FAT_STATIC_LABEL;
            data_ptr++;
	    classif_ptr++;
         }
         classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
         data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
      }
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
      data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
   }

   VipSetBorderLevel(classif,BORDER_LABEL);
   vcs = VipGetConnectivityStruct( classif, CONNECTIVITY_26 );

   /*VipWriteTivoliVolume(classif,"avant");*/

   printf("------------------------------\n");
   printf("Classification regularisation:\n");
   printf("------------------------------\n");
   
   printf("[%d-%d]: CSF,skull / brain tissues\n",T_VOID_GRAY_LOW,T_VOID_GRAY_HIGH);
   printf("[%d-%d]: brain tissues / fat, vascular\n",T_WHITE_FAT_LOW,T_WHITE_FAT_HIGH);

 /*Minimisation*/
   KPOTTS = 0.3;

   if(dumb==VFALSE) printf("Classification regularisation...\n");
   nmodif = -1;
   for(n=1;(n<=nb_iterations)&&(nmodif!=0);n++)
     {
       nmodif = 0;
       data_ptr = data_first;
       classif_ptr = classif_first;
       for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
	 {
	   for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	     {
	       for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
		 {
		   currentlabel = *classif_ptr;
		   if(currentlabel%2==0)
		     {
		       currentlabelsur2 = currentlabel/2;
		       if(currentlabel==GRAY_LABEL) newlabel = VOID_LABEL;
		       else if (currentlabel==VOID_LABEL) newlabel = GRAY_LABEL;
		       else if (currentlabel==WHITE_LABEL) newlabel = FAT_LABEL;
		       else if(currentlabel == FAT_LABEL) newlabel = WHITE_LABEL;
		       else newlabel = currentlabel;

		       if(newlabel!=currentlabel)
			   {
			       newlabelsur2 = newlabel/2;

			       deltaU = 0.;
			       for(icon=0 ; icon<vcs->nb_neighbors ; icon++)
				   {
				       neighbor_label = *(classif_ptr + vcs->offset[icon]);
				       if(currentlabelsur2== (neighbor_label/2)) deltaU += KPOTTS;
				       else if(newlabelsur2==(neighbor_label/2)) deltaU -= KPOTTS;
				   }
			       /* printf("delta Potts: %f\n",deltaU);*/
			       deltaU += NewPotentiel(newlabel,*data_ptr,ana) - NewPotentiel(currentlabel,*data_ptr,ana);
			       /*printf(" Pot %f -> %f\n",NewPotentiel(currentlabel,temp,ana),NewPotentiel(newlabel,temp,ana));
				 scanf("%s",&c);*/
			       if(deltaU<0)
				   {
				       nmodif++;
				       *classif_ptr = newlabel;
				   }
			   }
		     }
		   data_ptr++;
		   classif_ptr++;
		 }
	       classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
	       data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
	     }
	   classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
	   data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
	 }
       if(dumb==VFALSE) printf("iteration %d/%d, modif: %d\n",n,nb_iterations,nmodif);
     }
   
   /*VipWriteTivoliVolume(classif,"apres");*/

  classif_ptr = classif_first;

  /*end of the stuff*/

   for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
   {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
            if(*classif_ptr%2==1) *classif_ptr -=1;
	    if(*classif_ptr==GRAY_LABEL) *classif_ptr = BRAIN_LABEL;
	    else if(*classif_ptr==WHITE_LABEL) *classif_ptr = BRAIN_LABEL;
	    classif_ptr++;
         }
         classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
      }
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
   }


   return(classif);
}


/*---------------------------------------------------------------------------*/
static double Potentiel(int label, int graylevel, VipT1HistoAnalysis *ana)
/*---------------------------------------------------------------------------*/
  {
    double result, x;

  if(ana->gray!=NULL && ana->white!=NULL)
    {
      switch(label)
	{
	case GRAY_LABEL:
	  if(graylevel<ana->gray->mean)
	    {
	      if(ana->sequence==MRI_T1_SPGR && ana->partial_volume_effect==VFALSE)
		x = (double)(graylevel-ana->gray->mean)/(double)(2.5*ana->gray->left_sigma);
	      else
		x = (double)(graylevel-ana->gray->mean)/(double)(2*ana->gray->left_sigma);
	    }
	  else
	    {
	      x = (double)(graylevel-ana->gray->mean)/(double)(2*ana->gray->right_sigma);
	    }
	  result = x*x / (1+x*x);
	  break;
	case WHITE_LABEL :
	  x = (double)(graylevel-ana->white->mean)/(double)(2*ana->white->left_sigma);
	  result = x*x / (1+x*x);
	  break;
	case VOID_LABEL :
	  if(ana->sequence==MRI_T1_SPGR) result=0.5;
	  else
	    {
	      x = (double)(graylevel-ana->gray->mean)/(double)(2*ana->gray->left_sigma);
	      result = 1. - x*x / (1+x*x);
	    }
	  break;
	case FAT_LABEL :
	  result = 0.5;
	  break;
	case BRAIN_LABEL :
	  if(graylevel<=ana->gray->mean)
	    {
	      if(ana->sequence==MRI_T1_SPGR && ana->partial_volume_effect==VFALSE)
		x = (double)(graylevel-ana->gray->mean)/(double)(2.5*ana->gray->left_sigma);
	      else
		x = (double)(graylevel-ana->gray->mean)/(double)(2*ana->gray->left_sigma);
	    }
	  else
	    {
	      x = (double)(graylevel-ana->white->mean)/(double)(2*ana->white->right_sigma);
	    }
	  result = x*x / (1+x*x);
	  break;
	default:
	  VipPrintfError("You can not use this label to get a potential!");
	  fprintf(stderr,"label: %d\n",label);
	  VipPrintfExit("Potentiel (Vip_Potts)");
	  return(PB);
	}
    }
  else
    {
 switch(label)
	{
	case VOID_LABEL :
	  if(ana->sequence==MRI_T1_SPGR) result=0.5;
	  else
	    {
	      x = (double)(graylevel-ana->brain->mean)/(double)(2.5*ana->brain->left_sigma);
	      result = 1. - x*x / (1+x*x);
	    }
	  break;
	case FAT_LABEL :
	  result = 0.5;
	  break;
	case BRAIN_LABEL :
	  if(graylevel<=ana->brain->mean)
	    {
	      x = (double)(graylevel-ana->brain->mean)/(double)(2.5*ana->brain->left_sigma);
	    }
	  else
	    {
	      x = (double)(graylevel-ana->brain->mean)/(double)(2*ana->brain->right_sigma);
	    }
	  result = x*x / (1+x*x);
	  break;
	default:
	  VipPrintfError("You can not use this label to get a potential (no gray/white cascades...)!");
	  fprintf(stderr,"label: %d\n",label);
	  VipPrintfExit("Potentiel (Vip_Potts)");
	  return(PB);
	}
    }
    return(result);
	
  }
/*---------------------------------------------------------------------------*/
static double PotentielForRobust(int label, int graylevel, VipT1HistoAnalysis *ana)
/*---------------------------------------------------------------------------*/
  {
    double result, x;

  if(ana->gray!=NULL && ana->white!=NULL)
    {
      switch(label)
	{
	case GRAY_LABEL:	  
	  x = (double)(graylevel-ana->gray->mean)/(double)(ana->gray->sigma);
	  result = x*x;      
	  break;
	case WHITE_LABEL :
	  x = (double)(graylevel-ana->white->mean)/(double)(ana->white->sigma);
	  result = x*x;
	  break;
	case VOID_LABEL :
	  if(ana->sequence==MRI_T1_SPGR) result=0.5;
	  else
	    {
	      x = (double)(graylevel-ana->gray->mean)/(double)(2*ana->gray->left_sigma);
	      result = 1. - x*x / (1+x*x);
	    }
	  break;
	case FAT_LABEL :
	  result = 0.5;
	  break;
	case BRAIN_LABEL :
	  if(graylevel<=ana->gray->mean)
	    {
	      if(ana->sequence==MRI_T1_SPGR && ana->partial_volume_effect==VFALSE)
		x = (double)(graylevel-ana->gray->mean)/(double)(2.5*ana->gray->left_sigma);
	      else
		x = (double)(graylevel-ana->gray->mean)/(double)(2*ana->gray->left_sigma);
	    }
	  else
	    {
	      x = (double)(graylevel-ana->white->mean)/(double)(2*ana->white->right_sigma);
	    }
	  result = x*x / (1+x*x);
	  break;
	default:
	  VipPrintfError("You can not use this label to get a potential!");
	  fprintf(stderr,"label: %d\n",label);
	  VipPrintfExit("Potentiel (Vip_Potts)");
	  return(PB);
	}
    }
  else
    {
 switch(label)
	{
	case VOID_LABEL :
	  if(ana->sequence==MRI_T1_SPGR) result=0.5;
	  else
	    {
	      x = (double)(graylevel-ana->brain->mean)/(double)(2.5*ana->brain->left_sigma);
	      result = 1. - x*x / (1+x*x);
	    }
	  break;
	case FAT_LABEL :
	  result = 0.5;
	  break;
	case BRAIN_LABEL :
	  if(graylevel<=ana->brain->mean)
	    {
	      x = (double)(graylevel-ana->brain->mean)/(double)(2.5*ana->brain->left_sigma);
	    }
	  else
	    {
	      x = (double)(graylevel-ana->brain->mean)/(double)(2*ana->brain->right_sigma);
	    }
	  result = x*x / (1+x*x);
	  break;
	default:
	  VipPrintfError("You can not use this label to get a potential (no gray/white cascades...)!");
	  fprintf(stderr,"label: %d\n",label);
	  VipPrintfExit("Potentiel (Vip_Potts)");
	  return(PB);
	}
    }
    return(result);
	
  }

/*---------------------------------------------------------------------------*/
static double PotentielForRobust2005(int label, int graylevel, VipT1HistoAnalysis *ana)
/*---------------------------------------------------------------------------*/
  {
    double result, x;

  if(ana->gray!=NULL && ana->white!=NULL)
    {
      switch(label)
	{
	case GRAY_LABEL:	  
	  x = (double)(graylevel-ana->gray->mean)/(double)(ana->gray->sigma);
	  result = x*x;      
	  break;
	case WHITE_LABEL :
	  x = (double)(graylevel-ana->white->mean)/(double)(ana->white->sigma);
	  result = x*x;
	  break;
	case VOID_LABEL :
	  if(ana->sequence==MRI_T1_SPGR) result=0.5;
	  else
	    {
	      x = (double)(graylevel-ana->gray->mean)/(double)(2*ana->gray->sigma);
	      result = 1. - x*x / (1+x*x);
	    }
	  break;
	case FAT_LABEL :
	  result = 0.5;
	  break;
	case BRAIN_LABEL :
	  if(graylevel<=ana->gray->mean)
	    {
	      if(ana->sequence==MRI_T1_SPGR && ana->partial_volume_effect==VFALSE)
		x = (double)(graylevel-ana->gray->mean)/(double)(2.5*ana->gray->sigma);
	      else
		x = (double)(graylevel-ana->gray->mean)/(double)(2*ana->gray->sigma);
	    }
	  else
	    {
	      x = (double)(graylevel-ana->white->mean)/(double)(2*ana->white->sigma);
	    }
	  result = x*x / (1+x*x);
	  break;
	default:
	  VipPrintfError("You can not use this label to get a potential!");
	  fprintf(stderr,"label: %d\n",label);
	  VipPrintfExit("Potentiel (Vip_Potts)");
	  return(PB);
	}
    }
  else
    {
 switch(label)
	{
	case VOID_LABEL :
	  if(ana->sequence==MRI_T1_SPGR) result=0.5;
	  else
	    {
	      x = (double)(graylevel-ana->brain->mean)/(double)(2.5*ana->brain->sigma);
	      result = 1. - x*x / (1+x*x);
	    }
	  break;
	case FAT_LABEL :
	  result = 0.5;
	  break;
	case BRAIN_LABEL :
	  if(graylevel<=ana->brain->mean)
	    {
	      x = (double)(graylevel-ana->brain->mean)/(double)(2.5*ana->brain->sigma);
	    }
	  else
	    {
	      x = (double)(graylevel-ana->brain->mean)/(double)(2*ana->brain->sigma);
	    }
	  result = x*x / (1+x*x);
	  break;
	default:
	  VipPrintfError("You can not use this label to get a potential (no gray/white cascades...)!");
	  fprintf(stderr,"label: %d\n",label);
	  VipPrintfExit("Potentiel (Vip_Potts)");
	  return(PB);
	}
    }
    return(result);
	
  }
/*---------------------------------------------------------------------------*/
Volume *VipGrayWhiteClassificationRegularisationForVoxelBasedAna(Volume *vol,VipT1HistoAnalysis *ana,
int dumb, int nb_iterations, float KPOTTS, int connectivity)
/*---------------------------------------------------------------------------*/
{
  Volume *classif;
  Vip_S16BIT *classif_ptr, *classif_first;
  Vip_S16BIT *data_ptr, *data_first;
  int T_VOID_GRAY_HIGH=0;
  int T_GRAY_WHITE_LOW=0;
  int T_GRAY_WHITE_HIGH=0;
  int T_WHITE_FAT = 0;
  VipOffsetStruct *data_vos, *classif_vos;
  VipConnectivityStruct *vcs;
  int icon; 
  int ix, iy, iz, n;
  int temp;
  int nmodif;
  int currentlabel, neighbor_label, currentlabelsur2, newlabel=0, newlabelsur2;
  double deltaU;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipGrayWhiteClassificationRegularisationForVoxelBasedAna");
      return(PB);
    }
  if(!ana)
    {
      VipPrintfError("Null VipT1HistoAnalysis!");
      VipPrintfExit("(VipGrayWhiteClassificationRegularisationForVoxelBasedAna");
      return(PB);
    }

  if(ana->gray!=NULL && ana->white!=NULL)
    {
      T_VOID_GRAY_HIGH = (int)(ana->gray->mean - 4* ana->gray->sigma);
      T_GRAY_WHITE_LOW = (int)(ana->gray->mean + 0.5*ana->gray->sigma);
      T_GRAY_WHITE_HIGH = (int)(ana->white->mean - 0.5*ana->white->sigma);
      T_WHITE_FAT = (int)(ana->white->mean + 4 * ana->white->sigma);
    } 
  else
    {
      VipPrintfError("The histogram scale space analysis did not give gray and white object");
      VipPrintfExit("VipGrayWhiteClassificationRegularisation");
      return(PB);
    }
  if (T_VOID_GRAY_HIGH<=0) T_VOID_GRAY_HIGH=1;

  classif = VipDuplicateVolumeStructure( vol, "classif");
  if(classif == PB) return(PB);
  if(mVipVolBorderWidth(classif)<=0) VipSetBorderWidth(classif,1);
  if (VipAllocateVolumeData( classif )==PB) return(PB);

  data_vos = VipGetOffsetStructure(vol);
  data_first = VipGetDataPtr_S16BIT( vol ) + data_vos->oFirstPoint;
  classif_vos = VipGetOffsetStructure(classif);
  classif_first = VipGetDataPtr_S16BIT( classif ) + classif_vos->oFirstPoint;
  data_ptr = data_first;
  classif_ptr = classif_first;

  /*initialisation*/

  for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
    {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	{
	  for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	    {
	      temp = *data_ptr;
	      if(temp<T_VOID_GRAY_HIGH)
		{
		    *classif_ptr = VOID_STATIC_LABEL;
		}
	      else if (temp<=T_GRAY_WHITE_LOW) *classif_ptr = GRAY_STATIC_LABEL;
	      else if (temp<T_GRAY_WHITE_HIGH)
		{
		  if(PotentielForVoxelBasedAna(GRAY_LABEL,temp,ana)<PotentielForVoxelBasedAna(WHITE_LABEL,temp,ana))
		    *classif_ptr = GRAY_LABEL;
		  else *classif_ptr = WHITE_LABEL;
		}
	      else if (temp<T_WHITE_FAT) *classif_ptr = WHITE_STATIC_LABEL;
	      else *classif_ptr = VOID_STATIC_LABEL;
	      data_ptr++;
	      classif_ptr++;
	    }
	  classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
	  data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
	}
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
      data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
    }

  VipSetBorderLevel(classif,BORDER_LABEL);
  vcs = VipGetConnectivityStruct( classif, connectivity );
  KPOTTS /= vcs->nb_neighbors; /* Convention to simplify user interface feeling */

  /*Minimisation*/

  if(dumb==VFALSE)
    {
      printf("------------------------------\n");
      printf("Classification regularisation:\n");
      printf("------------------------------\n");
      printf("[%d-%d]: supposed to be only grey matter\n",T_VOID_GRAY_HIGH,T_GRAY_WHITE_LOW);
      printf("[%d-%d]: competition gray matter / white matter\n",T_GRAY_WHITE_LOW,T_GRAY_WHITE_HIGH);
      printf("[%d-%d]: supposed to be only white matter\n",T_GRAY_WHITE_HIGH,T_WHITE_FAT);
    }
  nmodif = -1;
  for(n=1;(n<=nb_iterations)&&(nmodif!=0);n++)
    {
      nmodif = 0;
      data_ptr = data_first;
      classif_ptr = classif_first;
      for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
	{
	  for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	    {
	      for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
		{
		  currentlabel = *classif_ptr;
		  if(currentlabel%2==0)
		    {
		      currentlabelsur2 = currentlabel/2;
		      temp = *data_ptr;
		       if(currentlabel==GRAY_LABEL) newlabel = WHITE_LABEL;
		       else if (currentlabel==WHITE_LABEL) newlabel = GRAY_LABEL;
		       else newlabel = currentlabel;
		      newlabelsur2 = newlabel/2;

		      if(newlabel!=currentlabel)
			{
			  deltaU = 0.;
			  for(icon=0 ; icon<vcs->nb_neighbors ; icon++)
			    {
			      neighbor_label = *(classif_ptr + vcs->offset[icon]);
			      if(currentlabelsur2== (neighbor_label/2)) deltaU += KPOTTS;
			      else if(newlabelsur2==(neighbor_label/2)) deltaU -= KPOTTS;
			    }
			  /* printf("delta Potts: %f\n",deltaU);*/
			  deltaU += PotentielForVoxelBasedAna(newlabel,temp,ana) - PotentielForVoxelBasedAna(currentlabel,temp,ana);
			  /*printf(" Pot %f -> %f\n",Potentiel(currentlabel,temp,ana),Potentiel(newlabel,temp,ana));
			    scanf("%s",&c);*/
			  if(deltaU<0)
			    {
			      nmodif++;
			      *classif_ptr = newlabel;
			    }
			}
		    }
		  data_ptr++;
		  classif_ptr++;
		}
	      classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
	      data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
	    }
	  classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
	  data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
	}
      if(dumb==VFALSE) printf("iteration %d/%d, modif: %d\n",n,nb_iterations,nmodif);
    }
   
  classif_ptr = classif_first;

  /*end of the stuff*/

  for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
    {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	{
	  for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	    {
	      if(*classif_ptr%2==1) *classif_ptr -=1;
	      classif_ptr++;
	    }
	  classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
	}
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
    }


  return(classif);
}

/*---------------------------------------------------------------------------*/
static double PotentielForVoxelBasedAna(int label, int graylevel, VipT1HistoAnalysis *ana)
/*---------------------------------------------------------------------------*/
  {
    double result=0.;
    double x;

  if(ana->gray!=NULL && ana->white!=NULL)
    {
      switch(label)
	{
	case GRAY_LABEL:	  
	  x = (double)(graylevel-ana->gray->mean)/(double)(ana->gray->sigma);
	  result = x*x;
	  break;
	case WHITE_LABEL :
	  x = (double)(graylevel-ana->white->mean)/(double)(ana->white->sigma);
	  result = x*x;
	  break;
	default:
	  VipPrintfError("You can not use this label to get a potential!");
	  fprintf(stderr,"label: %d\n",label);
	  VipPrintfExit("Potentiel (Vip_Potts)");
	  return(PB);
	}
    }
    return(result);
	
  }

/*---------------------------------------------------------------------------*/
Volume *VipGrayWhiteClassificationForVoxelBasedAna(Volume *vol, VipT1HistoAnalysis *ana, float threshold)
/*---------------------------------------------------------------------------*/
{
    Volume *classif;
    Vip_S16BIT *classif_ptr, *classif_first, *cvoisin;
    Vip_S16BIT *data_ptr, *data_first, *gvoisin;
    VipOffsetStruct *data_vos, *classif_vos;
    VipConnectivityStruct *vcs26;
    int T_VOID_GRAY_HIGH=0;
    int T_GRAY_WHITE_LOW=0;
    int T_GRAY_WHITE_HIGH=0;
    int T_WHITE_FAT = 0;
    int ix, iy, iz, icon;
    int temp;
    double mW, mG, sW, sG;
    int nbW, nbG;

    if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
        VipPrintfExit("(VipGrayWhiteClassificationForVoxelBasedAna");
        return(PB);
    }
    if(!ana)
    {
        VipPrintfError("Null VipT1HistoAnalysis!");
        VipPrintfExit("(VipGrayWhiteClassificationForVoxelBasedAna");
        return(PB);
    }

    if(ana->gray!=NULL && ana->white!=NULL)
    {
        T_VOID_GRAY_HIGH = (int)(ana->gray->mean - 4* ana->gray->sigma);
        T_GRAY_WHITE_LOW = (int)(ana->gray->mean + threshold*ana->gray->sigma);
        T_GRAY_WHITE_HIGH = (int)(ana->white->mean - threshold*ana->white->sigma);
        T_WHITE_FAT = (int)(ana->white->mean + 4 * ana->white->sigma);
    }
    else
    {
        VipPrintfError("The histogram scale space analysis did not give gray and white object");
        VipPrintfExit("VipGrayWhiteClassificationForVoxelBasedAna");
        return(PB);
    }
    if (T_VOID_GRAY_HIGH<=0) T_VOID_GRAY_HIGH=1;

    classif = VipDuplicateVolumeStructure( vol, "classif");
    if(classif == PB) return(PB);
    if(mVipVolBorderWidth(classif)<=0) VipSetBorderWidth(classif,1);
    if (VipAllocateVolumeData( classif )==PB) return(PB);

    data_vos = VipGetOffsetStructure(vol);
    data_first = VipGetDataPtr_S16BIT( vol ) + data_vos->oFirstPoint;
    classif_vos = VipGetOffsetStructure(classif);
    classif_first = VipGetDataPtr_S16BIT( classif ) + classif_vos->oFirstPoint;
    data_ptr = data_first;
    classif_ptr = classif_first;

    vcs26 = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );

    /*initialisation*/

    for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
    {
	for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	{
	    for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	    {
		temp = *data_ptr;
		if(temp<T_VOID_GRAY_HIGH)
		{
		    *classif_ptr = VOID_STATIC_LABEL;
		}
		else if (temp<=T_GRAY_WHITE_LOW) *classif_ptr = GRAY_STATIC_LABEL;
		else if (temp<T_GRAY_WHITE_HIGH)
		{
		    if(PotentielForVoxelBasedAna(GRAY_LABEL,temp,ana)<PotentielForVoxelBasedAna(WHITE_LABEL,temp,ana))
			*classif_ptr = GRAY_LABEL;
		    else *classif_ptr = WHITE_LABEL;
		}
		else if (temp<T_WHITE_FAT) *classif_ptr = WHITE_STATIC_LABEL;
		else *classif_ptr = VOID_STATIC_LABEL;
		data_ptr++;
		classif_ptr++;
	    }
	    classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
	    data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
	}
	classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
	data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
    }

    VipSetBorderLevel(classif,BORDER_LABEL);

    return(classif);
}

/*---------------------------------------------------------------------------*/
int VipErosionClassificationNeighbourhood( Volume *classif, Volume *graylevel, VipT1HistoAnalysis *ana, int nb_iteration, int object, int inside, int outside )
/*-------------------------------------------------------------------------*/
{
    VipIntBucket *buck, *nextbuck;
    Topology26Neighborhood *topo26;
    VipConnectivityStruct *vcs6, *vcs26;
    int loop, count, totalcount;
    Vip_S16BIT *cfirst, *cptr, *cvoisin, *gfirst, *gptr, *gvoisin;
    int *buckptr;
    int i;
    int icon;
    int valeur;
    int temp;
    double mW, mG, sW, sG;
    int nbW, nbG, nnG, nnW;
    float KW, KG, deltaU;
    
    if (VipVerifyAll(classif)==PB)
	{
	VipPrintfExit("(skeleton)VipHomotopicErosionFromInside");
	return(PB);
	}
    if (VipTestType(classif,S16BIT)!=OK)
	{
	VipPrintfError("Sorry,  VipHomotopicErosionFromInside is only implemented for S16BIT volume");
	VipPrintfExit("(skeleton)VipHomotopicErosionFromInside");
	return(PB);
	}
    if (mVipVolBorderWidth(classif) < 1)
	{
	VipPrintfError("Sorry, VipHomotopicErosionFromInside is only implemented with border");
	VipPrintfExit("(skeleton)VipHomotopicErosionFromInside");
	return(PB);
	}

    if(object==inside)
	{
	VipPrintfError("object value = inside value!");
	VipPrintfExit("(skeleton)VipHomotopicErosionFromInside");
	return(PB);
	}
    printf("Homotopic erosion from inside...\n");
    
    VipSetBorderLevel( classif, outside );

    buck = VipCreateFrontIntBucketForErosionFromOutside( classif, CONNECTIVITY_6, VIP_FRONT, object, outside);
    if(buck==PB) return(PB);
    nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
    if(nextbuck==PB) return(PB);

    topo26 = VipCreateTopology26Neighborhood( classif );
    if(topo26==PB) return(PB);

    vcs6 = VipGetConnectivityStruct( classif, CONNECTIVITY_6 );
    vcs26 = VipGetConnectivityStruct( classif, CONNECTIVITY_26 );

    nextbuck->n_points = 0;

    cfirst = VipGetDataPtr_S16BIT(classif);
    gfirst = VipGetDataPtr_S16BIT(graylevel);

    /*main loop*/
    loop=0;
    count = 1;
    totalcount = 0;
    KG = 2*1 - 0.00001;
    KW = 4*1 + 0.00001;
    printf("loop: %3d, Added %6d",loop,0);

    while((loop++<nb_iteration)&&(count)&&(buck->n_points>0))
    {
	if(loop==1) count=0;
	totalcount += count;
	count = 0;
	printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bloop: %3d, Deleted: %6d",loop,totalcount);
	fflush(stdout);
	
	buckptr = buck->data;
	for(i=buck->n_points;i--;)
	{
	    cptr = cfirst + *buckptr;
	    gptr = gfirst + *buckptr++;
	    if(*cptr==VIP_FRONT)
	    {
		temp = *gptr;
		mW=0;
		mG=0;
		sW=0;
		sG=0;
		nbW=0;
		nbG=0;
		nnG = 0;
		nnW = 0;
		deltaU = 0;
		for (icon=0; icon<vcs6->nb_neighbors; icon++)
		{
		    cvoisin = cptr + vcs6->offset[icon];
		    if(*cvoisin==inside || *cvoisin==object) nnW++;
		    else nnG++;
		}
		for (icon=0; icon<vcs26->nb_neighbors; icon++)
		{
		    gvoisin = gptr + vcs26->offset[icon];
		    cvoisin = cptr + vcs26->offset[icon];
		    if(*cvoisin==WHITE_STATIC_LABEL || *cvoisin==WHITE_LABEL)
		    {
			mW += (float)(*gvoisin);
			nbW++;
		    }
		    else if(*cvoisin==GRAY_STATIC_LABEL || *cvoisin==GRAY_LABEL)
		    {
			mG += (float)(*gvoisin);
			nbG++;
		    }
		}
		if(nbW>1) mW /= (float)nbW;
		else mW = (mW + ana->white->mean)/(1+nbW);
		if(nbG>1) mG /= (float)nbG;
		else mG = (mG + ana->gray->mean)/(1+nbG);
		for (icon=0; icon<vcs26->nb_neighbors; icon++)
		{
		    gvoisin = gptr + vcs26->offset[icon];
		    cvoisin = cptr + vcs26->offset[icon];
		    if(*cvoisin==WHITE_STATIC_LABEL || *cvoisin==WHITE_LABEL)
		    {
			sW += ((float)(*gvoisin) - mW)*((float)(*gvoisin) - mW);
		    }
		    else if(*cvoisin==GRAY_STATIC_LABEL || *cvoisin==GRAY_LABEL)
		    {
			sG += ((float)(*gvoisin) - mG)*((float)(*gvoisin) - mG);
		    }
		}
		if(nbW>1) sW = sqrt(sW/nbW);
		else sW = ana->white->sigma;
		if(nbG>1) sG = sqrt(sG/nbG);
		else sW = ana->gray->sigma;
// 		deltaU = -1*(nnG-nnW);
// 		deltaU += deltaGPotentialWtoG(*gptr,KG,mG,sG,KW,mW,sW);
// 		if(deltaU<0)
// 		{
// 		    *cptr = GRAY_STATIC_LABEL;
// 		}
		if(PotentielForVoxelBasedAna2011(GRAY_LABEL,temp,mW,mG,sW,sG)<PotentielForVoxelBasedAna2011(WHITE_LABEL,temp,mW,mG,sW,sG))// || nbW==0)
		{
		    *cptr = GRAY_LABEL;
		    count++;
		}
		else *cptr = WHITE_LABEL;
	    }
	}
	VipFillNextFrontFromOldFrontForErosion(cfirst,buck,nextbuck,vcs6,object,VIP_FRONT,outside,inside);
	
	/*bucket permutation*/
	VipPermuteTwoIntBucket(&buck, &nextbuck);
	nextbuck->n_points = 0;
    }
    printf("\n");
//     VipChangeIntLabel(classif,VIP_FRONT,object);
//     VipChangeIntLabel(classif,VIP_IMMORTAL,object);

    VipFreeIntBucket(buck);
    VipFreeIntBucket(nextbuck);

    return(OK);
}

/*---------------------------------------------------------------------------*/
int VipGrayWhiteClassificationForVoxelBasedNeighbourhood(Volume *vol, Volume *classif, Volume *matter,
int dumb, int nb_iterations, int threshold, int connectivity, int label)
/*---------------------------------------------------------------------------*/
{
    Volume *topo=NULL;
    VipIntBucket *buck, *nextbuck, *buck_immortals;
    Vip_S16BIT *classif_ptr, *classif_first, *classif_voisin;
    Vip_S16BIT *mean_data_ptr, *mean_data_first;
    Vip_S16BIT *data_ptr, *data_first, *voisin;
    Vip_S16BIT *matter_ptr, *matter_first, *matter_voisin;
    Vip_S16BIT *topo_ptr, *topo_first;
    VipOffsetStruct *data_vos, *classif_vos, *matter_vos;
    VipConnectivityStruct *vcs6, *vcs18, *vcs26;
    Volume *ventricules=NULL;
    int *buckptr;
    int icon; 
    int i, n;
    int temp;
    int nmodif;
    int currentlabel, neighbor_label, currentlabelsur2, newlabel=0, newlabelsur2;
    int mask_val, partial_val, next_val, next_val2, mask_val2;
    double deltaU;
    int front = VFALSE;
    int ix, iy, iz;
    int loop, count;
    int delta_white, delta_gray, m;
    int nb6, wnb;
    double wmean, wsigma;

    if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB || VipVerifyAll(classif)==PB || VipTestType(classif,S16BIT)==PB)
    {
        VipPrintfExit("(VipGrayWhiteClassificationForVoxelBasedAna");
        return(PB);
    }
    
    VipSetBorderLevel( matter, 1 ); /* already done before but security */
    
    vcs26 = VipGetConnectivityStruct( matter, CONNECTIVITY_26 );
    vcs18 = VipGetConnectivityStruct( matter, CONNECTIVITY_18 );
    vcs6 = VipGetConnectivityStruct( matter, connectivity );
    
    buck = VipCreateFrontIntBucketGWClassifObject(matter, classif, connectivity, VIP_FRONT, 0, 255);
    if(buck==PB) return(PB);
    nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
    if(nextbuck==PB) return(PB);
    
    nextbuck->n_points = 0;
    
    data_first = VipGetDataPtr_S16BIT( vol );
    matter_first = VipGetDataPtr_S16BIT( matter );
    classif_first = VipGetDataPtr_S16BIT( classif );
    
    loop=0;
    count = 1;
    printf("loop:    ,count:      ");
    while((loop++<nb_iterations)&&(count))
    {
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        printf("loop: %3d,count: %5d",loop, count);
        fflush(stdout);
        count = 0;
        m = 0;
        
        topo = VipCreateSingleThresholdedVolume(matter, EQUAL_TO, 255, BINARY_RESULT);
        VipTopologicalClassificationForLabel(topo, 0);
        topo_first = VipGetDataPtr_S16BIT( topo );
        
        buckptr = buck->data;
        for(i=buck->n_points;i--;)
        {
            data_ptr = data_first + *buckptr;
            classif_ptr = classif_first + *buckptr;
            matter_ptr = matter_first + *buckptr;
            topo_ptr = topo_first + *buckptr;
            
            switch(label)
            {
                case GRAY_LABEL:
                    if(10<*topo_ptr && *topo_ptr<255 && *classif_ptr!=WHITE_STATIC_LABEL)
                    {
                        if(PotentielForVoxelBasedNeighbourhood(vol, classif, data_ptr, classif_ptr, CONNECTIVITY_6, GRAY_STATIC_LABEL)<PotentielForVoxelBasedNeighbourhood(vol, classif, data_ptr, classif_ptr, CONNECTIVITY_26, WHITE_LABEL))
                        {
                            *matter_ptr = 255;
                            *classif_ptr = GRAY_STATIC_LABEL;
                            count++;
                        }
                    }
                    break;
                case WHITE_LABEL:
                    if(*classif_ptr==WHITE_LABEL || *classif_ptr==255)
                    {
                        if(PotentielForVoxelBasedNeighbourhood(vol, classif, data_ptr, classif_ptr, CONNECTIVITY_26, GRAY_LABEL)>PotentielForVoxelBasedNeighbourhood(vol, classif, data_ptr, classif_ptr, CONNECTIVITY_26, WHITE_STATIC_LABEL))
                        {
                            *matter_ptr = 255;
                            *classif_ptr = WHITE_STATIC_LABEL;
                            count++;
                        }
                        else
                        {
                            *matter_ptr = VIP_IMMORTAL;
//                             *classif_ptr = VIP_IMMORTAL;
                        }
                    }
                    break;
                case 1:
                    if(10<*topo_ptr && *topo_ptr<255 && *classif_ptr!=WHITE_STATIC_LABEL && *classif_ptr!=GRAY_STATIC_LABEL && *data_ptr>threshold)
                    {
                        nb6 = 0;
                        for(icon=0;icon<vcs6->nb_neighbors;icon++)
                        {
                            classif_voisin = classif_ptr + vcs6->offset[icon];
                            if (*classif_voisin==WHITE_STATIC_LABEL)
                            {
                                nb6++;
                            }
                        }
                        if(PotentielForVoxelBasedNeighbourhood(vol, classif, data_ptr, classif_ptr, CONNECTIVITY_18, GRAY_STATIC_LABEL)>PotentielForVoxelBasedNeighbourhood(vol, classif, data_ptr, classif_ptr, CONNECTIVITY_18, WHITE_STATIC_LABEL))
                        {
                            if(nb6>0)
                            {
                                *matter_ptr = 255;
                                *classif_ptr = WHITE_STATIC_LABEL;
                                count++;
                            }
                        }
//                         else
//                         {
//                             *classif_ptr = GRAY_STATIC_LABEL;
//                         }
                    }
                    break;
                case 2:
                    if(*matter_ptr==VIP_FRONT && *classif_ptr>WHITE_LABEL-1)
                    {
//                         nb6=0;
//                         for(icon=0;icon<vcs18->nb_neighbors;icon++)
//                         {
//                             voisin = matter_ptr + vcs18->offset[icon];
//                             if (*voisin==255)
//                             {
//                                 nb6++;
//                             }
//                         }
                        if(PotentielForVoxelBasedNeighbourhood(vol, classif, data_ptr, classif_ptr, CONNECTIVITY_26, 2)>PotentielForVoxelBasedNeighbourhood(vol, matter, data_ptr, matter_ptr, CONNECTIVITY_26, WHITE_STATIC_LABEL))
                        {
                            *matter_ptr = 255;
//                             *classif_ptr = WHITE_STATIC_LABEL;
                            count++;
                        }
                    }
                    break;
                case 3:
                    if(*matter_ptr==VIP_FRONT && *classif_ptr>WHITE_LABEL-1)
                    {
                        if(PotentielForVoxelBasedNeighbourhood(vol, classif, data_ptr, classif_ptr, CONNECTIVITY_26, 1)>PotentielForVoxelBasedNeighbourhood(vol, matter, data_ptr, matter_ptr, CONNECTIVITY_26, WHITE_STATIC_LABEL))
                        {
                            *matter_ptr = 255;
                            count++;
                        }
                        else *matter_ptr = VIP_IMMORTAL;
                    }
                    break;
                case 4:
                    if(*matter_ptr==VIP_FRONT && (*classif_ptr>WHITE_LABEL-1 || *classif_ptr==100))
                    {
                        wmean = 0;
                        wnb = 0;
                        for(icon=0;icon<vcs6->nb_neighbors;icon++)
                        {
                            voisin = data_ptr + vcs6->offset[icon];
                            matter_voisin = matter_ptr + vcs6->offset[icon];
                            if (*matter_voisin==255)
                            {
                                wmean += (double)(*voisin);
                                wnb++;
                            }
                        }
                        wmean /= (double)wnb;
                        for(icon=0;icon<vcs6->nb_neighbors;icon++)
                        {
                            voisin = data_ptr + vcs6->offset[icon];
                            matter_voisin = matter_ptr + vcs6->offset[icon];
                            if (*matter_voisin==255)
                            {
                                wsigma += ((double)(*voisin)-wmean)*((double)(*voisin)-wmean);
                            }
                        }
                        wsigma = sqrt(wsigma/wnb);
                        if(PotentielForVoxelBasedNeighbourhood(vol, matter, data_ptr, matter_ptr, CONNECTIVITY_26, GRAY_STATIC_LABEL)>PotentielForVoxelBasedNeighbourhood(vol, matter, data_ptr, matter_ptr, CONNECTIVITY_26, WHITE_STATIC_LABEL))
//                        if(*data_ptr>wmean-wsigma)
                        {
                            *matter_ptr = 255;
                            count++;
                        }
//                        else *matter_ptr = VIP_IMMORTAL;
                    }
                    break;
                case 5:
//                    if(*matter_ptr==VIP_FRONT && *classif_ptr>WHITE_LABEL-1)
                    if(*classif_ptr==255)
                    {
                        wmean = 0;
                        wnb = 0;
                        for(icon=0;icon<vcs6->nb_neighbors;icon++)
                        {
                            voisin = data_ptr + vcs6->offset[icon];
                            matter_voisin = matter_ptr + vcs6->offset[icon];
                            if (*matter_voisin==255)
                            {
                                wmean += (double)(*voisin);
                                wnb++;
                            }
                        }
                        wmean /= (double)wnb;
//                        if(*data_ptr > wmean - threshold)
                        if(PotentielForVoxelBasedNeighbourhood(vol, classif, data_ptr, classif_ptr, CONNECTIVITY_18, 3)>PotentielForVoxelBasedNeighbourhood(vol, matter, data_ptr, matter_ptr, CONNECTIVITY_18, WHITE_STATIC_LABEL))
                        {
                            *matter_ptr = 255;
                            count++;
                        }
//                        else *matter_ptr = VIP_IMMORTAL;
                    }
                    break;
                default:
                    VipPrintfError("You can not use this label");
                    fprintf(stderr,"label: %d\n",label);
                    VipPrintfExit("Neighbour (Vip_Potts)");
                    return(PB);
            }
            buckptr++;
        }
        VipFillNextFrontFromOldFrontGWClassifObject( matter_first, classif_first, buck, nextbuck, vcs6, VIP_FRONT, 0, 255 );

        VipFreeVolume(topo);
        
        /*bucket permutation*/
        VipPermuteTwoIntBucket(&buck, &nextbuck);
        nextbuck->n_points = 0;
    }
    VipFreeIntBucket(buck);
    VipFreeIntBucket(nextbuck);
    
    printf("\n");
    
  return(OK);
}
/*---------------------------------------------------------------------------*/
int VipCleaningTopo(Volume *vol, Volume *matter, Volume *classif, int dumb, int nb_iterations, int label, int threshold)
/*---------------------------------------------------------------------------*/
{
    Volume *topo=NULL;
    VipIntBucket *buck, *nextbuck;
    Vip_S16BIT *topo_ptr, *topo_first;
    Vip_S16BIT *data_ptr, *data_first, *voisin;
    Vip_S16BIT *matter_ptr, *matter_first;
    Vip_S16BIT *classif_ptr, *classif_first;
    VipConnectivityStruct *vcs6, *vcs26;
    int *buckptr;
    int i;
    int loop, count;

    if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB || VipVerifyAll(matter)==PB || VipTestType(matter,S16BIT)==PB)
    {
        VipPrintfExit("(VipCleaningTopo");
        return(PB);
    }
    
    VipSetBorderLevel( matter, 255 ); /* already done before but security */
    
    vcs26 = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );
    vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );

    buck = VipCreateFrontIntBucketForDilation(matter, CONNECTIVITY_6, VIP_FRONT, 255, 0, FRONT_RANDOM_ORDER);
    if(buck==PB) return(PB);
    nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
    if(nextbuck==PB) return(PB);
    nextbuck->n_points = 0;

    data_first = VipGetDataPtr_S16BIT( vol );
    matter_first = VipGetDataPtr_S16BIT( matter );
    classif_first = VipGetDataPtr_S16BIT( classif );
    
    loop=0;
    count = 1;
    printf("loop:    ,count:      ");
    while((loop++<nb_iterations)&&(count))
    {
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        printf("loop: %3d,count: %5d",loop, count);
        fflush(stdout);
        count = 0;
        
        topo = VipCreateSingleThresholdedVolume(matter, EQUAL_TO, 255, BINARY_RESULT);
        VipTopologicalClassificationForLabel(topo, 0);
        topo_first = VipGetDataPtr_S16BIT( topo );
        
        buckptr = buck->data;
        for(i=buck->n_points;i--;)
        {
            data_ptr = data_first + *buckptr;
            matter_ptr = matter_first + *buckptr;
            topo_ptr = topo_first + *buckptr;
            classif_ptr = classif_first + *buckptr;
	    
            if(*topo_ptr==20) *matter_ptr = 255;
            if(30<*topo_ptr && *topo_ptr<255)
            {
                switch(label)
                {
                    case WHITE_LABEL:
                        if((PotentielForVoxelBasedNeighbourhood(vol, classif, data_ptr, classif_ptr, CONNECTIVITY_26, GRAY_LABEL)>PotentielForVoxelBasedNeighbourhood(vol, classif, data_ptr, classif_ptr, CONNECTIVITY_26, WHITE_STATIC_LABEL)) || *data_ptr>threshold)
                        {
                            *matter_ptr = 255;
                            count++;
                        }
                        break;
                    case GRAY_LABEL:
                        if((PotentielForVoxelBasedNeighbourhood(vol, classif, data_ptr, classif_ptr, CONNECTIVITY_26, GRAY_STATIC_LABEL)<PotentielForVoxelBasedNeighbourhood(vol, classif, data_ptr, classif_ptr, CONNECTIVITY_26, WHITE_LABEL)) || *data_ptr<threshold)
                        {
                            *matter_ptr = 255;
                            count++;
                        }
                        break;
                    case 1:
                        *matter_ptr = 255;
                        count++;
                        break;
                    default:
                        VipPrintfError("You can not use this label!");
                        fprintf(stderr,"label: %d\n",label);
                        VipPrintfExit("VipCleaningTopo (Vip_Potts)");
                        return(PB);
                }
            }
            buckptr++;
        }
        VipFillNextFrontFromOldFrontForDilation( matter_first, buck, nextbuck, vcs6, 0, VIP_FRONT, 255 );
        VipFreeVolume(topo);
        
        /*bucket permutation*/
        VipPermuteTwoIntBucket(&buck, &nextbuck);
        nextbuck->n_points = 0;
    }
    VipFreeIntBucket(buck);
    VipFreeIntBucket(nextbuck);
    
    VipChangeIntLabel(matter,VIP_FRONT,0);
    
    printf("\n");
    
    return(OK);
}
/*---------------------------------------------------------------------------*/
int VipCleaningConnectivity(Volume *vol, int connectivity, int type)
/*---------------------------------------------------------------------------*/
{
    VipIntBucket *buck, *nextbuck;
    Vip_S16BIT *ptr, *first, *voisin;
    VipConnectivityStruct *vcs, *vcs6;
    int *buckptr;
    int i, icon;
    int loop, count;
    int nb, nb6;
    
    if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
        VipPrintfExit("(VipCleaningConnectivity");
        return(PB);
    }
    
    VipSetBorderLevel( vol, 0 ); /* already done before but security */
    
    vcs = VipGetConnectivityStruct( vol, connectivity );
    vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );

    buck = VipCreateFrontIntBucket(vol, connectivity, VIP_FRONT);
    if(buck==PB) return(PB);
    nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
    if(nextbuck==PB) return(PB);
    nextbuck->n_points = 0;
    
    first = VipGetDataPtr_S16BIT( vol );
    
    loop=0;
    count = 1;
    nb = vcs->nb_neighbors;
    printf("loop:    ,count:      ");
    while((loop++<500)&&(count))
    {
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        printf("loop: %3d,count: %5d",loop, count);
        fflush(stdout);
        count = 0;
        
        buckptr = buck->data;
        for(i=buck->n_points;i--;)
        {
            ptr = first + *buckptr;
            
            nb=0;
            for(icon=0;icon<vcs->nb_neighbors;icon++)
            {
                voisin = ptr + vcs->offset[icon];
                if(*voisin==255 || *voisin==VIP_FRONT) nb++;
            }
//             nb6=0;
//             for(icon=0;icon<vcs6->nb_neighbors;icon++)
//             {
//                 voisin = ptr + vcs6->offset[icon];
//                 if(*voisin==255 || *voisin==VIP_FRONT) nb6++;
//             }
            if(vcs->nb_neighbors>6 && nb<5 && type==1)
            {
                *ptr=0;
                count++;
            }
            else if(vcs->nb_neighbors==6 && nb<2 && type==1)
            {
                *ptr=0;
                count++;
            }
            else if(vcs->nb_neighbors==6 && nb<3 && type==2)
            {
                *ptr=0;
                count++;
            }
            buckptr++;
        }
        VipFillNextFrontFromOldFront(first, buck, nextbuck, vcs, 255, VIP_FRONT);
        
        /*bucket permutation*/
        VipPermuteTwoIntBucket(&buck, &nextbuck);
        nextbuck->n_points = 0;
    }
    VipFreeIntBucket(buck);
    VipFreeIntBucket(nextbuck);
    
    VipChangeIntLabel(vol,VIP_FRONT,255);
    
    printf("\n");
    
    return(OK);
}
/*---------------------------------------------------------------------------*/
Volume *VipGrayWhiteRegularisationForVoxelBasedAna(Volume *vol, Volume *classif, VipT1HistoAnalysis *ana, int dumb, int nb_iterations, float KPOTTS, int connectivity)
/*---------------------------------------------------------------------------*/
{
    Vip_S16BIT *classif_ptr, *classif_first;
    Vip_S16BIT *data_ptr, *data_first;
    VipOffsetStruct *data_vos, *classif_vos;
    VipConnectivityStruct *vcs;
    int icon; 
    int ix, iy, iz, n;
    int temp;
    int nmodif;
    int currentlabel, neighbor_label, currentlabelsur2, newlabel=0, newlabelsur2;
    double deltaU;

    if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
        VipPrintfExit("(VipGrayWhiteRegularisationForVoxelBasedAna");
        return(PB);
    }

    data_vos = VipGetOffsetStructure(vol);
    data_first = VipGetDataPtr_S16BIT( vol ) + data_vos->oFirstPoint;
    classif_vos = VipGetOffsetStructure(classif);
    classif_first = VipGetDataPtr_S16BIT( classif ) + classif_vos->oFirstPoint;

    VipSetBorderLevel(classif,BORDER_LABEL);
    vcs = VipGetConnectivityStruct( classif, connectivity );
    KPOTTS /= vcs->nb_neighbors; /* Convention to simplify user interface feeling */

    /*Minimisation*/
    nmodif = -1;
    for(n=1;(n<=nb_iterations)&&(nmodif!=0);n++)
    {
        nmodif = 0;
        data_ptr = data_first;
        classif_ptr = classif_first;
        for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
        {
            for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
            {
                for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
                {
                    currentlabel = *classif_ptr;
                    if(currentlabel%2==0)
                    {
                        currentlabelsur2 = currentlabel/2;
                        temp = *data_ptr;
                        if(currentlabel==GRAY_LABEL) newlabel = WHITE_LABEL;
                        else if (currentlabel==WHITE_LABEL) newlabel = GRAY_LABEL;
                        else newlabel = currentlabel;
                        newlabelsur2 = newlabel/2;

                        if(newlabel!=currentlabel)
                        {
                            deltaU = 0.;
                            for(icon=0 ; icon<vcs->nb_neighbors ; icon++)
                            {
                                neighbor_label = *(classif_ptr + vcs->offset[icon]);
                                if(currentlabelsur2== (neighbor_label/2)) deltaU += KPOTTS;
                                else if(newlabelsur2==(neighbor_label/2)) deltaU -= KPOTTS;
                            }
                            /* printf("delta Potts: %f\n",deltaU);*/
                            deltaU += PotentielForVoxelBasedAna(newlabel,temp,ana) - PotentielForVoxelBasedAna(currentlabel,temp,ana);
			  /*printf(" Pot %f -> %f\n",Potentiel(currentlabel,temp,ana),Potentiel(newlabel,temp,ana));
                            scanf("%s",&c);*/
                            if(deltaU<0)
                            {
                                nmodif++;
                                *classif_ptr = newlabel;
                            }
                        }
                    }
                    data_ptr++;
                    classif_ptr++;
                }
                classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
                data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
            }
            classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
            data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
        }
        if(dumb==VFALSE) printf("iteration %d/%d, modif: %d\n",n,nb_iterations,nmodif);
    }
    
//     classif_ptr = classif_first;
//     /*end of the stuff*/
// 
//     for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
//     {
//         for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
//         {
//             for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
//             {
//                 if(*classif_ptr%2==1) *classif_ptr -=1;
//                 classif_ptr++;
//             }
//             classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
//         }
//         classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
//     }
    
    return(classif);
}

/*---------------------------------------------------------------------------*/
static double PotentielForVoxelBasedAna2011(int label, int graylevel, double mW, double mG, double sW, double sG)
/*---------------------------------------------------------------------------*/
{
  double result=0.;
  double x;

  if(mW!=0 && mG!=0)
  {
      switch(label)
      {
          case GRAY_LABEL:
              x = (double)(graylevel-mG)/(double)(sG);
              result = x*x;
              break;
          case WHITE_LABEL:
              x = (double)(graylevel-mW)/(double)(sW);
              result = x*x;
              break;
          default:
              VipPrintfError("You can not use this label to get a potential!");
              fprintf(stderr,"label: %d\n",label);
              VipPrintfExit("Potentiel (Vip_Potts)");
              return(PB);
      }
  }
  return(result);	
}

/*---------------------------------------------------------------------------*/
static double PotentielForVoxelBasedNeighbourhood(Volume *vol, Volume *classif, Vip_S16BIT *vol_ptr, Vip_S16BIT *classif_ptr, int connectivity, int label)
/*---------------------------------------------------------------------------*/
{
    int icon;
    VipConnectivityStruct *vcs, *vcs6;
    Vip_S16BIT *classif_voisin, *vol_voisin, *white_voisin;
    double result;
    double mean;
    int nb, nb6;
    
    vcs = VipGetConnectivityStruct( vol, connectivity );
    vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );
    mean = 0.;
    nb = 0;
    
    switch(label)
    {
        case WHITE_STATIC_LABEL:
            for(icon=0;icon<vcs->nb_neighbors;icon++)
            {
                classif_voisin = classif_ptr + vcs->offset[icon];
                vol_voisin = vol_ptr + vcs->offset[icon];
                if(*classif_voisin==WHITE_STATIC_LABEL || *classif_voisin==255)//*classif_voisin==WHITE_LABEL
                {
                    mean += (double)(*vol_voisin);
                    nb++;
                }
            }
            nb6 = 0;
            for(icon=0;icon<vcs6->nb_neighbors;icon++)
            {
                classif_voisin = classif_ptr + vcs6->offset[icon];
                if (*classif_voisin==WHITE_STATIC_LABEL)
                {
                    nb6++;
                }
            }
            mean /= (double)nb;
//             if(mean < (double)(*vol_ptr) && nb6 > 0) result = mean - (double)(*vol_ptr);
// //             else if(nb6 < 2) result = ;
            result = mean - (double)(*vol_ptr);
            break;
        case GRAY_LABEL:
            for(icon=0;icon<vcs->nb_neighbors;icon++)
            {
                classif_voisin = classif_ptr + vcs->offset[icon];
                vol_voisin = vol_ptr + vcs->offset[icon];
                if (*classif_voisin==WHITE_LABEL || *classif_voisin==GRAY_LABEL)//*classif_voisin!=11 && *classif_voisin!=WHITE_STATIC_LABEL && *classif_voisin!=GRAY_STATIC_LABEL
                {
                    mean += (double)(*vol_voisin);
                    nb++;
                }
            }
//             for(icon=0;icon<vcs6->nb_neighbors;icon++)
//             {
//                 classif_voisin = classif_ptr + vcs->offset[icon];
//                 if (*classif_voisin==WHITE_STATIC_LABEL)
//                 {
//                     nb6++;
//                 }
//             }
            if(nb!=0)// && nb6!=0)
            {
                mean /= (double)nb;
                if(mean < (double)(*vol_ptr)) result = (double)(*vol_ptr) - mean;
                else result = 0.;
            }
            else result = 0.;
            break;
        case WHITE_LABEL:
            for(icon=0;icon<vcs->nb_neighbors;icon++)
            {
                classif_voisin = classif_ptr + vcs->offset[icon];
                vol_voisin = vol_ptr + vcs->offset[icon];
                if(*classif_voisin==GRAY_LABEL || *classif_voisin==WHITE_LABEL)
                {
                    mean += (double)(*vol_voisin);
                    nb++;
                }
            }
//             nb6 = 0;
//             for(icon=0;icon<vcs6->nb_neighbors;icon++)
//             {
//                 classif_voisin = classif_ptr + vcs6->offset[icon];
//                 if (*classif_voisin==WHITE_STATIC_LABEL)
//                 {
//                     nb6++;
//                 }
//             }
            if(nb!=0)
            {
                mean /= (double)nb;
                if(mean > (double)(*vol_ptr)) result = mean - (double)(*vol_ptr);
                else result = (double)(*vol_ptr) - mean;
            }
            else result = 2*(double)(*vol_ptr);
            break;
        case GRAY_STATIC_LABEL:
            for(icon=0;icon<vcs->nb_neighbors;icon++)
            {
                classif_voisin = classif_ptr + vcs->offset[icon];
                vol_voisin = vol_ptr + vcs->offset[icon];
                if (*classif_voisin==GRAY_STATIC_LABEL || *classif_voisin==0 || *classif_voisin==VIP_IMMORTAL)
                {
                    mean += (double)(*vol_voisin);
                    nb++;
                }
            }
//             for(icon=0;icon<vcs6->nb_neighbors;icon++)
//             {
//                 classif_voisin = classif_ptr + vcs->offset[icon];
//                 if (*classif_voisin==WHITE_STATIC_LABEL)
//                 {
//                     nb6++;
//                 }
//             }
            if(nb!=0)
            {
                mean /= (double)nb;
                if(mean < (double)(*vol_ptr)) result = (double)(*vol_ptr) - mean;
                else result = 0.;
            }
//            else result = 2*(double)(*vol_ptr);
            else result = 0.;
            break;
        case 1:
            for(icon=0;icon<vcs->nb_neighbors;icon++)
            {
                classif_voisin = classif_ptr + vcs->offset[icon];
                vol_voisin = vol_ptr + vcs->offset[icon];
                if(*classif_voisin==GRAY_LABEL || *classif_voisin==VIP_IMMORTAL)
                {
                    mean += (double)(*vol_voisin);
                    nb++;
                }
            }
            if(nb!=0)
            {
                mean /= (double)nb;
                if(mean < (double)(*vol_ptr)) result = (double)(*vol_ptr) - mean;
                else result = 0.;
            }
            else result = 0.;
            break;
        case 2:
            for(icon=0;icon<vcs->nb_neighbors;icon++)
            {
                classif_voisin = classif_ptr + vcs->offset[icon];
                vol_voisin = vol_ptr + vcs->offset[icon];
                if(*classif_voisin==GRAY_LABEL || *classif_voisin==GRAY_STATIC_LABEL)
                {
                    mean += (double)(*vol_voisin);
                    nb++;
                }
            }
            if(nb!=0)
            {
                mean /= (double)nb;
                if(mean < (double)(*vol_ptr)) result = (double)(*vol_ptr) - mean;
                else result = 0.;
            }
            else result = 2*(double)(*vol_ptr);
            break;
        case 3:
            for(icon=0;icon<vcs->nb_neighbors;icon++)
            {
                classif_voisin = classif_ptr + vcs->offset[icon];
                vol_voisin = vol_ptr + vcs->offset[icon];
                if(*classif_voisin==0)
                {
                    mean += (double)(*vol_voisin);
                    nb++;
                }
            }
            if(nb!=0)
            {
                mean /= (double)nb;
                if(mean < (double)(*vol_ptr)) result = (double)(*vol_ptr) - mean;
                else result = 0.;
            }
            else result = 0.;
            break;
        default:
            VipPrintfError("You can not use this label to get a potential!");
            fprintf(stderr,"label: %d\n",label);
            VipPrintfExit("Potentiel (Vip_Potts)");
            return(PB);
    }

    return(result);
}

/*----------------------------------------------------------------------------*/
static VipIntBucket *VipCreateFrontIntBucketGWClassifObject( Volume *vol, Volume *classif, int connectivity, int front_value, int domain, int outside)
{
    Vip_S16BIT *ptr, *ptr_classif, *voisin;
    int i, NbTotalPts;
    VipIntBucket *buck;
    VipConnectivityStruct *vcs;
    int icon;
    int front;

    if (VipVerifyAll(vol)==PB)
    {
        VipPrintfExit("(skeleton)VipCreateFrontIntBucketGWClassifObject");
        return(PB);
    }
    if (VipTestType(vol,S16BIT)!=OK)
    {
        VipPrintfError("Sorry,  VipCreateFrontIntBucketGWClassifObject is only implemented for S16BIT volume");
        VipPrintfExit("(skeleton)VipCreateFrontIntBucketGWClassifObject");
        return(PB);
    }
    if (mVipVolBorderWidth(vol) < 1)
    {
        VipPrintfError("Sorry,  VipCreateFrontIntBucketGWClassifObject is only implemented with border");
        VipPrintfExit("(skeleton)VipCreateFrontIntBucketGWClassifObject");
        return(PB);
    }

    vcs = VipGetConnectivityStruct( vol, connectivity );

    ptr = VipGetDataPtr_S16BIT(vol);
    ptr_classif = VipGetDataPtr_S16BIT(classif);

    NbTotalPts = VipOffsetVolume(vol);

    buck = VipAllocIntBucket(VIP_INITIAL_FRONT_SIZE);
    
    for ( i=0; i<NbTotalPts; i++ )
    {
        if (*ptr==domain)
        {
            front = VTRUE;
//             for (icon=0;icon<vcs->nb_neighbors;icon++)
//             {
//                 voisin = ptr + vcs->offset[icon];
//                 if (*voisin==VIP_IMMORTAL)
//                 {
//                     front = VFALSE;
//                 }
//             }
//             if(*ptr>=WHITE_LABEL) front = VTRUE;
            
            for (icon=0;icon<vcs->nb_neighbors;icon++)
            {
                voisin = ptr + vcs->offset[icon];
                if((*voisin==outside && front==VTRUE))
                {
                    if(buck->n_points==buck->size)
                    {
                        if(VipIncreaseIntBucket(buck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
                    }
                    buck->data[buck->n_points++] = i;
                    *ptr = front_value;
                    break;
                }
            }
        }
        ptr++;
        ptr_classif++;
    }
    
    if(VipRandomizeFrontOrder(buck,10)==PB) return(PB);
    
    VipFreeConnectivityStruct(vcs);
    
    return(buck);
}

/*-------------------------------------------------------------------------*/
static int VipFillNextFrontFromOldFrontGWClassifObject(
Vip_S16BIT *first_vol_point,
Vip_S16BIT *first_classif_point,
VipIntBucket *buck,
VipIntBucket *nextbuck,
VipConnectivityStruct *vcs,
int front_value,
int domain,
int outside)
/*-------------------------------------------------------------------------*/
{
    int *buckptr, *dirptr;
    Vip_S16BIT *ptr, *ptr_classif;
    Vip_S16BIT *ptr_neighbor, *ptr_neighbor2, *ptr_neighbor_classif;
    int i, dir, dir2;
    int front;

    if(first_vol_point==NULL)
    {
        VipPrintfError("NULL pointer in VipFillNextFrontFromOldFrontGWClassifObject");
        VipPrintfExit("VipFillNextFrontFromOldFrontGWClassifObject");
        return(PB);
    }
    if((buck==NULL) || (nextbuck==NULL))
    {
        VipPrintfError("One NULL bucket in VipFillNextFrontFromOldFrontGWClassifObject");
        VipPrintfExit("VipFillNextFrontFromOldFrontGWClassifObject");
        return(PB);
    }
    if(vcs==NULL)
    {
        VipPrintfError("NULL VipConnectivityStruct pointer in VipFillNextFrontFromOldFrontGWClassifObject");
        VipPrintfExit("VipFillNextFrontFromOldFrontGWClassifObject");
        return(PB);
    }

    buckptr = buck->data;
    for(i=buck->n_points;i--;)
    {
        ptr = first_vol_point + *buckptr;
        ptr_classif = first_classif_point + *buckptr;
        if(*ptr==front_value || *ptr==VIP_IMMORTAL) /*the point has not be put to outside*/
        {
            if(nextbuck->n_points==nextbuck->size)
            {
                if(VipIncreaseIntBucket(nextbuck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
            }
            nextbuck->data[nextbuck->n_points++]=*buckptr;
        }
        if( *ptr==outside ) /*the point has been put to outside*/
        {
            for(dir=0; dir<vcs->nb_neighbors; dir++)
            {
                ptr_neighbor = ptr + vcs->offset[dir];
                ptr_neighbor_classif = ptr_classif + vcs->offset[dir];
                front = VTRUE;
//                 for(dir2=0; dir2<vcs->nb_neighbors; dir2++)
//                 {
//                     ptr_neighbor2 = ptr_neighbor + vcs->offset[dir2];
//                     if (*ptr_neighbor2==VIP_IMMORTAL)
//                     {
//                         front = VFALSE;
//                     }
//                 }
//                 if(*ptr_neighbor_classif>=WHITE_LABEL) front = VTRUE;

                if((*ptr_neighbor==domain && front==VTRUE)) /*NOT ALREADY IN FRONT*/
                {
                    *ptr_neighbor = front_value;
                    if(nextbuck->n_points==nextbuck->size)
                    {
                        if(VipIncreaseIntBucket(nextbuck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
                    }
                    nextbuck->data[nextbuck->n_points++]=*buckptr + vcs->offset[dir];
                }
            }
        }
        buckptr++;
    }
    return(OK);
}

/*-------------------------------------------------------------------------*/
int VipGrayRegularisation(Volume *vol)
/*-------------------------------------------------------------------------*/
{
    Vip_S16BIT *ptr, *first, *voisin;
    VipOffsetStruct *vos;
    VipConnectivityStruct *vcs6, *vcs18, *vcs26;
    int icon;
    int ix, iy, iz, n;
    int nb_vcs6, nb_vcs18, nb_vcs26;
    
    if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
        VipPrintfExit("(VipGrayRegularisation");
        return(PB);
    }
    
    vos = VipGetOffsetStructure(vol);
    first = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
    ptr = first;
    
    vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );
    vcs18 = VipGetConnectivityStruct( vol, CONNECTIVITY_18 );
    vcs26 = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );
    
    for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
    {
        for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
        {
            for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
            {
                if(*ptr==GRAY_STATIC_LABEL)
                {
                    nb_vcs6=0;
                    for (icon=0;icon<vcs6->nb_neighbors;icon++)
                    {
                        voisin = ptr + vcs6->offset[icon];
                        if(*voisin==GRAY_LABEL || *voisin>=WHITE_LABEL)
                        {
                            nb_vcs6++;
                        }
                    }
                    nb_vcs18=0;
                    for (icon=0;icon<vcs18->nb_neighbors;icon++)
                    {
                        voisin = ptr + vcs18->offset[icon];
                        if(*voisin==GRAY_LABEL || *voisin>=WHITE_LABEL)
                        {
                            nb_vcs18++;
                        }
                    }
                    if(nb_vcs6>4 && nb_vcs18>14) *ptr=GRAY_LABEL;
                }
                ptr++;
            }
            ptr += vos->oPointBetweenLine;  /*skip border points*/
        }
        ptr += vos->oLineBetweenSlice; /*skip border lines*/
    }

    return(OK);
}

/*---------------------------------------------------------------------------*/
/* WARNING: the erosion is not homotopic!! No topology test is ever done, the
   code simply does not make use of topo26. */
int VipHomotopicErosionFromInsideSnakeNeighbourhood(
Volume *vol,
Volume *graylevel,
Volume *classif,
int nb_iteration,
int object,
int inside,
int outside,
float KISING,
float sigmaG,
float sigmaW,
int label
)
/*-------------------------------------------------------------------------*/

{
  VipIntBucket *buck, *nextbuck;
  Topology26Neighborhood *topo26;
  VipConnectivityStruct *vcs6, *vcs26;
  int loop, count;
  Vip_S16BIT *first, *ptr, *voisin, *voisin2;
  Vip_S16BIT *gfirst, *gptr, *gvoisin;
  Vip_S16BIT *cfirst, *cptr, *cvoisin;
  int *buckptr;
  int i;
  float KW, KG, mW, mG, sW, sG;
  int nnG, nnW, nbG, nbW;
  float deltaU;
  int icon;
  int totalcount;
  int valeur;
  int immortality = VFALSE;
  int temp;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipHomotopicErosionFromInsideSnake");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipHomotopicErosionFromInsideSnake is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipHomotopicErosionFromInsideSnake");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipHomotopicErosionFromInsideSnake is only implemented with border");
      VipPrintfExit("(skeleton)VipHomotopicErosionFromInsideSnake");
      return(PB);
    }

  if(object==inside)
    {
      VipPrintfError("object value = inside value!");
      VipPrintfExit("(skeleton)VipHomotopicErosionFromInsideSnake");
      return(PB);
    }
  printf("Homotopic snake erosion from inside...\n");
  /*
  printf("Initialization (object:%d, inside:%d, outside:%d)...\n",object,inside,outside);
  */
  VipSetBorderLevel( vol, outside ); 

  buck = VipCreateFrontIntBucketForErosionFromOutside( vol, CONNECTIVITY_6, VIP_FRONT, object, inside);
  if(buck==PB) return(PB);
  nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
  if(nextbuck==PB) return(PB);

  topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

  vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );
  vcs26 = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );

  nextbuck->n_points = 0;

  first = VipGetDataPtr_S16BIT(vol);
  gfirst = VipGetDataPtr_S16BIT(graylevel);
  cfirst = VipGetDataPtr_S16BIT(classif);

  /*main loop*/
  loop=0;
  count = 1;
  totalcount = 0;
  KG = 2*KISING - 0.00001;
  KW = 4*KISING + 0.00001;
  /*
  printf("gray matter (mean:%f, sigma:%f), white matter (mean:%f, sigma:%f)\n",
	 mG,sigmaG,mW,sigmaW);
  */
  printf("loop: %3d, Deleted: %6d",loop,0);	      	  

  while((loop++<nb_iteration)&&(count)&&(buck->n_points>0))
      {
	if(loop==1) count=0;
	totalcount += count;
	count = 0;
	printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bloop: %3d, Deleted: %6d",loop,totalcount);	      	  
	  fflush(stdout);
	  /*	  printf("Front size: %d\n",buck->n_points);*/
	  
	  buckptr = buck->data;
	  for(i=buck->n_points;i--;)
	    {
	      ptr = first + *buckptr;
	      gptr = gfirst + *buckptr;
              cptr = cfirst + *buckptr++;
	      nnG = 0;
	      nnW = 0;
              mG = 0.;
              mW = 0.;
              sG = 0.;
              sW = 0.;
	      nbG = 0;
	      nbW = 0;
	      deltaU = 0;
	      for (icon=0; icon<vcs6->nb_neighbors; icon++)
		  {
		      voisin = ptr + vcs6->offset[icon];	      
		      if(*voisin==inside) nnW++;
		      else nnG++;
//		      if(*voisin==VIP_IMMORTAL) 
//                      {
//                          voisin2 = ptr + 2*vcs6->offset[icon];
//                          if(*voisin2==inside) immortality = VTRUE;
//                      }
		  }

              switch(label)
              {
                  case WHITE_STATIC_LABEL:
                      for (icon=0; icon<vcs26->nb_neighbors; icon++)
		          {
                              voisin = ptr + vcs26->offset[icon];
                              gvoisin = gptr + vcs26->offset[icon];
                              cvoisin = cptr + vcs26->offset[icon]; 	      
		              if(*voisin==inside)
//		              if(*cvoisin>199)
                              {
                                  mW += (float)(*gvoisin);
                                  nbW++;
                              }
//                              else if(*cvoisin<200 && *cvoisin>11)
                              else if(*cvoisin>11)
                              {
                                  mG += (float)(*gvoisin);
                                  nbG++;
                              }
                          }
                      mW /= (float)nbW;
                      mG /= (float)nbG;
                      for (icon=0; icon<vcs26->nb_neighbors; icon++)
	        	  {
                              voisin = ptr + vcs26->offset[icon];
                              gvoisin = gptr + vcs26->offset[icon];
                              cvoisin = cptr + vcs26->offset[icon]; 	      
		              if(*voisin==inside)
//	        	      if(*cvoisin>199)
                              {
                                  sW += ((float)(*gvoisin) - mW)*((float)(*gvoisin) - mW);
                              }
//                              else if(*cvoisin<200 && *cvoisin>11)
                              else if(*cvoisin>11)
                              {
                                  sG += ((float)(*gvoisin) - mG)*((float)(*gvoisin) - mG);
                              }
                          }
                      sW = sqrt(sW/nbW);
                      sG = sqrt(sG/nbG);
                      deltaU = -KISING*(nnW-nnG);
	              deltaU += deltaGPotentialGtoW(*gptr,KG,mG,sG,KW,mW,sW);
//	              deltaU += deltaGPotentialWtoG(*gptr,KG,mG,sigmaG,KW,mW,sigmaW);
//	              if((deltaU<=0 && *cptr>199) || nbG==0)
      	              if(deltaU<=0 && *cptr>199)
    		      {
		          *ptr = inside;
		          count++;
		      }
                      break;
                  case WHITE_LABEL:
                      for (icon=0; icon<vcs26->nb_neighbors; icon++)
		          {
                              voisin = ptr + vcs26->offset[icon];
                              gvoisin = gptr + vcs26->offset[icon];
                              cvoisin = cptr + vcs26->offset[icon]; 	      
//		              if(*voisin==inside)
		              if(*cvoisin>199)
                              {
                                  mW += (float)(*gvoisin);
                                  nbW++;
                              }
//                              else if(*cvoisin<200 && *cvoisin>11)
                              else if(*cvoisin>11)
//                              else if(*cvoisin==101)
                              {
                                  mG += (float)(*gvoisin);
                                  nbG++;
                              }
                          }
                      mW /= (float)nbW;
                      mG /= (float)nbG;
                      for (icon=0; icon<vcs26->nb_neighbors; icon++)
	        	  {
                              voisin = ptr + vcs26->offset[icon];
                              gvoisin = gptr + vcs26->offset[icon];
                              cvoisin = cptr + vcs26->offset[icon]; 	      
//		              if(*voisin==inside)
	        	      if(*cvoisin>199)
                              {
                                  sW += ((float)(*gvoisin) - mW)*((float)(*gvoisin) - mW);
                              }
//                              else if(*cvoisin<200 && *cvoisin>11)
                              else if(*cvoisin>11)
//                              else if(*cvoisin==101)
                              {
                                  sG += ((float)(*gvoisin) - mG)*((float)(*gvoisin) - mG);
                              }
                          }
                      sW = sqrt(sW/nbW);
                      sG = sqrt(sG/nbG);
                      deltaU = -KISING*(nnW-nnG);
	              deltaU += deltaGPotentialGtoW(*gptr,KG,mG,sG,KW,mW,sW);
//	              deltaU += deltaGPotentialWtoG(*gptr,KG,mG,sigmaG,KW,mW,sigmaW);
//	              if((deltaU<=0 && *cptr>199) || nbG==0)
	              if(deltaU<=0 && *cptr>199 && immortality==VFALSE)
	   	      {
	                  *ptr = inside;
                          count++;
		      }
//	              else *ptr = VIP_IMMORTAL;
                      break;
                  case GRAY_LABEL:
                      for (icon=0; icon<vcs26->nb_neighbors; icon++)
		          {
                              voisin = ptr + vcs26->offset[icon];
                              gvoisin = gptr + vcs26->offset[icon];
                              cvoisin = cptr + vcs26->offset[icon]; 	      
		              if(*voisin==inside)
//		              if(*voisin==inside || *voisin==VIP_FRONT)
//		              if(*cvoisin>199)
                              {
                                  mW += (float)(*gvoisin);
                                  nbW++;
                              }
                              else if(*cvoisin<200 && *cvoisin>11 && *voisin!=VIP_FRONT)
//                              else if(*cvoisin==101)
                              {
                                  mG += (float)(*gvoisin);
                                  nbG++;
                              }
                          }
                      mW /= (float)nbW;
                      mG /= (float)nbG;
                      for (icon=0; icon<vcs26->nb_neighbors; icon++)
	        	  {
                              voisin = ptr + vcs26->offset[icon];
                              gvoisin = gptr + vcs26->offset[icon];
                              cvoisin = cptr + vcs26->offset[icon]; 	      
		              if(*voisin==inside)
//		              if(*voisin==inside || *voisin==VIP_FRONT)
//	        	      if(*cvoisin>199)
                              {
                                  sW += ((float)(*gvoisin) - mW)*((float)(*gvoisin) - mW);
                              }
                              else if(*cvoisin<200 && *cvoisin>11 && *voisin!=VIP_FRONT)
//                              else if(*cvoisin==101)
                              {
                                  sG += ((float)(*gvoisin) - mG)*((float)(*gvoisin) - mG);
                              }
                          }
                      sW = sqrt(sW/nbW);
                      sG = sqrt(sG/nbG);
                      deltaU = -KISING*(nnW-nnG);
	              deltaU += deltaGPotentialGtoW(*gptr,KG,mG,sG,KW,mW,sW);
//	              deltaU += deltaGPotentialWtoG(*gptr,KG,mG,sigmaG,KW,mW,sigmaW);
//	              if((deltaU<=0 && *cptr>199) || nbG==0)
	              if(deltaU<=0 && *cptr!=101)
	   	      {
	                  *ptr = inside;
                          count++;
		      }
                      break;
		  case 1:
		      temp = *gptr;
		      for (icon=0; icon<vcs26->nb_neighbors; icon++)
		      {
			  voisin = ptr + vcs26->offset[icon];
			  gvoisin = gptr + vcs26->offset[icon];
			  cvoisin = cptr + vcs26->offset[icon]; 	      
			  if(*voisin==inside)
			  {
			      mW += (float)(*gvoisin);
			      nbW++;
			  }
			  else if(*voisin==object)
			  {
			      mG += (float)(*gvoisin);
			      nbG++;
			  }
		      }
		      mW /= (float)nbW;
		      mG /= (float)nbG;
		      for (icon=0; icon<vcs26->nb_neighbors; icon++)
		      {
			  voisin = ptr + vcs26->offset[icon];
			  gvoisin = gptr + vcs26->offset[icon];
			  cvoisin = cptr + vcs26->offset[icon];
			  if(*voisin==inside)
			  {
			      sW += ((float)(*gvoisin) - mW)*((float)(*gvoisin) - mW);
			  }
			  else if(*voisin==255)
			  {
			      sG += ((float)(*gvoisin) - mG)*((float)(*gvoisin) - mG);
			  }
		      }
		      sW = sqrt(sW/nbW);
                      sG = sqrt(sG/nbG);
		      if(PotentielForVoxelBasedAna2011(GRAY_LABEL,temp,mW,mG,sW,sG)>PotentielForVoxelBasedAna2011(WHITE_LABEL,temp,mW,mG,sW,sG))
		      {
			  *ptr = inside;
			  count++;
		      }
		      break;
                  default:
                      VipPrintfError("You can not use this label to get a potential!");
                      fprintf(stderr,"label: %d\n",label);
                      VipPrintfExit("Potentiel (Vip_Potts)");
                      return(PB);
              }
	    }
	      	      			
	  VipFillNextFrontFromOldFrontForErosionFromOutside(first,buck,nextbuck,vcs6,object,VIP_FRONT,inside,outside);
		  
	  /*bucket permutation*/
	  VipPermuteTwoIntBucket(&buck, &nextbuck);
	  nextbuck->n_points = 0;
	  
      }
  printf("\n");
  VipChangeIntLabel(vol,VIP_FRONT,object);
  VipChangeIntLabel(vol,VIP_IMMORTAL,object);  
  
  VipFreeIntBucket(buck);
  VipFreeIntBucket(nextbuck);
  
  return(OK);
}      

/*---------------------------------------------------------------------------*/
static float deltaGPotentialGtoW(int glevel,float KG,float mG,float sigmaG,
			     float KW,float mW,float sigmaW)
/*---------------------------------------------------------------------------*/
{
    float potG, potW;
    /*try to favorize white more because of partial volume in thin gyri*/
    /* 
    if(glevel<=(mG+sigmaG)) potG = -1.;
    else if(glevel>=(mG+2*sigmaG)) potG = 0.;
    else potG = -1. + (glevel-mG-sigmaG)/sigmaG;

    if(glevel<(mW-2*sigmaW)) potW = 0.;
    else if(glevel>(mW-sigmaW)) potW = -1.;
    else potW = -1. - (glevel-mW+sigmaW)/sigmaW;
    */
    if(glevel<=(mG+sigmaG)) potG = -1.;
    else if(glevel>=(mG+2*sigmaG)) potG = 0.;
    else potG = -1. + (glevel-mG-sigmaG)/sigmaG;
 
    if(glevel<(mW-2*sigmaW)) potW = 0.;
    else if(glevel>(mW-1.0*sigmaW)) potW = -1.;
    else potW = -1. - 1/1.0*(glevel-mW+sigmaW)/sigmaW;
    return(KW*potW-KG*potG);
}

/*---------------------------------------------------------------------------*/
static float deltaGPotentialWtoG(int glevel,float KG,float mG,float sigmaG,
			     float KW,float mW,float sigmaW)
/*---------------------------------------------------------------------------*/
{
    float potG, potW;

    if(glevel<=(mG+sigmaG)) potG = -1.;
    else if(glevel>=(mG+2*sigmaG)) potG = 0.;
    else potG = -1. + (glevel-mG-sigmaG)/sigmaG;

    if(glevel<(mW-2*sigmaW)) potW = 0.;
    else if(glevel>(mW-sigmaW)) potW = -1.;
    else potW = -1. - (glevel-mW+sigmaW)/sigmaW;

    return(KG*potG-KW*potW);
}
