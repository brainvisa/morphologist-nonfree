/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : skeleton             * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 27/07/99
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 *****************************************************************************
 *
 * DESCRIPTION : homotopic geodesic erosion
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

/*--------------------------------------------------------------------*/
/*Points become immortals according to immortal_elixir*/
/*The front is managed according to front_mode*/


#include <vip/math.h>
#include <vip/skeleton.h>
#include <vip/skeleton_static.h>


/*---------------------------------------------------------------------------*/
//static VipIntBucket *VipCreateFrontIntBucketForErosionFromOutside( Volume *vol, int connectivity, int front_value,
//				       int object, int outside);
/*---------------------------------------------------------------------------*/
//
/*-------------------------------------------------------------------------*/
//static int VipFillNextFrontFromOldFrontForErosionFromOutside(
//  Vip_S16BIT *first_vol_point,
//  VipIntBucket *buck,
//  VipIntBucket *nextbuck,
//  VipConnectivityStruct *vcs,
//  int next_value,
//  int front_value,
//  int outside,
//  int inside);
/*-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static float deltaGPotentialGtoW(int glevel,float KG,float mG,float sigmaG,
			     float KW,float mW,float sigmaW);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipHomotopicErosionFromInsideSnake( Volume *vol, Volume *graylevel, int nb_iteration,
				  int object, int inside, int outside,
					float KISING, float mG, float sigmaG, float mW, float sigmaW)
/*-------------------------------------------------------------------------*/

{
  VipIntBucket *buck, *nextbuck;
  Topology26Neighborhood *topo26;
  VipConnectivityStruct *vcs6;
  int loop, count;
  Vip_S16BIT *first, *ptr, *gfirst, *gptr, *voisin;
  int *buckptr;
  int i;
  float KW, KG;
  int nnG, nnW;
  float deltaU;
  int icon;
  int totalcount;
  int valeur;

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

  nextbuck->n_points = 0;

  first = VipGetDataPtr_S16BIT(vol);
  gfirst = VipGetDataPtr_S16BIT(graylevel);

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
	      gptr = gfirst + *buckptr++;
	      nnG = 0;
	      nnW = 0;
	      deltaU = 0;
	      for ( icon = 0;icon<vcs6->nb_neighbors;icon++)
		  {
		      voisin = ptr + vcs6->offset[icon];	      
		      if(*voisin==inside) nnW++;
		      else nnG++;
		  }
	      deltaU = -KISING*(nnW-nnG);
	      deltaU += deltaGPotentialGtoW(*gptr,KG,mG,sigmaG,KW,mW,sigmaW);
	      if(deltaU<=0)
		  {
		      if (VipComputeTopologicalClassificationForTwoLabelComplement_S16BIT(topo26, ptr, inside, outside)
			  ==TOPO_BORDER_POINT)
			  {
			      *ptr = inside;
			      count++;
			  }
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
  
  VipFreeTopology26Neighborhood(topo26);
  VipFreeConnectivityStruct(vcs6);
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
int VipHomotopicGeodesicErosionFromOutside( Volume *vol, int nb_iteration,
				  int object, int inside, int outside)
/*-------------------------------------------------------------------------*/

{
  VipIntBucket *buck, *nextbuck;
  Topology26Neighborhood *topo26;
  VipConnectivityStruct *vcs6;
  int loop, count;
  Vip_S16BIT *first, *ptr;
  int *buckptr;
  int i;
  int totalcount;
  int valeur;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipHomotopicGeodesicErosionFromOutside");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipHomotopicGeodesicErosionFromOutside is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipHomotopicGeodesicErosionFromOutside");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipHomotopicGeodesicErosionFromOutside is only implemented with border");
      VipPrintfExit("(skeleton)VipHomotopicGeodesicErosionFromOutside");
      return(PB);
    }

  if(object==outside)
    {
      VipPrintfError("object value = outside value!");
      VipPrintfExit("(skeleton)VipHomotopicGeodesicErosionFromOutside");
      return(PB);
    }
  printf("Homotopic erosion from outside...\n");
  /*
    printf("Initialization (object:%d, inside:%d, outside:%d)...\n",object,inside,outside);
  */
  VipSetBorderLevel( vol, outside ); 

  buck = VipCreateFrontIntBucketForErosionFromOutside( vol, CONNECTIVITY_6, VIP_FRONT, object, outside);
  if(buck==PB) return(PB);
  nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
  if(nextbuck==PB) return(PB);

  topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

  vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );

  nextbuck->n_points = 0;

  first = VipGetDataPtr_S16BIT(vol);

  /*main loop*/
  loop=0;
  count = 1;
  totalcount = 0;
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
	  ptr = first + *buckptr++;
	  if (VipComputeTopologicalClassificationForTwoLabelComplement_S16BIT(topo26, ptr, inside, outside)==TOPO_BORDER_POINT)
	    {
	      *ptr = outside;
	      count++;
	    }
	}
      VipFillNextFrontFromOldFrontForErosionFromOutside(first,buck,nextbuck,vcs6,object,VIP_FRONT,outside,inside);
		  
      /*bucket permutation*/
      VipPermuteTwoIntBucket(&buck, &nextbuck);
      nextbuck->n_points = 0;
    }
  printf("\n");
  VipChangeIntLabel(vol,VIP_FRONT,object);
  VipChangeIntLabel(vol,VIP_IMMORTAL,object);  
  
  VipFreeConnectivityStruct(vcs6);
  VipFreeTopology26Neighborhood(topo26);
  VipFreeIntBucket(buck);
  VipFreeIntBucket(nextbuck);
  
  return(OK);
}      

