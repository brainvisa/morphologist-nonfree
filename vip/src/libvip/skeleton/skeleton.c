/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_skeleton.c        * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 12/04/97
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 *****************************************************************************
 *
 * DESCRIPTION : homotopic skeletonizations
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

#include <vip/math.h>
#include <vip/geometry.h>
#include <vip/util.h>
#include <vip/gaussian.h>
#include <vip/connex.h>
#include <vip/skeleton.h>
#include <vip/skeleton_static.h>

/*--------------------------------------------------------------------*/
Volume *ConvertBrainToAltitude(Volume *brain,float sigma,
			       float level0, float levelmax, float wave_erosion, int bwidth, float threshold)
/*--------------------------------------------------------------------*/
{
    Volume *gaussian;
    Volume *meancurv;
    Volume *altitude;
    VipOffsetStruct *vosalt, *vosmc;
    int ix, iy, iz;
    int xsize, ysize, zsize;
    Vip_FLOAT *ptrmc;
    Vip_S16BIT *ptralt;
    int amax;
    float temp;
    char name[VIP_NAME_MAXLEN];
    int base = 0;

    
    if (VipVerifyAll(brain)==PB)
	{
	    VipPrintfExit("(skeleton)ConvertBrainToAltitude");
	    return(PB);
	}
    if((bwidth<0)||(bwidth>50))
	{
	    VipPrintfError("Strange border width\n");
	    VipPrintfExit("(skeleton)ConvertBrainToAltitude");
	    return(PB);
	}

   printf("Watershed based on: init level: %.3f, max level: %.3f, wave erosion: %.3f (threshold %.3f)\n",
	   level0, levelmax, wave_erosion, threshold);

    if((wave_erosion<=0.)||(levelmax<level0))
	{
	    VipPrintfError("Strange erosion parameters\n");
	    VipPrintfExit("(skeleton)ConvertBrainToAltitude");
	    return(PB);
	}
    if(threshold>level0)
      {
	VipPrintfWarning("threshold higher than the water zero level in ConvertBrainToAltitude");
      }
    
    printf("-----------------------------------------------\n");
    printf("Computing ridge measure for the watershed based homotopic skeletonization\n");
    printf("-----------------------------------------------\n");

    amax = (int)((levelmax-level0)/wave_erosion)+1;
    if(amax>120)
	{
	    VipPrintfError("This erosion will last thousand years...\n");
	    VipPrintfExit("(skeleton)ConvertBrainToAltitude");
	    return(PB);
	}

    (void)printf("Computing Gaussian smoothing (sigma %f) of %s...\n",sigma, mVipVolName(brain));
    gaussian = VipDeriche3DGaussian( brain, sigma, NEW_FLOAT_VOLUME);


    (void)printf("Computing %s isophote mean curvature...\n",mVipVolName(brain));
    meancurv = Vip3DGeometry(gaussian,MEAN_CURVATURE);
    VipFreeVolume(gaussian);
    

    (void)strcpy(name,mVipVolName(brain));
    (void)strcat(name,"_altitude");
    altitude = VipDuplicateVolumeStructure (brain,name);
    VipSetType(altitude,S16BIT);
    VipSetBorderWidth(altitude,bwidth);
    if(VipAllocateVolumeData(altitude)==PB)
	{
	    VipPrintfExit("ConvertBrainToAltitude");
	    return(PB);
	}

   vosalt = VipGetOffsetStructure(altitude);
   ptralt = VipGetDataPtr_S16BIT(altitude) + vosalt->oFirstPoint;
   vosmc = VipGetOffsetStructure(meancurv);
   ptrmc = VipGetDataPtr_VFLOAT(meancurv) + vosmc->oFirstPoint;
   xsize = mVipVolSizeX(altitude);
   ysize = mVipVolSizeY(altitude);
   zsize = mVipVolSizeZ(altitude);

   for ( iz = 0; iz < zsize; iz++ )                /* loop on slices */
   {
      for ( iy = 0; iy < ysize; iy++ )             /* loop on lines */
      {
         for ( ix = 0; ix < xsize; ix++ )          /* loop on points */
         {
	     temp = *ptrmc;
	     if(temp>=levelmax) *ptralt = base+amax;
	     else if(temp<=level0)
	       {
		 if(temp>=threshold) *ptralt = -1;
		 else *ptralt = VIP_CANNOT_BECOME_IMMORTAL;
	       }
	     else *ptralt = base+(int)((temp-level0)/wave_erosion)+1;
	     ptralt++;
	     ptrmc++;
         }
         ptralt += vosalt->oPointBetweenLine;  /*skip border points*/
         ptrmc += vosmc->oPointBetweenLine;  /*skip border points*/
      }
      ptralt += vosalt->oLineBetweenSlice; /*skip border lines*/
      ptrmc += vosmc->oLineBetweenSlice; /*skip border lines*/
   }

  VipWriteTivoliVolume(meancurv,"meancurv");
   VipFreeVolume(meancurv);
   
   printf("Writing altitude...\n");
   VipWriteTivoliVolume(altitude,"altitude");
   
   return(altitude);
}
 


