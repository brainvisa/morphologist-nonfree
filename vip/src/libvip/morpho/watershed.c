/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_morpho.c         * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 04/08/1999
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : watershed
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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include <vip/util.h>
#include <vip/volume.h>
#include <vip/morpho.h>
#include <vip/distmap.h>
#include <vip/gaussian.h>
#include <vip/geometry.h>
#include <vip/skeleton.h>

/*---------------------------------------------------------------------------*/
static int VipFillBassinDepth( Volume *vol, Volume *dvol, int nseed, int *depth,
			       int forbidden, int domain);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static VipIntBucket *VipCreateFrontIntBucketForWatershed( Volume *vol, 
							  int connectivity,
							  int front_value,
							  int forbidden,
							  int domain);
 /*---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static int VipFillNextFrontFromOldFrontForWatershed(
  Vip_S16BIT *first_vol_point,
  Vip_S16BIT *first_depth_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  VipConnectivityStruct *vcs,
  int next_value,
  int front_value,
  int STOP);
/*-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static int VipFillAvailableLevel(Volume *vol, int *available_level, int size);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int VipPutTranslationInWatershed(Volume *label, int nseed, int *translation,
					int forbidden, int domain);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static Volume *VipMorphoDistmapPreprocessing(Volume *distmap, int forbidden);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static Volume *VipGaussianCurvatureDistmapPreprocessing(Volume *distmap,
							Volume *vol, Volume *saddle_point,
							int forbidden, int outside);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
Volume *VipSulcalRootsWatershedVoronoi(
				Volume *vol,
				Volume *saddle_point,
				int connectivity_extrema,
				int mode,
				int domain,
				int forbidden,
				int outside,
				int hight_threshold
)
/*---------------------------------------------------------------------------*/
{
    Volume *watershed, *distmap;
    int locforbidden = -46;
    int nseed;

    if (VipVerifyAll(vol)==PB || VipVerifyAll(saddle_point)==PB)
	{
	    VipPrintfExit("(morpho)VipSulcalRootsWatershedVoronoi");
	    return(PB);
	}
    if (VipTestType(vol,S16BIT)!=OK|| VipTestType(saddle_point,S16BIT)!=OK)
	{
	    VipPrintfError("Sorry,  VipSulcalRootsWatershedVoronoi is only implemented for S16BIT volume");
	    VipPrintfExit("(morpho)VipSulcalRootsWatershedVoronoi");
	    return(PB);
	}
    if (mVipVolBorderWidth(vol) < 1)
	{
	    VipPrintfError("Sorry, VipSulcalRootsWatershedVoronoi is only implemented with border");
	    VipPrintfExit("(morpho)VipSulcalRootsWatershedVoronoi");
	    return(PB);
	}
    if (mVipVolBorderWidth(vol) != mVipVolBorderWidth(saddle_point))
	{
	    VipPrintfError("Sorry, cortex and saddle_point volume should have same border width");
	    VipPrintfExit("(morpho)VipSulcalRootsWatershedVoronoi");
	    return(PB);
	}

    printf("------------------------------------------------------------\n");
    printf("Preprocessing for the watershed applied to geodesic depth...\n");
    printf("------------------------------------------------------------\n");
    distmap = VipCopyVolume(vol,"distmap");
    if(distmap==PB) return(PB);
    VipChangeIntLabel(distmap,outside,domain-1); /*object*/
    if(VipSetBorderLevel(distmap,forbidden)==PB) return(PB);

    if(mode==VIP_GEODESIC_DEPTH)
	{
	    printf("Computing geodesic distance map to outside...\n");
	    if( VipComputeCustomizedFrontPropagationChamferDistanceMap (distmap, 
									domain, forbidden,
									VIP_NO_LIMIT_IN_PROPAGATION, 0,
									5, 5, 5, 100)==PB)
		return(PB);
	}
    else if(mode==VIP_BIRDFLY_DEPTH)
	{
	    printf("Computing bird fly distance map to outside...\n");
	    VipChangeIntLabel(distmap,forbidden,domain);
	    if( VipComputeFrontPropagationChamferDistanceMap (distmap, 
							      domain, forbidden,
							      VIP_NO_LIMIT_IN_PROPAGATION, 0)==PB)
		return(PB);

	    VipMerge(distmap, vol, VIP_MERGE_ONE_TO_ONE, forbidden, 0);
	}
    else
	{
	    VipPrintfError("Unknown mode");
	    VipPrintfExit("(morpho)VipSulcalRootsWatershedVoronoi");
	    return(PB);
	}    

 
    printf("Decreasing depth at the level of saddle points...\n");
    distmap = VipGaussianCurvatureDistmapPreprocessing(distmap, vol, saddle_point, forbidden, outside);
    if(distmap == PB) return(PB);
    
    printf("Morphological filtering to reduce the number of extrema and the depth of sharp ones\n");
    distmap = VipMorphoDistmapPreprocessing(distmap, 0);
    if(distmap == PB) return(PB);
    

    printf("Detecting local maxima which will represent sulcal roots...\n");
    watershed = VipLocalExtrema( distmap, connectivity_extrema, forbidden, MAXIMA);
    if(watershed==PB) return(PB);
    VipMerge(watershed, distmap, VIP_MERGE_ONE_TO_ONE, 0, 0);

    /*
      printf("maxima geodesic dilation\n");

      VipDilation(watershed, CHAMFER_BALL_3D, 2.);
    */
    if(VipConnexVolumeFilter( watershed, CONNECTIVITY_26, 0, CONNEX_LABEL)==PB)
	return(PB);
    nseed = VipGetVolumeMax(watershed);
    printf("Initial number of seeds: %d\n",nseed);

    if(VipSetBorderLevel(watershed,locforbidden)==PB) return(PB);
    VipMerge(watershed, vol, VIP_MERGE_ONE_TO_ONE, forbidden, locforbidden);
    VipMerge(watershed, vol, VIP_MERGE_ONE_TO_ONE, outside, locforbidden);

    if(VipComputeWatershedVoronoi(distmap,watershed,CONNECTIVITY_26,
				  0,locforbidden,nseed,hight_threshold)==PB)
	return(PB);

    VipFreeVolume(distmap);

    return(watershed);
      
}

