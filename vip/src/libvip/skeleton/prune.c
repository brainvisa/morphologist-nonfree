/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : skeleton             * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 23/07/99
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 *****************************************************************************
 *
 * DESCRIPTION : prune skeletons.
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


#include <vip/math.h>
#include <vip/skeleton.h>
#include <vip/skeleton_static.h>


/*---------------------------------------------------------------------------*/
static VipIntBucket *VipCreateFrontIntBucketForOutsidePruning( 
							Volume *vol,
							int front_value,
							int inside, 
							int outside);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
static int VipFillNextFrontFromOldFrontForOutsidePruning(
						       Topology26Neighborhood *topo26,
						       VipConnectivityStruct *vcs6,
						       VipConnectivityStruct *vcs26,
						       Vip_S16BIT *first_vol_point,
						       VipIntBucket *buck,
						       VipIntBucket *nextbuck,
						       int front_value,
						       int inside,
						       int outside);
/*-------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static VipIntBucket *VipCreateFrontIntBucketForCurvePruning( 
							Volume *vol, 
							int front_value,
							int inside, 
							int outside);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static int VipFillNextFrontFromOldFrontForCurvePruning(
						       Topology26Neighborhood *topo26,
						       VipConnectivityStruct *vcs,
						       Vip_S16BIT *first_vol_point,
						       VipIntBucket *buck,
						       VipIntBucket *nextbuck,
						       int front_value,
						       int inside,
						       int outside);
/*-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipHomotopicOutsidePruning( Volume *vol, int inside, int outside )
/*---------------------------------------------------------------------------*/
{
  VipIntBucket *buck, *nextbuck;
  Topology26Neighborhood *topo26;
  VipConnectivityStruct *vcs6;
  VipConnectivityStruct *vcs26;
  int loop, count;
  Vip_S16BIT *first, *ptr, *voisin;
  int *buckptr;
  int i;
  int icon;
  int front_value;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipHomotopicOutsidePruning");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipHomotopicOutsidePruning is only implemented with border");
      VipPrintfExit("(skeleton)VipHomotopicOutsidePruning");
      return(PB);
    }

  printf("---------------------------------------\n");
  printf("Pruning outside part of the skeleton...\n");
  printf("---------------------------------------\n");

  front_value = -321;

  printf("Initialization...\n");


  buck = VipCreateFrontIntBucketForOutsidePruning( vol, front_value, inside, outside);
  if(buck==PB) return(PB);
  nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
  if(nextbuck==PB) return(PB);

  topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

  vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );
  vcs26 = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );

  nextbuck->n_points = 0;

  first = VipGetDataPtr_S16BIT(vol);

  /*main loop*/
  loop=0;
  count = 1;

  printf("Deleted points: ");
  while((loop++<1000)&&(count)&&(buck->n_points>0))
      {
	  count = 0;
	  /*
	  printf("Front size: %d\n",buck->n_points);
	  */
	  buckptr = buck->data;
	  for(i=buck->n_points;i--;)
	    {
	      ptr = first + *buckptr++;
		 
	      if (VipComputeTopologicalClassificationForTwoLabelComplement_S16BIT(topo26,ptr,inside,outside)==
		  TOPO_BORDER_POINT)
		  {
		      for ( icon=0;icon<vcs6->nb_neighbors;icon++)
			{
			    voisin = ptr + vcs6->offset[icon];	      
			    if(*voisin==inside)
			      {
				VipPrintfError("Inconsistency, this point should not have inside 6-neighbor");
				VipPrintfExit("VipHomotopicOutsidePruning");
				VipWriteTivoliVolume(vol,"skeleton");
				
				return(PB);
			      }
			    else if (*voisin == outside)
				{
				    *ptr = outside;
				    break;
				}
			}
		      if(icon==vcs6->nb_neighbors)
			  {
			      VipPrintfError("inconsistency, this point should have an outside 6-neighbor");
			      VipPrintfExit("VipHomotopicCurvePruning");
			      return(PB);
			  }
		      count++;
		  }
	    }
	      	      			
	  VipFillNextFrontFromOldFrontForOutsidePruning(topo26,vcs6,vcs26,first,buck,nextbuck,front_value,inside,outside);
		  
	  /*bucket permutation*/
	  VipPermuteTwoIntBucket(&buck, &nextbuck);
	  nextbuck->n_points = 0;
	  
	  printf("%d, ",count);	      	  
      }

  printf("0\n");
     
  VipFreeTopology26Neighborhood(topo26);
  VipFreeIntBucket(buck);
  VipFreeIntBucket(nextbuck);

  VipFreeConnectivityStruct(vcs26);
  VipFreeConnectivityStruct(vcs6);

  VipChangeIntLabel(vol,front_value,290);
  
  return(OK);
}      