/*---------------------------------------------------------------------------*/
int VipHomotopicErosionFromInside( Volume *vol, Volume *graylevel, int nb_iteration,
				  int object, int inside, int outside )
/*-------------------------------------------------------------------------*/

{
  VipIntBucket *buck, *nextbuck;
  Topology26Neighborhood *topo26;
  VipConnectivityStruct *vcs6;
  int loop, count, totalcount;
  Vip_S16BIT *first, *ptr, *gfirst, *gptr, *voisin;
  int *buckptr;
  int i;
  int icon;
  int valeur;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipHomotopicErosionFromInside");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipHomotopicErosionFromInside is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipHomotopicErosionFromInside");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
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
  /*
  printf("Initialization (object:%d, inside:%d, outside:%d)...\n",object,inside,outside);
  */
  VipSetBorderLevel( vol, outside ); 

  buck = VipCreateFrontIntBucketForErosionFromOutside( vol, CONNECTIVITY_6, VIP_FRONT, object, inside);
  if(buck==PB) return(PB);
  nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
  if(nextbuck==PB) return(PB);

  topo26 = VipCreateTopology26Neighborhood( vol );
  if(topo26==PB) return(PB);

  vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );

  nextbuck->n_points = 0;

  first = VipGetDataPtr_S16BIT(vol);
  gfirst = VipGetDataPtr_S16BIT(graylevel);

  /*main loop*/
  loop=0;
  count = 1;
  totalcount = 0;
  printf("loop: %3d, Deleted %6d",loop,0);

  while((loop++<nb_iteration)&&(count)&&(buck->n_points>0))
  {
      if(loop==1) count=0;
      totalcount += count;
      count = 0;
      printf("\rloop: %3d, Deleted: %6d",loop,totalcount);
      fflush(stdout);
      
      buckptr = buck->data;
      for(i=buck->n_points;i--;)
      {
          ptr = first + *buckptr;
          gptr = gfirst + *buckptr++;
          if(*gptr>1)
          {
              if(VipComputeTopologicalClassificationForTwoLabelComplement_S16BIT(topo26,ptr,inside,outside)==TOPO_BORDER_POINT)
              {
                  *ptr = inside;
                  count++;
              }
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
  
  VipFreeConnectivityStruct(vcs6);
  VipFreeTopology26Neighborhood(topo26);
  VipFreeIntBucket(buck);
  VipFreeIntBucket(nextbuck);
  
  return(OK);
}

/*---------------------------------------------------------------------------*/
VipIntBucket *VipCreateFrontIntBucketForErosionFromOutside( Volume *vol, int connectivity, int front_value,
				       int object, int outside)
/*---------------------------------------------------------------------------*/
{ 
 
  Vip_S16BIT *ptr, *voisin;
  int i, NbTotalPts;
  VipIntBucket *buck;
  VipConnectivityStruct *vcs;
  int icon;
  

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipCreateFrontIntBucketForErosionFromOutside");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipCreateFrontIntBucketForErosionFromOutside is only implemented for S16BIT volume");
      VipPrintfExit("(skeleton)VipCreateFrontIntBucketForErosionFromOutside");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1) 
    {
      VipPrintfError("Sorry,  VipCreateFrontIntBucketForErosionFromOutside is only implemented with border");
      VipPrintfExit("(skeleton)VipCreateFrontIntBucketForErosionFromOutside");
      return(PB);
    }

  vcs = VipGetConnectivityStruct( vol, connectivity );

  ptr = VipGetDataPtr_S16BIT(vol);

  NbTotalPts = VipOffsetVolume(vol);
 
  buck = VipAllocIntBucket(VIP_INITIAL_FRONT_SIZE);

  for ( i=0; i<NbTotalPts; i++ )
    {
      if (*ptr == object)            
	{	  
	  for ( icon=0;icon<vcs->nb_neighbors;icon++)
	    {
	      voisin = ptr + vcs->offset[icon];	      
	      if(*voisin==outside)
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
int VipFillNextFrontFromOldFrontForErosionFromOutside(
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  VipConnectivityStruct *vcs,
  int next_value,
  int front_value,
  int outside,
  int inside)
/*-------------------------------------------------------------------------*/
{
    int *buckptr, *dirptr;
    Vip_S16BIT *ptr, *ptr_neighbor;
    int i, dir2;

    if(first_vol_point==NULL)
	{
	    VipPrintfError("NULL pointer in VipFillNextFrontFromOldFrontForErosionFromOutside");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForErosionFromOutside");
	    return(PB);
	}
    if((buck==NULL) || (nextbuck==NULL))
	{
	    VipPrintfError("One NULL bucket in VipFillNextFrontFromOldFrontForErosionFromOutside");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForErosionFromOutside");
	    return(PB);
	}
    if(vcs==NULL)
	{
	    VipPrintfError("NULL VipConnectivityStruct pointer in VipFillNextFrontFromOldFrontForErosionFromOutside");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForErosionFromOutside");
	    return(PB);
	}

    buckptr = buck->data;
    for(i=buck->n_points;i--;)
	{
	    ptr = first_vol_point + *buckptr;
	    if(*ptr==front_value) /*the point was not simple and has not been deleted from the object*/
		{
		    dirptr = vcs->offset;
		    for(dir2=vcs->nb_neighbors;dir2--;)
			{
			    ptr_neighbor = ptr + *dirptr;
			    if(*ptr_neighbor==inside) /*the current point will never be deleted*/
				{		     
				    *ptr=VIP_IMMORTAL;
				    break;
				}
			    dirptr++;
			}
		    if(*ptr!=VIP_IMMORTAL)
			{
			    if(nextbuck->n_points==nextbuck->size)
				{
				    if(VipIncreaseIntBucket(nextbuck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
				}
			    nextbuck->data[nextbuck->n_points++]=*buckptr;
			}
		}
	    else if (*ptr==outside ) /*the point has been added to the object*/
		{
		    dirptr = vcs->offset;
		    for(dir2=vcs->nb_neighbors;dir2--;)
			{
			    ptr_neighbor = ptr + *dirptr;
			    if(*ptr_neighbor==next_value) /*NOT ALREADY IN FRONT and not immortal*/
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
int VipFillNextFrontFromOldFrontForErosion(
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  VipConnectivityStruct *vcs,
  int next_value,
  int front_value,
  int outside,
  int inside)
/*-------------------------------------------------------------------------*/
{
    int *buckptr, *dirptr;
    Vip_S16BIT *ptr, *ptr_neighbor;
    int i, dir2;

    if(first_vol_point==NULL)
	{
	    VipPrintfError("NULL pointer in VipFillNextFrontFromOldFrontForErosion");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForErosion");
	    return(PB);
	}
    if((buck==NULL) || (nextbuck==NULL))
	{
	    VipPrintfError("One NULL bucket in VipFillNextFrontFromOldFrontForErosion");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForErosion");
	    return(PB);
	}
    if(vcs==NULL)
	{
	    VipPrintfError("NULL VipConnectivityStruct pointer in VipFillNextFrontFromOldFrontForErosion");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForErosion");
	    return(PB);
	}

    buckptr = buck->data;
    for(i=buck->n_points;i--;)
    {
	ptr = first_vol_point + *buckptr;
	if(*ptr==front_value) /*the point was not simple and has not been deleted from the object*/
	{
	    if(nextbuck->n_points==nextbuck->size)
	    {
		if(VipIncreaseIntBucket(nextbuck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
	    }
	    nextbuck->data[nextbuck->n_points++]=*buckptr;
	}
	else if (*ptr==outside) /*the point has been deleted from the object*/
	{
	    dirptr = vcs->offset;
	    for(dir2=vcs->nb_neighbors;dir2--;)
	    {
		ptr_neighbor = ptr + *dirptr;
		if(*ptr_neighbor==next_value) /*NOT ALREADY IN FRONT and not immortal*/
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

int
VipHomotopicWellComposedErosionFromOutside (Volume * vol,
                                            int nb_iteration,
                                            Vip_S16BIT object,
                                            Vip_S16BIT inside,
                                            Vip_S16BIT outside)
{
  VipIntBucket *buck, *nextbuck;
  VipConnectivityStruct *vcs6;
  VipOffsetStruct *vos;
  unsigned int loop, count, count2, totalcount;
  Vip_S16BIT *first, *ptr;
  int *buckptr;
  int i;
  int ret = PB;

  if (VipVerifyAll (vol) == PB)
    {
      VipPrintfExit ("(skeleton)VipHomotopicWellComposedErosionFromOutside");
      return (PB);
    }
  if (VipTestType (vol, S16BIT) != OK)
    {
      VipPrintfError
        ("Sorry,  VipHomotopicWellComposedErosionFromOutside is only implemented for S16BIT volume");
      VipPrintfExit ("(skeleton)VipHomotopicWellComposedErosionFromOutside");
      return (PB);
    }
  if (mVipVolBorderWidth (vol) < 1)
    {
      VipPrintfError
        ("Sorry, VipHomotopicWellComposedErosionFromOutside is only implemented with border");
      VipPrintfExit ("(skeleton)VipHomotopicWellComposedErosionFromOutside");
      return (PB);
    }
  if (0)                        /* debug output */
    {
      printf ("Initialization (object:%d, inside:%d, outside:%d)...\n",
              object, inside, outside);
    }
  printf ("Homotopic well-composed erosion from outside...\n");

  buck =
    VipCreateFrontIntBucketForErosionFromOutside (vol, CONNECTIVITY_6,
                                                  VIP_FRONT, object, outside);
  if (!buck)
    goto VipHomotopicWellComposedErosionFromOutside_cleanup_buck;

  nextbuck =
    VipAllocIntBucket (mVipMax (VIP_INITIAL_FRONT_SIZE, buck->n_points));
  if (!nextbuck)
    goto VipHomotopicWellComposedErosionFromOutside_cleanup_nextbuck;
  nextbuck->n_points = 0;

  vcs6 = VipGetConnectivityStruct (vol, CONNECTIVITY_6);
  if (!vcs6)
    goto VipHomotopicWellComposedErosionFromOutside_cleanup_vcs6;

  vos = VipGetOffsetStructure (vol);
  if (!vos)
    goto VipHomotopicWellComposedErosionFromOutside_cleanup_vos;

  first = VipGetDataPtr_S16BIT (vol);

  /* main loop */
  loop = 0;
  count = 1;
  totalcount = 0;
  printf ("loop: %3d, Added %6d", loop, 0);

  while ((loop++ < nb_iteration) && (count) && (buck->n_points > 0))
    {
      if (loop == 1)
        count = 0;
      totalcount += count;
      count = 0;
      printf ("\rloop: %3d, Added %6d", loop, totalcount);
      fflush (stdout);
      count = 0;

      do {
        count2 = 0;

        buckptr = buck->data;
        for (i = buck->n_points; i--;)
          {
            ptr = first + *buckptr++;

            if (*ptr == VIP_FRONT &&
                VipIsTopologicallyWellComposedForTwoLabel_S16BIT
                (vos, ptr, outside, inside, outside))
              {
                *ptr = outside;
                count2++;
              }
          }
        count += count2;
      } while(count2);

      ret = VipFillNextFrontFromOldFrontForErosionFromOutside
        (first, buck, nextbuck, vcs6, object, VIP_FRONT, outside, inside);
      if(ret == PB)
        goto VipHomotopicWellComposedErosionFromOutside_cleanup_all;

      /* bucket permutation */
      VipPermuteTwoIntBucket (&buck, &nextbuck);
      nextbuck->n_points = 0;
    }

  printf ("\n");
  VipChangeIntLabel (vol, VIP_FRONT, object);
  VipChangeIntLabel (vol, VIP_IMMORTAL, object);

  ret = OK;

 VipHomotopicWellComposedErosionFromOutside_cleanup_all:
  VipFree (vos);
 VipHomotopicWellComposedErosionFromOutside_cleanup_vos:
  VipFreeConnectivityStruct (vcs6);
 VipHomotopicWellComposedErosionFromOutside_cleanup_vcs6:
  VipFreeIntBucket (nextbuck);
 VipHomotopicWellComposedErosionFromOutside_cleanup_nextbuck:
  VipFreeIntBucket (buck);
 VipHomotopicWellComposedErosionFromOutside_cleanup_buck:

  if(ret == PB)
    VipPrintfExit("(skeleton)VipHomotopicWellComposedErosionFromOutside");

  return ret;
}