/*--------------------------------------------------------------------*/
Volume *ConvertBrainToSaddlePoint(Volume *brain, float sigma, int bwidth, float threshold)
/*--------------------------------------------------------------------*/
{
    Volume *gaussian;
    Volume *gaussiancurv;
    Volume *saddle;
    VipOffsetStruct *vosalt, *vosmc;
    int ix, iy, iz;
    int xsize, ysize, zsize;
    Vip_FLOAT *ptrgc;
    Vip_S16BIT *ptralt;
    float temp;
    char name[VIP_NAME_MAXLEN];

    
    if (VipVerifyAll(brain)==PB)
	{
	    VipPrintfExit("(morpho)ConvertBrainToSaddlePoint");
	    return(PB);
	}
    if((bwidth<0)||(bwidth>50))
	{
	    VipPrintfError("Strange border width\n");
	    VipPrintfExit("(morpho)ConvertBrainToSaddlePoint");
	    return(PB);
	}
    if(threshold>0)
      {
	VipPrintfWarning("Positive threshold on Gaussian curvature to get saddle points?");
      }

    printf("-----------------------------------------------\n");
    printf("Detecting saddle points of gray/white interface = pli de passage markers\n");
    printf("-----------------------------------------------\n");

    (void)printf("Computing Gaussian smoothing (sigma %f) of %s...\n",sigma, mVipVolName(brain));
    gaussian = VipDeriche3DGaussian( brain, sigma, NEW_FLOAT_VOLUME);
	
    (void)printf("Computing %s isophote gaussian curvature...\n",mVipVolName(brain));
    gaussiancurv = Vip3DGeometry(gaussian,GAUSSIAN_CURVATURE);
    VipFreeVolume(gaussian);
    

    (void)strcpy(name,mVipVolName(brain));
    (void)strcat(name,"_saddlepoint");
    saddle = VipDuplicateVolumeStructure (brain,name);
    VipSetType(saddle,S16BIT);
    VipSetBorderWidth(saddle,bwidth);
    if(VipAllocateVolumeData(saddle)==PB)
	{
	    VipPrintfExit("ConvertBrainToSaddlePoint");
	    return(PB);
	}

   vosalt = VipGetOffsetStructure(saddle);
   ptralt = VipGetDataPtr_S16BIT(saddle) + vosalt->oFirstPoint;
   vosmc = VipGetOffsetStructure(gaussiancurv);
   ptrgc = VipGetDataPtr_VFLOAT(gaussiancurv) + vosmc->oFirstPoint;
   xsize = mVipVolSizeX(saddle);
   ysize = mVipVolSizeY(saddle);
   zsize = mVipVolSizeZ(saddle);

   /* it is just a threshold*/

   for ( iz = 0; iz < zsize; iz++ )                /* loop on slices */
   {
      for ( iy = 0; iy < ysize; iy++ )             /* loop on lines */
      {
         for ( ix = 0; ix < xsize; ix++ )          /* loop on points */
         {
	     temp = *ptrgc;
	     if(temp<threshold) *ptralt = VTRUE;
	     else *ptralt = VFALSE;
	     ptralt++;
	     ptrgc++;
         }
         ptralt += vosalt->oPointBetweenLine;  /*skip border points*/
         ptrgc += vosmc->oPointBetweenLine;  /*skip border points*/
      }
      ptralt += vosalt->oLineBetweenSlice; /*skip border lines*/
      ptrgc += vosmc->oLineBetweenSlice; /*skip border lines*/
   }

   VipFreeVolume(gaussiancurv);
   /*
     printf("Writing saddle point volume...\n");
     VipWriteTivoliVolume(saddle,"saddle");
   */
   return(saddle);
}
/*---------------------------------------------------------------------------*/
static Volume *VipGaussianCurvatureDistmapPreprocessing(Volume *distmap,
							Volume *vol, Volume *saddle_point, int forbidden, int outside)