/*--------------------------------------------------------------------*/
/*Points become immortals according to immortal_elixir*/ 
/*The front is managed according to front_mode*/
/*WARNING: the erosion is restricted to inside side*/
int VipWatershedHomotopicSkeleton( Volume *vol, Volume *altitude, int immortal_elixir,
				   int inside, int outside)
{
  VipIntBucket *buck, *nextbuck;
  Topology26Neighborhood *topo26;
  VipConnectivityStruct *vcs6;
  int loop, count, immortals;
  int dir2;
  Vip_S16BIT *first, *ptr, *ptr_neighbor;
  Vip_S16BIT *afirst, *aptr, *naptr; 
  int *buckptr, *dirptr;
  int Cbar, Cstar;
  int i;
  Volume *postprocessing;
  Vip3DBucket_S16BIT *border, *borderptr;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipWatershedHomotopicSkeleton");
      return(PB);
    }
  if (VipVerifyAll(altitude)==PB)
    {
      VipPrintfExit("(skeleton)VipWatershedHomotopicSkeleton");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipWatershedHomotopicSkeleton is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipWatershedHomotopicSkeleton");
      return(PB);
    }
  if (VipTestType(altitude,S16BIT)!=OK)
    {
      VipPrintfError("Sorry, altitude should be a S16BIT volume");
      VipPrintfExit("(skeleton)VipWatershedHomotopicSkeleton");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipWatershedHomotopicSkeleton is only implemented with border");
      VipPrintfExit("(skeleton)VipWatershedHomotopicSkeleton");
      return(PB);
    }
  if(VipTestEqualBorderWidth(vol,altitude)==PB)
    {
      VipPrintfError("Sorry, skeletonized voule and altitude should have the same borderwidth");
      VipPrintfExit("(skeleton)VipWatershedHomotopicSkeleton");
      return(PB);
    }
  if((immortal_elixir!=NON_SIMPLE_AND_NON_VOLUME_BECOME_IMMORTAL)
     &&(immortal_elixir!=SURFACES_BECOME_IMMORTAL)
     &&(immortal_elixir!=CURVES_BECOME_IMMORTAL)
     &&(immortal_elixir!=NOBODY_BECOME_IMMORTAL))
    {
      VipPrintfError("Sorry,  Unknown mode to become immortal");
      VipPrintfExit("(skeleton)VipWatershedHomotopicSkeleton");
      return(PB);
    }

  printf("--------------------------------------------\n");
  printf("Watershed driven homotopic skeletonization...\n");
  printf("--------------------------------------------\n");

  printf("Initialization...\n");

  VipSetBorderLevel( vol, outside ); /* already done before but security */

  buck = VipCreateFrontIntBucketHollowObject( vol, CONNECTIVITY_6, VIP_FRONT, inside, outside);
  if(buck==PB) return(PB);
  nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
  if(nextbuck==PB) return(PB);

  topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

  vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );

  first = VipGetDataPtr_S16BIT(vol);
  afirst = VipGetDataPtr_S16BIT(altitude);

  /*initialisation*/
  /*Mark all initial immortals and dwindle the front*/
  
  immortals = 0;

  /*main loop*/
  loop=0;
  count = 1;
  printf("loop:    , immortals:       ");
  while( (loop++<500)&&(count)&&(buck->n_points>0))
    {
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
      printf("loop: %3d, immortals: %6d",loop,immortals);
      fflush(stdout);

      count = 0;

      /*first scan: mark points submitted to erosion*/
      buckptr = buck->data;
      for(i=buck->n_points;i--;) /*erosion from 6-neighbor loop*/
	{
	  ptr = first + *buckptr;
	  if (*ptr!=inside)
	    {
	      if(*(ptr+vcs6->offset[0])==inside || *(ptr+vcs6->offset[1])==inside
		 || *(ptr+vcs6->offset[2])==inside ||
		 *(ptr+vcs6->offset[3])==inside || *(ptr+vcs6->offset[4])==inside
		 || *(ptr+vcs6->offset[5])==inside)
		{
		  aptr = afirst + *buckptr;
		  if(*aptr<=0)
		    *ptr=VIP_CANDIDATE;
		  else
		    *aptr -= 1;
		}
	      else
		{
		  VipPrintfWarning("this point should be 6-neighbor to inside");
		}
	    }
	  buckptr++;
	}
      /*second scan*/
      buckptr = buck->data;
      for(i=buck->n_points;i--;)
	{
	  ptr = first + *buckptr;
	  aptr = afirst + *buckptr;
	  if((*ptr!=inside) && (*ptr != VIP_IMMORTAL)) /*the bucket point is still alive*/
	    {
	      if( VipWillBecomeImmortalForTwoLabelComplement_S16BIT(topo26,immortal_elixir,ptr,
								    inside,outside,&Cbar,&Cstar)==VTRUE)
		{
		  if(loop>1 && (*aptr!=VIP_CANNOT_BECOME_IMMORTAL || loop>5))
		    /*to prevent bad immortal to be created (bad ss point caracterization)*/
		    {
		      *ptr = VIP_IMMORTAL; 
		      immortals++;
		    }
		  else *ptr = VIP_FRONT;
		}
	      else
		{			
		  if (*ptr==VIP_CANDIDATE) /*the only one that can die at this moment*/
		    {
		      if((Cbar==1)&&(Cstar==1)) /*simple point*/
			{
			  *ptr = inside;
			  count++;
			}
		      else *ptr = VIP_FRONT;
		    }
		  else *ptr = VIP_FRONT; 
		}
	    } 
	  if(*ptr==inside) /*include the case of point dead in previous lines*/
	    {       /*in fact we treat here all "non volume" points not in front*/
	      dirptr = vcs6->offset; /*a priori, 18-neighbors should not see increase of Cbar*/
	      for(dir2=vcs6->nb_neighbors;dir2--;) /*therefore, will not become surface point*/
		{
		  ptr_neighbor = ptr + *dirptr;
		  if((*ptr_neighbor!=inside)&&(*ptr_neighbor!=outside)
		     &&(*ptr_neighbor!=VIP_IMMORTAL)) /*can be in front*/
		    {
		      if( VipWillBecomeImmortalForTwoLabelComplement_S16BIT(topo26,immortal_elixir,
									    ptr_neighbor,inside,
									    outside,&Cbar,&Cstar)==VTRUE)
			{
			  naptr = aptr + *dirptr;
			  if(loop>1 && (*naptr!=VIP_CANNOT_BECOME_IMMORTAL || loop>4)) 
			    /*to prevent bad immortal to be created (bad ss point caracterization)*/
			    {
			      *ptr_neighbor = VIP_IMMORTAL; 
			      immortals++;
			    }
			} /*otherwise this point wait until being in the front*/			  
		    }
		  dirptr++;
		}
	    }
	  buckptr++; 
	} /*here the vc6 choice is a choice on the front propagation mode*/
      VipFillNextFrontFromOldFrontHollowObject(first,buck,nextbuck,vcs6,VIP_FRONT,
					       inside, outside);
		  
      /*bucket permutation*/
      VipPermuteTwoIntBucket(&buck, &nextbuck);
      nextbuck->n_points = 0;
		  
	  
    }

  printf("\n");
  
  VipFreeIntBucket(buck);
  VipFreeIntBucket(nextbuck);
  VipWriteVolume(vol, "whaterskeleton_bf_postproc");
  /*because of the bad trick related to the multiresolution implementation,
    cf construction of the front*/
  postprocessing = VipCreateSingleThresholdedVolume(vol,EQUAL_TO,inside,BINARY_RESULT);
  if(postprocessing==PB) return(PB);
  border = VipGetDualFiltered3DConnex(postprocessing, CONNECTIVITY_6, -1);
  VipFreeVolume(postprocessing);

  if(border!=NULL)
    {
      borderptr = border;
      while(borderptr!=NULL)
	{
	  VipWriteCoordBucketInVolume(borderptr,vol,outside);
	  borderptr = borderptr->next;
	}
    }
  VipFree3DBucketList_S16BIT(border);
	
  /*
    if(VipSingleThreshold(vol,NOT_EQUAL_TO,0,BINARY_RESULT)==PB) return(PB);
  */
  
  return(OK);
}