/*---------------------------------------------------------------------------*/
int VipHomotopicCurvePruning( Volume *vol, int inside, int outside )
/*---------------------------------------------------------------------------*/
{
  VipIntBucket *buck, *nextbuck;
  Topology26Neighborhood *topo26;
  VipConnectivityStruct *vcs6;
  VipConnectivityStruct *vcs26;
  int loop, count;
  Vip_S16BIT *first, *ptr, *voisin;
  int *buckptr;
  int i;
  int icon;
  int front_value;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(skeleton)VipHomotopicCurvePruning");
      return(PB);
    }
  if (mVipVolBorderWidth(vol) < 1)
    {
      VipPrintfError("Sorry, VipHomotopicCurvePruning is only implemented with border");
      VipPrintfExit("(skeleton)VipHomotopicCurvePruning");
      return(PB);
    }

  printf("---------------------------------------\n");
  printf("Pruning 3D curves of the skeleton...\n");
  printf("---------------------------------------\n");

  front_value = -4321;

  printf("Initialization...\n");

  buck = VipCreateFrontIntBucketForCurvePruning( vol, front_value, inside, outside);
  if(buck==PB) return(PB);
  nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
  if(nextbuck==PB) return(PB);

  topo26 = VipCreateTopology26Neighborhood( vol);
  if(topo26==PB) return(PB);

  vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );
  vcs26 = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );

  nextbuck->n_points = 0;

  first = VipGetDataPtr_S16BIT(vol);

  /*main loop*/
  loop=0;
  count = 1;

  printf("Deleted points: ");
  while((loop++<1000)&&(count)&&(buck->n_points>0))
      {
	  count = 0;
	  /*
	  printf("Front size: %d\n",buck->n_points);
	  */
	  buckptr = buck->data;
	  for(i=buck->n_points;i--;)
	    {
	      ptr = first + *buckptr++;
		 
	      if (VipComputeTopologicalClassificationForTwoLabelComplement_S16BIT(topo26,ptr,inside,outside)==
		  TOPO_BORDER_POINT)
		  {
		      for ( icon=0;icon<vcs6->nb_neighbors;icon++)
			{
			    voisin = ptr + vcs6->offset[icon];	      
			    if(*voisin==inside)
				{
				    *ptr = inside;
				    break;
				}
			    else if (*voisin == outside)
				{
				    *ptr = outside;
				    break;
				}
			}
		      if(icon==vcs6->nb_neighbors)
			  {
			      VipPrintfError("inconsistence, this should be a border point");
			      VipPrintfExit("VipHomotopicCurvePruning");
			      return(PB);
			  }
		      count++;
		  }
	    }
	      	      			
	  VipFillNextFrontFromOldFrontForCurvePruning(topo26,vcs26,first,buck,nextbuck,front_value,inside,outside);
		  
	  /*bucket permutation*/
	  VipPermuteTwoIntBucket(&buck, &nextbuck);
	  nextbuck->n_points = 0;
	  
	  printf("%d, ",count);	      	  
      }
     
  printf("0\n");
  VipFreeIntBucket(buck);
  VipFreeIntBucket(nextbuck);
  
  VipFreeTopology26Neighborhood(topo26);
  VipFreeConnectivityStruct(vcs26);
  VipFreeConnectivityStruct(vcs6);

  return(OK);
}      