/*---------------------------------------------------------------------------*/
{
    VipConnectivityStruct *vcs6, *vcsdil;
    Vip_S16BIT *dptr, *vptr, *voisin;
    Vip_S16BIT *gcptr, *gcvoisin;
    int i, NbTotalPts;
    int icon;
    int temp;
    int count = 0;

    if(mVipVolBorderWidth(distmap)!=mVipVolBorderWidth(vol) || 
       mVipVolBorderWidth(distmap)!=mVipVolBorderWidth(saddle_point))
      {
	VipPrintfError("All volume should have the same border width");
	VipPrintfExit("VipGaussianCurvatureDistmapPreprocessing");
	return(PB);
      }
    if(mVipVolBorderWidth(distmap)<2)
      {
	VipPrintfWarning("A border width of 2 could be better there (if your brain reaches image border)...");
	VipPrintfInfo("a coward message from VipGaussianCurvatureDistmapPreprocessing");
      }
    vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );
    if(vcs6==PB) return(PB);
    vcsdil = VipGetConnectivityStruct( vol, CONNECTIVITY_2x26 );
    if(vcsdil==PB) return(PB);

    vptr = VipGetDataPtr_S16BIT(vol);
    dptr = VipGetDataPtr_S16BIT(distmap);
    gcptr = VipGetDataPtr_S16BIT(saddle_point);
    if(VipSetBorderLevel(vol,forbidden)==PB) return(PB);

    NbTotalPts = VipOffsetVolume(vol);
 
    for ( i=0; i<NbTotalPts; i++ )
	{
	    if (*vptr!=forbidden && *vptr!=outside && *gcptr==VTRUE)            
		{	  
		   for ( icon=0; icon<vcs6->nb_neighbors;icon++)
			{
			    voisin = vptr + vcs6->offset[icon];
			    gcvoisin = gcptr + vcs6->offset[icon];
			    if(*voisin==forbidden && *gcvoisin==VTRUE) break;
			}
		   if(icon!=vcs6->nb_neighbors)/*point selle*/
		       {
			   count++;
			   if(*dptr>0)
			       {
				 temp = *dptr-20; /*Jeff 3-1-2000, passe de 100 a 20*/
				   *dptr = mVipMax(0,temp);
			       }
			   for(icon=0;icon<vcsdil->nb_neighbors;icon++) 
			       {
				   voisin = dptr + vcsdil->offset[icon];
				   if(*voisin>0)
				       {
					   temp = *voisin-20;
					   *voisin = mVipMax(0,temp);
				       }
			       }
		       }
		}
	    vptr++;
	    dptr++;
	    gcptr++;
	}

    printf("%d saddle point marker found\n",count);

    VipFreeConnectivityStruct(vcs6);
    VipFreeConnectivityStruct(vcsdil);

    return(distmap);
}