/*--------------------------------------------------------------------*/
/*Points become immortals according to immortal_elixir*/
/*The front is managed according to front_mode*/

int VipHomotopicSkeleton( Volume *vol, int immortal_elixir, int front_mode )
{
  VipIntBucket *buck, *nextbuck;
  Topology26Neighborhood *topo26;
  VipConnectivityStruct *vcs6;
  int loop, count, immortals;
  int dir, dir2;
  Vip_S16BIT *first, *ptr, *ptr_neighbor;
  int *buckptr, *dirptr, diroffset;
  int Cbar, Cstar;
  int i;

  if((immortal_elixir!=NON_SIMPLE_AND_NON_VOLUME_BECOME_IMMORTAL)
     &&(immortal_elixir!=SURFACES_BECOME_IMMORTAL)
     &&(immortal_elixir!=CURVES_BECOME_IMMORTAL)
     &&(immortal_elixir!=NOBODY_BECOME_IMMORTAL))
  {
    VipPrintfError("Sorry,  Unknown mode to become immortal");
    VipPrintfExit("(skeleton)VipHomotopicSkeleton");
    return(PB);
  }
  if(front_mode!=FRONT_6CONNECTIVITY_DIRECTION)     
  {
    VipPrintfError("Sorry,  Unknown front management mode");
    VipPrintfExit("(skeleton)VipHomotopicSkeleton");
    return(PB);
  }
  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipHomotopicSkeleton");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipHomotopicSkeleton6cDistanceRelatedFront is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipHomotopicSkeleton");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipHomotopicSkeleton6cDistanceRelatedFront is only implemented with border");
      VipPrintfExit("(skeleton)VipHomotopicSkeleton");
      return(PB);
    }

  printf("Initialization...\n");

  VipSetBorderLevel( vol, 0 ); /* already done before but security */


  VipSingleThreshold( vol, NOT_EQUAL_TO, 0, BINARY_RESULT ); /* put object (including potential VIP_IMMORTAL)
								label  to 255 */
  buck = VipCreateFrontIntBucket( vol, CONNECTIVITY_6, VIP_FRONT);
  if(buck==PB) return(PB);
  nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
  if(nextbuck==PB) return(PB);




  topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

  vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );

  first = VipGetDataPtr_S16BIT(vol);

  /*initialisation*/
  /*Mark all initial immortals and dwindle the front*/
  immortals = 0;

  VipCleanUpFrontFromImmortals(topo26,immortal_elixir,first,buck,nextbuck,&immortals,VIP_FRONT);
    /*bucket permutation*/
  VipPermuteTwoIntBucket(&buck, &nextbuck);
  nextbuck->n_points = 0;


  printf("loop: 0, just killed: 0, total immortals: %d\n",immortals);


  /*main loop*/
  loop=0;
  count = 1;


  while((loop++<500)&&(count)&&(buck->n_points>0))
      {
	  count = 0;
	  printf("Front size: %d\n",buck->n_points);
	  for (dir=0;dir<6;dir++)
	      {
		  diroffset = vcs6->offset[(dir+count)%6];
		  /*first scan: mark direction-related candidates*/
		  buckptr = buck->data;
		  for(i=buck->n_points;i--;)
		      {
 			  ptr = first + *buckptr;
			  if ((*ptr) && (!*(ptr+diroffset)))
			      {				  
				  *ptr=VIP_CANDIDATE;
			      }
			  buckptr++;
		      }
		  /*second scan*/
		  buckptr = buck->data;
		  for(i=buck->n_points;i--;)
		      {
			  ptr = first + *buckptr;
			  if((*ptr) && (*ptr != VIP_IMMORTAL)) /*the bucket point is still alive*/
			      {
				  if( VipWillBecomeImmortalForLabelComplement_S16BIT(topo26,immortal_elixir,ptr,0,&Cbar,&Cstar)==VTRUE)
				      {
					  *ptr = VIP_IMMORTAL; 
					  immortals++;
				      }
				  else
				      {			
					  if (*ptr==VIP_CANDIDATE) /*the only one that can die at this moment*/
					      {
						  if((Cbar==1)&&(Cstar==1)) /*simple point*/
						      {
							  *ptr = 0;
							  count++;
						      }
						  else *ptr = VIP_FRONT;
					      }
					  else *ptr = VIP_FRONT;
				      }
			      } 
			  if(!*ptr) /*include the case of point dead in previous lines*/
			      {       /*in fact we treat here all "non volume" points not in front*/
				  dirptr = vcs6->offset;
				  for(dir2=vcs6->nb_neighbors;dir2--;)
				      {
					  ptr_neighbor = ptr + *dirptr;
					  if((*ptr_neighbor)&&(*ptr_neighbor!=VIP_IMMORTAL)) /*can be in front*/
					      {
						  if( VipWillBecomeImmortalForLabelComplement_S16BIT(topo26,immortal_elixir,ptr_neighbor,0,&Cbar,&Cstar)==VTRUE)
						      {
							  *ptr_neighbor = VIP_IMMORTAL; 
							  immortals++;
						      } /*otherwise this point wait until being in the front*/			  
					      }
					  dirptr++;
				      }
			      }
			  buckptr++; 
		      }
		  VipFillNextFrontFromOldFront(first,buck,nextbuck,vcs6,255,VIP_FRONT);
		  
		  /*bucket permutation*/
		  VipPermuteTwoIntBucket(&buck, &nextbuck);
		  nextbuck->n_points = 0;
		  
		  printf("loop: %d.%d, total killed: %d, total immortals: %d\n",loop,dir+1,count,immortals);
	      }
	  
      }
  
  VipFreeIntBucket(buck);
  VipFreeIntBucket(nextbuck);

  if(VipSingleThreshold(vol,NOT_EQUAL_TO,0,BINARY_RESULT)==PB) return(PB);

  return(OK);
}      