/*---------------------------------------------------------------------------*/
static VipIntBucket *VipCreateFrontIntBucketForOutsidePruning( 
							Volume *vol,
							int front_value,
							int inside, 
							int outside)
/*-------------------------------------------------------------------------*/
{ 
 
    Vip_S16BIT *ptr, *voisin;
    int i, NbTotalPts;
    VipIntBucket *buck;
    VipConnectivityStruct *vcs6;
    int icon;
    int some_outside_6neighbor;
  

    if (VipVerifyAll(vol)==PB)
	{
	    VipPrintfExit("(skeleton)VipCreateFrontIntBucketForOutsidePruning");
	    return(PB);
	}
    if (mVipVolBorderWidth(vol) < 1) 
	{
	    VipPrintfError("Sorry,  VipCreateFrontIntBucketForOutsidePruning is only implemented with border");
	    VipPrintfExit("(skeleton)VipCreateFrontIntBucketForOutsidePruning");
	    return(PB);
	}

    VipSetBorderLevel( vol, outside ); 

    vcs6 = VipGetConnectivityStruct( vol, CONNECTIVITY_6 );

    ptr = VipGetDataPtr_S16BIT(vol);

    NbTotalPts = VipOffsetVolume(vol);
 
    buck = VipAllocIntBucket(VIP_INITIAL_FRONT_SIZE);

    for ( i=0; i<NbTotalPts; i++ )
	{
	    if (*ptr != outside && *ptr!=inside)            
		{	  
		    some_outside_6neighbor = VFALSE;
		    for ( icon=0;icon<vcs6->nb_neighbors;icon++)
			{
			    voisin = ptr + vcs6->offset[icon];	      
			    if(*voisin==outside)
			      {
				some_outside_6neighbor = VTRUE;
				break;
			      }
			}
		    if(some_outside_6neighbor==VTRUE)
			{		  		    
			    if(buck->n_points==buck->size)
				{
				    if(VipIncreaseIntBucket(buck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
				}
			    buck->data[buck->n_points++] = i;
			    *ptr = front_value;
			} 	  
		}
	    ptr++;
	}

    printf("Initial front : %d points\n", buck->n_points);

    VipFreeConnectivityStruct(vcs6);

    return(buck);
}

/*-------------------------------------------------------------------------*/
static int VipFillNextFrontFromOldFrontForOutsidePruning(
						       Topology26Neighborhood *topo26,
						       VipConnectivityStruct *vcs6,
						       VipConnectivityStruct *vcs26,
						       Vip_S16BIT *first_vol_point,
						       VipIntBucket *buck,
						       VipIntBucket *nextbuck,
						       int front_value,
						       int inside,
						       int outside)
/*-------------------------------------------------------------------------*/
{
    int *buckptr, *dirptr;
    Vip_S16BIT *ptr, *ptr_neighbor, *voisin;
    int i, dir2;
    int icon;
    int some_outside_6neighbor;

    if(first_vol_point==NULL)
	{
	    VipPrintfError("NULL pointer in VipFillNextFrontFromOldFrontForOutsidePruning");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForOutsidePruning");
	    return(PB);
	}
    if((buck==NULL) || (nextbuck==NULL))
	{
	    VipPrintfError("One NULL bucket in VipFillNextFrontFromOldFrontForOutsidePruning");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForOutsidePruning");
	    return(PB);
	}
    if(vcs6==NULL || vcs26==NULL)
	{
	    VipPrintfError("NULL VipConnectivityStruct pointer in VipFillNextFrontFromOldFrontForOutsidePruning");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForOutsidePruning");
	    return(PB);
	}

    buckptr = buck->data;
    for(i=buck->n_points;i--;)
	{
	    ptr = first_vol_point + *buckptr;
	    if(*ptr==inside || *ptr==outside) /*the point has been deleted */
		{
		    dirptr = vcs26->offset;
		    for(dir2=vcs26->nb_neighbors;dir2--;)
			{
			    ptr_neighbor = ptr + *dirptr;
			    if(*ptr_neighbor!=inside && *ptr_neighbor!=outside) 
				{		   
				  some_outside_6neighbor = VFALSE;
				  for ( icon=0;icon<vcs6->nb_neighbors;icon++)
					{
					    voisin = ptr_neighbor + vcs6->offset[icon];	      
					    if(*voisin==outside)
					      {
						some_outside_6neighbor=VTRUE;
						break;
					      }
					}
				  if(some_outside_6neighbor==VTRUE)
				    {	
				      if(
					 VipComputeTopologicalClassificationForTwoLabelComplement_S16BIT(
													 topo26,ptr_neighbor,inside,outside)
					== TOPO_BORDER_POINT)
					{
					  if(nextbuck->n_points==buck->size)
					    {
					      if(VipIncreaseIntBucket(nextbuck,
								      VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
					    }
					  nextbuck->data[nextbuck->n_points++] = *buckptr + *dirptr;
					  *ptr_neighbor = front_value;
					} 
				    }
				}
			    dirptr++;
			}
		}	    
	    buckptr++;
	}
        
    return(OK);
}


/*---------------------------------------------------------------------------*/
static VipIntBucket *VipCreateFrontIntBucketForCurvePruning( 
							Volume *vol,
							int front_value,
							int inside, 
							int outside)
/*-------------------------------------------------------------------------*/
{ 
 
    Vip_S16BIT *ptr, *voisin;
    int i, NbTotalPts;
    VipIntBucket *buck;
    VipConnectivityStruct *vcs;
    int icon;
    int nb_nonzero_neighbor;
    int nb_border_neighbor;
    int nb_nonborder_noncurve;
  

    if (VipVerifyAll(vol)==PB)
	{
	    VipPrintfExit("(skeleton)VipCreateFrontIntBucketForCurvePruning");
	    return(PB);
	}
    if (mVipVolBorderWidth(vol) < 1) 
	{
	    VipPrintfError("Sorry,  VipCreateFrontIntBucketForDilation is only implemented with border");
	    VipPrintfExit("(skeleton)VipCreateFrontIntBucketForDilation");
	    return(PB);
	}

    VipSetBorderLevel( vol, outside ); 

    printf("Topological classification...\n");

    if (VipTopologicalClassificationForTwoLabelComplement(vol, inside, outside) == PB) return(PB);

    vcs = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );

    ptr = VipGetDataPtr_S16BIT(vol);

    NbTotalPts = VipOffsetVolume(vol);
 
    buck = VipAllocIntBucket(VIP_INITIAL_FRONT_SIZE);

    for ( i=0; i<NbTotalPts; i++ )
	{
	    if (*ptr == TOPO_BORDER_POINT)            
		{	  
		    nb_nonzero_neighbor = 0;
		    nb_border_neighbor = 0;
		    nb_nonborder_noncurve = 0;
		    for ( icon=0;icon<vcs->nb_neighbors;icon++)
			{
			    voisin = ptr + vcs->offset[icon];	      
			    if(*voisin!=inside && *voisin!=outside)
			    {
				nb_nonzero_neighbor++;
				if (*voisin == TOPO_BORDER_POINT || *voisin==front_value)
				    {
					nb_border_neighbor++;
				    }
				else if (*voisin==TOPO_SURFACE_POINT || *voisin==TOPO_JUNCTION_SURFACE_CURVES_POINT
					 || *voisin==TOPO_JUNCTION_SURFACES_POINT
					 || *voisin==TOPO_JUNCTION_SURFACES_CURVES_POINT)
				    {
					nb_nonborder_noncurve++;
				    }
			    }
			}
		    if(nb_nonzero_neighbor==1 || nb_border_neighbor==0 || nb_nonborder_noncurve==0)
			{		  		    
			    if(buck->n_points==buck->size)
				{
				    if(VipIncreaseIntBucket(buck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
				}
			    buck->data[buck->n_points++] = i;
			    *ptr = front_value;
			} 	  
		}
	    ptr++;
	}

    printf("Initial front : %d points\n", buck->n_points);

    VipFreeConnectivityStruct(vcs);

    return(buck);
}

/*-------------------------------------------------------------------------*/
static int VipFillNextFrontFromOldFrontForCurvePruning(
						       Topology26Neighborhood *topo26,
						       VipConnectivityStruct *vcs,
						       Vip_S16BIT *first_vol_point,
						       VipIntBucket *buck,
						       VipIntBucket *nextbuck,
						       int front_value,
						       int inside,
						       int outside)
/*-------------------------------------------------------------------------*/
{
    int *buckptr, *dirptr;
    Vip_S16BIT *ptr, *ptr_neighbor, *voisin;
    int i, dir2;
    int icon;
    int nb_nonzero_neighbor;
    int nb_border_neighbor;
    int nb_nonborder_noncurve;

    if(first_vol_point==NULL)
	{
	    VipPrintfError("NULL pointer in VipFillNextFrontFromOldFrontForCurvePruning");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForCurvePruning");
	    return(PB);
	}
    if((buck==NULL) || (nextbuck==NULL))
	{
	    VipPrintfError("One NULL bucket in VipFillNextFrontFromOldFrontForCurvePruning");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForCurvePruning");
	    return(PB);
	}
    if(vcs==NULL)
	{
	    VipPrintfError("NULL VipConnectivityStruct pointer in VipFillNextFrontFromOldFrontForCurvePruning");
	    VipPrintfExit("VipFillNextFrontFromOldFrontForCurvePruning");
	    return(PB);
	}

    buckptr = buck->data;
    for(i=buck->n_points;i--;)
	{
	    ptr = first_vol_point + *buckptr;
	    if(*ptr==inside || *ptr==outside) /*the point has been deleted */
		{
		    dirptr = vcs->offset;
		    for(dir2=vcs->nb_neighbors;dir2--;)
			{
			    ptr_neighbor = ptr + *dirptr;
			    if(*ptr_neighbor!=inside && *ptr_neighbor!=outside) 
				{		   
				    *ptr_neighbor = 
					VipComputeTopologicalClassificationForTwoLabelComplement_S16BIT(
									       topo26,ptr_neighbor,inside,outside);
				}
			    dirptr++;
			}
		    
		    dirptr = vcs->offset;
		    for(dir2=vcs->nb_neighbors;dir2--;)
			{
			    ptr_neighbor = ptr + *dirptr;
			    if(*ptr_neighbor==TOPO_BORDER_POINT)
				{
				    nb_nonzero_neighbor = 0;
				    nb_border_neighbor = 0;
				    nb_nonborder_noncurve = 0;
				    for ( icon=0;icon<vcs->nb_neighbors;icon++)
					{
					    voisin = ptr_neighbor + vcs->offset[icon];	      
					    if(*voisin!=inside && *voisin!=outside)
						{
						    nb_nonzero_neighbor++;
					    
						    if (*voisin == TOPO_BORDER_POINT || *voisin==front_value) 
							nb_border_neighbor++;
						    else if (*voisin==TOPO_SURFACE_POINT 
							     || *voisin==TOPO_JUNCTION_SURFACE_CURVES_POINT
							     || *voisin==TOPO_JUNCTION_SURFACES_POINT
							     || *voisin==TOPO_JUNCTION_SURFACES_CURVES_POINT)
							{
							    nb_nonborder_noncurve++;
							}
						}
					}
				    if(nb_nonzero_neighbor==1 || nb_border_neighbor==0 || nb_nonborder_noncurve==0)
					{		  		    
					    if(nextbuck->n_points==buck->size)
						{
						    if(VipIncreaseIntBucket(nextbuck,
									    VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
						}
					    nextbuck->data[nextbuck->n_points++] = *buckptr + *dirptr;
					    *ptr_neighbor = front_value;
					} 	  
				}
			    dirptr++;
			}
		}
	    else
		{
		    *ptr = VipComputeTopologicalClassificationForTwoLabelComplement_S16BIT(
											   topo26,ptr,inside,outside);
		}
	    
	    buckptr++;
	}
    
    return(OK);
}