/*---------------------------------------------------------------------------*/
static Volume *VipMorphoDistmapPreprocessing(Volume *distmap, int forbidden)
/*---------------------------------------------------------------------------*/
{
  /* Vip_S16BIT *cptr, *dptr;
     int i;
     int NbTotalPts;
     Volume *distmapsave;*/
  Volume *cdistmap;

    printf("Gray level opening (3x3x3 flat structuring element)  is applied to the distance map...\n");
    cdistmap = VipGrayLevelPlanErosionDilation(distmap,CONNECTIVITY_26,forbidden);
    if(cdistmap==PB) return(PB);
    VipFreeVolume(distmap);
    distmap = cdistmap;   
    printf("Gray level closing (3x3x3 flat structuring element)  is applied to the distance map...\n");
    cdistmap = VipGrayLevelPlanErosionDilation(distmap,CONNECTIVITY_26,forbidden);
    if(cdistmap==PB) return(PB);
    VipFreeVolume(distmap);
    distmap = cdistmap;

  
  
    /*
    cdistmap = VipGrayLevelPlanErosionDilation(distmap,CONNECTIVITY_26,forbidden);
    if(cdistmap==PB) return(PB);
    VipFreeVolume(distmap);
    distmap = cdistmap;
    */
    /*
    printf("Remove the result of mexican hat to increase the pli de passage...\n");
    printf("Geodesic Gray level closing (7x7x7)  is applied to the distance map...\n");
    cdistmap = VipGrayLevelPlanDilation(distmap,CONNECTIVITY_26,forbidden);
    if(cdistmap==PB) return(PB);
    distmapsave = distmap;
    distmap = cdistmap;
    cdistmap = VipGrayLevelPlanDilation(distmap,CONNECTIVITY_26,forbidden);
    if(cdistmap==PB) return(PB);
    VipFreeVolume(distmap);
    distmap = cdistmap;
    cdistmap = VipGrayLevelPlanDilation(distmap,CONNECTIVITY_26,forbidden);
    if(cdistmap==PB) return(PB);
    VipFreeVolume(distmap);
    distmap = cdistmap;
    cdistmap = VipGrayLevelPlanDilation(distmap,CONNECTIVITY_26,forbidden);
    if(cdistmap==PB) return(PB);
    VipFreeVolume(distmap);
    distmap = cdistmap;
    cdistmap = VipGrayLevelPlanErosion(distmap,CONNECTIVITY_26,forbidden);
    if(cdistmap==PB) return(PB);
    VipFreeVolume(distmap);
    distmap = cdistmap;
    cdistmap = VipGrayLevelPlanErosion(distmap,CONNECTIVITY_26,forbidden);
    if(cdistmap==PB) return(PB);
    VipFreeVolume(distmap);
    distmap = cdistmap;
    cdistmap = VipGrayLevelPlanErosion(distmap,CONNECTIVITY_26,forbidden);
    if(cdistmap==PB) return(PB);
    VipFreeVolume(distmap);
    distmap = cdistmap;
    cdistmap = VipGrayLevelPlanErosion(distmap,CONNECTIVITY_26,forbidden);
    if(cdistmap==PB) return(PB);
    VipFreeVolume(distmap);
    distmap = distmapsave;

    dptr = VipGetDataPtr_S16BIT(distmap);
    cptr = VipGetDataPtr_S16BIT(cdistmap);

    NbTotalPts = VipOffsetVolume(distmap);
 
    for ( i=0; i<NbTotalPts; i++ )
	{
	    if (*dptr!=forbidden)            
		{
		    *cptr = mVipMax(0,(*dptr - (*cptr-*dptr))); 
		}
	    cptr++;
	    dptr++;
	}

    */
    return(cdistmap);
}

/*NB: a good method should cope with morphomath methods*/
/*---------------------------------------------------------------------------*/
Volume *VipLocalExtrema(Volume *vol, int connectivity, int forbidden, int extrematype)
/*---------------------------------------------------------------------------*/
{
  Volume *out;
  VipConnectivityStruct *vcs;
  int icon;
  Vip_S16BIT *ptr, *outptr;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  int offset, *offsetptr;
  int isextremum, theval;
  int nval;

   if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("VipLocalExtrema");
      return(PB);
    }

   if(mVipVolBorderWidth(vol)<=0)
     {
       VipPrintfError("non zero borderwidth required");
       VipPrintfExit("VipLocalExtrema");
       return(PB);
     }

   if(extrematype!=MINIMA && extrematype!=MAXIMA 
      && extrematype!=STRICTMINIMA && extrematype!=STRICTMAXIMA)
       {
	   VipPrintfError("Unkown extrematype");
	   VipPrintfExit("VipLocalExtrema");
	   return(PB);
       }

   if(VipSetBorderLevel(vol,forbidden)==PB) return(PB);

   out = VipDuplicateVolumeStructure( vol, "copy");
   if(out==PB) return(PB);
   if(VipAllocateVolumeData(out)==PB) return(PB);

   vcs = VipGetConnectivityStruct( vol, connectivity );
   if(vcs==PB) return(PB);

   printf("Neighborhood size: %d\n",vcs->nb_neighbors);

   vos = VipGetOffsetStructure(vol);

   ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
   outptr = VipGetDataPtr_S16BIT( out ) + vos->oFirstPoint;

   (void)printf("Slice:   ");

   for ( iz = 0; iz < mVipVolSizeZ(vol); iz++ )               /* loop on slices */
   {
      (void)printf("\b\b\b%3d",iz+1);
      fflush(stdout);
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
	    theval = *ptr;
	    if(theval!=forbidden)
		{
		    isextremum = VTRUE;
		    offsetptr = vcs->offset;
		    if(extrematype==MAXIMA)
			{
			    for(icon=vcs->nb_neighbors ; icon--;)
				{
				    offset = *offsetptr++;
				    nval = *(ptr+offset);
				    if(nval!=forbidden && nval>theval)
					{
					    isextremum = VFALSE;
					    break;
					}
				}
			}
		    else if(extrematype==STRICTMAXIMA)
			{
			    for(icon=vcs->nb_neighbors ; icon--;)
				{
				    offset = *offsetptr++;
				    nval = *(ptr+offset);
				    if(nval!=forbidden && nval>=theval)
					{
					    isextremum = VFALSE;
					    break;
					}
				}
			}
		    else if(extrematype==MINIMA)
			{
			    for(icon=vcs->nb_neighbors ; icon--;)
				{
				    offset = *offsetptr++;
				    nval = *(ptr+offset);
				    if(nval!=forbidden && nval<theval)
					{
					    isextremum = VFALSE;
					    break;
					}
				}
			}
		    else if(extrematype==STRICTMINIMA)
			{
			    for(icon=vcs->nb_neighbors ; icon--;)
				{
				    offset = *offsetptr++;
				    nval = *(ptr+offset);
				    if(nval!=forbidden && nval<=theval)
					{
					    isextremum = VFALSE;
					    break;
					}
				}
			}

		}
	    else isextremum = VFALSE;

	    if(isextremum==VTRUE) *outptr++ = 255;
	    else *outptr++ = 0;
	    ptr++;
         }
         ptr += vos->oPointBetweenLine;  /*skip border points*/
         outptr += vos->oPointBetweenLine;  /*skip border points*/
      }
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
      outptr += vos->oLineBetweenSlice; /*skip border lines*/
   }

   printf("\n");

   VipFreeConnectivityStruct(vcs);

   return(out);
 
}

