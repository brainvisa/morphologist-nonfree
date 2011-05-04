/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_brain.c          * TYPE     : sources
 * AUTHOR      : MANGIN J.F.          * CREATION : 20/06/1997
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : segmentation du cerveau
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
#include <vip/morpho.h>	
#include <vip/distmap.h>	
#include <vip/deriche.h>	
#include <vip/gaussian.h>	
#include <vip/histo.h>				
#include <vip/brain.h>
#include <vip/talairach.h>
#include <vip/skeleton.h>
/*---------------------------------------------------------------------------*/
extern Volume *VipComputeRawWhiteMask(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float white_erosion_size,
float white_recovering_size,
int T_white_fat_low,
int T_gray_white);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int GetSinusBucket(  Volume *vol, Volume *brain,
			    VipTalairach *tal,  
			    VipT1HistoAnalysis *ana, Vip3DBucket_S16BIT **buck);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipDilateInPartialVolume(Volume *vol, Volume *mask)
{

	Vip_S16BIT *vol_ptr;
	Vip_S16BIT *mask_ptr, *voisin;
	int mask_val, partial_val, next_val;
	VipOffsetStruct *vos;
	VipConnectivityStruct *vcs6;
	int icon;
	int ix, iy, iz;

	if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB || VipVerifyAll(mask)==PB)
	{
		VipPrintfExit("(VipDilateInPartialVolume");
		return(PB);
	}
    
	vos = VipGetOffsetStructure(vol);
	vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );
	  
	vol_ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
	mask_ptr = VipGetDataPtr_S16BIT( mask ) + vos->oFirstPoint;	  
	  
	for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
	{
		for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
		{
			for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
			{
				if(!(*mask_ptr)) 
				{
					for ( icon = 0;icon<vcs6->nb_neighbors;icon++)
					{
						voisin = mask_ptr + vcs6->offset[icon];
						if (*voisin==255)
						{	        
							mask_val = *(vol_ptr + vcs6->offset[icon]);
							partial_val = *vol_ptr;
							next_val = *(vol_ptr - vcs6->offset[icon]);
							if ((partial_val - next_val)>(mask_val-partial_val))
							{
								*mask_ptr=512;
								break;
							}
						
						}
					}
				}
						
				mask_ptr++;
				vol_ptr++;
			}
			mask_ptr += vos->oPointBetweenLine;  /*skip border points*/
			vol_ptr += vos->oPointBetweenLine;  /*skip border points*/
		}
		mask_ptr += vos->oLineBetweenSlice; /*skip border lines*/
		vol_ptr += vos->oLineBetweenSlice; /*skip border lines*/
	}

	VipSingleThreshold( mask, GREATER_OR_EQUAL_TO,  1, BINARY_RESULT );

		
	return(OK);
		
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipDilateInPartialVolumeFar(Volume *vol, Volume *mask, int layer)
{

  Vip_S16BIT *vol_ptr;
  Vip_S16BIT *mask_ptr, *voisin;
  int mask_val, partial_val, next_val, next_val2;
  VipOffsetStruct *vos;
  VipConnectivityStruct *vcs6;
  int icon;
  int ix, iy, iz;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB || VipVerifyAll(mask)==PB)
    {
      VipPrintfExit("(VipDilateInPartialVolume");
      return(PB);
    }

   if (layer>2)
   {
      VipPrintfExit("(VipDilateInPartialVolumeFar, layer not implemented beyond 2 voxels");
      return(PB);
    }
          vos = VipGetOffsetStructure(vol);
	  vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );
	  
	  vol_ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
	  mask_ptr = VipGetDataPtr_S16BIT( mask ) + vos->oFirstPoint;	  
	  
	  for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
	    {
	      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
		{
		  for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
		    {
		      if(!(*mask_ptr))
			{
			   for ( icon = 0;icon<vcs6->nb_neighbors;icon++)
			  	{
			            voisin = mask_ptr + vcs6->offset[icon];
				    if (*voisin==255)
				     {
			                mask_val = *(vol_ptr + vcs6->offset[icon]);
			                partial_val = *vol_ptr;
                                        if (layer==1)
					{
						next_val = *(vol_ptr - vcs6->offset[icon]);
						if ((partial_val - next_val)>(mask_val-partial_val))
						{
					   		*mask_ptr=512;
					   		break;
						}
					}
                                        else if (layer==2)
					{
						next_val = *(vol_ptr - vcs6->offset[icon]);
						next_val2 = *(vol_ptr - 2*vcs6->offset[icon]);
						if ((partial_val - next_val)>(mask_val-partial_val) && (next_val-next_val2)<(partial_val - next_val))
						{
					   		*mask_ptr=512;
					   		break;
						}
						else if ((next_val-next_val2)>2*(mask_val-partial_val) && (next_val-next_val2)>2*(partial_val - next_val))
						{
					   		*mask_ptr=512;
                                                        *(mask_ptr- vcs6->offset[icon]) = 512;
					   		break;
						}
					}
				    }
				 }
			   }	
		      mask_ptr++;
		      vol_ptr++;
		    }
		  mask_ptr += vos->oPointBetweenLine;  /*skip border points*/
		  vol_ptr += vos->oPointBetweenLine;  /*skip border points*/
		}
	      mask_ptr += vos->oLineBetweenSlice; /*skip border lines*/
	      vol_ptr += vos->oLineBetweenSlice; /*skip border lines*/
	    }
    return(OK);	
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipDilateVolumeBorder(Volume *vol, Volume *mask, int T_GRAY_CSF, int T_WHITE_FAT, int connectivity)
{
    Vip_S16BIT *vol_ptr;
    Vip_S16BIT *mask_ptr;
    Vip_S16BIT *voisin;
    VipOffsetStruct *vos;
    VipConnectivityStruct *vcs;
    int icon;
    int ix, iy, iz;

    if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB || VipVerifyAll(mask)==PB)
    {
        VipPrintfExit("(VipDilateVolumeBorder");
        return(PB);
    }
    
    vos = VipGetOffsetStructure(vol);
    vcs = VipGetConnectivityStruct( vol, connectivity );

    vol_ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
    mask_ptr = VipGetDataPtr_S16BIT( mask ) + vos->oFirstPoint;

    for ( iz = mVipVolSizeZ(vol); iz-- ; )      /* loop on slices */
    {
        for ( iy = mVipVolSizeY(vol); iy-- ; )      /* loop on lines */
        {
            for ( ix = mVipVolSizeX(vol); ix-- ; )      /* loop on points */
            {
                if(!(*mask_ptr))
                {
                    for ( icon=0; icon<vcs->nb_neighbors; icon++ )
                    {
                        voisin = mask_ptr + vcs->offset[icon];
                        if(*voisin==255)
                        {
                            if(*vol_ptr>T_GRAY_CSF && *vol_ptr<T_WHITE_FAT)
                            {
                                *mask_ptr=512;
                                break;
                            }
                        }
                    }
                }
                mask_ptr++;
                vol_ptr++;
            }
            mask_ptr += vos->oPointBetweenLine;  /*skip border points*/
            vol_ptr += vos->oPointBetweenLine;  /*skip border points*/
        }
        mask_ptr += vos->oLineBetweenSlice; /*skip border lines*/
        vol_ptr += vos->oLineBetweenSlice; /*skip border lines*/
    }
    return(OK);
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
// VipHisto *VipCreateHistogram(Volume *vol, Volume *mask, Volume *edges, int connectivity)
int VipCreateHistogram(Volume *vol, Volume *mask, Volume *edges, int connectivity)
{
    VipHisto *histo_border, *histo_border_cumule;
    int volmax, volmin;
    VipOffsetStruct *vos;
    VipConnectivityStruct *vcs;
    Vip_S16BIT *vol_ptr, *mask_ptr;
    Vip_S16BIT *edges_ptr, *voisin;
    int histo_val, nb_total = 0;
    int icon, ix, iy, iz;
    float mean = 0, std = 0, mediane;
    
    if (VipVerifyAll(vol)==PB)
    {
        VipPrintfExit("(histo)ComputeVolumeHisto");
        return(PB);
    }
    if (VipTestType(vol,S16BIT)!=OK )
    {
        VipPrintfError("Sorry, ComputeVolumeHisto is only implemented for S16BIT volume");
        VipPrintfExit("(histo)ComputeVolumeHisto");
        return(PB);
    }
    
    volmax = (int)VipGetVolumeMax(vol) + 1;
    volmin = (int)VipGetVolumeMin(vol);
    if(volmin>0) volmin = 0;
    histo_border = VipCreateHisto(volmin,volmax);
    histo_border_cumule = VipCreateHisto(volmin,volmax);
    if(histo_border==PB || histo_border_cumule==PB) return(PB);
    printf("histomin=%d, histomax=%d\n", volmin, volmax);
    fflush(stdout);
    
    vos = VipGetOffsetStructure(vol);
    vcs = VipGetConnectivityStruct( vol, connectivity );
    
    vol_ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
    mask_ptr = VipGetDataPtr_S16BIT( mask ) + vos->oFirstPoint;
    edges_ptr = VipGetDataPtr_S16BIT( edges ) + vos->oFirstPoint;
    
    for ( iz = mVipVolSizeZ(vol); iz-- ; )    /* loop on slices */
    {
        for ( iy = mVipVolSizeY(vol); iy-- ; )    /* loop on lines */
        {
            for ( ix = mVipVolSizeX(vol); ix-- ; )    /* loop on points */
            {
//                 if(*mask_ptr==255)
//                 {
// //                     if(*mask_ptr<200)
// //                     {
//                         histo_val = *vol_ptr;
//                         histo_border->val[histo_val]++;
//                         mean += (float)(*vol_ptr);
//                         nb_total++;
// //                     }
//                 }
                if(!(*mask_ptr))
                {
                    for ( icon = 0; icon<vcs->nb_neighbors; icon++ )
                    {
                        voisin = mask_ptr + vcs->offset[icon];
                        if(*voisin==255)
                        {
                            if((*edges_ptr)>25 && (*edges_ptr)<500)
                            {
                                histo_val = *vol_ptr;
                                histo_border->val[histo_val]++;
                                mean += (float)(*vol_ptr);
                                nb_total++;
//                                 nb_border_total++;
                                break;
                            }
                        }
                    }
                }
                vol_ptr++;
                mask_ptr++;
                edges_ptr++;
            }
            vol_ptr += vos->oPointBetweenLine;  /*skip border points*/
            mask_ptr += vos->oPointBetweenLine;  /*skip border points*/
            edges_ptr += vos->oPointBetweenLine;  /*skip border points*/
        }
        vol_ptr += vos->oLineBetweenSlice; /*skip border lines*/
        mask_ptr += vos->oLineBetweenSlice; /*skip border lines*/
        edges_ptr += vos->oLineBetweenSlice; /*skip border lines*/
    }
    mean /= nb_total;
    printf("mean=%f\n", mean), fflush(stdout);

    histo_border_cumule->val[0] = histo_border->val[0];
    for ( icon=1; icon<volmax; icon++ )
    {
        histo_border_cumule->val[icon] = histo_border->val[icon] + histo_border_cumule->val[icon-1];
        if(histo_border_cumule->val[icon]>0.80*nb_total)
        {
            mediane = icon;
            break;
        }
    }
    printf("mediane=%f\n", mediane), fflush(stdout);

    vol_ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
    mask_ptr = VipGetDataPtr_S16BIT( mask ) + vos->oFirstPoint;
    edges_ptr = VipGetDataPtr_S16BIT( edges ) + vos->oFirstPoint;
    for ( iz = mVipVolSizeZ(vol); iz-- ; )    /* loop on slices */
    {
        for ( iy = mVipVolSizeY(vol); iy-- ; )    /* loop on lines */
        {
            for ( ix = mVipVolSizeX(vol); ix-- ; )    /* loop on points */
            {
//                 if(*mask_ptr==255)
//                 {
// //                     if(*mask_ptr<200)
// //                     {
//                         std += ((float)(*vol_ptr) - mean)*((float)(*vol_ptr) - mean);
// //                     }
//                 }
                if(!(*mask_ptr))
                {
                    for ( icon = 0; icon<vcs->nb_neighbors; icon++ )
                    {
                        voisin = mask_ptr + vcs->offset[icon];
                        if(*voisin==255)
                        {
                            if((*edges_ptr)>25 && (*edges_ptr)<500)
                            {
                                std += ((float)(*vol_ptr) - mean)*((float)(*vol_ptr) - mean);
                                break;
                            }
                        }
                    }
                }
                vol_ptr++;
                mask_ptr++;
                edges_ptr++;
            }
            vol_ptr += vos->oPointBetweenLine;  /*skip border points*/
            mask_ptr += vos->oPointBetweenLine;  /*skip border points*/
            edges_ptr += vos->oPointBetweenLine;  /*skip border points*/
        }
        vol_ptr += vos->oLineBetweenSlice; /*skip border lines*/
        mask_ptr += vos->oLineBetweenSlice; /*skip border lines*/
        edges_ptr += vos->oLineBetweenSlice; /*skip border lines*/
    }
    std = sqrt(std/nb_total);
    printf("sdt=%f\n", std), fflush(stdout);

//     return(histo_border);
    return(mediane);
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipVolumeEdges(Volume *edges, Vip_S16BIT *edges_ptr, int seuil)
{
    int icon;
    VipConnectivityStruct *vcs6, *vcs18;
    Vip_S16BIT *voisin1, *voisin2, *voisin3;

    if (VipVerifyAll(edges)==PB || VipTestType(edges,S16BIT)==PB)
    {
        VipPrintfExit("(VipVolumeEdges");
        return(PB);
    }

    if(*(edges_ptr)>seuil) return(1);

    vcs6 = VipGetConnectivityStruct( edges, CONNECTIVITY_6 );
    for ( icon = 0;icon<vcs6->nb_neighbors;icon++)
    {
        voisin1 = edges_ptr + vcs6->offset[icon];
        voisin2 = edges_ptr - vcs6->offset[icon];
        if((*voisin1)>seuil && (*voisin2)>seuil) return(1);
    }

    vcs18 = VipGetConnectivityStruct( edges, CONNECTIVITY_18 );
    voisin1 = edges_ptr + vcs18->offset[0];
    voisin2 = edges_ptr + vcs18->offset[11];
    voisin3 = edges_ptr + vcs18->offset[15];
    if(((*voisin1)>seuil && (*voisin2)>seuil) || ((*voisin1)>seuil && (*voisin3)>seuil)) return(1);
    voisin1 = edges_ptr + vcs18->offset[1];
    voisin2 = edges_ptr + vcs18->offset[9];
    voisin3 = edges_ptr + vcs18->offset[15];
    if(((*voisin1)>seuil && (*voisin2)>seuil) || ((*voisin1)>seuil && (*voisin3)>seuil)) return(1);
    voisin1 = edges_ptr + vcs18->offset[3];
    voisin2 = edges_ptr + vcs18->offset[8];
    voisin3 = edges_ptr + vcs18->offset[15];
    if(((*voisin1)>seuil && (*voisin2)>seuil) || ((*voisin1)>seuil && (*voisin3)>seuil)) return(1);
    voisin1 = edges_ptr + vcs18->offset[4];
    voisin2 = edges_ptr + vcs18->offset[6];
    voisin3 = edges_ptr + vcs18->offset[15];
    if(((*voisin1)>seuil && (*voisin2)>seuil) || ((*voisin1)>seuil && (*voisin3)>seuil)) return(1);
    voisin1 = edges_ptr + vcs18->offset[5];
    voisin2 = edges_ptr + vcs18->offset[9];
    voisin3 = edges_ptr + vcs18->offset[11];
    if(((*voisin1)>seuil && (*voisin2)>seuil) || ((*voisin1)>seuil && (*voisin3)>seuil)) return(1);
    voisin1 = edges_ptr + vcs18->offset[7];
    voisin2 = edges_ptr + vcs18->offset[8];
    voisin3 = edges_ptr + vcs18->offset[11];
    if(((*voisin1)>seuil && (*voisin2)>seuil) || ((*voisin1)>seuil && (*voisin3)>seuil)) return(1);
    voisin1 = edges_ptr + vcs18->offset[10];
    voisin2 = edges_ptr + vcs18->offset[9];
    voisin3 = edges_ptr + vcs18->offset[6];
    if(((*voisin1)>seuil && (*voisin2)>seuil) || ((*voisin1)>seuil && (*voisin3)>seuil)) return(1);
    voisin1 = edges_ptr + vcs18->offset[12];
    voisin2 = edges_ptr + vcs18->offset[6];
    voisin3 = edges_ptr + vcs18->offset[8];
    if(((*voisin1)>seuil && (*voisin2)>seuil) || ((*voisin1)>seuil && (*voisin3)>seuil)) return(1);
    voisin1 = edges_ptr + vcs18->offset[13];
    voisin2 = edges_ptr + vcs18->offset[2];
    voisin3 = edges_ptr + vcs18->offset[11];
    if(((*voisin1)>seuil && (*voisin2)>seuil) || ((*voisin1)>seuil && (*voisin3)>seuil)) return(1);
    voisin1 = edges_ptr + vcs18->offset[14];
    voisin2 = edges_ptr + vcs18->offset[9];
    voisin3 = edges_ptr + vcs18->offset[2];
    if(((*voisin1)>seuil && (*voisin2)>seuil) || ((*voisin1)>seuil && (*voisin3)>seuil)) return(1);
    voisin1 = edges_ptr + vcs18->offset[16];
    voisin2 = edges_ptr + vcs18->offset[2];
    voisin3 = edges_ptr + vcs18->offset[8];
    if(((*voisin1)>seuil && (*voisin2)>seuil) || ((*voisin1)>seuil && (*voisin3)>seuil)) return(1);
    voisin1 = edges_ptr + vcs18->offset[17];
    voisin2 = edges_ptr + vcs18->offset[2];
    voisin3 = edges_ptr + vcs18->offset[6];
    if(((*voisin1)>seuil && (*voisin2)>seuil) || ((*voisin1)>seuil && (*voisin3)>seuil)) return(1);

    return(0);
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipCreateBrainSeed(
Volume *vol,
Volume *var,
VipT1HistoAnalysis *ana,
int T_GRAY_WHITE,
int T_WHITE_FAT,
int SEUIL_VAR
)
{
    Volume *mask;

    VipDoubleThreshold( vol, VIP_BETWEEN_OR_EQUAL_TO, T_GRAY_WHITE, T_WHITE_FAT, BINARY_RESULT );

    mask = VipCreateSingleThresholdedVolume( var, LOWER_OR_EQUAL_TO, SEUIL_VAR, BINARY_RESULT );
    if(VipConnexVolumeFilter( mask, CONNECTIVITY_26, -1, CONNEX_BINARY )==PB) return(PB);

    if(VipMerge( vol, mask, VIP_MERGE_ONE_TO_ONE, 0, 0 )==PB) return(PB);

    return(OK);
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipPropagationConnectivity(
Volume *mask,
Volume *vol1,
Volume *vol2,
int mode,
int seuil_vol1,
int seuil_vol2,
int connectivity
)
{
    int ix, iy, iz, icon;
    VipOffsetStruct *vos;
    VipConnectivityStruct *vcs;
    Vip_S16BIT *vol1_ptr, *vol2_ptr;
    Vip_S16BIT *mask_ptr;
    Vip_S16BIT *voisin;

    if ((VipVerifyAll(vol1)==PB && VipVerifyAll(vol2)==PB) || VipVerifyAll(mask)==PB)
    {
        VipPrintfExit("(VipPropagationConnectivity");
        return(PB);
    }
  
    vos = VipGetOffsetStructure(mask);
    vcs = VipGetConnectivityStruct( mask, connectivity );
  
    if(vol1!=NULL) vol1_ptr = VipGetDataPtr_S16BIT( vol1 ) + vos->oFirstPoint;
    if(vol2!=NULL) vol2_ptr = VipGetDataPtr_S16BIT( vol2 ) + vos->oFirstPoint;
    mask_ptr = VipGetDataPtr_S16BIT( mask ) + vos->oFirstPoint;
  
    for ( iz = mVipVolSizeZ(mask); iz-- ; )     /* loop on slices */
    {
        for ( iy = mVipVolSizeY(mask); iy-- ; )     /* loop on lines */
        {
            for ( ix = mVipVolSizeX(mask); ix-- ; )     /* loop on points */
            {
                if(!(*mask_ptr))
                {
                    for (icon = 0;icon<vcs->nb_neighbors;icon++)
                    {
                        voisin = mask_ptr + vcs->offset[icon];
                        if(*voisin==255)
                        {
                            switch(mode)
                            {
                                case LOWER_THAN:
                                    if(*(vol1_ptr)<seuil_vol1 && *(vol2_ptr)==seuil_vol2) *mask_ptr=512;
                                    break;
                                case EQUAL_TO:
                                    if(*(vol1_ptr)==seuil_vol1 && *(vol2_ptr)==seuil_vol2) *mask_ptr=512;
                                    break;
                                case GREATER_THAN:
                                    if(*(vol1_ptr)>seuil_vol1 && *(vol2_ptr)==seuil_vol2) *mask_ptr=512;
                                    break;
                                default:
                                    VipPrintfError("Not implemented yet\n");
                                    return(VIP_CL_ERROR);
                            }
                        }
                    }
                }
                mask_ptr++;
                vol1_ptr++;
                vol2_ptr++;
            }
            mask_ptr += vos->oPointBetweenLine;  /*skip border points*/
            vol1_ptr += vos->oPointBetweenLine;  /*skip border points*/
            vol2_ptr += vos->oPointBetweenLine;  /*skip border points*/
        }
        mask_ptr += vos->oLineBetweenSlice; /*skip border lines*/
        vol1_ptr += vos->oLineBetweenSlice; /*skip border lines*/
        vol2_ptr += vos->oLineBetweenSlice; /*skip border lines*/
    }
    return(OK);
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipPropagationWithRefluxConnectivity(
Volume *mask,
Volume *vol,
Volume *variance,
int distance,
int mode,
int seuil1,
int seuil2,
int connectivity
)
{
    Volume *copymask;
    VipIntBucket *buck, *nextbuck;
    int i, m;
    VipConnectivityStruct *vcs;
    Vip_S16BIT *first, *ptr, *ptr_neighbor;
    Vip_S16BIT *vfirst, *vptr;
    int *buckptr;

    if ((VipVerifyAll(vol)==PB && VipVerifyAll(variance)==PB) || VipVerifyAll(mask)==PB)
    {
        VipPrintfExit("(VipPropagationConnectivity");
        return(PB);
    }

    copymask = VipCopyVolume( mask, "copymask" );
    
    VipChangeIntLabel( mask, 255, 512 );
    if(VipMerge(vol,mask,VIP_MERGE_ONE_TO_ONE,512,0)==PB) return(PB);
    if(VipMerge(mask,vol,VIP_MERGE_SAME_VALUES,0,0)==PB) return(PB);
    
    VipSetBorderLevel( mask, 0 ); /* already done before but security */

    buck = VipCreateFrontIntBucketHollowObject( mask, connectivity, VIP_FRONT, 512, 0);
    if(buck==PB) return(PB);
    nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
    if(nextbuck==PB) return(PB);

    vcs = VipGetConnectivityStruct( mask, connectivity );
  
    first = VipGetDataPtr_S16BIT(mask);
    vfirst = VipGetDataPtr_S16BIT(variance);

    for(m=0;m<distance;m++)
    {
        buckptr = buck->data;
        for(i=buck->n_points;i--;)
        {
            ptr = first + *buckptr;
            if(*ptr==VIP_FRONT)
            {
                vptr = vfirst + *buckptr;
                switch(mode)
                {
                    case LOWER_THAN:
                        if(*vptr<seuil1) *ptr=512;
                        break;
                    case EQUAL_TO:
                        if(*vptr==seuil1) *ptr=512;
                        break;
                    case GREATER_THAN:
                        if(*vptr>seuil1) *ptr=512;
                        break;
                    case VIP_BETWEEN:
                        if(*vptr>seuil1 && *vptr<seuil2) *ptr=512;
                        break;
                    default:
                        VipPrintfError("Not implemented yet\n");
                        return(VIP_CL_ERROR);
                }
            }
            buckptr++;
        }
        VipFillNextFrontFromOldFrontHollowObject(first,buck,nextbuck,vcs,VIP_FRONT,512, 0);

        /*bucket permutation*/
        VipPermuteTwoIntBucket(&buck, &nextbuck);
        nextbuck->n_points = 0;
    }
    
    /*Last scan to select the too big cc*/
    buckptr = buck->data;
    for(i=buck->n_points;i--;)
    {
        ptr = first + *buckptr;
        if(*ptr==VIP_FRONT)
        {
            vptr = vfirst + *buckptr;
            switch(mode)
            {
                case LOWER_THAN:
                    if(*vptr<seuil1) *ptr=640;
                    break;
                case EQUAL_TO:
                    if(*vptr==seuil1) *ptr=640;
                    break;
                case GREATER_THAN:
                    if(*vptr>seuil1) *ptr=640;
                    break;
                case VIP_BETWEEN:
                    if(*vptr>seuil1 && *vptr<seuil2) *ptr=640;
                    break;
                default:
                    VipPrintfError("Not implemented yet\n");
                    return(VIP_CL_ERROR);
            }
        }
        buckptr++;
    }
    
    VipFreeIntBucket(buck);
    VipFreeIntBucket(nextbuck);
    
//     VipFreeVolume(mask);
//     mask = VipCreateSingleThresholdedVolume(vol,GREATER_THAN,255,GREYLEVEL_RESULT);
    if(VipSingleThreshold( mask, GREATER_THAN, 255, GREYLEVEL_RESULT )==PB) return(PB);
    VipChangeIntLabel( mask, 512, 255 );
    
    if(VipMerge( mask, copymask, VIP_MERGE_ONE_TO_ONE, 255, 0 ) == PB) return(PB);
    
    VipChangeIntLabel( mask, 0, -100 );

    if(VipComputeFrontPropagationConnectivityDistanceMap(mask,255,-100,VIP_NO_LIMIT_IN_PROPAGATION,(distance + 1),connectivity)==PB) return(PB);

    if(VipSingleThreshold( mask, GREATER_OR_EQUAL_TO, 10000, BINARY_RESULT )==PB) return(PB);
    
    return(OK);
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipPropagationReflux(
Volume *mask,
Volume *vol1,
Volume *vol2,
int nb_iterations,
int mode,
int seuil_vol1,
int seuil_vol2,
int connectivity
)
{
    Volume *copymask;
    int i;

    copymask = VipCopyVolume( mask, "copymask" );

    for(i=0;i<nb_iterations;i++)
    {
        VipPropagationConnectivity(mask, vol1, vol2, mode, seuil_vol1, seuil_vol2, connectivity);
        VipSingleThreshold( mask, GREATER_OR_EQUAL_TO, 1, BINARY_RESULT );
    }
    VipPropagationConnectivity(mask, vol1, vol2, mode, seuil_vol1, seuil_vol2, connectivity);

    if(VipMerge( mask, copymask, VIP_MERGE_ONE_TO_ONE, 255, 0 ) == PB) return(PB);

    VipChangeIntLabel( mask, 0, -100 );

    if(VipComputeFrontPropagationConnectivityDistanceMap(mask,255,-100,VIP_NO_LIMIT_IN_PROPAGATION,(nb_iterations + 1),CONNECTIVITY_26)==PB) return(PB);

    if(VipSingleThreshold( mask, GREATER_OR_EQUAL_TO, 10000, BINARY_RESULT )==PB) return(PB);

    if(VipMerge( mask, copymask, VIP_MERGE_ONE_TO_ONE, 255, 255 )== PB) return(PB);

    if(VipConnexVolumeFilter( mask, CONNECTIVITY_26, -1, CONNEX_BINARY ) == PB) return(PB);

    return(OK);
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipGetBrain2010(
Volume *vol,
Volume *variance,
Volume *edges,
Volume *ridge,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float brain_erosion_size,
int variance_threshold,
int nb_iterations,
float CA[3],
float CP[3],
float P[3]
)
/*---------------------------------------------------------------------------*/
{

  Volume *brain=NULL, *copyvol=NULL, *distmap=NULL, *classif=NULL;
  Volume *skin=NULL, *brainball=NULL;
  Volume *var=NULL, *mask=NULL, *mask2=NULL, *white=NULL;
  int i, nb_clara;
  int T_VOID_GRAY_1 = 0, T_VOID_GRAY_2 = 0, T_VOID_GRAY_LOW = 0, T_VOID_GRAY_HIGH = 0;
  int T_GRAY_WHITE_LOW = 0, T_GRAY_WHITE_HIGH = 0, T_GRAY_WHITE_SECUREGRAY;
  int T_WHITE_FAT_LOW_1 = 0, T_WHITE_FAT_LOW_2 = 0, T_WHITE_FAT_HIGH = 0;
  int T_GRAY_CSF = 0, nb_total = 0;
  float little_opening_size;
  int var_seuil;
  int icon, ix, iy, iz;
  int goodseed;
  VipOffsetStruct *vos;
  VipConnectivityStruct *vcs;
  Vip_S16BIT *skin_ptr, *mask_ptr;
  Vip_S16BIT *vol_ptr, *edges_ptr, *voisin;
  Vip_S16BIT *ptr;
  int seuil = 0;
  VipHisto *histo;
  char histoname[1024];
  float ptPlanHemi[3];
  float d[3], pt = 0.;
  float mean=0;
  float sigma = 0;
  
  
  dumb=VFALSE;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("VipGetBrain2010");
      return(PB);
    }
  if(!ana)
    {
      VipPrintfError("Null VipT1HistoAnalysis!");
      VipPrintfExit("VipGetBrain2010");
      return(PB);
    }

  if(ana->gray!=NULL && ana->white!=NULL)
    {
        T_VOID_GRAY_1 = ana->gray->mean - 0.2*ana->gray->sigma;
        T_VOID_GRAY_2 = ana->gray->mean + 0.2*(ana->white->mean - ana->gray->mean);
	T_VOID_GRAY_LOW = ana->gray->mean - 2.5*ana->gray->sigma;
	T_VOID_GRAY_HIGH = ana->gray->mean - 1.7*ana->gray->sigma;
	T_GRAY_WHITE_LOW = ana->gray->mean + (ana->white->mean - ana->gray->mean)/2;
        T_GRAY_WHITE_HIGH = ana->gray->mean + 0.75*(ana->white->mean - ana->gray->mean);
	T_GRAY_WHITE_SECUREGRAY = ana->gray->mean + 2*ana->gray->sigma;
        T_WHITE_FAT_LOW_1 = ana->white->mean + 2.5*ana->white->sigma;
	T_WHITE_FAT_LOW_2 = ana->white->mean + 4*ana->white->sigma;
	T_WHITE_FAT_HIGH = ana->white->mean + 6*ana->white->sigma;
    }
  else 
    {
      VipPrintfError("Bad Histogram Analysis!");
      return(PB);
    }

  copyvol = VipCopyVolume(vol,"volcopy");

/*------ Classification => Masque de la tete ------*/
  classif = VipCSFGrayWhiteFatClassificationRegularisationForRobustApproach( vol, ana, dumb, nb_iterations, T_VOID_GRAY_LOW, T_VOID_GRAY_HIGH, T_WHITE_FAT_LOW_2, T_WHITE_FAT_HIGH, T_GRAY_WHITE_LOW);
  if(classif==PB) return(PB);

  brain = vol;
  VipSetVolumeLevel(brain, 0);
  if(VipMerge(brain, classif, VIP_MERGE_ONE_TO_ONE, BRAIN_LABEL, 255) == PB) return(PB);

  VipFreeVolume(classif);
  classif=NULL;

  little_opening_size = 0.9;
  if(mVipVolVoxSizeX(brain)>little_opening_size) little_opening_size=mVipVolVoxSizeX(brain)+0.1;
  if(mVipVolVoxSizeY(brain)>little_opening_size) little_opening_size=mVipVolVoxSizeY(brain)+0.1;
  if(mVipVolVoxSizeZ(brain)>little_opening_size) little_opening_size=mVipVolVoxSizeZ(brain)+0.1;
  printf("little_opening_size=%f\n", little_opening_size), fflush(stdout);

//   if(VipOpening( brain, CHAMFER_BALL_3D, little_opening_size )==PB) return(PB);
  if(VipConnexVolumeFilter( brain, CONNECTIVITY_6, -1, CONNEX_BINARY ) == PB) return(PB);


/*------ Creation de la graine du cerveau ------*/
//----Correction de l'image de variance
  classif = VipCreateSingleThresholdedVolume(copyvol,GREATER_THAN,mVipMax(mVipMin(ana->gray->mean - 5*ana->gray->sigma,ana->gray->mean/2),20),BINARY_RESULT);
  if( VipConnexVolumeFilter( classif, CONNECTIVITY_26, -1, CONNEX_BINARY) == PB) return(PB);
  if(VipMerge(variance, classif, VIP_MERGE_ONE_TO_ONE, 0, VipGetVolumeMax(variance)) == PB) return(PB);
  
//----Creation du volume skin
  skin = VipCopyVolume(classif,"skin");
  mask = VipCopyVolume(classif,"skin2");
  VipExtedge(skin,EXTEDGE2D_ALL_EXCEPT_Y_BOTTOM,SAME_VOLUME);
  VipExtedge(mask,EXTEDGE2D_ALL_EXCEPT_Y_TOP,SAME_VOLUME);
  if(VipMerge(skin, mask, VIP_MERGE_ONE_TO_ONE, 0, 0) == PB) return(PB);

  if(VipConnexVolumeFilter( skin, CONNECTIVITY_18, -1, CONNEX_BINARY ) == PB) return(PB);
  if(VipDilation( skin, CHAMFER_BALL_2D, 4 ) == PB) return(PB);

  VipFreeVolume(classif);
  VipFreeVolume(mask);
  classif=NULL;
  mask=NULL;

//----Creation du noyau de matiere blanche
  white = VipCopyVolume(copyvol, "white");
  classif = VipGrayWhiteClassificationRegularisationForRobust2005( copyvol, ana, T_VOID_GRAY_LOW, T_VOID_GRAY_HIGH, ana->gray->mean + ana->gray->right_sigma, ana->white->mean - ana->white->left_sigma, ana->white->mean + 3*ana->white->sigma, dumb);
//   classif = VipGrayWhiteClassificationRegularisation( copyvol, ana, dumb);
  VipSetVolumeLevel(white,0);
  if(VipMerge(white,classif,VIP_MERGE_ONE_TO_ONE,WHITE_LABEL,255) == PB) return(PB);
// // //   VipDoubleThreshold( white, VIP_BETWEEN_OR_EQUAL_TO, ana->white->mean - ana->white->left_sigma, ana->white->mean + 3*ana->white->sigma, BINARY_RESULT );
//   if(VipConnexVolumeFilter( white, CONNECTIVITY_18, -1, CONNEX_BINARY )==PB) return(PB);

  VipFreeVolume(classif);
  classif=NULL;

//   classif = VipCSFGrayWhiteFatClassificationRegularisationForRobustApproach( copyvol, ana, dumb, nb_iterations, T_VOID_GRAY_1, T_VOID_GRAY_2, T_WHITE_FAT_LOW_2, T_WHITE_FAT_HIGH, T_GRAY_WHITE_LOW);
//   VipSetVolumeLevel(white, 0);
//   VipMerge( white, classif, VIP_MERGE_ONE_TO_ONE, BRAIN_LABEL, 255 );
//   VipSingleThreshold( white, EQUAL_TO, 0, BINARY_RESULT );
//   VipComputeFrontPropagationChamferDistanceMap(white,0,-1,VIP_PUT_LIMIT_TO_INFINITE,6);
//   VipWriteVolume(white,"distmap2010bis");
//   VipSingleThreshold( white, GREATER_OR_EQUAL_TO, (int)(1.3*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT );
//   VipConnexVolumeFilter( white, CONNECTIVITY_26, -1, CONNEX_BINARY );

//----Creation du masque "brain" erode
  distmap = VipCreateSingleThresholdedVolume(brain,EQUAL_TO,0,BINARY_RESULT);
  VipComputeFrontPropagationChamferDistanceMap( distmap, 0, -1, VIP_PUT_LIMIT_TO_INFINITE, 6 );

  classif = VipCreateSingleThresholdedVolume( distmap, GREATER_OR_EQUAL_TO, (int)(brain_erosion_size*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT );
  if(VipConnexVolumeFilter( classif, CONNECTIVITY_26, -1, CONNEX_BINARY ) == PB) return(PB);

  if(VipMerge( white, classif, VIP_MERGE_ONE_TO_ONE, 0, 0 ) == PB) return(PB);
  if(VipConnexVolumeFilter( white, CONNECTIVITY_26, -1, CONNEX_BINARY ) == PB) return(PB);

  VipComputeRobustStatInMaskVolume(variance,white, &mean, &sigma, VFALSE);
  printf("mean: %f, sigma: %f\n", mean, sigma), fflush(stdout);

//----Creation du noyau grace a la variance et l'intensite

  var_seuil = (int)(mean + sigma + 0.5);
  if (var_seuil<10) var_seuil = 10;
  
  mask2 = VipCopyVolume(copyvol, "seed2");

  if((ana->white->mean-2*ana->white->left_sigma) > T_GRAY_WHITE_LOW)
  {
      if(VipCreateBrainSeed( mask2, variance, ana, ana->white->mean-2*ana->white->left_sigma, T_WHITE_FAT_LOW_1, 5 ) == PB) return(PB);
  }
  else VipCreateBrainSeed( mask2, variance, ana, T_GRAY_WHITE_LOW, T_WHITE_FAT_LOW_1, 5 );
  if(VipConnexVolumeFilter( mask2, CONNECTIVITY_26, 200, CONNEX_BINARY ) == PB) return(PB);

  vos = VipGetOffsetStructure(skin);

  goodseed=VFALSE;
  while(goodseed==VFALSE)
  {
      mask = VipCopyVolume(copyvol, "seed");
      if(VipMerge( mask, white, VIP_MERGE_ONE_TO_ONE, 0, 0 ) == PB) return(PB);
      if(VipCreateBrainSeed( mask, variance, ana, T_GRAY_WHITE_HIGH, T_WHITE_FAT_LOW_1, var_seuil ) == PB) return(PB);
      if(VipMerge( mask, mask2, VIP_MERGE_ONE_TO_ONE, 255, 255 ) == PB) return(PB);
      
      if(VipConnexVolumeFilter( mask, CONNECTIVITY_26, -1, CONNEX_BINARY ) == PB) return(PB);
      
      skin_ptr = VipGetDataPtr_S16BIT( skin ) + vos->oFirstPoint;
      mask_ptr = VipGetDataPtr_S16BIT( mask ) + vos->oFirstPoint;
      
      goodseed=VTRUE;
      for ( iz = mVipVolSizeZ(skin); iz-- ; )               /* loop on slices */
      {
          for ( iy = mVipVolSizeY(skin); iy-- ; )            /* loop on lines */
          {
              for ( ix = mVipVolSizeX(skin); ix-- ; )           /* loop on points */
              {
                  if((*mask_ptr) && (*skin_ptr)) 
                  {
                      goodseed=VFALSE;
                  }
                  mask_ptr++;
                  skin_ptr++;
              }
              if(goodseed==VFALSE) break;
              mask_ptr += vos->oPointBetweenLine;  /*skip border points*/
              skin_ptr += vos->oPointBetweenLine;  /*skip border points*/
          }
          if(goodseed==VFALSE) break;
          mask_ptr += vos->oLineBetweenSlice; /*skip border lines*/
          skin_ptr += vos->oLineBetweenSlice; /*skip border lines*/
      }

      if(goodseed==VTRUE)
      {
          goodseed=VTRUE;
          VipFreeVolume(skin);
          skin = NULL;
          VipFreeVolume(mask2);
          mask2 = NULL;
          printf("Applied variance seuil:%d\n",var_seuil);
          fflush(stdout);
      }
      else if(var_seuil<5)
      {
          goodseed=VTRUE;
          VipFreeVolume(skin);
          skin = NULL;
          VipFreeVolume(mask2);
          mask2 = NULL;
          printf("Coudn't applied a variance seuil\n");
          fflush(stdout);
      }
      else
      {
          VipFreeVolume(mask);
          var_seuil --;
      }
  }
  if(var_seuil<5) return(PB);
  
/*------ Creation du masque "brain" ------*/

  mask2 = VipCopyVolume(mask, "test");

  if( VipMerge( white, mask2, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);
  VipChangeIntLabel( white, 0, -2 );
  if(VipComputeFrontPropagationChamferDistanceMap(white,255,-2,VIP_PUT_LIMIT_TO_LIMIT,10)==PB) return(PB);

  VipFreeVolume(mask);
  mask = NULL;

  mask = VipCreateDoubleThresholdedVolume( white, VIP_BETWEEN_OR_EQUAL_TO, 0, (int)(1.2*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT );
  VipSingleThreshold( white, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT );

  skin = VipCopyVolume(white, "white_fin");
  if( VipMerge(skin, mask, VIP_MERGE_ONE_TO_ONE, 255, 0)== PB) return(PB);

  VipComputeRobustStatInMaskVolume(variance,skin, &mean, &sigma, VFALSE);
  printf("mean: %f, sigma: %f\n", mean, sigma);
  fflush(stdout);

  //Recuperation de la matiere blanche des gyri fins
  var = VipCreateSingleThresholdedVolume( variance, LOWER_THAN, (int)(1.5*mean+0.5), BINARY_RESULT );
//    if( VipOpening( var, CHAMFER_BALL_3D, little_opening_size )==PB) return(PB);
  if( VipConnexVolumeFilter( var, CONNECTIVITY_6, -1, CONNEX_BINARY ) == PB) return(PB);

  VipFreeVolume(mask2);
  mask2 = NULL;
  mask2 = VipCopyVolume(mask, "test2");
  
// // //   for(i=0;i<12;i++)
// // //   {
// // //       VipPropagationConnectivity( mask, var, white, EQUAL_TO, 255, 255, CONNECTIVITY_26 );
// // //       VipSingleThreshold( mask, GREATER_OR_EQUAL_TO, 1, BINARY_RESULT );
// // //   }
// // //   VipPropagationConnectivity( mask, var, white, EQUAL_TO, 255, 255, CONNECTIVITY_26 );
// // //   if(VipMerge( mask, mask2, VIP_MERGE_ONE_TO_ONE, 255, 0 ) == PB) return(VIP_CL_ERROR);
// // //   VipChangeIntLabel(mask,0,-100);
// // //   if(VipComputeFrontPropagationConnectivityDistanceMap(mask,255,-100,VIP_NO_LIMIT_IN_PROPAGATION,13,CONNECTIVITY_26)==PB) return(PB);
// // //   /*  VipWriteVolume(mask, "distmap_white");  */
// // //   VipSingleThreshold( mask, GREATER_OR_EQUAL_TO, 10000, BINARY_RESULT );
  
  VipPropagationWithRefluxConnectivity( mask, white, var, 12, EQUAL_TO, 255, 0, CONNECTIVITY_26 );
  
  if( VipMerge( mask, mask2, VIP_MERGE_ONE_TO_ONE, 255, 255 )== PB) return(PB);
  
  if( VipConnexVolumeFilter( mask, CONNECTIVITY_26, -1, CONNEX_BINARY ) == PB) return(PB);

  VipFreeVolume(mask2);
  mask2 = NULL;
  VipFreeVolume(white);
  white=NULL;
  VipFreeVolume(var);

  /*
    histo = VipCreateHistogram(variance, mask, edges, CONNECTIVITY_26);
    if(histo==PB) return(PB);
  */
  
//----Dilatation avec deux tailles differentes en avant et en arriere du cerveau
  if( VipMerge( classif, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);
  VipChangeIntLabel(classif,0,-2);
  if(VipComputeFrontPropagationChamferDistanceMap(classif,255,-2,VIP_PUT_LIMIT_TO_LIMIT,10)==PB) return(PB);

  VipFreeVolume(mask);
  mask = NULL;
  
//   mask = VipCreateSingleThresholdedVolume( brain, GREATER_OR_EQUAL_TO, (int)(6*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT);
  mask = VipCreateDoubleThresholdedVolume( classif, VIP_BETWEEN_OR_EQUAL_TO, 0, (int)(2.5*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT );
  mask2 = VipCreateDoubleThresholdedVolume( classif, VIP_BETWEEN_OR_EQUAL_TO, 0, (int)(2.9*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT );

  VipSingleThreshold( classif, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT );

  seuil = 0.5*(CP[1] + CA[1]);
  
  vos = VipGetOffsetStructure( mask2 );
  ptr = VipGetDataPtr_S16BIT( mask2 ) + vos->oFirstPoint + seuil*vos->oLine;
    for ( iz = mVipVolSizeZ(mask2); iz-- ; )   /* loop on slices */
  {
      for ( iy = (mVipVolSizeY(mask2) - seuil) ; iy-- ; )  /* loop on lines */
      {
            for ( ix = mVipVolSizeX(mask2); ix-- ; )   /* loop on points */
          {
              *ptr = 0;
              ptr++;
          }
            ptr += vos->oPointBetweenLine;  /*skip border points*/
      }
      ptr = ptr + vos->oLineBetweenSlice + seuil*vos->oLine; /*skip border lines*/
  }

  if( VipMerge( mask, mask2, VIP_MERGE_ONE_TO_ONE, 255, 255 )== PB) return(PB);

  VipFreeVolume(mask2);
  mask2 = NULL;

//----Recuperation des noyaux gris et des endroits ou la matiere grise est tres epaisse
  mask2 = VipCopyVolume(mask, "brainmask");

  skin = VipCopyVolume(mask, "brainball");
  if( VipClosing( skin, CHAMFER_BALL_3D, 15 )== PB) return(PB);
  if( VipErosion( skin, CHAMFER_BALL_3D, 1 )== PB) return(PB);

  brainball = VipCopyVolume(skin, "brainballbis");
  VipErosion( brainball, CHAMFER_BALL_3D, 9 );
  
  d[0] = d[1] = d[2] = 1000.0;
  Vip3DPlanesResolution(CA, CP, P, d, &(ptPlanHemi[0]), &(ptPlanHemi[1]), &(ptPlanHemi[2]));

  vos = VipGetOffsetStructure(skin);
  ptr = VipGetDataPtr_S16BIT( skin ) + vos->oFirstPoint + (int)(CP[1])*vos->oLine;
  for ( iz = mVipVolSizeZ(skin); iz-- ; )   /* loop on slices */
  {
      for ( iy = (mVipVolSizeY(skin) - (int)(CP[1])) ; iy-- ; )  /* loop on lines */
      {
          for ( ix = mVipVolSizeX(skin); ix-- ; )   /* loop on points */
          {
              pt = ptPlanHemi[0]*(ix) + ptPlanHemi[1]*(iy) + ptPlanHemi[2]*(iz) - 1000.0;
              if(-180<(int)(pt) && (int)(pt)<180) *ptr = 0;
              ptr++;
          }
          ptr = ptr + vos->oPointBetweenLine;  /*skip border points*/
      }
      ptr = ptr + vos->oLineBetweenSlice + (int)(CP[1])*vos->oLine; /*skip border lines*/
  }
  
  if( VipMerge( skin, brainball, VIP_MERGE_ONE_TO_ONE, 255, 255 )== PB) return(PB);
  
  if( VipMerge( classif, skin, VIP_MERGE_ONE_TO_ONE, 0, 0 )== PB) return(PB);
  
  var = VipCreateSingleThresholdedVolume( variance, LOWER_THAN, 30, BINARY_RESULT );
  if(var==PB) return(PB);
  if( VipErosion( var, CHAMFER_BALL_3D, little_opening_size )== PB) return(PB);
  if( VipConnexVolumeFilter( var, CONNECTIVITY_6, -1, CONNEX_BINARY ) == PB) return(PB);
  if( VipDilation( var, CHAMFER_BALL_3D, little_opening_size )== PB) return(PB);

// // //   for(i=0;i<15;i++)
// // //   {
// // //       VipPropagationConnectivity( mask, var, classif, EQUAL_TO, 255, 255, CONNECTIVITY_26 );
// // //       VipSingleThreshold( mask, GREATER_OR_EQUAL_TO, 1, BINARY_RESULT );
// // //   }
// // //   VipPropagationConnectivity( mask, var, classif, EQUAL_TO, 255, 255, CONNECTIVITY_26 );
// // //   if(VipMerge( mask, mask2, VIP_MERGE_ONE_TO_ONE, 255, 0 ) == PB) return(VIP_CL_ERROR);
// // //   VipChangeIntLabel(mask,0,-100);
// // //   if(VipComputeFrontPropagationConnectivityDistanceMap(mask,255,-100,VIP_NO_LIMIT_IN_PROPAGATION,16,CONNECTIVITY_26)==PB) return(PB);
// // //   VipSingleThreshold( mask, GREATER_OR_EQUAL_TO, 10000, BINARY_RESULT );

  VipPropagationWithRefluxConnectivity( mask, classif, var, 15, EQUAL_TO, 255, 0, CONNECTIVITY_26 );
  
  if( VipMerge( mask, mask2, VIP_MERGE_ONE_TO_ONE, 255, 255 )== PB) return(PB);
  if( VipConnexVolumeFilter( mask, CONNECTIVITY_6, -1, CONNEX_BINARY ) == PB) return(PB);
  
  VipFreeVolume(brainball);
  brainball = NULL;
  VipFreeVolume(var);
  var = NULL;
  
  if( VipMerge( mask, mask2, VIP_MERGE_ONE_TO_ONE, 255, 0 )== PB) return(PB);
//   VipHysteresisThresholding( mask2, CONNECTIVITY_26, 10, CONNEX_BINARY, 0, 0, HYSTE_NUMBER, 1);
  if( VipMerge( mask, skin, VIP_MERGE_ONE_TO_ONE, 0, 0 )== PB) return(PB);
  if( VipMerge( classif, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);
  VipChangeIntLabel(classif,0,-2);
  if(VipComputeFrontPropagationChamferDistanceMap(classif,255,-2,VIP_PUT_LIMIT_TO_LIMIT,4)==PB) return(PB);
  
  VipFreeVolume(mask);
  VipFreeVolume(skin);
  mask = NULL;
  skin = NULL;
  
  mask = VipCreateDoubleThresholdedVolume( classif, VIP_BETWEEN_OR_EQUAL_TO, 0, (int)(2.5*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT);
  if( VipMerge( mask, mask2, VIP_MERGE_ONE_TO_ONE, 255, 255 )== PB) return(PB);
  VipSingleThreshold( classif, GREATER_OR_EQUAL_TO, 0, BINARY_RESULT );
  
  VipFreeVolume(mask2);
  mask2 = NULL;

//----Recuperation des gyri manquants
//   mask2 = VipCopyVolume(mask, "brainmask");
// 
// //   seuil = yCA - 0.8*(yCP - yCA);
//   skin = VipCopyVolume( mask, "half" );
//   VipClosing( skin, CHAMFER_BALL_3D, 25 );
//   VipDilation( skin, CHAMFER_BALL_3D, 8 );
//   
//   brainball = VipCopyVolume(skin, "brainballbis");
//   VipErosion( brainball, CHAMFER_BALL_3D, 14 );
// 
//   vos = VipGetOffsetStructure(skin);
//   ptr = VipGetDataPtr_S16BIT( skin ) + vos->oFirstPoint + yCP*vos->oLine + (xCP - 15);
//   for ( iz = mVipVolSizeZ(skin); iz-- ; )   /* loop on slices */
//   {
//         for ( iy = (mVipVolSizeY(skin) - yCP) ; iy-- ; )  /* loop on lines */
//       {
//             for ( ix = 30; ix-- ; )   /* loop on points */
//           {
//               *ptr = 0;
//               ptr++;
//           }
//           ptr = ptr + vos->oPointBetweenLine + (mVipVolSizeX(skin) - 30);  /*skip border points*/
//       }
//         ptr = ptr + vos->oLineBetweenSlice + yCP*vos->oLine; /*skip border lines*/
//   }
//   VipMerge( skin, brainball, VIP_MERGE_ONE_TO_ONE, 255, 255 );
//   VipWriteVolume(skin,"front");
//   
//   for(i=0;i<5;i++)
//   {
//       VipPropagationConnectivity( mask, skin, classif, EQUAL_TO, 255, 255, CONNECTIVITY_26 );
//       VipSingleThreshold( mask, GREATER_OR_EQUAL_TO, 1, BINARY_RESULT );
//   }
//   VipPropagationConnectivity( mask, skin, classif, EQUAL_TO, 255, 255, CONNECTIVITY_26 );
//   VipWriteVolume(mask,"mask");
//   if(VipMerge( mask, mask2, VIP_MERGE_ONE_TO_ONE, 255, 0 ) == PB) return(VIP_CL_ERROR);
//   VipChangeIntLabel(mask,0,-100);
//   if(VipComputeFrontPropagationConnectivityDistanceMap(mask,255,-100,VIP_NO_LIMIT_IN_PROPAGATION,6,CONNECTIVITY_26)==PB) return(PB);
//   VipSingleThreshold( mask, GREATER_OR_EQUAL_TO, 10000, BINARY_RESULT );
//   if( VipMerge( mask, mask2, VIP_MERGE_ONE_TO_ONE, 255, 255 )== PB) return(VIP_CL_ERROR);
// 
//   VipFreeVolume(mask2);
//   mask2 = NULL;
//   VipFreeVolume(skin);
//   skin = NULL;
//   VipFreeVolume(brainball);
//   brainball = NULL;

  VipFreeVolume(classif);

//----Premiere dilatation dans le masque erode
  classif = VipCreateSingleThresholdedVolume( distmap, GREATER_OR_EQUAL_TO, (int)((brain_erosion_size-0.5)*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT );
  if( VipConnexVolumeFilter( classif, CONNECTIVITY_26, -1, CONNEX_BINARY ) == PB) return(PB);
  
  if( VipConnexVolumeFilter( mask, CONNECTIVITY_26, -1, CONNEX_BINARY ) == PB) return(PB);
  if( VipMerge( classif, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);
  VipChangeIntLabel(classif,0,-2);
  
  VipFreeVolume(mask);
  mask = NULL;
  
  if(VipComputeFrontPropagationChamferDistanceMap(classif,255,-2,VIP_PUT_LIMIT_TO_LIMIT,6)==PB) return(PB);

  mask = VipCreateSingleThresholdedVolume( classif, GREATER_OR_EQUAL_TO, (int)(2.8*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT);
  VipSingleThreshold( classif, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT );

  if( VipMerge( classif, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);

  VipFreeVolume(mask);
  mask = NULL;

  VipChangeIntLabel(classif,0,-100);

  if(VipComputeFrontPropagationChamferDistanceMap(classif,255,-100,VIP_PUT_LIMIT_TO_LIMIT,6)==PB) return(PB);

  mask = VipCreateSingleThresholdedVolume( classif, GREATER_OR_EQUAL_TO, (int)(4*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT ); // Av commit 2
  VipSingleThreshold( classif, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT );
  
  if( VipConnexVolumeFilter( mask, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);

//----Recuperation des gyri tres fins manquants
  var = VipCreateSingleThresholdedVolume( variance, LOWER_THAN, 30, BINARY_RESULT );
  if( VipConnexVolumeFilter( var, CONNECTIVITY_6, -1, CONNEX_BINARY ) == PB) return(PB);

  if( VipMerge( classif, var, VIP_MERGE_ONE_TO_ONE, 0, 0 )== PB) return(PB);
  
  mask2 = VipCopyVolume(mask, "brainmask");

// // //   for(i=0;i<5;i++)
// // //   {
// // //       VipPropagationConnectivity( mask, var, classif, EQUAL_TO, 255, 255, CONNECTIVITY_26 );
// // //       VipSingleThreshold( mask, GREATER_OR_EQUAL_TO, 1, BINARY_RESULT );
// // //   }
// // //   VipPropagationConnectivity( mask, var, classif, EQUAL_TO, 255, 255, CONNECTIVITY_26 );
// // //   if(VipMerge( mask, mask2, VIP_MERGE_ONE_TO_ONE, 255, 0 ) == PB) return(VIP_CL_ERROR);
// // //   VipChangeIntLabel(mask,0,-100);
// // //   if(VipComputeFrontPropagationConnectivityDistanceMap(mask,255,-100,VIP_NO_LIMIT_IN_PROPAGATION,6,CONNECTIVITY_26)==PB) return(PB);
// // // //   VipWriteVolume(mask,"brainmaskb");
// // //   VipSingleThreshold( mask, GREATER_OR_EQUAL_TO, 10000, BINARY_RESULT );

  VipPropagationWithRefluxConnectivity( mask, classif, edges, 5, GREATER_THAN, -1, 0, CONNECTIVITY_26 ); /*A tester*/

  VipHysteresisThresholding( mask, CONNECTIVITY_26, 5, CONNEX_BINARY, 0, 0, HYSTE_NUMBER, 1);
  if( VipMerge( mask, mask2, VIP_MERGE_ONE_TO_ONE, 255, 255 )== PB) return(PB);
  if( VipConnexVolumeFilter( mask, CONNECTIVITY_26, -1, CONNEX_BINARY ) == PB) return(PB);

  VipFreeVolume(var);
  VipFreeVolume(mask2);
  mask2 = NULL;
  var = NULL;

//----Deuxieme dilatation dans le masque final
  if( VipMerge( brain, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);
  VipChangeIntLabel(brain,0,-2);
  
  VipFreeVolume(mask);
  mask = NULL;
  
  if(VipComputeFrontPropagationChamferDistanceMap(brain,255,-2,VIP_PUT_LIMIT_TO_LIMIT,(brain_erosion_size+4))==PB) return(PB);
  
  mask = VipCreateSingleThresholdedVolume( brain, GREATER_OR_EQUAL_TO, (int)((brain_erosion_size+3)*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT);
  VipSingleThreshold( brain, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT );
  
  if( VipMerge( brain, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);

  VipFreeVolume(mask);
  mask = NULL;

  VipChangeIntLabel(brain,0,-100);

  if(VipComputeFrontPropagationChamferDistanceMap(brain,255,-100,VIP_PUT_LIMIT_TO_LIMIT,5)==PB) return(PB);

  VipSingleThreshold( brain, GREATER_OR_EQUAL_TO, (int)(3*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT );
  
  if( VipConnexVolumeFilter( brain, CONNECTIVITY_6, -1, CONNEX_BINARY)== PB) return(PB);
  
//----Recuperation de la matiere grise manquante en frontale
//   vos = VipGetOffsetStructure(vol);
//   vcs = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );
//   vol_ptr = VipGetDataPtr_S16BIT( copyvol ) + vos->oFirstPoint;
//   mask_ptr = VipGetDataPtr_S16BIT( brain ) + vos->oFirstPoint;
//   edges_ptr = VipGetDataPtr_S16BIT( edges ) + vos->oFirstPoint;
//   
//   mean=0;
//   sigma=0;
//   for ( iz = mVipVolSizeZ(vol); iz-- ; )    /* loop on slices */
//   {
//       for ( iy = mVipVolSizeY(vol); iy-- ; )    /* loop on lines */
//       {
//           for ( ix = mVipVolSizeX(vol); ix-- ; )    /* loop on points */
//           {
//               if(!(*mask_ptr))
//               {
//                   for ( icon = 0; icon<vcs->nb_neighbors; icon++ )
//                   {
//                       voisin = mask_ptr + vcs->offset[icon];
//                       if(*voisin==255)
//                       {
//                           if((*edges_ptr)>25 && (*edges_ptr)<500)
//                           {
//                               mean += *vol_ptr;
//                               nb_total++;
//                               break;
//                           }
//                       }
//                   }
//               }
//               vol_ptr++;
//               mask_ptr++;
//               edges_ptr++;
//           }
//           vol_ptr += vos->oPointBetweenLine;  /*skip border points*/
//           mask_ptr += vos->oPointBetweenLine;  /*skip border points*/
//           edges_ptr += vos->oPointBetweenLine;  /*skip border points*/
//       }
//       vol_ptr += vos->oLineBetweenSlice; /*skip border lines*/
//       mask_ptr += vos->oLineBetweenSlice; /*skip border lines*/
//       edges_ptr += vos->oLineBetweenSlice; /*skip border lines*/
//   }
//   mean /= nb_total;
//   
//   vol_ptr = VipGetDataPtr_S16BIT( copyvol ) + vos->oFirstPoint;
//   mask_ptr = VipGetDataPtr_S16BIT( brain ) + vos->oFirstPoint;
//   edges_ptr = VipGetDataPtr_S16BIT( edges ) + vos->oFirstPoint;
//   for ( iz = mVipVolSizeZ(vol); iz-- ; )    /* loop on slices */
//   {
//       for ( iy = mVipVolSizeY(vol); iy-- ; )    /* loop on lines */
//       {
//           for ( ix = mVipVolSizeX(vol); ix-- ; )    /* loop on points */
//           {
//               if(!(*mask_ptr))
//               {
//                   for ( icon = 0; icon<vcs->nb_neighbors; icon++ )
//                   {
//                       voisin = mask_ptr + vcs->offset[icon];
//                       if(*voisin==255)
//                       {
//                           if((*edges_ptr)>25 && (*edges_ptr)<500)
//                           {
//                               sigma += ((float)(*vol_ptr) - mean)*((float)(*vol_ptr) - mean);
//                               break;
//                           }
//                       }
//                   }
//               }
//               vol_ptr++;
//               mask_ptr++;
//               edges_ptr++;
//           }
//           vol_ptr += vos->oPointBetweenLine;  /*skip border points*/
//           mask_ptr += vos->oPointBetweenLine;  /*skip border points*/
//           edges_ptr += vos->oPointBetweenLine;  /*skip border points*/
//       }
//       vol_ptr += vos->oLineBetweenSlice; /*skip border lines*/
//       mask_ptr += vos->oLineBetweenSlice; /*skip border lines*/
//       edges_ptr += vos->oLineBetweenSlice; /*skip border lines*/
//   }
//   sigma = sqrt(sigma/nb_total);
//   T_GRAY_CSF = mean + sigma;
  T_GRAY_CSF = VipCreateHistogram(copyvol, brain, edges, CONNECTIVITY_6);
  printf("T_GRAY_CSF=%d\n", T_GRAY_CSF);
  fflush(stdout);
  
  var = VipCreateSingleThresholdedVolume( variance, LOWER_THAN, 30, BINARY_RESULT );
  if(VipOpening( var, CHAMFER_BALL_3D, little_opening_size )==PB) return(PB);
  if(VipConnexVolumeFilter( var, CONNECTIVITY_26, -1, CONNEX_BINARY)==PB) return(PB);

//   if( VipMerge( var, skin, VIP_MERGE_ONE_TO_ONE, 0, 0 )== PB) return(VIP_CL_ERROR);

  mask2 = VipCopyVolume(brain, "brainmask");
  
  seuil = CA[1] - 0.8*(CP[1] - CA[1]);
//   seuil = CA[1];
  vos = VipGetOffsetStructure( var );
  ptr = VipGetDataPtr_S16BIT( var ) + vos->oFirstPoint + seuil*vos->oLine;
  for ( iz = mVipVolSizeZ(var) ; iz-- ; )   /* loop on slices */
  {
      for ( iy = (mVipVolSizeY(var) - seuil) ; iy-- ; )  /* loop on lines */
      {
          for ( ix = mVipVolSizeX(var) ; ix-- ; )   /* loop on points */
          {
              *ptr = 0;
              ptr++;
          }
          ptr = ptr + vos->oPointBetweenLine;  /*skip border points*/
      }
      ptr = ptr + vos->oLineBetweenSlice + seuil*vos->oLine; /*skip border lines*/
  }

// // //   for(i=0;i<12;i++)
// // //   {
// // //       VipPropagationConnectivity( brain, copyvol, var, GREATER_THAN, T_GRAY_CSF, 255, CONNECTIVITY_6 );
// // //       VipSingleThreshold( brain, GREATER_OR_EQUAL_TO, 1, BINARY_RESULT );
// // //   }
// // //   VipPropagationConnectivity( brain, copyvol, var, GREATER_THAN, T_GRAY_CSF, 255, CONNECTIVITY_6 );
// // //   if(VipMerge( brain, mask2, VIP_MERGE_ONE_TO_ONE, 255, 0 ) == PB) return(VIP_CL_ERROR);
// // //   VipChangeIntLabel(brain,0,-100);
// // //   if(VipComputeFrontPropagationConnectivityDistanceMap(brain,255,-100,VIP_NO_LIMIT_IN_PROPAGATION,13,CONNECTIVITY_6)==PB) return(PB);
// // // //   VipWriteVolume(brain,"brainmaskBdist");
// // //   VipSingleThreshold( brain, GREATER_OR_EQUAL_TO, 10000, BINARY_RESULT );

  VipPropagationWithRefluxConnectivity( brain, var, copyvol, 12, VIP_BETWEEN, T_GRAY_CSF, ana->white->mean + ana->white->sigma, CONNECTIVITY_6 );

  if( VipMerge( brain, mask2, VIP_MERGE_ONE_TO_ONE, 255, 255 )== PB) return(PB);
  if( VipConnexVolumeFilter( brain, CONNECTIVITY_6, -1, CONNEX_BINARY ) == PB) return(PB);

//----Ouverture sur le masque final
  if( VipErosion( brain, CHAMFER_BALL_3D, little_opening_size ) == PB) return(PB);
  if( VipDilation( brain, CHAMFER_BALL_3D, little_opening_size ) == PB) return(PB);
  if( VipConnexVolumeFilter( brain, CONNECTIVITY_6, -1, CONNEX_BINARY ) == PB) return(PB);

//----Affinement de la frontiere
  //Rajout des voxels de la frontiere ayant ete declare appartenant a la matiere grise
  if(VipDilateVolumeBorder(copyvol, brain, T_GRAY_CSF, T_WHITE_FAT_LOW_1, CONNECTIVITY_6) == PB) return(PB);
  VipSingleThreshold( brain, GREATER_OR_EQUAL_TO,  1, BINARY_RESULT );

  return(OK);
}


/*---------------------------------------------------------------------------*/
int VipGetBrain2005(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float little_opening_size,
float max_cortical_thickness,
float white_erosion_size,
float white_recovering_size,
int nb_iterations,
Volume *ridge
)
/*---------------------------------------------------------------------------*/
/*

 */
{
  Volume *mask=NULL, *classif=NULL, *white=NULL, *volcopy=NULL;
  float erosion_size;
  float dilation_size;
  float threshold_dist;
  int T_VOID_GRAY_LOW=0, T_VOID_GRAY_HIGH=0, T_WHITE_FAT_LOW=0, T_WHITE_FAT_HIGH=0;
  int T_GRAY_WHITE=0, T_GRAY_WHITE_SECUREGRAY=0,T_GRAY_WHITE_LOW =0,T_GRAY_WHITE_HIGH=0;

  dumb=VFALSE;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipGetBrain2005");
      return(PB);
    }
  if(!ana)
    {
      VipPrintfError("Null VipT1HistoAnalysis!");
      VipPrintfExit("(VipGetBrainCustomized");
      return(PB);
    }

  if(ana->gray!=NULL && ana->white!=NULL)
    {  
      T_VOID_GRAY_LOW = ana->gray->mean - 2.5*ana->gray->sigma;
      T_VOID_GRAY_HIGH = ana->gray->mean - 2.*ana->gray->sigma;
      T_GRAY_WHITE = ana->gray->mean + (ana->white->mean - ana->gray->mean)/2;
      T_GRAY_WHITE_SECUREGRAY = ana->gray->mean + 2*ana->gray->sigma;
      T_WHITE_FAT_LOW = ana->white->mean + 2.5*ana->white->sigma;
      T_WHITE_FAT_HIGH = ana->white->mean + 3* ana->white->sigma;
      T_GRAY_WHITE_LOW = ana->gray->mean + ana->gray->right_sigma;
      T_GRAY_WHITE_HIGH = ana->white->mean - ana->white->left_sigma;
    }
  else 
    {
      VipPrintfError("Bad Histogram Analysis!");
      return(PB);
    }

  if(dumb!=VTRUE)
    {
      printf("-----------------------------\n");
      printf("   White matter extraction\n");
      printf("-----------------------------\n");
    }
  printf("Erosion:%f, Recover:%f\n",white_erosion_size,white_recovering_size);
  erosion_size = white_erosion_size;
  dilation_size = (int)(erosion_size+1.);
  threshold_dist = white_recovering_size + dilation_size;

  white = VipCopyVolume(vol,"volcopy");
  classif = VipGrayWhiteClassificationRegularisationForRobust2005( white, ana, T_VOID_GRAY_LOW, 
                                                               T_VOID_GRAY_HIGH, T_GRAY_WHITE_LOW, T_GRAY_WHITE_HIGH, T_WHITE_FAT_HIGH, dumb);
  if(classif==PB) return(PB);
  VipSetVolumeLevel(white,0);
  VipMerge(white,classif,VIP_MERGE_ONE_TO_ONE,WHITE_LABEL,255);
  VipFreeVolume(classif);

  mask = VipCopyVolume( white, "mask");
  if(mask==PB)
    {
      VipPrintfExit("VipGetBrain2005");
    }
  if(dumb!=VTRUE)
    {
      printf("Erosion (mm)...\n");
      fflush(stdout);
    }  
  if( VipErosion( mask, CHAMFER_BALL_3D, erosion_size) == PB) return(PB);

  if(dumb!=VTRUE)
    {
      printf("Selecting largest 3D 26-connected component (to get white matter seed)...\n");
      fflush(stdout);
    }
  if( VipConnexVolumeFilter( mask, CONNECTIVITY_26, -1, CONNEX_BINARY) == PB) return(PB);
  if( VipMerge( white, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);
  VipFreeVolume(mask);
  if(debug==VTRUE)
    {
      VipPrintfInfo("Debug mode: writing white matter + seed");
      VipWriteVolume(white,"white+seed");
    }
  if(ridge!=NULL)
    {
      if( VipMerge( white, ridge, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);
    }
  if(debug==VTRUE)
    {
      VipPrintfInfo("Debug mode: writing white matter + seed + ridge");
      VipWriteVolume(white,"white+seed+ridge");


    }
  VipChangeIntLabel(white,0,-1000);

  if(dumb!=VTRUE)
    {
      printf("Computing geodesic distance to white matter seed...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(white,255,-1000,VIP_PUT_LIMIT_TO_LIMIT,(int)(threshold_dist+1))==PB) return(PB);

  if(debug==VTRUE)
    {
      VipPrintfInfo("Debug mode: writing distance to white matter seed");
      VipWriteVolume(white,"dist_to_wseed");
    }
  
  if(dumb!=VTRUE)
    {
      printf("Correction for opening effect (dil:%f/correction:%f)...\n", dilation_size, threshold_dist);
      fflush(stdout);
    }
  
  mask = VipCreateSingleThresholdedVolume( white, GREATER_OR_EQUAL_TO,  (int)(threshold_dist*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT);
  if(mask==PB)
    {
      VipPrintfExit("VipGetBrainForRenderingDoubleCustomized");
    }
  VipSingleThreshold( white, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT );
  if( VipMerge( white, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);
  VipFreeVolume(mask);
  VipChangeIntLabel(white,0,-100);
  if(dumb!=VTRUE)
    {
      printf("Computing distance to white matter complement...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(white,255,-100,VIP_PUT_LIMIT_TO_LIMIT,(int)(threshold_dist-dilation_size+1))==PB) return(PB);

  if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing distance to white matter complement"); 
	 VipWriteVolume(white,"dist_to_wcomp");
      }

 if(dumb!=VTRUE)
    {
      printf("Selecting largest 3D 26-connected component (recovering can disconnect some objects)...\n");
      fflush(stdout);
    }

 VipSingleThreshold( white, GREATER_OR_EQUAL_TO,  (int)(VIP_USUAL_DISTMAP_MULTFACT*(threshold_dist-dilation_size)), BINARY_RESULT );
 if( VipConnexVolumeFilter( white, CONNECTIVITY_26, -1, CONNEX_BINARY) == PB) return(PB);


  if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing white matter"); 
	 VipWriteVolume(white,"white");
         VipWriteTivoliVolume(white,"white");
      }

  if(dumb!=VTRUE)
    {
      printf("-----------------------------\n");
      printf("   Whole brain extraction\n");
      printf("-----------------------------\n");
    }

  /* 
  if(mVipVolVoxSizeX(vol)>little_opening_size) little_opening_size=mVipVolVoxSizeX(vol)+0.1;
  if(mVipVolVoxSizeY(vol)>little_opening_size) little_opening_size=mVipVolVoxSizeY(vol)+0.1;
  if(mVipVolVoxSizeZ(vol)>little_opening_size) little_opening_size=mVipVolVoxSizeZ(vol)+0.1;
  */
  dilation_size = max_cortical_thickness;
  threshold_dist = 1.6;

  VipSingleThreshold( vol, GREATER_OR_EQUAL_TO, T_VOID_GRAY_HIGH , GREYLEVEL_RESULT );

  mask = VipCopyVolume(white,"mask");
  if(mask==PB)
    {
      VipPrintfExit("VipGetBrain2005");
    }
  VipDilation(mask,CHAMFER_BALL_3D,5.);
  VipMaskVolume(vol,mask);
  VipFreeVolume(mask);
 
  volcopy = VipCopyVolume(vol,"volcopy");
  if(volcopy==PB)
    {
      VipPrintfExit("VipGetBrain2005");
    }

  VipSingleThreshold( volcopy, GREATER_OR_EQUAL_TO, 1 , BINARY_RESULT );
  VipMerge(volcopy,white,VIP_MERGE_ONE_TO_ONE,255,512);

  VipChangeIntLabel(volcopy,0,-2);
  if(VipComputeFrontPropagationChamferDistanceMap(volcopy,255,-2,VIP_PUT_LIMIT_TO_LIMIT,(int)(threshold_dist+1))==PB) return(PB);
  VipFreeVolume(white);
  white = VipCreateDoubleThresholdedVolume(volcopy,VIP_BETWEEN_OR_EQUAL_TO,0,(int)(threshold_dist*VIP_USUAL_DISTMAP_MULTFACT),BINARY_RESULT);
  VipFreeVolume(volcopy);

   if(dumb!=VTRUE)
    {
      printf("Opening (to get accurate brain surface)...\n");
      fflush(stdout);
    }

   VipDoubleThreshold( vol, VIP_BETWEEN_OR_EQUAL_TO,1, T_GRAY_WHITE_SECUREGRAY, BINARY_RESULT );
  if( VipMerge( vol, white, VIP_MERGE_ONE_TO_ONE, 255, 255 )== PB) return(PB);

  if(VipOpening(vol,CHAMFER_BALL_3D,little_opening_size)==PB) return(PB);

  if( VipMerge( vol, white, VIP_MERGE_ONE_TO_ONE, 255, 255 )== PB) return(PB);

  if( VipConnexVolumeFilter( vol, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);
  
  return(OK);
}


/*---------------------------------------------------------------------------*/
int VipGetBrainStandard(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float brain_erosion_size,
float brain_recovering_size,
int nb_iterations,
int iterative_erosion
)
/*---------------------------------------------------------------------------*/
/*

NEW 10-9-2003, je vais tenter de supprimer le masque de la mati�re blanche,
qui cr�e des probl�me avec les images buit�e,
cette procedure est un clone de: VipGetBrainCustomized


Cette fonction est effroyable, comme un organisme vivant,
j'essaie de faire le point sur son fonctionnement,
pour en maitriser mieux l'interface ds brainVISA: 20-5-2003, JFM
1)fixe differents seuils, en gerant 2 types de sequences IR/standard ou contrast/pas contrast;
2)La peau:
  a)Seuil tissu/pas tissu
  b)bord externe 2D sauf vers le bas, a cause d'une image patho: shiva, bof...
  c)Elimine comp 26-connexe de moins de 100 points
  d)Dilate de 5mm en 2D 
  Le resultat est une bande consideree comme la peau.
3) Calcule une classif regularisee CSF/GREY/white: VipCSFGrayWhiteFatClassificationRegularisationForRobustApproach
4) Cree le volume brain et efface la classif. BRAIN_LABEL devient 255
5) 
   a) erosion non iterative: erosion + plus grande 26CC
   b) erosion iterative, pas de 0.2mm: 
      1) carte de distance ds le masque;
      2) seuillage + plus grande 26CC
      3) stop si intersection vide entre la graine et la peau. (on pourrait refaire le test apres reconstruction,
                                                                avec le risque que la peau inclue du cerveau,
                                                                ou en prenant que la bande externe de la peau.?)
      4) Si l'erosion est plus que 3.5mm, on fait erosion minimum 2.5mm
   c) Ouverture du masquee binaire de 1.9mm
   d) dilatation de la graine precedente trop grandee, puis reflux.
   E) Garde plus grande 26CC
   
   
    


 */
{
  Volume *brain=NULL, *mask=NULL, *classif=NULL;
  Volume *skin=NULL;
  Volume *distmap=NULL;
  int goodseed;
  float erosion_size;
  float dilation_size;
  float threshold_dist;
  float little_opening_size;
  int ix, iy, iz;
  int T_VOID_GRAY_LOW=0, T_VOID_GRAY_HIGH=0, T_WHITE_FAT_LOW=0, T_WHITE_FAT_HIGH=0;
  int T_GRAY_WHITE=0, T_GRAY_WHITE_SECUREGRAY;
  VipOffsetStruct *vos;
  Vip_S16BIT *skin_ptr;
  Vip_S16BIT *mask_ptr;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipGetBrainCustomized");
      return(PB);
    }
  if(!ana)
    {
      VipPrintfError("Null VipT1HistoAnalysis!");
      VipPrintfExit("(VipGetBrainCustomized");
      return(PB);
    }

  if(ana->gray!=NULL && ana->white!=NULL)
    {  
      if(ana->sequence==MRI_T1_SPGR)
	{
	  T_VOID_GRAY_LOW = (int)(ana->gray->mean - 2.5*ana->gray->sigma);
	  T_VOID_GRAY_HIGH =(int)( ana->gray->mean - 1.7*ana->gray->sigma);
	}
      else 
	{
	  T_VOID_GRAY_LOW = ana->gray->mean - 2.2*ana->gray->sigma;
	  T_VOID_GRAY_HIGH = ana->gray->mean - 1.7*ana->gray->sigma;
	}
      T_GRAY_WHITE = ana->gray->mean + (ana->white->mean - ana->gray->mean)/2;
      T_GRAY_WHITE_SECUREGRAY = ana->gray->mean + 2*ana->gray->sigma;
      T_WHITE_FAT_LOW = ana->white->mean + 4*ana->white->sigma;
      T_WHITE_FAT_HIGH = ana->white->mean + 6*ana->white->sigma;
    }
  else 
    {
      VipPrintfError("Bad Histogram Analysis!");
      return(PB);
    }

  if(debug==VTRUE)
    {
      VipPrintfWarning("The DEBUG mode generates a lot of images (you need at least 100M of free disk space)");
    }
  
  if(iterative_erosion==VTRUE)
    {
      if(dumb==VFALSE)
	{
	  printf("--------------------------------------------------------------------\n");
	  if(dumb==VFALSE) printf("Computing skin layer for iterative erosion...\n");
	}

      skin = VipCopyVolume(vol,"skin");
      if(skin==PB) return(PB);

      VipSingleThreshold(skin,GREATER_THAN,mVipMax(ana->gray->mean - 3*ana->gray->sigma,20),BINARY_RESULT);
      if( VipConnexVolumeFilter( skin, CONNECTIVITY_26, -1, CONNEX_BINARY) == PB) return(PB);

      VipExtedge(skin,EXTEDGE2D_ALL_EXCEPT_Y_BOTTOM,SAME_VOLUME);
      /*shiva a la tete trop bas, et le skin tape ds le cerveau,
	ya surement d'autre maniere de s'en sortir, mais
	de toutes facons les pb de graine c'est jamais vers le bas...*/
      if( VipConnexVolumeFilter( skin, CONNECTIVITY_26, 100, CONNEX_BINARY) == PB) return(PB);
      VipDilation(skin,CHAMFER_BALL_2D,5);
      if (debug==VTRUE) VipWriteTivoliVolume(skin,"skinlayer");
    }

  classif = VipCSFGrayWhiteFatClassificationRegularisationForRobustApproach( vol, ana, dumb, nb_iterations,
T_VOID_GRAY_LOW, T_VOID_GRAY_HIGH, T_WHITE_FAT_LOW, T_WHITE_FAT_HIGH, T_GRAY_WHITE);
  if(classif==PB) return(PB);


  if(dumb!=VTRUE)
    {
      printf("-----------------------------\n");
      printf("   Whole brain extraction\n");
      printf("-----------------------------\n");
    }

  erosion_size = brain_erosion_size;


  brain = vol;
  VipSetVolumeLevel(brain,0);
  VipMerge(brain,classif,VIP_MERGE_ONE_TO_ONE,BRAIN_LABEL,255);
  
  VipFreeVolume(classif);
  
  classif=NULL;
  

  if(iterative_erosion==VFALSE)
    {
      printf("--------------------------------------\n");
      printf("Only one erosion size will be used:\n");	  
      printf("Erosion: %f mm, Opening recover: %f mm\n",erosion_size, brain_recovering_size);
      printf("--------------------------------------\n");
      fflush(stdout);

      mask = VipCopyVolume( brain, "mask");
      if( VipErosion( mask, CHAMFER_BALL_3D, erosion_size) == PB) return(PB);
  
      if(dumb!=VTRUE)
	{
	  printf("Selecting largest 3D 6-connected component (to get brain seed)...\n");
	  fflush(stdout);
	}
      if( VipConnexVolumeFilter( mask, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);
    }
  else
    {
      printf("--------------------------------------\n");
      printf("Erosion size will be increased step by step until 4mm,\n");	  
      printf("to find a seed farther than 5mm from the skin.\n");	  
      printf("Initial erosion: %f mm, Opening recover: %f mm\n",erosion_size, brain_recovering_size);
      printf("--------------------------------------\n");
      fflush(stdout);

      distmap = VipCreateSingleThresholdedVolume(brain,EQUAL_TO,0,BINARY_RESULT);
      VipComputeFrontPropagationChamferDistanceMap(distmap,0,-1,VIP_PUT_LIMIT_TO_INFINITE,4);

      vos = VipGetOffsetStructure(skin);

      goodseed = VFALSE;
      while(goodseed==VFALSE)
	{
	  mask = VipCreateSingleThresholdedVolume(
				     distmap,GREATER_THAN,
				     (int)(erosion_size*VIP_USUAL_DISTMAP_MULTFACT),
				     BINARY_RESULT);
	  if( VipConnexVolumeFilter( mask, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);

	  skin_ptr = VipGetDataPtr_S16BIT( skin ) + vos->oFirstPoint;
	  mask_ptr = VipGetDataPtr_S16BIT( mask ) + vos->oFirstPoint;

	  goodseed=VTRUE;
	  for ( iz = mVipVolSizeZ(skin); iz-- ; )               /* loop on slices */
	    {
	      for ( iy = mVipVolSizeY(skin); iy-- ; )            /* loop on lines */
		{
		  for ( ix = mVipVolSizeX(skin); ix-- ; )/* loop on points */
		    {
		      if((*mask_ptr) && (*skin_ptr)) 
			{
			  goodseed=VFALSE;
			}
		      mask_ptr++;
		      skin_ptr++;
		    }
		  if(goodseed==VFALSE) break;
		  mask_ptr += vos->oPointBetweenLine;  /*skip border points*/
		  skin_ptr += vos->oPointBetweenLine;  /*skip border points*/
		}
	      if(goodseed==VFALSE) break;
	      mask_ptr += vos->oLineBetweenSlice; /*skip border lines*/
	      skin_ptr += vos->oLineBetweenSlice; /*skip border lines*/
	    }
	  if(erosion_size>3.5)
	    {
	      VipPrintfWarning("Something was wrong with the iterative erosion!");
	      printf("Back to 2.5mm erosion...\n");
	      VipFreeVolume(mask);
	      erosion_size=2.5;
	      mask = VipCreateSingleThresholdedVolume(
				     distmap,GREATER_THAN,
				     (int)(erosion_size*VIP_USUAL_DISTMAP_MULTFACT),
				     BINARY_RESULT);
	      if( VipConnexVolumeFilter( mask, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);
	    goodseed = VTRUE;
	    }
	  if ( goodseed==VTRUE) 
	    {
	      goodseed=VTRUE;
	      VipFreeVolume(skin);
	      VipFreeVolume(distmap);
	      printf("Applied erosion size:%f\n",erosion_size);
	    }
	  else 
	    {
	      VipFreeVolume(mask);
	      erosion_size += 0.2;
	      printf("New erosion size:%f\n",erosion_size);
	      fflush(stdout);
	    }

	}
    }
  dilation_size = (erosion_size+0.5);
  threshold_dist = dilation_size + brain_recovering_size;

  little_opening_size = 1.9;
  if(mVipVolVoxSizeX(brain)>little_opening_size) little_opening_size=mVipVolVoxSizeX(brain)+0.1;
  if(mVipVolVoxSizeY(brain)>little_opening_size) little_opening_size=mVipVolVoxSizeY(brain)+0.1;
  if(mVipVolVoxSizeZ(brain)>little_opening_size) little_opening_size=mVipVolVoxSizeZ(brain)+0.1;

  
  if(dumb!=VTRUE)
    {
      printf("Opening (to get accurate brain surface)...\n");
      fflush(stdout);
    }
  if(VipOpening(brain,CHAMFER_BALL_3D,little_opening_size)==PB) return(PB);

  if( VipMerge( brain, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);

  VipFreeVolume(mask);
  mask = NULL;

  if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing brain + seed"); 
	 VipWriteVolume(brain,"brain+seed");
      }

  VipChangeIntLabel(brain,0,-2);
  
  if(dumb!=VTRUE)
    {
      printf("Computing geodesic distance to brain seed...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(brain,255,-2,VIP_PUT_LIMIT_TO_LIMIT,(int)(threshold_dist+1))==PB) return(PB);
  
  if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing distance to brain seed"); 
	 VipWriteVolume(brain,"dist_to_bseed");
      }

  if(dumb!=VTRUE)
    {
      printf("Correction for opening effect (dil:%f/correction:%f)...\n", dilation_size, threshold_dist);
      fflush(stdout);
    }
  mask = VipCreateSingleThresholdedVolume( brain, GREATER_OR_EQUAL_TO,  (int)(threshold_dist*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT);
  if(mask==PB)
    {
      VipPrintfExit("VipGetBrainForRendering");
    }

  VipSingleThreshold( brain, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT );

  if( VipMerge( brain, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);

  VipFreeVolume(mask);
  mask = NULL;
 
  VipChangeIntLabel(brain,0,-100);

  if(dumb!=VTRUE)
    {
      printf("Computing distance to brain matter complement...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(brain,255,-100,VIP_PUT_LIMIT_TO_LIMIT,(int)(threshold_dist-dilation_size+1))==PB) return(PB);

 if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing distance to brain complement");  
	 VipWriteVolume(brain,"dist_to_bcomp");
      }

  VipSingleThreshold( brain, GREATER_OR_EQUAL_TO,  VIP_USUAL_DISTMAP_MULTFACT*(int)(threshold_dist-dilation_size), BINARY_RESULT );
   
  if(dumb!=VTRUE)
    {
      printf("Selecting largest 3D 6-connected component (recovering can disconnect some objects)...\n");
      fflush(stdout);
    }
  if( VipConnexVolumeFilter( brain, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);
  
  return(OK);
}

/*---------------------------------------------------------------------------*/
int VipGetBrainCustomized(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float brain_erosion_size,
float brain_recovering_size,
float white_erosion_size,
float white_recovering_size,
int nb_iterations,
VipTalairach *tal,
int iterative_erosion
)
/*---------------------------------------------------------------------------*/
/*
Cette fonction est effroyable, comme un organisme vivant,
j'essaie de faire le point sur son fonctionnement,
pour en maitriser mieux l'interface ds brainVISA: 20-5-2003, JFM
1)fixe differents seuils, en gerant 2 types de sequences IR/standard ou contrast/pas contrast;
2)La peau:
  a)Seuil tissu/pas tissu
  b)bord externe 2D sauf vers le bas, a cause d'une image patho: shiva, bof...
  c)Elimine comp 26-connexe de moins de 100 points
  d)Dilate de 5mm en 2D 
  Le resultat est une bande consideree comme la peau.
3)Les meninges:
  a) double seuil: (T_VOID_GRAY_LOW+T_VOID_GRAY_HIGH)/2,T_WHITE_FAT_HIGH,BINARY_RESULT
  b) Erosion de 3mm en 3D
4)La matiere blanche:
  a) VipComputeRawWhiteMask(...,white_erosion_size,white_recovering_size,T_WHITE_FAT_LOW,T_GRAY_WHITE)
     1)VipGrayWhiteClassificationRegularisationForRobust
     2)erosion
     3)plus grande 3D cc
     4)dilatation + recovering
     5) plus grande 26cc
  b) Dilatation de 3mm en 3D
5)Merge white mask ds meninge
6) efface ds IRM les points de meninge>T_GRAY_WHITE_SECUREGRAY
7) Libere meninge mask
8) Cree un bucket des sinus a partir de l'IRM et du masque blanc
9) Libere le masque blanc
10)Si CUTBUCK TRUE, efface le bucket de sinus
11) Calcule une classif regularisee CSF/GREY/white: VipCSFGrayWhiteFatClassificationRegularisationForRobustApproach
12) Cree le volume brain et efface la classif. BRAIN_LABEL devient 255
13) 
   a) erosion non iterative: erosion + plus grande 26CC
   b) erosion iterative, pas de 0.2mm: 
      1) carte de distance ds le masque;
      2) seuillage + plus grande 26CC
      3) stop si intersection vide entre la graine et la peau. (on pourrait refaire le test apres reconstruction,
                                                                avec le risque que la peau inclue du cerveau,
                                                                ou en prenant que la bande externe de la peau.?)
      4) Si l'erosion est plus que 3.5mm, on fait erosion minimum 2.5mm
   c) Ouverture du masquee binaire de 1.9mm
   d) dilatation de la graine precedente trop grandee, puis reflux.
   E) Garde plus grande 26CC
   
   
    


 */
{
  Volume *brain=NULL, *mask=NULL, *classif=NULL, *whitemask=NULL, *meningemask=NULL;
  Volume *skin=NULL;
  Volume *distmap=NULL;
  int goodseed;
  float erosion_size;
  float dilation_size;
  float threshold_dist;
  float little_opening_size;
  int ix, iy, iz;
  Vip_S16BIT *classif_ptr, *classif_first;
  Vip_S16BIT *data_ptr, *data_first;
  int T_VOID_GRAY_LOW=0, T_VOID_GRAY_HIGH=0, T_WHITE_FAT_LOW=0, T_WHITE_FAT_HIGH=0;
  int T_GRAY_WHITE=0, T_GRAY_WHITE_SECUREGRAY;
  VipOffsetStruct *data_vos, *classif_vos;
  Vip3DBucket_S16BIT *cutbuck=NULL;
  VipOffsetStruct *vos;
  Vip_S16BIT *skin_ptr;
  Vip_S16BIT *mask_ptr;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipGetBrainCustomized");
      return(PB);
    }
  if(!ana)
    {
      VipPrintfError("Null VipT1HistoAnalysis!");
      VipPrintfExit("(VipGetBrainCustomized");
      return(PB);
    }

  if(ana->gray!=NULL && ana->white!=NULL)
    {  
      if(ana->sequence==MRI_T1_SPGR)
	{
	  T_VOID_GRAY_LOW = (int)(ana->gray->mean - 2.5*ana->gray->sigma);
	  T_VOID_GRAY_HIGH =(int)( ana->gray->mean - 1.7*ana->gray->sigma);
	}
      else 
	{
	  T_VOID_GRAY_LOW = ana->gray->mean - 2.2*ana->gray->sigma;
	  T_VOID_GRAY_HIGH = ana->gray->mean - 1.7*ana->gray->sigma;
	}
      T_GRAY_WHITE = ana->gray->mean + (ana->white->mean - ana->gray->mean)/2;
      T_GRAY_WHITE_SECUREGRAY = ana->gray->mean + 2*ana->gray->sigma;
      T_WHITE_FAT_LOW = ana->white->mean + 4*ana->white->sigma;
      T_WHITE_FAT_HIGH = ana->white->mean + 6* ana->white->sigma;
    }
  else 
    {
      VipPrintfError("Bad Histogram Analysis!");
      return(PB);
    }

  if(debug==VTRUE)
    {
      VipPrintfWarning("The DEBUG mode generates a lot of images (you need at least 100M of free disk space)");
    }
  
  if(iterative_erosion==VTRUE)
    {
      if(dumb==VFALSE)
	{
	  printf("--------------------------------------------------------------------\n");
	  if(dumb==VFALSE) printf("Computing skin layer for iterative erosion...\n");
	}

      skin = VipCopyVolume(vol,"skin");
      if(skin==PB) return(PB);
      /*
     VipDeriche3DGaussian(skin,mVipVolVoxSizeZ(skin),SAME_VOLUME);
      */
      VipSingleThreshold(skin,GREATER_THAN,mVipMax(ana->gray->mean - 3*ana->gray->sigma,20),BINARY_RESULT);
      /*
      VipExtedge(skin,EXTEDGE2D_ALL,SAME_VOLUME);
      */
      VipExtedge(skin,EXTEDGE2D_ALL_EXCEPT_Y_BOTTOM,SAME_VOLUME);
      /*shiva a la tete trop bas, et le skin tape ds le cerveau,
	ya surement d'autre maniere de s'en sortir, mais
	de toutes facons les pb de graine c'est jamais vers le bas...*/
      if( VipConnexVolumeFilter( skin, CONNECTIVITY_26, 100, CONNEX_BINARY) == PB) return(PB);
      VipDilation(skin,CHAMFER_BALL_2D,5);
      if (debug==VTRUE) VipWriteTivoliVolume(skin,"skinlayer");
    }
  if(dumb==VFALSE)
    {
      printf("--------------------------------------------------------------------\n");
      printf("Try to remove bright parts of meninges, Gibbs artefacts, vasculature...\n");
      printf("--------------------------------------------------------------------\n");
    }
  meningemask = VipCreateDoubleThresholdedVolume(vol,VIP_BETWEEN_OR_EQUAL_TO,(int)(T_VOID_GRAY_LOW+T_VOID_GRAY_HIGH)/2,T_WHITE_FAT_HIGH,BINARY_RESULT); 

/*ici l'idee c'est d'avoir des trous ds le crane qui vont marquer les meninges, donc on
essaie d'eviter les trous ailleurs, meme si ya en plus une protection qui va venir
de la segmentation du blanc*/

  if(meningemask==PB) return(PB);

  /*VipConnectivityChamferClosing(meningemask,1,CONNECTIVITY_6,FRONT_PROPAGATION);*/
  VipErosion (meningemask,CHAMFER_BALL_3D,3);



   if(dumb==VFALSE)
     {
       printf("----------------------------------------------\n");
       printf("Computing raw white matter mask to preserve it...\n");
       printf("----------------------------------------------\n");
     }
   whitemask = VipComputeRawWhiteMask(vol,ana,dumb,debug,white_erosion_size,
white_recovering_size,T_WHITE_FAT_LOW,T_GRAY_WHITE);
   if(whitemask==PB) return(PB);  
   /*   VipConnectivityChamferDilation (whitemask,1,CONNECTIVITY_26,FRONT_PROPAGATION); */

   /*VipComputeChamferDistanceMap(whitemask);

   distmap = VipCopyVolume(whitemask);
	VipSingleThreshold(whitemask,LOWER_THAN,3,BINARY_RESULT);*/
   VipDilation(whitemask,CHAMFER_BALL_3D,3.);

/*pour eviter des effets de regul qui creent des diff/trous*/
  VipMerge(meningemask,whitemask,VIP_MERGE_ALL_TO_ONE,255,255);
   if (debug==VTRUE) VipWriteTivoliVolume(meningemask,"meningemask");

  data_vos = VipGetOffsetStructure(vol);
  data_first = VipGetDataPtr_S16BIT( vol ) + data_vos->oFirstPoint;
  classif_vos = VipGetOffsetStructure(meningemask);
  classif_first = VipGetDataPtr_S16BIT( meningemask ) + classif_vos->oFirstPoint; 
  data_ptr = data_first;
  classif_ptr = classif_first;
   for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
   {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
      {
         for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
         {
	   if ((*data_ptr>=T_GRAY_WHITE_SECUREGRAY) && !(*classif_ptr))
	     *data_ptr=0; /*T_WHITE_FAT_HIGH +100; like remove...*/
	   data_ptr++;
	   classif_ptr++;
         }
         classif_ptr += classif_vos->oPointBetweenLine;  /*skip border points*/
         data_ptr += data_vos->oPointBetweenLine;  /*skip border points*/
      }
      classif_ptr += classif_vos->oLineBetweenSlice; /*skip border lines*/
      data_ptr += data_vos->oLineBetweenSlice; /*skip border lines*/
   }

   VipFreeVolume(meningemask);

  if(tal!=NULL)
    {
      if(dumb==VFALSE)
	{
	  printf("----------------------------------------------\n");
	  printf("Try to detect sinus in interhemispheric plane to remove it...\n");
	  printf("----------------------------------------------\n");
	}
      if(GetSinusBucket(  vol, whitemask, tal, ana, &cutbuck)==PB)
	{
	  printf("Problem with GetSinusBucket");
	  fflush(stdout);
	  return(VIP_CL_ERROR);
	}
    }
  VipFreeVolume(whitemask);
   if (cutbuck!=NULL)
    {
      VipWriteCoordBucketInVolume(cutbuck,vol,0); 
/*T_WHITE_FAT_HIGH +300);*/
      printf("%d points removed\n",cutbuck->n_points);
    }
   if (debug==VTRUE) VipWriteTivoliVolume(vol,"clean");
  
  
  classif = VipCSFGrayWhiteFatClassificationRegularisationForRobustApproach( vol, ana, dumb, nb_iterations,
T_VOID_GRAY_LOW, T_VOID_GRAY_HIGH, T_WHITE_FAT_LOW, T_WHITE_FAT_HIGH, T_GRAY_WHITE);
  if(classif==PB) return(PB);


  if(dumb!=VTRUE)
    {
      printf("-----------------------------\n");
      printf("   Whole brain extraction\n");
      printf("-----------------------------\n");
    }

  erosion_size = brain_erosion_size;


  brain = vol;
  VipSetVolumeLevel(brain,0);
  VipMerge(brain,classif,VIP_MERGE_ONE_TO_ONE,BRAIN_LABEL,255);
  
  VipFreeVolume(classif);
  
  classif=NULL;
  

  if(iterative_erosion==VFALSE)
    {
      printf("--------------------------------------\n");
      printf("Only one erosion size will be used:\n");	  
      printf("Erosion: %f mm, Opening recover: %f mm\n",erosion_size, brain_recovering_size);
      printf("--------------------------------------\n");
      fflush(stdout);

      mask = VipCopyVolume( brain, "mask");
      if( VipErosion( mask, CHAMFER_BALL_3D, erosion_size) == PB) return(PB);
  
      if(dumb!=VTRUE)
	{
	  printf("Selecting largest 3D 6-connected component (to get brain seed)...\n");
	  fflush(stdout);
	}
      if( VipConnexVolumeFilter( mask, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);
    }
  else
    {
      printf("--------------------------------------\n");
      printf("Erosion size will be increased step by step until 4mm,\n");	  
      printf("to find a seed farther than 5mm from the skin.\n");	  
      printf("Initial erosion: %f mm, Opening recover: %f mm\n",erosion_size, brain_recovering_size);
      printf("--------------------------------------\n");
      fflush(stdout);

      distmap = VipCreateSingleThresholdedVolume(brain,EQUAL_TO,0,BINARY_RESULT);
      VipComputeFrontPropagationChamferDistanceMap(distmap,0,-1,VIP_PUT_LIMIT_TO_INFINITE,4);

      vos = VipGetOffsetStructure(skin);

      goodseed = VFALSE;
      while(goodseed==VFALSE)
	{
	  mask = VipCreateSingleThresholdedVolume(
				     distmap,GREATER_THAN,
				     (int)(erosion_size*VIP_USUAL_DISTMAP_MULTFACT),
				     BINARY_RESULT);
	  if( VipConnexVolumeFilter( mask, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);

	  skin_ptr = VipGetDataPtr_S16BIT( skin ) + vos->oFirstPoint;
	  mask_ptr = VipGetDataPtr_S16BIT( mask ) + vos->oFirstPoint;

	  goodseed=VTRUE;
	  for ( iz = mVipVolSizeZ(skin); iz-- ; )               /* loop on slices */
	    {
	      for ( iy = mVipVolSizeY(skin); iy-- ; )            /* loop on lines */
		{
		  for ( ix = mVipVolSizeX(skin); ix-- ; )/* loop on points */
		    {
		      if((*mask_ptr) && (*skin_ptr)) 
			{
			  goodseed=VFALSE;
			}
		      mask_ptr++;
		      skin_ptr++;
		    }
		  if(goodseed==VFALSE) break;
		  mask_ptr += vos->oPointBetweenLine;  /*skip border points*/
		  skin_ptr += vos->oPointBetweenLine;  /*skip border points*/
		}
	      if(goodseed==VFALSE) break;
	      mask_ptr += vos->oLineBetweenSlice; /*skip border lines*/
	      skin_ptr += vos->oLineBetweenSlice; /*skip border lines*/
	    }
	  if(erosion_size>3.5)
	    {
	      VipPrintfWarning("Something was wrong with the iterative erosion!");
	      printf("Back to 2.5mm erosion...\n");
	      VipFreeVolume(mask);
	      erosion_size=2.5;
	      mask = VipCreateSingleThresholdedVolume(
				     distmap,GREATER_THAN,
				     (int)(erosion_size*VIP_USUAL_DISTMAP_MULTFACT),
				     BINARY_RESULT);
	      if( VipConnexVolumeFilter( mask, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);
	    goodseed = VTRUE;
	    }
	  if ( goodseed==VTRUE) 
	    {
	      goodseed=VTRUE;
	      VipFreeVolume(skin);
	      VipFreeVolume(distmap);
	      printf("Chosen erosion size:%f\n",erosion_size);
	    }
	  else 
	    {
	      VipFreeVolume(mask);
	      erosion_size += 0.2;
	      printf("New erosion size:%f\n",erosion_size);
	      fflush(stdout);
	    }

	}
    }
  dilation_size = (erosion_size+0.5);
  threshold_dist = dilation_size + brain_recovering_size;

  little_opening_size = 1.9;
  if(mVipVolVoxSizeX(brain)>little_opening_size) little_opening_size=mVipVolVoxSizeX(brain)+0.1;
  if(mVipVolVoxSizeY(brain)>little_opening_size) little_opening_size=mVipVolVoxSizeY(brain)+0.1;
  if(mVipVolVoxSizeZ(brain)>little_opening_size) little_opening_size=mVipVolVoxSizeZ(brain)+0.1;

  
  if(dumb!=VTRUE)
    {
      printf("Opening (to get accurate brain surface)...\n");
      fflush(stdout);
    }
  if(VipOpening(brain,CHAMFER_BALL_3D,little_opening_size)==PB) return(PB);

  if( VipMerge( brain, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);

  VipFreeVolume(mask);
  mask = NULL;

  if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing brain + seed"); 
	 VipWriteVolume(brain,"brain+seed");
      }

  VipChangeIntLabel(brain,0,-2);
  
  if(dumb!=VTRUE)
    {
      printf("Computing geodesic distance to brain seed...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(brain,255,-2,VIP_PUT_LIMIT_TO_LIMIT,(int)(threshold_dist+1))==PB) return(PB);
  
  if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing distance to brain seed"); 
	 VipWriteVolume(brain,"dist_to_bseed");
      }

  if(dumb!=VTRUE)
    {
      printf("Correction for opening effect (dil:%f/correction:%f)...\n", dilation_size, threshold_dist);
      fflush(stdout);
    }
  mask = VipCreateSingleThresholdedVolume( brain, GREATER_OR_EQUAL_TO,  (int)(threshold_dist*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT);
  if(mask==PB)
    {
      VipPrintfExit("VipGetBrainForRendering");
    }

  VipSingleThreshold( brain, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT );

  if( VipMerge( brain, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);

  VipFreeVolume(mask);
  mask = NULL;
 
  VipChangeIntLabel(brain,0,-100);

  if(dumb!=VTRUE)
    {
      printf("Computing distance to brain matter complement...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(brain,255,-100,VIP_PUT_LIMIT_TO_LIMIT,(int)(threshold_dist-dilation_size+1))==PB) return(PB);

 if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing distance to brain complement");  
	 VipWriteVolume(brain,"dist_to_bcomp");
      }

  VipSingleThreshold( brain, GREATER_OR_EQUAL_TO,  VIP_USUAL_DISTMAP_MULTFACT*(int)(threshold_dist-dilation_size), BINARY_RESULT );
   
  if(dumb!=VTRUE)
    {
      printf("Selecting largest 3D 6-connected component (recovering can disconnect some objects)...\n");
      fflush(stdout);
    }
  if( VipConnexVolumeFilter( brain, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);
  
  return(OK);
}

/*---------------------------------------------------------------------------*/
int VipGetBrain(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug
)
/*---------------------------------------------------------------------------*/
{
  return(VipGetBrainCustomized(vol,ana,dumb,debug,2.5,3,3,30,1,NULL,VTRUE));
}

/*---------------------------------------------------------------------------*/
int VipPutOneSliceTwoZero(Volume *vol, int z)
/*---------------------------------------------------------------------------*/
{
    VipOffsetStruct *vos;
    int ix, iy;
    Vip_S16BIT *ptr;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
	VipPrintfError("S16BIT type");
	VipPrintfExit("(VipPutOneSliceTwoZero");
	return(PB);
    }
  if(z<0 || z>=mVipVolSizeZ(vol))
      {
	  VipPrintfWarning("This slice is out of volume (VipPutOneSliceTwoZero)");
	  return(OK);
      }

   vos = VipGetOffsetStructure(vol);
   if(!vos) return(PB);

   ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint + z*vos->oSlice;

   for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
       {
	   for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	       {
		   *ptr++ = 0;
         }
         ptr += vos->oPointBetweenLine;  /*skip border points*/
      }
 
   return(OK);
}

/*---------------------------------------------------------------------------*/
int VipFillWhiteCavities(Volume *vol, Volume *brainmask, VipT1HistoAnalysis *ana, int maxsize)
/*---------------------------------------------------------------------------*/
{
  /*input: mask of the brain, outpur, mask with filled cavities of maxsize
    where their greylevel is above white matter mean,
    when most of the voxelx are like that (to get rid of some deep vascular structures)*/
  Volume *voldual=NULL, *extray=NULL;
  Vip3DBucket_S16BIT *cclist, *ccptr;
  int nabove;
  VipOffsetStruct *vos;
  Vip_S16BIT *bufptr, *ptr, *maskptr;
  int i;
  int thigh, tlow;
  int ncc;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipFillWhiteCavities");
      return(PB);
    }
  if (VipVerifyAll(brainmask)==PB || VipTestType(brainmask,S16BIT)==PB)
    {
      VipPrintfExit("(VipFillWhiteCavities");
      return(PB);
    }
  if(!ana)
    {
      VipPrintfError("Null VipT1HistoAnalysis!");
      VipPrintfExit("(VipFillWhiteCavities");
      return(PB);
    }

  voldual = VipCopyVolume(brainmask,"dual");
  if(!voldual) return(PB);

  extray = VipExtRay(voldual,EXTEDGE3D_ALL,NEW_VOLUME);
  if(!extray) return(PB);

  VipInvertBinaryVolume(voldual);
  if(VipMerge(voldual,extray,VIP_MERGE_ALL_TO_ONE,0,0)==PB) return(PB);
  VipFreeVolume(extray);
  extray = NULL;

  vos = VipGetOffsetStructure(vol);
  if(!vos) return(PB);

  fflush(stdout);

  cclist = VipGetDualFiltered3DConnex(voldual,CONNECTIVITY_6,maxsize);
  VipFreeVolume(voldual);
  voldual=NULL;
  
  bufptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
  maskptr = VipGetDataPtr_S16BIT( brainmask ) + vos->oFirstPoint;
  
  ccptr = cclist;
  thigh = ana->white->mean+(8* ana->white->sigma);
  tlow = ana->white->mean-(2* ana->white->sigma);
  ncc = 0;
  while(ccptr!=NULL)
    {
      nabove = 0;
      for(i=0;i<ccptr->n_points;i++)
	{
	  ptr = bufptr + ccptr->data[i].x
	    + ccptr->data[i].y * vos->oLine + 
	    ccptr->data[i].z * vos->oSlice;
	  if(*ptr<thigh && *ptr>tlow) nabove++;
	}
      
      if(((float)nabove/(float)ccptr->n_points)>0.9)
	{
	  /*
	  printf("%d %f\n",ccptr->n_points,(float)nabove/(float)ccptr->n_points);
	  */
	  ncc++;
	  for(i=0;i<ccptr->n_points;i++)
	    {
	      ptr = maskptr + ccptr->data[i].x
		+ ccptr->data[i].y * vos->oLine + 
		ccptr->data[i].z * vos->oSlice;
	      *ptr = 255;
	    }
	}
      
      ccptr = ccptr->next;
    }
  printf("%d cavities filled\n",ncc);

  return(OK);
}

/*---------------------------------------------------------------------------*/
Volume *VipComputeRawWhiteMask(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float white_erosion_size,
float white_recovering_size,
int T_white_fat_low,
int T_gray_white
)
/*---------------------------------------------------------------------------*/
{
  Volume *volcopy=NULL, *mask=NULL, *white=NULL, *classif=NULL;
  float erosion_size;
  float dilation_size;
  float threshold_dist;
  int bidon; /*compilation warning*/
  bidon = T_white_fat_low;
  bidon = T_gray_white;
  bidon = 0;

  volcopy = VipCopyVolume(vol,"volcopy");

  classif = VipGrayWhiteClassificationRegularisationForRobust( volcopy, ana, dumb);
  if(classif==PB) return(PB);

  /*VipWriteTivoliVolume(classif,"classif");*/
  if(dumb!=VTRUE)
    {
      printf("-----------------------------\n");
      printf("   White matter extraction\n");
      printf("-----------------------------\n");
    }
  printf("Erosion:%f, Recover:%f\n",white_erosion_size,white_recovering_size);
  /*1 = white part perserved whatever happened in regul*/
  /*VipDoubleThreshold( volcopy, VIP_BETWEEN_OR_EQUAL_TO, 
		      T_gray_white, T_white_fat_low,
		      BINARY_RESULT);*/
  /*
  VipMaskVolumeLevel(volcopy,classif,WHITE_LABEL);
  VipMerge(classif,volcopy,VIP_MERGE_ONE_TO_ONE,255,1);
  */
  white = volcopy;
  VipSetVolumeLevel(white,0);
  VipMerge(white,classif,VIP_MERGE_ONE_TO_ONE,WHITE_LABEL,255);
  VipFreeVolume(classif);
  erosion_size = white_erosion_size;
  dilation_size = (int)(erosion_size+1.);
  threshold_dist = white_recovering_size + dilation_size;

  mask = VipCopyVolume( white, "white");
  if(mask==PB)
    {
      VipPrintfExit("VipComputeRawWhiteMask");
    }
    
  if(dumb!=VTRUE)
    {
      printf("Erosion (mm)...\n");
      fflush(stdout);
    }
  if( VipErosion( mask, CHAMFER_BALL_3D, erosion_size) == PB) return(PB);
  
  if(dumb!=VTRUE)
    {
      printf("Selecting largest 3D 26-connected component (to get white matter seed)...\n");
      fflush(stdout);
    }
  if( VipConnexVolumeFilter( mask, CONNECTIVITY_26, -1, CONNEX_BINARY) == PB) return(PB);
   
  if( VipMerge( white, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);

  VipFreeVolume(mask);

  if(debug==VTRUE)
    {
      VipPrintfInfo("Debug mode: writing white matter + seed");
      VipWriteVolume(white,"white+seed");
    }

  VipChangeIntLabel(white,0,-1000);

  if(dumb!=VTRUE)
    {
      printf("Computing geodesic distance to white matter seed...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(white,255,-1000,VIP_PUT_LIMIT_TO_LIMIT,(int)(threshold_dist+1))==PB) return(PB);

    if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing distance to white matter seed");
	 VipWriteVolume(white,"dist_to_wseed");
      }
    if(dumb!=VTRUE)
      {
	printf("Correction for opening effect (dil:%f/correction:%f)...\n", dilation_size, threshold_dist);
	fflush(stdout);
      }

    mask = VipCreateSingleThresholdedVolume( white, GREATER_OR_EQUAL_TO,  (int)(threshold_dist*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT);
    if(mask==PB)
      {
	VipPrintfExit("VipGetBrainForRenderingDoubleCustomized");
      }
    VipSingleThreshold( white, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT );
    if( VipMerge( white, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);
    VipFreeVolume(mask);

  VipChangeIntLabel(white,0,-100);
  if(dumb!=VTRUE)
    {
      printf("Computing distance to white matter complement...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(white,255,-100,VIP_PUT_LIMIT_TO_LIMIT,(int)(threshold_dist-dilation_size+1))==PB) return(PB);

  if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing distance to white matter complement"); 
	 VipWriteVolume(white,"dist_to_wcomp");
      }

 if(dumb!=VTRUE)
    {
      printf("Selecting largest 3D 26-connected component (recovering can disconnect some objects)...\n");
      fflush(stdout);
    }

 VipSingleThreshold( white, GREATER_OR_EQUAL_TO,  (int)(VIP_USUAL_DISTMAP_MULTFACT*(threshold_dist-dilation_size)), BINARY_RESULT );

 if( VipConnexVolumeFilter( white, CONNECTIVITY_26, -1, CONNEX_BINARY) == PB) return(PB);

  if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing white matter"); 
	 VipWriteVolume(white,"white");
      }

  return(white);
}

/*---------------------------------------------------------------------------*/
int VipGetBrainForRenderingDouble(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug
)
/*---------------------------------------------------------------------------*/
{
  return(VipGetBrainForRenderingDoubleCustomized(vol,ana,dumb,debug,3.1,30,3.1,3,1));
}
/*---------------------------------------------------------------------------*/
int VipGetBrainForRenderingDoubleCustomized(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float white_erosion_size,
int white_recovering_size,
float brain_erosion_size,
int brain_recovering_size,
int nb_iterations
)
/*---------------------------------------------------------------------------*/
{
  Volume *brain=NULL, *mask=NULL, *white=NULL, *classif=NULL;
  float erosion_size;
  int dilation_size;
  int threshold_dist;
  int T_csf_gray, T_white_fat_low, T_gray_white;
  int bidon; /*compilation warning*/
  bidon = nb_iterations;
  bidon = 0;

  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipGetBrainForRenderingDoubleCustomized");
      return(PB);
    }
  if(!ana)
    {
      VipPrintfError("Null VipT1HistoAnalysis!");
      VipPrintfExit("(VipGetBrainForRenderingDoubleCustomized");
      return(PB);
    }

  if(debug==VTRUE)
    {
      VipPrintfWarning("The DEBUG mode generates a lot of images (you need at least 100M of free disk space)");
    }
 
  if(ana->gray!=NULL && ana->white!=NULL)
    {
       if(ana->sequence==MRI_T1_SPGR && ana->partial_volume_effect==VFALSE)
	 T_csf_gray = ana->gray->mean - (2.5* ana->gray->left_sigma);
       else T_csf_gray = ana->gray->mean - (2* ana->gray->left_sigma);
       T_gray_white = (int)(ana->white->mean -(float)( ana->white->mean-ana->gray->mean)
			    /(1.+(float)ana->gray->right_sigma/(float)ana->white->left_sigma) + 0.5);
       T_gray_white = mVipMin(ana->white->mean - 2*ana->white->left_sigma,T_gray_white);
       T_white_fat_low = ana->white->mean + 3* ana->white->right_sigma;
    }
  else if(ana->brain!=NULL)
    {
      VipPrintfError("The histogram scale space analysis did not give gray and white object");
      VipPrintfExit("VipGetBrainForRenderingDoubleCustomized");
      return(PB);
    }
  else
    {
      VipPrintfError("This Histogram analysis has failed somewhere...");
      VipPrintfExit("VipGetBrainForRenderingDoubleCustomized");
      return(PB);
    }

  if(dumb!=VTRUE)
    {
      printf("Initial classification:\n");
      printf("[%d-%d] gray matter\n",T_csf_gray,T_gray_white);
      printf("[%d-%d] white matter\n",T_gray_white,T_white_fat_low);
    }

  classif = VipGrayWhiteClassificationRegularisation( vol, ana, dumb);
  if(classif==PB) return(PB);


  if(dumb!=VTRUE)
    {
      printf("-----------------------------\n");
      printf("   White matter extraction\n");
      printf("-----------------------------\n");
    }
  /*1 = white part perserved whatever happened in morphomath*/
  VipDoubleThreshold( vol, VIP_BETWEEN_OR_EQUAL_TO, 
		      T_gray_white, T_white_fat_low,
		      BINARY_RESULT);
  VipMaskVolumeLevel(vol,classif,WHITE_LABEL);
  VipMerge(classif,vol,VIP_MERGE_ONE_TO_ONE,255,1);

  white = vol;
  VipMerge(white,classif,VIP_MERGE_ONE_TO_ONE,WHITE_LABEL,255);
 
  erosion_size = white_erosion_size;
  dilation_size = (int)(erosion_size+1.);
  threshold_dist = white_recovering_size + dilation_size;

  mask = VipCopyVolume( white, "white");
  if(mask==PB)
    {
      VipPrintfExit("VipGetBrainForRenderingDoubleCustomized");
    }
    
  if(dumb!=VTRUE)
    {
      printf("Erosion (mm)...\n");
      fflush(stdout);
    }
  if( VipErosion( mask, CHAMFER_BALL_3D, erosion_size) == PB) return(PB);
  
  if(dumb!=VTRUE)
    {
      printf("Selecting largest 3D 26-connected component (to get white matter seed)...\n");
      fflush(stdout);
    }
  if( VipConnexVolumeFilter( mask, CONNECTIVITY_26, -1, CONNEX_BINARY) == PB) return(PB);
   
  if( VipMerge( white, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);

  VipFreeVolume(mask);

  if(debug==VTRUE)
    {
      VipPrintfInfo("Debug mode: writing white matter + seed");
      VipWriteVolume(white,"white+seed");
    }

  VipChangeIntLabel(white,0,-1000);

  if(dumb!=VTRUE)
    {
      printf("Computing geodesic distance to white matter seed...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(white,255,-1000,VIP_PUT_LIMIT_TO_LIMIT,threshold_dist)==PB) return(PB);

    if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing distance to white matter seed");
	 VipWriteVolume(white,"dist_to_wseed");
      }
    if(dumb!=VTRUE)
      {
	printf("Correction for opening effect (dil:%d/correction:%d)...\n", dilation_size, threshold_dist);
	fflush(stdout);
      }

    mask = VipCreateSingleThresholdedVolume( white, GREATER_OR_EQUAL_TO,  threshold_dist*VIP_USUAL_DISTMAP_MULTFACT, BINARY_RESULT);
    if(mask==PB)
      {
	VipPrintfExit("VipGetBrainForRenderingDoubleCustomized");
      }
    VipSingleThreshold( white, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT );
    if( VipMerge( white, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);
    VipFreeVolume(mask);



  VipChangeIntLabel(white,0,-100);
  if(dumb!=VTRUE)
    {
      printf("Computing distance to white matter complement...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(white,255,-100,VIP_PUT_LIMIT_TO_LIMIT,threshold_dist-dilation_size)==PB) return(PB);

  if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing distance to white matter complement"); 
	 VipWriteVolume(white,"dist_to_wcomp");
      }

 if(dumb!=VTRUE)
    {
      printf("Selecting largest 3D 26-connected component (recovering can disconnect some objects)...\n");
      fflush(stdout);
    }

 VipSingleThreshold( white, GREATER_OR_EQUAL_TO,  VIP_USUAL_DISTMAP_MULTFACT*(threshold_dist-dilation_size), BINARY_RESULT );

 if( VipConnexVolumeFilter( white, CONNECTIVITY_26, -1, CONNEX_BINARY) == PB) return(PB);


  if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing white matter"); 
	 VipWriteVolume(white,"white");
      }

 
  if(dumb!=VTRUE)
    {
      printf("-----------------------------\n");
      printf("   Whole brain extraction\n");
      printf("-----------------------------\n");
    }

  erosion_size = brain_erosion_size;
  dilation_size = (int)(erosion_size+1);
  threshold_dist = dilation_size + brain_recovering_size;
  
  brain = white;
  VipMerge(brain,classif,VIP_MERGE_ONE_TO_ONE,1,255); /*initial white mask in short range*/
  VipMerge(brain,classif,VIP_MERGE_ONE_TO_ONE,GRAY_LABEL,255);
  VipFreeVolume(classif);

  mask = VipCopyVolume( brain, "mask");

  if(dumb!=VTRUE)
    {
      printf("Erosion of %f (mm)...\n", erosion_size);
      fflush(stdout);
    }
  if( VipErosion( mask, CHAMFER_BALL_3D, erosion_size) == PB) return(PB);
  
  if(dumb!=VTRUE)
    {
      printf("Selecting largest 3D 6-connected component (to get brain seed)...\n");
      fflush(stdout);
    }
  if( VipConnexVolumeFilter( mask, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);
     
  if(dumb!=VTRUE)
    {
      printf("26-connectivity opening of size 1 (to get brain surface)...\n");
      fflush(stdout);
    }

  if(VipConnectivityChamferOpening(brain,1,CONNECTIVITY_26,FRONT_PROPAGATION)==PB) return(PB);
  
  if( VipMerge( brain, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);

  VipFreeVolume(mask);

  if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing brain + seed"); 
	 VipWriteVolume(brain,"brain+seed");
      }

  VipChangeIntLabel(brain,0,-2);
  
  if(dumb!=VTRUE)
    {
      printf("Computing geodesic distance to brain seed...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(brain,255,-2,VIP_PUT_LIMIT_TO_LIMIT,threshold_dist)==PB) return(PB);
  
  if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing distance to brain seed"); 
	 VipWriteVolume(brain,"dist_to_bseed");
      }

  if(dumb!=VTRUE)
    {
      printf("Correction for opening effect (dil:%d/correction:%d)...\n", dilation_size, threshold_dist);
      fflush(stdout);
    }
  mask = VipCreateSingleThresholdedVolume( brain, GREATER_OR_EQUAL_TO,  threshold_dist*VIP_USUAL_DISTMAP_MULTFACT, BINARY_RESULT);
  if(mask==PB)
    {
      VipPrintfExit("VipGetBrainForRendering");
    }

  VipSingleThreshold( brain, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT );

  if( VipMerge( brain, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);

  VipFreeVolume(mask);
 
  VipChangeIntLabel(brain,0,-100);

  if(dumb!=VTRUE)
    {
      printf("Computing distance to brain matter complement...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(brain,255,-100,VIP_PUT_LIMIT_TO_LIMIT,threshold_dist-dilation_size)==PB) return(PB);

 if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing distance to brain complement");  
	 VipWriteVolume(brain,"dist_to_bcomp");
      }

  VipSingleThreshold( brain, GREATER_OR_EQUAL_TO,  VIP_USUAL_DISTMAP_MULTFACT*(threshold_dist-dilation_size), BINARY_RESULT );
   
  if(dumb!=VTRUE)
    {
      printf("Selecting largest 3D 6-connected component (recovering can disconnect some objects)...\n");
      fflush(stdout);
    }
  if( VipConnexVolumeFilter( brain, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);

  return(OK);
}


/*---------------------------------------------------------------------------*/
int VipGetBrainForRenderingFast(
Volume *vol,
int dumb,
int debug,
int Tlow,
int Thigh
)
/*---------------------------------------------------------------------------*/
{
  return(VipGetBrainForRenderingFastCustomized(vol,dumb,debug,3.1,3.1,Tlow,Thigh));
}
/*---------------------------------------------------------------------------*/
int VipGetBrainForRenderingFastCustomized(
Volume *vol,
int dumb,
int debug,
float brain_erosion_size,
float brain_dilation_size,
int Tlow,
int Thigh
)
/*---------------------------------------------------------------------------*/
{
  Volume *mask;
  int dilsize;

  dilsize = (int)(brain_dilation_size+1);
 
  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipGetBrainForRenderingSimpleCustomized");
      return(PB);
    }

  if(dumb!=VTRUE)
    printf("Thresholding for range ]%d,%d[\n",Tlow,Thigh);

  if(VipDoubleThreshold(vol,VIP_BETWEEN,Tlow,Thigh,BINARY_RESULT)==PB)
    return(PB);

  mask = VipCopyVolume(vol,"copy");
  if(mask==PB) return(PB);

 if(dumb!=VTRUE)
    {
      printf("Erosion (mm)...\n");
      fflush(stdout);
    }
  if( VipErosion( mask, CHAMFER_BALL_3D, brain_erosion_size) == PB) return(PB);
 
  if(dumb!=VTRUE)
    {
      printf("Selecting largest 3D 6-connected component (to get brain seed)...\n");
      fflush(stdout);
    }
  if( VipConnexVolumeFilter( mask, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);

  if( VipMerge( vol, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);

  if(debug==VTRUE)
      {
	  VipWriteVolume(vol,"binary+seed");
      }
  VipFreeVolume(mask);
  VipChangeIntLabel(vol,0,30000);

  if(dumb!=VTRUE)
    {
      printf("Computing geodesic distance to brain seed...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(vol,255,30000,VIP_PUT_LIMIT_TO_LIMIT,dilsize+1)==PB) return(PB);
  VipSingleThreshold( vol, LOWER_OR_EQUAL_TO,  dilsize*VIP_USUAL_DISTMAP_MULTFACT, BINARY_RESULT );
      
  return(OK);
}

/*---------------------------------------------------------------------------*/
int VipGetBrainForRenderingSimple(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug
)
/*---------------------------------------------------------------------------*/
{
  return(VipGetBrainForRenderingSimpleCustomized(vol,ana,dumb,debug,3.1,3,1));
}
/*---------------------------------------------------------------------------*/
int VipGetBrainForRenderingSimpleCustomized(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float brain_erosion_size,
int brain_recovering_size,
int nb_iterations
)
/*---------------------------------------------------------------------------*/
{
  Volume *brain=NULL, *mask=NULL, *classif=NULL;
  float erosion_size;
  int dilation_size;
  int threshold_dist;
  /*int T_csf_gray, T_white_fat_low;*/
  float little_opening_size;


  if (VipVerifyAll(vol)==PB || VipTestType(vol,S16BIT)==PB)
    {
      VipPrintfExit("(VipGetBrainForRenderingSimpleCustomized");
      return(PB);
    }
  if(!ana)
    {
      VipPrintfError("Null VipT1HistoAnalysis!");
      VipPrintfExit("(VipGetBrainForRenderingSimpleCustomized");
      return(PB);
    }

  if(debug==VTRUE)
    {
      VipPrintfWarning("The DEBUG mode generates a lot of images (you need at least 50M of free disk space)");
    }
  
  classif = VipCSFGrayWhiteFatClassificationRegularisation( vol, ana, dumb, nb_iterations);
  if(classif==PB) return(PB);
  
  if(dumb!=VTRUE)
    {
      printf("-----------------------------\n");
      printf("   Whole brain extraction\n");
      printf("-----------------------------\n");
    }

  erosion_size = brain_erosion_size;
  dilation_size = (int)(erosion_size+1);
  threshold_dist = dilation_size + brain_recovering_size;
  
  brain = vol;
  VipSetVolumeLevel(brain,0);
  VipMerge(brain,classif,VIP_MERGE_ONE_TO_ONE,BRAIN_LABEL,255);
  
  VipFreeVolume(classif);
  
  classif=NULL;
  
  mask = VipCopyVolume( brain, "mask");

  if(dumb!=VTRUE)
    {
      printf("Erosion (mm)...\n");
      fflush(stdout);
    }
  if( VipErosion( mask, CHAMFER_BALL_3D, erosion_size) == PB) return(PB);
  
  if(dumb!=VTRUE)
    {
      printf("Selecting largest 3D 6-connected component (to get brain seed)...\n");
      fflush(stdout);
    }
  if( VipConnexVolumeFilter( mask, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);
     
  little_opening_size = 1.9;
  if(mVipVolVoxSizeX(brain)>little_opening_size) little_opening_size=mVipVolVoxSizeX(brain)+0.1;
  if(mVipVolVoxSizeY(brain)>little_opening_size) little_opening_size=mVipVolVoxSizeY(brain)+0.1;
  if(mVipVolVoxSizeZ(brain)>little_opening_size) little_opening_size=mVipVolVoxSizeZ(brain)+0.1;

  
  if(dumb!=VTRUE)
    {
      printf("Opening (to get accurate brain surface)...\n");
      fflush(stdout);
    }
  if(VipOpening(brain,CHAMFER_BALL_3D,little_opening_size)==PB) return(PB);

  if( VipMerge( brain, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);

  VipFreeVolume(mask);
  mask = NULL;

  if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing brain + seed"); 
	 VipWriteVolume(brain,"brain+seed");
      }

  VipChangeIntLabel(brain,0,-2);
  
  if(dumb!=VTRUE)
    {
      printf("Computing geodesic distance to brain seed...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(brain,255,-2,VIP_PUT_LIMIT_TO_LIMIT,threshold_dist)==PB) return(PB);
  
  if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing distance to brain seed"); 
	 VipWriteVolume(brain,"dist_to_bseed");
      }

  if(dumb!=VTRUE)
    {
      printf("Correction for opening effect (dil:%d/correction:%d)...\n", dilation_size, threshold_dist);
      fflush(stdout);
    }
  mask = VipCreateSingleThresholdedVolume( brain, GREATER_OR_EQUAL_TO,  threshold_dist*VIP_USUAL_DISTMAP_MULTFACT, BINARY_RESULT);
  if(mask==PB)
    {
      VipPrintfExit("VipGetBrainForRendering");
    }

  VipSingleThreshold( brain, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT );

  if( VipMerge( brain, mask, VIP_MERGE_ONE_TO_ONE, 255, 512 )== PB) return(PB);

  VipFreeVolume(mask);
  mask = NULL;
 
  VipChangeIntLabel(brain,0,-100);

  if(dumb!=VTRUE)
    {
      printf("Computing distance to brain matter complement...\n");
      fflush(stdout);
    }
  if(VipComputeFrontPropagationChamferDistanceMap(brain,255,-100,VIP_PUT_LIMIT_TO_LIMIT,threshold_dist-dilation_size)==PB) return(PB);

 if(debug==VTRUE)
      {
	 VipPrintfInfo("Debug mode: writing distance to brain complement");  
	 VipWriteVolume(brain,"dist_to_bcomp");
      }

  VipSingleThreshold( brain, GREATER_OR_EQUAL_TO,  VIP_USUAL_DISTMAP_MULTFACT*(threshold_dist-dilation_size), BINARY_RESULT );
   
  if(dumb!=VTRUE)
    {
      printf("Selecting largest 3D 6-connected component (recovering can disconnect some objects)...\n");
      fflush(stdout);
    }
  if( VipConnexVolumeFilter( brain, CONNECTIVITY_6, -1, CONNEX_BINARY) == PB) return(PB);
  
  return(OK);
}

/*----------------------------------------------------------------------------*/
int GetSinusBucket(  Volume *initvolsafe, Volume *initbrain,
  VipTalairach *tal,   VipT1HistoAnalysis *ana, Vip3DBucket_S16BIT **buck)
/*----------------------------------------------------------------------------*/
{
  Volume *brain=NULL, *initvol=NULL;
   Vip_S16BIT *ptr;
  float input_point[3], output_point[3];
  VipOffsetStruct *vos;
  int ix, iy, iz;

  if(!initvolsafe || !initbrain || !ana || ! tal || !buck)
    {
      VipPrintfError("Empty arg in GetSinusBucket");
      fflush(stdout);
      return(PB);
    }
  printf("Computing brain external band:\n");

  initvol = VipCopyVolume(initvolsafe,"copy");
  if(!initvol) return(PB);

  brain = VipCopyVolume(initbrain,"temp");
  if(!brain) return(PB);

  if (VipClosing(brain,CHAMFER_BALL_3D,20)==PB) return(PB); /*init = dilated white*/
  if (VipDilation(brain,CHAMFER_BALL_3D,10)==PB) return(PB);
  VipSingleThreshold( brain, EQUAL_TO,  0, BINARY_RESULT );
  VipComputeFrontPropagationChamferDistanceMap(brain,0,-1,
					       VIP_PUT_LIMIT_TO_OUTSIDE,10);
  VipSingleThreshold( brain, 
		      GREATER_OR_EQUAL_TO, 1, BINARY_RESULT);

  
  /*VipWriteTivoliVolume( brain, "bande");*/
  
  VipMerge(brain,initbrain,VIP_MERGE_ALL_TO_ONE,255,0);

  /* VipWriteTivoliVolume( brain, "sinuskiller");*/

  VipMaskVolume(initvol,brain);
  VipFreeVolume(brain);

 *buck = VipAlloc3DBucket_S16BIT(100000);
  if(!*buck) return(PB);

   vos = VipGetOffsetStructure(initvol);
   ptr = VipGetDataPtr_S16BIT( initvol ) + vos->oFirstPoint;

   for ( iz=0; iz<mVipVolSizeZ(initvol); iz++ )               /* loop on slices */
   {
      for ( iy=0; iy < mVipVolSizeY(initvol); iy++ )            /* loop on lines */
      {
         for ( ix = 0; ix < mVipVolSizeX(initvol); ix++ )/* loop on points */
         {
            if(*ptr > (ana->gray->mean-2*ana->gray->sigma)) 
	      {
	      input_point[0] = ix * mVipVolVoxSizeX(initvol);
	      input_point[1] = iy * mVipVolVoxSizeY(initvol);
	      input_point[2] = iz * mVipVolVoxSizeZ(initvol);
	      VipTalairachTransformation(input_point,tal,output_point);
	      if (fabs(output_point[0])<2 && output_point[1]> 0
		  && output_point[2]<20 && (*buck)->n_points<100000)
		{
		  (*buck)->data[(*buck)->n_points].x = ix;
		  (*buck)->data[(*buck)->n_points].y = iy;
		  (*buck)->data[(*buck)->n_points].z = iz;
		  (*buck)->n_points++;
		}
	    else if (fabs(output_point[0])<1 && output_point[1]<= 0
		  && output_point[2]<20 && (*buck)->n_points<100000)
		{
		  (*buck)->data[(*buck)->n_points].x = ix;
		  (*buck)->data[(*buck)->n_points].y = iy;
		  (*buck)->data[(*buck)->n_points].z = iz;
		  (*buck)->n_points++;
		}
	      }
            ptr++;
         }
         ptr += vos->oPointBetweenLine;  /*skip border points*/
      }
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
   }

  VipFreeVolume(initvol);

  return(OK);
}
/******************************************************/
