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
static double NewPotentiel(int label, int graylevel, VipT1HistoAnalysis *ana);
/*---------------------------------------------------------------------------*/

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