/*NB: a good method should cope with morphomath methods*/

/*----------------------------------------------------------------------------*/
int VipComputeWatershedVoronoi(
  Volume *vol,
  Volume *label,
  int connectivity,
  int domain,
  int forbidden,
  int nseed,
  int height_threshold 
)
/*---------------------------------------------------------------------------*/
    /*warning this procedure makes water flow from the ceiling...*/
{ 
  VipIntBucket *buck, *nextbuck;
  VipConnectivityStruct *vcs ;
  int count;
  Vip_S16BIT *vfirst, *lfirst, *lptr, *vptr, *voisin, *vvptr;
  int *available_level;
  int *buckptr;
  int i, bp;
  int waterlevel;
  int waterlevelmax;
  int icon;
  int *depth;
  int *height;
  int *translation;
  int *father;
  int surviving_seeds;
  int temp;
  int STOP=-1;
  int nlab;
  int l,lab[26];
  int countlab[26];
  int labcountmax;

  if (VipVerifyAll(vol)==PB || VipVerifyAll(label)==PB)
    {
      VipPrintfExit("(morpho)VipComputeWatershedVoronoi");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK || VipTestType(label,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipComputeWatershedVoronoi is only implemented for S16BIT volume");
      VipPrintfExit("(morpho)VipComputeWatershedVoronoi");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipComputeWatershedVoronoi is only implemented with border");
      VipPrintfExit("(morpho)VipComputeWatershedVoronoi");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) !=mVipVolBorderWidth(label) )
    {
      VipPrintfError("Sorry, VipComputeWatershedVoronoi needs same borderwidth for all volumes");
      VipPrintfExit("(morpho)VipComputeWatershedVoronoi");
      return(PB);
    }
  if(nseed<=0 || nseed>100000)
    {
      VipPrintfError("Sorry, bad number of seeds");
      VipPrintfExit("(morpho)VipComputeWatershedVoronoi");
      return(PB);
    }

  printf("----------------------------------------------------------------\n");
  printf("Computing watershed with pruning of shallow catchment bassins...\n");
  printf("----------------------------------------------------------------\n");
  /*    
  printf("Initialization (domain:%d, forbidden:%d, nseeds: %d)...\n",
	 domain,forbidden,nseed);
  */
  depth = (int *)VipCalloc(nseed+1,sizeof(int),"VipComputeWatershedVoronoi");
  if(!depth) return(PB);

  height = (int *)VipCalloc(nseed+1,sizeof(int),"VipComputeWatershedVoronoi");
  if(!height) return(PB);

  translation = (int *)VipCalloc(nseed+1,sizeof(int),"VipComputeWatershedVoronoi");
  if(!translation) return(PB);
  for(i=0;i<=nseed;i++) translation[i] = i;

  father = (int *)VipCalloc(nseed+1,sizeof(int),"VipComputeWatershedVoronoi");
  if(!father) return(PB);
  for(i=0;i<=nseed;i++) father[i] = i;

  VipSetBorderLevel( label, forbidden ); 

  VipFillBassinDepth( label, vol, nseed, depth, forbidden, domain);

  buck = VipCreateFrontIntBucketForWatershed( label, connectivity,
					      VIP_FRONT, forbidden, domain);
  if(buck==PB) return(PB);
  nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
  if(nextbuck==PB) return(PB);
  nextbuck->n_points = 0;

  vcs  = VipGetConnectivityStruct( vol, connectivity );
  if(vcs==PB) return(PB);

  vfirst = VipGetDataPtr_S16BIT(vol);
  lfirst = VipGetDataPtr_S16BIT(label);

  count = 1;

  waterlevel = VipGetVolumeMax(vol);
  waterlevelmax = VipGetVolumeMin(vol);
  waterlevelmax = STOP;
  available_level = (int *)VipCalloc(waterlevel+1,sizeof(int),"VipComputeWatershedVoronoi");
  if(!available_level) return(PB);
  if(VipFillAvailableLevel(vol, available_level, waterlevel)==PB) return(PB);;

  surviving_seeds = nseed;
 
  printf("Water will go from %d down to %d\n",waterlevel,waterlevelmax);
  printf("waterlevel: %5d, bassins:%4d",waterlevel,surviving_seeds);	      	  
  while((waterlevel>waterlevelmax)&&(buck->n_points>0))
    {
      waterlevel -= 1;
      if(available_level[waterlevel]==VFALSE)
	{
	  continue;
	}
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bwaterlevel: %5d, bassins:%4d",waterlevel,surviving_seeds);	      	  
      fflush(stdout);
      /*if(waterlevel%500==0)
	if(VipRandomizeFrontOrder(buck,10)==PB) return(PB);*/ 
      count = 1;
      while(count>0)
	{
	  /*printf("Front size: %d\n",buck->n_points);*/
	  count = 0;
	  buckptr = buck->data;
	  for(bp=buck->n_points;bp--;)
	    {
	      vptr = vfirst + *buckptr;
	      lptr = lfirst + *buckptr++;				
	      if(*vptr>=waterlevel && *lptr==VIP_FRONT)
		{
		  nlab = 0;
		  for ( icon=bp; icon<(bp+vcs->nb_neighbors);icon++) /*random ordering of neighbors*/
		    {
		      voisin = lptr + vcs->offset[icon%vcs->nb_neighbors];
		      if(*voisin!=domain && *voisin!=forbidden && *voisin!= VIP_FRONT)
			{
			  vvptr = vptr + vcs->offset[icon%vcs->nb_neighbors]; /*dvp test*/	      	      
			  if(*vvptr<waterlevel)
			    {
			      VipPrintfWarning("strange flow");
			      printf("voisin depth %d/%d, point: %d\n, lab:%d",*vvptr,
				     waterlevel,*vptr,translation[*voisin]);
			    }
			  for(l=0;l<nlab;l++) /*repertorie les differentes etiquettes qui atteignent le point*/
			    {
			      if(lab[l]==translation[*voisin])
				{
				  countlab[l]++;
				  break;
				}
			    }
			  if(l==nlab)
			    {
			      countlab[nlab] = 1;
			      lab[nlab++] = translation[*voisin];
			    }
			}
		    }
		  if(nlab>0)
		    {
		      labcountmax = 0;
		      count++;
		      for(l=0;l<nlab;l++)
			{
			  if(countlab[l]>countlab[labcountmax]) labcountmax = l;
			}
		      *lptr=translation[lab[labcountmax]];
		      for(l=0;l<nlab;l++)
			{
			  if(l!=labcountmax)
			    {
			      if(translation[lab[l]]!=translation[*lptr]) /*otherwise pb?*/
				/*watershed point*/
				{
				  if (depth[translation[lab[l]]]>=depth[translation[*lptr]])
				    {
				      if(height[translation[*lptr]]==0)
					{
					  height[translation[*lptr]]=
					    depth[translation[*lptr]]-waterlevel;
					  if(height[translation[*lptr]]<0)
					    {
					      VipPrintfWarning("negative height");
					    }
					  else if (height[translation[*lptr]]==0)
					    height[translation[*lptr]]=1; /*palier*/
					  father[translation[*lptr]]=translation[lab[l]];
					  if(height[translation[*lptr]]<height_threshold)
					    {
					      temp = translation[*lptr];
					      for(i=1;i<=nseed;i++)
						{
						  if(translation[i]==temp)
						    {
						      translation[i]=translation[lab[l]];
						    }
						}
					    }
					}
				    }
				  else if (depth[translation[lab[l]]]<depth[translation[*lptr]])
				    {
				      if(height[translation[lab[l]]]==0)
					{
					  height[translation[lab[l]]]=
					    depth[translation[lab[l]]]-waterlevel;
					  if(height[translation[lab[l]]]<0)
					    {
					      VipPrintfWarning("negative height");
					    }
					  else if (height[translation[lab[l]]]==0)
					    height[translation[lab[l]]]=1; /*palier*/
					  father[translation[lab[l]]]=translation[*lptr];
					  if(height[translation[lab[l]]]<height_threshold)
					    {
					      temp = translation[lab[l]];
					      for(i=1;i<=nseed;i++)
						{
						  if(translation[i]==temp)
						    {
						      translation[i]=translation[*lptr];
						    }
						}
					    }
					}
				    }
				}
			    }  
			}
		    }
		}
	    }
	  /* printf("\nWaterlevel: %d\n",waterlevel);*/
	  surviving_seeds = 0;
	  for(i=1;i<=nseed;i++)
	    {
	      /* printf("%3d ",translation[i]);*/
	      if(translation[i]==i) surviving_seeds++;
	    }
	  /*printf("\n");*/
	  /*printf("surviving: %d\n",surviving_seeds);*/
	    
	  VipFillNextFrontFromOldFrontForWatershed(lfirst,vfirst,buck,nextbuck,vcs,
						   domain,VIP_FRONT,STOP);
		  
	  /*bucket permutation*/
	  VipPermuteTwoIntBucket(&buck, &nextbuck);
	  nextbuck->n_points = 0;
	  
	}
    }
  printf("\n");
  /* bassin pruning study */
  /*
  for(i=1;i<=nseed;i++)
    {
      if(i==translation[i])
	printf("seed: %d, translation: %d, father: %d, depth: %d, height: %d\n",i, translation[i], 
	       father[i], depth[i], height[i]);
    }  
  */
  VipPutTranslationInWatershed(label, nseed, translation, forbidden, domain);

  free(depth);
  free(available_level);
  free(height);
  free(translation);

  VipFreeIntBucket(buck);
  VipFreeIntBucket(nextbuck);
  VipFreeConnectivityStruct(vcs);

  return(OK);
}