/*---------------------------------------------------------------------------*/
/*Put all non volume point in front*/
VipIntBucket *VipCreateFrontIntBucket( Volume *vol, int connectivity, int front_value)
{ 
 
  Vip_S16BIT *ptr, *voisin;
  int i, NbTotalPts;
  VipIntBucket *buck;
  VipConnectivityStruct *vcs;
  int icon;
  

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipCreateFrontIntBucket");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipCreateFrontIntBucket is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipCreateFrontIntBucket");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry,  VipCreateFrontIntBucket is only implemented with border");
      VipPrintfExit("(skeleton)VipCreateFrontIntBucket");
      return(PB);
    }

  VipSetBorderLevel( vol, 0 );
  /* NB: here the object is embeded in the border like in void */

  vcs = VipGetConnectivityStruct( vol, connectivity );

  ptr = VipGetDataPtr_S16BIT(vol);

  NbTotalPts = VipOffsetVolume(vol);
 
  buck = VipAllocIntBucket(VIP_INITIAL_FRONT_SIZE);

  for ( i=0; i<NbTotalPts; i++ )
    {
      if (*ptr)            
	{	  
	  for ( icon=0;icon<vcs->nb_neighbors;icon++)
	    {
	      voisin = ptr + vcs->offset[icon];	      
	      if(!*voisin)
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

/*---------------------------------------------------------------------------*/
/*Put all non volume point in front*/
VipIntBucket *VipCreateFrontIntBucketHollowObject( Volume *vol, int connectivity, int front_value,
						   int inside, int outside)
{ 
    /* the hollow object is eroded from inside*/
  Vip_S16BIT *ptr, *voisin;
  int i, NbTotalPts;
  VipIntBucket *buck;
  VipConnectivityStruct *vcs;
  int icon;
  

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipCreateFrontIntBucketHollowObject");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipCreateFrontIntBucketHollowObject is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipCreateFrontIntBucketHollowObject");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry,  VipCreateFrontIntBucketHollowObject is only implemented with border");
      VipPrintfExit("(skeleton)VipCreateFrontIntBucketHollowObject");
      return(PB);
    }

  VipSetBorderLevel( vol, inside );
  /* NB: here the border is put to inside, which is FALSE,
   but this is a trick to allow erosion in places where the multiresolution implementation of homotopic snake
lead to a bad effect, the border of the brain hull is the image border which is bad.
I guess it could be corrected there, but I do not see how for the moment (the initial image
should be embedded in a larger image). The problem occurs when the process is trigerred from
a normalized image where the field of view has been cropped at minimum Talairach by SPM.
This lead usually to a brain hull slightly inside the brain mask.
Maybe one day I will do better...
This trick implies a postprocessing to detect inside label connected component outside the brain*/

  vcs = VipGetConnectivityStruct( vol, connectivity );

  ptr = VipGetDataPtr_S16BIT(vol);

  NbTotalPts = VipOffsetVolume(vol);
 
  buck = VipAllocIntBucket(VIP_INITIAL_FRONT_SIZE);

  for ( i=0; i<NbTotalPts; i++ )
    {
      if (*ptr!=inside && *ptr!=outside)            
	{	  
	  for ( icon=0;icon<vcs->nb_neighbors;icon++)
	    {
	      voisin = ptr + vcs->offset[icon];	      
	      if(*voisin==inside)
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
/*---------------------------------------------------------------------------*/
int VipRandomizeFrontOrder(VipIntBucket *front, int nloop)
{ 
  int i;
  int temp;
  RandomBox *rb;
  int n1, n2;
   
  if(nloop>1000 || nloop<1)
     {
	 VipPrintfError("Strange nloop");
	  VipPrintfError("VipRandomizeFrontOrder");
	  return(PB);
      }

  rb = VipCreateRandomBoxCircularList(2*front->n_points+1111,255);
    if(!rb)
      {
	  VipPrintfError("VipRandomizeFrontOrder");
	  return(PB);
      }

  for(i=0;i<front->n_points * nloop;i++)
      {
	  n1 = (i+rb->label + 255*rb->next->label)% front->n_points;
	  rb = rb->next->next;
	  n2 = (i+rb->label + 255*rb->next->label)% front->n_points;
	  rb = rb->next->next;
	  temp = front->data[n1];
	  front->data[n1] = front->data[n2];
	  front->data[n2] = temp;
      }

	 
  VipFreeRandomBox(rb);
  return(OK);
}


/*-------------------------------------------------------------------------*/
int VipFillNextFrontFromOldFront(
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  VipConnectivityStruct *vcs,
  int next_value,
  int front_value)
{
  int *buckptr, *dirptr;
  Vip_S16BIT *ptr, *ptr_neighbor;
  int i, dir2;

  if(first_vol_point==NULL)
    {
      VipPrintfError("NULL pointer in VipFillNextFrontFromOldFront");
      VipPrintfExit("VipFillNextFrontFromOldFront");
      return(PB);
    }
 if((buck==NULL) || (nextbuck==NULL))
    {
      VipPrintfError("One NULL bucket in VipFillNextFrontFromOldFront");
      VipPrintfExit("VipFillNextFrontFromOldFront");
      return(PB);
    }
  if(vcs==NULL)
    {
      VipPrintfError("NULL VipConnectivityStruct pointer in VipFillNextFrontFromOldFront");
      VipPrintfExit("VipFillNextFrontFromOldFront");
      return(PB);
    }

 buckptr = buck->data;
 for(i=buck->n_points;i--;)
   {
     ptr = first_vol_point + *buckptr;
     if( *ptr==front_value)
       {
	 if(nextbuck->n_points==nextbuck->size)
	   {
	     if(VipIncreaseIntBucket(nextbuck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
	   }
	 nextbuck->data[nextbuck->n_points++]=*buckptr;
       }
     else if (!*ptr)
       {
	 dirptr = vcs->offset;
	 for(dir2=vcs->nb_neighbors;dir2--;)
	   {
	     ptr_neighbor = ptr + *dirptr;
	     if(*ptr_neighbor==next_value) /*non zero, not immortal, AND NOT ALREADY IN FRONT*/
	       {		     
		 *ptr_neighbor = front_value;
		 if(nextbuck->n_points==nextbuck->size)
		   {
		     if(VipIncreaseIntBucket(nextbuck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
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

/*-------------------------------------------------------------------------*/
int VipFillNextFrontFromOldFrontHollowObject(
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  VipConnectivityStruct *vcs,
  int front_value,
  int inside,
  int outside)
{
  int *buckptr, *dirptr;
  Vip_S16BIT *ptr, *ptr_neighbor;
  int i, dir2;

  if(first_vol_point==NULL)
    {
      VipPrintfError("NULL pointer in VipFillNextFrontFromOldFrontHollowObject");
      VipPrintfExit("VipFillNextFrontFromOldFrontHollowObject");
      return(PB);
    }
 if((buck==NULL) || (nextbuck==NULL))
    {
      VipPrintfError("One NULL bucket in VipFillNextFrontFromOldFrontHollowObject");
      VipPrintfExit("VipFillNextFrontFromOldFrontHollowObject");
      return(PB);
    }
  if(vcs==NULL)
    {
      VipPrintfError("NULL VipConnectivityStruct pointer in VipFillNextFrontFromOldFrontHollowObject");
      VipPrintfExit("VipFillNextFrontFromOldFrontHollowObject");
      return(PB);
    }

 buckptr = buck->data;
 for(i=buck->n_points;i--;)
   {
     ptr = first_vol_point + *buckptr;
     if( *ptr==front_value)
       {
	 if(nextbuck->n_points==nextbuck->size)
	   {
	     if(VipIncreaseIntBucket(nextbuck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
	   }
	 nextbuck->data[nextbuck->n_points++]=*buckptr;
       }
     else if (*ptr==inside)
       {
	 dirptr = vcs->offset;
	 for(dir2=vcs->nb_neighbors;dir2--;)
	   {
	     ptr_neighbor = ptr + *dirptr;
	     if(*ptr_neighbor!=inside && *ptr_neighbor!=outside 
		&& *ptr_neighbor!=VIP_IMMORTAL && *ptr_neighbor!=front_value)
		 /*non zero, not immortal, AND NOT ALREADY IN FRONT*/
	       {		     
		 *ptr_neighbor = front_value;
		 if(nextbuck->n_points==nextbuck->size)
		   {
		     if(VipIncreaseIntBucket(nextbuck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
		   }
		 nextbuck->data[nextbuck->n_points++]=*buckptr+*dirptr;
	       }
	     dirptr++;
	   }
       }
     else if (*ptr==outside)
	 {
	     VipPrintfWarning("This should be inside value");
	 }
     buckptr++;
   }

 return(OK);
}



/*-------------------------------------------------------------------------*/
/*Assume there is no volume point in front, remove all IMMORTALS (accordind to immortal_elixir) from front*/
/*permute buvk and nextbuck buffers*/
int VipCleanUpFrontFromImmortals(
  Topology26Neighborhood *topo26,
  int immortal_elixir,
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  int *immortals,
  int front_value)
{

  int *buckptr;
  Vip_S16BIT *ptr;
  int i;
  int Cbar, Cstar;

 if((immortal_elixir!=NON_SIMPLE_AND_NON_VOLUME_BECOME_IMMORTAL)
     &&(immortal_elixir!=SURFACES_BECOME_IMMORTAL)
     &&(immortal_elixir!=CURVES_BECOME_IMMORTAL)
    &&(immortal_elixir!=NOBODY_BECOME_IMMORTAL))
  {
    VipPrintfError("Sorry,  Unkown mode to become immortal");
      VipPrintfExit("(skeleton)VipCleanUpFrontFromImmortals");
      return(PB);
    }

  if(topo26==NULL)
    {
      VipPrintfError("NULL Topology26Neighborhood pointer in VipCleanUpFrontFromImmortals");
      VipPrintfExit("VipCleanUpFrontFromImmortals");
      return(PB);
    }
  if(first_vol_point==NULL)
    {
      VipPrintfError("NULL VipIntBucket pointer in VipCleanUpFrontFromImmortals");
      VipPrintfExit("VipCleanUpFrontFromImmortals");
      return(PB);
    }
 if((buck==NULL) || (nextbuck==NULL))
    {
      VipPrintfError("One NULL bucket in VipCleanUpFrontFromImmortals");
      VipPrintfExit("VipCleanUpFrontFromImmortals");
      return(PB);
    }

  buckptr = buck->data;

  for(i=buck->n_points;i--;) /*Mark all initial immortals and dwindle the front*/
    {
      ptr = first_vol_point + *buckptr;
      if( VipWillBecomeImmortalForLabelComplement_S16BIT(topo26,immortal_elixir,ptr,0,&Cbar,&Cstar)==VTRUE)	
	{
	  *ptr = VIP_IMMORTAL; 
	  (*immortals)++;
	}
      else
	{
	  *ptr=front_value; 
	  if(nextbuck->n_points==nextbuck->size)
	    {
	      if(VipIncreaseIntBucket(nextbuck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
	    }
	  nextbuck->data[nextbuck->n_points++] = *buckptr;
	}
      buckptr++;
    }

  return(OK);
}

/*-------------------------------------------------------------------------*/
/*Assume there is no volume point in front, remove all IMMORTALS (accordind to immortal_elixir) from front*/
/*permute buvk and nextbuck buffers*/
int VipCleanUpFrontFromImmortalsHollowObject(
  Topology26Neighborhood *topo26,
  int immortal_elixir,
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  int *immortals,
  int front_value,
  int inside,
  int outside)
{

  int *buckptr;
  Vip_S16BIT *ptr;
  int i;
  int Cbar, Cstar;

 if((immortal_elixir!=NON_SIMPLE_AND_NON_VOLUME_BECOME_IMMORTAL)
     &&(immortal_elixir!=SURFACES_BECOME_IMMORTAL)
     &&(immortal_elixir!=CURVES_BECOME_IMMORTAL)
    &&(immortal_elixir!=NOBODY_BECOME_IMMORTAL))
  {
    VipPrintfError("Sorry,  Unkown mode to become immortal");
      VipPrintfExit("(skeleton)VipCleanUpFrontFromImmortalsHollowObject");
      return(PB);
    }

  if(topo26==NULL)
    {
      VipPrintfError("NULL Topology26Neighborhood pointer in VipCleanUpFrontFromImmortalsHollowObject");
      VipPrintfExit("VipCleanUpFrontFromImmortalsHollowObject");
      return(PB);
    }
  if(first_vol_point==NULL)
    {
      VipPrintfError("NULL VipIntBucket pointer in VipCleanUpFrontFromImmortalsHollowObject");
      VipPrintfExit("VipCleanUpFrontFromImmortalsHollowObject");
      return(PB);
    }
 if((buck==NULL) || (nextbuck==NULL))
    {
      VipPrintfError("One NULL bucket in VipCleanUpFrontFromImmortalsHollowObject");
      VipPrintfExit("VipCleanUpFrontFromImmortalsHollowObject");
      return(PB);
    }

  buckptr = buck->data;

  for(i=buck->n_points;i--;) /*Mark all initial immortals and dwindle the front*/
    {
      ptr = first_vol_point + *buckptr;
      if( VipWillBecomeImmortalForTwoLabelComplement_S16BIT(topo26,immortal_elixir,ptr,
							    inside,outside,&Cbar,&Cstar)==VTRUE)	
	{
	  *ptr = VIP_IMMORTAL; 
	  (*immortals)++;
	}
      else
	{
	  *ptr=front_value; 
	  if(nextbuck->n_points==nextbuck->size)
	    {
	      if(VipIncreaseIntBucket(nextbuck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
	    }
	  nextbuck->data[nextbuck->n_points++] = *buckptr;
	}
      buckptr++;
    }

  return(OK);
}