/*---------------------------------------------------------------------------*/
static int VipFillAvailableLevel(Volume *vol, int *available_level, int size)
/*---------------------------------------------------------------------------*/
{
    Vip_S16BIT *ptr;
    int i, NbTotalPts;

    ptr = VipGetDataPtr_S16BIT(vol);

    NbTotalPts = VipOffsetVolume(vol);
 
    for(i=0;i<=size;i++) available_level[i] = VFALSE;

    for ( i=0; i<NbTotalPts; i++ )
	{
	    if (*ptr>=0)            
		{	  
		    if(*ptr>size)
			{
			    VipPrintfError("stange dist");
			    VipPrintfExit("VipFillAvailableLevel");
			    return(PB);
			}
		    available_level[*ptr] = VTRUE;
		}
	    ptr++;
	}

    return(OK);
}
/*---------------------------------------------------------------------------*/
static int VipPutTranslationInWatershed(Volume *vol, int nseed, int *translation, int forbidden, int domain)
/*---------------------------------------------------------------------------*/
{
    Vip_S16BIT *ptr;
    int i, NbTotalPts;

    ptr = VipGetDataPtr_S16BIT(vol);

    NbTotalPts = VipOffsetVolume(vol);
 
    for ( i=0; i<NbTotalPts; i++ )
	{
	    if (*ptr != domain && *ptr!=forbidden && *ptr!=VIP_FRONT)            
		{	  
		   if(*ptr<=0 || *ptr>nseed)
		       {
			   printf("value: %d\n",*ptr);
			   VipPrintfError("Strange value");
			   VipPrintfExit("VipPutTranslationInWatershed");
			   return(PB);
		       }
		   *ptr = translation[*ptr];
		}
	    ptr++;
	}

    return(OK);
}

/*---------------------------------------------------------------------------*/
static int VipFillBassinDepth( Volume *vol, Volume *dvol, int nseed, int *depth,
			       int forbidden, int domain)
/*---------------------------------------------------------------------------*/
{
    Vip_S16BIT *ptr, *dptr;
    int i, NbTotalPts;
    int count = 0;

    ptr = VipGetDataPtr_S16BIT(vol);
    dptr = VipGetDataPtr_S16BIT(dvol);

    NbTotalPts = VipOffsetVolume(vol);
 
    for ( i=0; i<NbTotalPts; i++ )
	{
	    if (*ptr != domain && *ptr!=forbidden)            
		{	  
		   if(*ptr<=0 || *ptr>nseed)
		       {
			   VipPrintfError("Strange value");
			   VipPrintfExit("VipFillBassinDepth");
			   return(PB);
		       }
		   if(depth[*ptr]==0)  count++;
		   if(depth[*ptr] < *dptr) depth[*ptr] = *dptr;
		}
	    ptr++;
	    dptr++;
	}

    if(count!=nseed)
	{
	    printf("found: %d, predicted:%d\n",count,nseed);
	    VipPrintfError("incoherent seed number");
	    VipPrintfExit("VipFillBassinDepth");
	    return(PB);
	}
    return(OK);
}
/*---------------------------------------------------------------------------*/
static VipIntBucket *VipCreateFrontIntBucketForWatershed( Volume *vol, 
							  int connectivity,
							  int front_value,
							  int forbidden,
							  int domain)
 /*---------------------------------------------------------------------------*/
{ 
 
    Vip_S16BIT *ptr, *voisin;
    int i, NbTotalPts;
    VipIntBucket *buck;
    VipConnectivityStruct *vcs;
    int icon;
  

    if (VipVerifyAll(vol)==PB)
	{
	    VipPrintfExit("(skeleton)VipCreateFrontIntBucketForWatershed");
	    return(PB);
	}
    if (VipTestType(vol,S16BIT)!=OK)
	{
	    VipPrintfError("Sorry,  VipCreateFrontIntBucketForWatershed is only implemented for S16BIT volume");
	    VipPrintfExit("(skeleton)VipCreateFrontIntBucketForWatershed");
	    return(PB);
	}
    if (mVipVolBorderWidth(vol) < 1) /*note that the border is supposed to have been put to forbiden*/
	{
	    VipPrintfError("Sorry,  VipCreateFrontIntBucketForWatershed is only implemented with border");
	    VipPrintfExit("(skeleton)VipCreateFrontIntBucketForWatershed");
	    return(PB);
	}

    vcs = VipGetConnectivityStruct( vol, connectivity );

    ptr = VipGetDataPtr_S16BIT(vol);

    NbTotalPts = VipOffsetVolume(vol);
 
    buck = VipAllocIntBucket(VIP_INITIAL_FRONT_SIZE);

    for ( i=0; i<NbTotalPts; i++ )
	{
	    if (*ptr == domain)            
		{	  
		    for ( icon=0; icon<vcs->nb_neighbors;icon++)
			{
			    voisin = ptr + vcs->offset[icon];	      
			    if(*voisin!=domain && *voisin!=forbidden  && *voisin!=front_value)
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
	}

    if(VipRandomizeFrontOrder(buck,10)==PB) return(PB); 

    VipFreeConnectivityStruct(vcs);

    return(buck);
}

/*-------------------------------------------------------------------------*/
static int VipFillNextFrontFromOldFrontForWatershed(
  Vip_S16BIT *first_vol_point,
  Vip_S16BIT *first_depth_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  VipConnectivityStruct *vcs,
  int next_value,
  int front_value,
  int STOP)
/*-------------------------------------------------------------------------*/
{
    int *buckptr, *dirptr;
    Vip_S16BIT *ptr, *ptr_neighbor, *dptr;
    int i, dir2;

    if(first_vol_point==NULL)
	{
	    VipPrintfError("NULL pointer in VipFillNextFrontFromOldFrontForWatershed");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForWatershed");
	    return(PB);
	}
    if((buck==NULL) || (nextbuck==NULL))
	{
	    VipPrintfError("One NULL bucket in VipFillNextFrontFromOldFrontForWatershed");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForWatershed");
	    return(PB);
	}
    if(vcs==NULL)
	{
	    VipPrintfError("NULL VipConnectivityStruct pointer in VipFillNextFrontFromOldFrontForWatershed");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForWatershed");
	    return(PB);
	}

    buckptr = buck->data;
    for(i=buck->n_points;i--;)
	{
	    ptr = first_vol_point + *buckptr;
	    dptr = first_depth_point + *buckptr;
	    if(*ptr==front_value) 
		{
		    if(nextbuck->n_points==nextbuck->size)
			{
			    if(VipIncreaseIntBucket(nextbuck,
						    VIP_FRONT_SIZE_INCREMENT)==PB)
				return(PB);
			}
		    nextbuck->data[nextbuck->n_points++]=*buckptr;
		}
	    else /*the point has been added a seed*/
		{
		    dirptr = vcs->offset;
		    for(dir2=vcs->nb_neighbors;dir2--;)
			{
			    ptr_neighbor = ptr + *dirptr;
			    if(*ptr_neighbor==next_value && (*(dptr+*dirptr)>STOP))
				/*NOT ALREADY IN FRONT*/
				{		     
				    *ptr_neighbor = front_value;
				    if(nextbuck->n_points==nextbuck->size)
					{
					    if(VipIncreaseIntBucket(nextbuck,
								    VIP_FRONT_SIZE_INCREMENT)==PB)
						return(PB);
					}
				    nextbuck->data[nextbuck->n_points++]=*buckptr+*dirptr;
				}
			    dirptr++;
			}
		}
	    buckptr++;
	}

    return(OK);
}
