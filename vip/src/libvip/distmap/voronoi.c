/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_voronoi.c        * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 04/02/1996
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : tout ce qui concerne les voronoi iteres
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include <vip/util.h>
#include <vip/volume.h>
#include <vip/connex.h>
#include <vip/distmap.h>
#include <vip/skeleton.h>

#include <vip/distmap_static.h>
#include <vip/skeleton_static.h>

int CreateAndSplitConnectivityMask(Volume*,int,DistmapMask**,DistmapMask**);
int VipCleanUpBorderFromImmortalsVoronoiObject( Volume *vol, VipIntBucket *buck, int immortals, int outside, int connectivity);

/*----------------------------------------------------------------------------*/
Volume *VipComputeVoronoi (
  Volume *vol
)
{
  return(VipComputeCustomizedVoronoi( vol, 3, 3, 3, VIP_USUAL_DISTMAP_MULTFACT));
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
Volume *VipComputeCustomizedVoronoi (
  Volume *vol,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor
)
/*---------------------------------------------------------------------------*/


{
  DistmapMask *f, *b;
  int nb_change;
  Volume *label;

  if(!VipVerifyAll(vol))
    {
      VipPrintfExit("(distmap)VipComputeCustomizedVoronoi");
      return(PB);
    }


  if(mult_factor<=0.)
    {
      VipPrintfError("Strange mult_factor (use VIP_USUAL_DISTMAP_MULTFACT)p");
      VipPrintfExit("(distmap)VipComputeCustomizedVoronoi");
      return(PB);

    }
  else VIP_USED_DISTMAP_MULTFACT = mult_factor;

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipComputeCustomizedVoronoi is only implemented for S16BIT volume");
      VipPrintfExit("(distmap)VipComputeCustomizedVoronoi");
      return(PB);
    }

  if (VipBorderWidthOf(vol)<=0)
    {
      /* I'd like to be sure of it...
	 VipPrintfWarning(" VipComputeCustomizedChamferDistanceMap could be faster with a borderwidth of 1 (or more)");*/
    }
  else
    {
      VipSetBorderLevel( vol, VIP_OUTSIDE_DOMAIN );
    }

  label = VipCopyVolume( vol, "voronoi");
  if (label==PB) return(PB);

  VolumeChamferPreparation( vol);

  if(CreateAndSplitCubicMask ( vol, xmask, ymask, zmask, &f, &b )==PB) return(PB);
   
  (void)printf("Forward.....\n");
  nb_change = ForwardSweepingVoronoiWithBorder( vol, label, f, VIP_OUTSIDE_DOMAIN);
  if(nb_change==0)
    {
      VipPrintfWarning("Strange image in VipComputeCustomizedVoronoi?");
    }
  (void)printf("Backward.....\n");
  (void)BackwardSweepingVoronoiWithBorder( vol, label, b, VIP_OUTSIDE_DOMAIN);

  FreeMask( f );
  FreeMask( b);

  return(label);

}

/*----------------------------------------------------------------------------*/
Volume *VipComputeConnectivityVoronoi (
  Volume *vol,
  int connectivity
)
/*---------------------------------------------------------------------------*/


{
  DistmapMask *f, *b;
  int nb_change;
  Volume *label;

  if(!VipVerifyAll(vol))
    {
      VipPrintfExit("(distmap)VipComputeConnectivityVoronoi");
      return(PB);
    }


  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipComputeConnectivityVoronoi is only implemented for S16BIT volume");
      VipPrintfExit("(distmap)VipComputeConnectivityVoronoi");
      return(PB);
    }

  if (VipBorderWidthOf(vol)<=0)
    {
      /* I'd like to be sure of it...
	 VipPrintfWarning(" VipComputeConnectivityChamferDistanceMap could be faster with a borderwidth of 1 (or more)");*/
    }
  else
    {
      VipSetBorderLevel( vol, VIP_OUTSIDE_DOMAIN );
    }

  label = VipCopyVolume( vol, "voronoi");
  if (label==PB) return(PB);

  VolumeChamferPreparation( vol);

  if(CreateAndSplitConnectivityMask ( vol, connectivity, &f, &b )==PB) return(PB);
   
  (void)printf("Forward.....\n");
  nb_change = ForwardSweepingVoronoiWithBorder( vol, label, f, VIP_OUTSIDE_DOMAIN);
  if(nb_change==0)
    {
      VipPrintfWarning("Strange image in VipComputeConnectivityVoronoi?");
    }
  (void)printf("Backward.....\n");
  (void)BackwardSweepingVoronoiWithBorder( vol,label,  b, VIP_OUTSIDE_DOMAIN);

  FreeMask( f );
  FreeMask( b);

  return(label);

}

/*----------------------------------------------------------------------------*/
Volume *VipComputeIteratedGeodesicVoronoi (
  Volume *vol,
  int domain,
  int outside_domain,
  int nbitermax
)
{
  return(VipComputeCustomizedIteratedGeodesicVoronoi( vol, domain, outside_domain, nbitermax,
						  3, 3, 3, VIP_USUAL_DISTMAP_MULTFACT));
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
Volume *VipComputeCustomizedIteratedGeodesicVoronoi (
  Volume *vol,
  int domain,
  int outside_domain,
  int nbitermax,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor
)
/*---------------------------------------------------------------------------*/
{
  DistmapMask *f, *b;
  int changef, changeb;
  int nbiter;
  Volume *label;

  if(!VipVerifyAll(vol))
    {
      VipPrintfExit("(distmap)VipComputeCustomizedIteratedGeodesicVoronoi");
      return(PB);
    }
  if ((nbitermax>100)||(nbitermax<=0))
    {
      VipPrintfError("Strange maximum iteration number");
      VipPrintfExit("(distmap)VipComputeCustomizedIteratedGeodesicVoronoi");
      return(PB);
    }

  if(mult_factor<=0.)
    {
      VipPrintfError("Strange mult_factor (use VIP_USUAL_DISTMAP_MULTFACT)");
      VipPrintfExit("(distmap)VipComputeCustomizedIteratedGeodesicVoronoi");
      return(PB);
    }
  else VIP_USED_DISTMAP_MULTFACT = mult_factor;

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipComputeCustomizedIteratedGeodesicVoronoi is only implemented for S16BIT volume");
      VipPrintfExit("(distmap)VipComputeCustomizedIteratedGeodesicVoronoi");
      return(PB);
    }

  if (VipBorderWidthOf(vol)<=0)
    {
      /* I'd like to be sure of it...
	 VipPrintfWarning(" VipComputeCustomizedGeodesicChamferDistanceMap could be faster with a borderwidth of 1 (or more)");*/
    }
  else
    {
      VipSetBorderLevel( vol, VIP_OUTSIDE_DOMAIN );
    }

  label = VipCopyVolume( vol, "voronoi");
  if (label==PB) return(PB);

  VolumeChamferPreparationGeodesic( vol, domain, outside_domain);

  CreateAndSplitCubicMask ( vol, xmask, ymask, zmask, &f, &b );

   
  changef = changeb = VTRUE;
  nbiter = 1;
  while( ((changeb!=VFALSE) || (changef!=VFALSE))&&(nbiter<=nbitermax))
    {
      (void)printf("Forward.....\n");
      changef = ForwardSweepingVoronoiWithBorder( vol, label, f, VIP_OUTSIDE_DOMAIN);
      (void)printf("Backward.....\n");
      changeb = BackwardSweepingVoronoiWithBorder( vol, label,  b, VIP_OUTSIDE_DOMAIN);
      (void)printf("iter: %d, modif: %d\n", nbiter++, changeb + changef);
    }

  VipChangeIntLabel(vol,VIP_OUTSIDE_DOMAIN, outside_domain);
  FreeMask( f );
  FreeMask( b);

  return(label);


}



/*----------------------------------------------------------------------------*/
Volume *VipComputeConnectivityIteratedGeodesicVoronoi (
  Volume *vol,
  int domain,
  int outside_domain,
  int nbitermax,
  int connectivity
)
/*---------------------------------------------------------------------------*/
{
  DistmapMask *f, *b;
  int changef, changeb;
  int nbiter;
  Volume *label;

  if(!VipVerifyAll(vol))
    {
      VipPrintfExit("(distmap)VipComputeConnectivityIteratedGeodesicVoronoi");
      return(PB);
    }
  if ((nbitermax>100)||(nbitermax<=0))
    {
      VipPrintfError("Strange maximum iteration number");
      VipPrintfExit("(distmap)VipComputeConnectivityIteratedGeodesicVoronoi");
      return(PB);
    }

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipComputeConnectivityIteratedGeodesicVoronoi is only implemented for S16BIT volume");
      VipPrintfExit("(distmap)VipComputeConnectivityIteratedGeodesicVoronoi");
      return(PB);
    }

  if (VipBorderWidthOf(vol)<=0)
    {
      /* I'd like to be sure of it...
	 VipPrintfWarning(" VipComputeCustomizedGeodesicChamferDistanceMap could be faster with a borderwidth of 1 (or more)");*/
    }
  else
    {
      VipSetBorderLevel( vol, VIP_OUTSIDE_DOMAIN );
    }

  label = VipCopyVolume( vol, "voronoi");

  if(label==PB) return(PB);

  VolumeChamferPreparationGeodesic( vol, domain, outside_domain);

  CreateAndSplitConnectivityMask ( vol, connectivity, &f, &b );
   
  changef = changeb = VTRUE;
  nbiter = 1;
  while( ((changeb!=VFALSE) || (changef!=VFALSE))&&(nbiter<=nbitermax))
    {
      (void)printf("Forward.....\n");
      changef = ForwardSweepingVoronoiWithBorder( vol, label, f, VIP_OUTSIDE_DOMAIN);
      (void)printf("Backward.....\n");
      changeb = BackwardSweepingVoronoiWithBorder( vol, label, b, VIP_OUTSIDE_DOMAIN);
      (void)printf("iter: %d, modif: %d\n", nbiter++, changeb + changef);
    }

  VipChangeIntLabel(vol,VIP_OUTSIDE_DOMAIN, outside_domain);
  FreeMask( f );
  FreeMask( b);

  return(label);

}

/*----------------------------------------------------------------------------*/
Volume *VipComputeFrontPropagationGeodesicVoronoi (
  Volume *vol,
  int domain,
  int outside_domain
)
/*---------------------------------------------------------------------------*/
{
  return(VipComputeCustomizedFrontPropagationGeodesicVoronoi(vol,domain,outside_domain,
							     3, 3, 3, VIP_USUAL_DISTMAP_MULTFACT));
}

/*----------------------------------------------------------------------------*/
Volume *VipComputeCustomizedFrontPropagationGeodesicVoronoi (
  Volume *vol,
  int domain,
  int outside_domain,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor
)
/*---------------------------------------------------------------------------*/
{
  DistmapMask *m;
  Volume *label;

  if(!VipVerifyAll(vol))
    {
      VipPrintfExit("(distmap)VipComputeCustomizedFrontPropagationGeodesicVoronoi");
      return(PB);
    }

  if(mult_factor<=0.)
    {
      VipPrintfError("Strange mult_factor (use VIP_USUAL_DISTMAP_MULTFACT)");
      VipPrintfExit("(distmap)VipComputeCustomizedFrontPropagationGeodesicVoronoi");
      return(PB);
    }
  else VIP_USED_DISTMAP_MULTFACT = mult_factor;

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipComputeCustomizedFrontPropagationGeodesicVoronoi is only implemented for S16BIT volume");
      VipPrintfExit("(distmap)VipComputeCustomizedFrontPropagationGeodesicVoronoi");
      return(PB);
    }

  if (VipBorderWidthOf(vol)<=0)
    {
       VipPrintfInfo("Volume should have non nul borderwidth");
      VipPrintfExit("VipComputeCustomizedFrontPropagationGeodesicVoronoi");
      return(PB);
    }
  else
    {
      VipSetBorderLevel( vol, VIP_OUTSIDE_DOMAIN );
    }

  label = VipCopyVolume( vol, "voronoi");
  if (label==PB) return(PB);

  VolumeChamferPreparationGeodesic( vol, domain, outside_domain);

  m = CreateSquareMask ( vol, xmask, ymask, zmask);
  if( (mVipVolBorderWidth(vol)<m->xcubesize)
      ||(mVipVolBorderWidth(vol)<m->ycubesize)
      ||(mVipVolBorderWidth(vol)<m->zcubesize)
      )
      {
      VipPrintfInfo("Volume should have borderwidth larger than chamfer mask dimensions");
      VipPrintfExit("VipComputeCustomizedFrontPropagationGeodesicVoronoi");
      return(PB);
    }
   
  if(FrontPropagationVoronoi(vol,label, m)==PB) return(PB);


  VipChangeIntLabel(vol,VIP_OUTSIDE_DOMAIN, outside_domain);
  FreeMask( m );

  return(label);


}

/*----------------------------------------------------------------------------*/
Volume *VipComputeConnectivityFrontPropagationGeodesicVoronoi (
  Volume *vol,
  int domain,
  int outside_domain,
  int connectivity
)
/*---------------------------------------------------------------------------*/
{
  DistmapMask *m;
  Volume *label;

  if(!VipVerifyAll(vol))
    {
      VipPrintfExit("(distmap)VipComputeConnectivityFrontPropagationGeodesicVoronoi");
      return(PB);
    }
  else VIP_USED_DISTMAP_MULTFACT = 1;

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipComputeConnectivityFrontPropagationGeodesicVoronoi is only implemented for S16BIT volume");
      VipPrintfExit("(distmap)VipComputeConnectivityFrontPropagationGeodesicVoronoi");
      return(PB);
    }

  if (VipBorderWidthOf(vol)<=0)
    {
       VipPrintfInfo("Volume should have non nul borderwidth");
      VipPrintfExit("VipComputeConnectivityFrontPropagationGeodesicVoronoi");
      return(PB);
    }
  else
    {
      VipSetBorderLevel( vol, VIP_OUTSIDE_DOMAIN );
    }

  label = VipCopyVolume( vol, "voronoi");
  if (label==PB) return(PB);

  VolumeChamferPreparationGeodesic( vol, domain, outside_domain);

  m = CreateConnectivityMask ( vol, connectivity);
   
  if(FrontPropagationConnectivityVoronoi(vol,label, m)==PB) return(PB);


  VipChangeIntLabel(vol,VIP_OUTSIDE_DOMAIN, outside_domain);
  FreeMask( m );

  return(label);


}

/*--------------------------------------------------------------------*/
int VipWatershedFrontPropagation( Volume *vol, Volume *altitude, Volume *plan_hemi, int int_min, int int_max, int domain, int outside, int nb_interval, int connectivity)
{
    VipIntBucket *buck, *nextbuck, *buck_immortals;
    VipConnectivityStruct *vcs;
    int loop, count, immortals;
    int dir;
    Vip_S16BIT *first, *ptr, *ptr_neighbor;
    Vip_S16BIT *afirst, *aptr, *phfirst = NULL, *phptr;
    int *buckptr;
    int i, m;
    float interval;

    if (VipVerifyAll(vol)==PB)
    {
        VipPrintfExit("(voronoi)VipWatershedVoronoi");
        return(PB);
    }
    if (VipVerifyAll(altitude)==PB)
    {
        VipPrintfExit("(voronoi)VipWatershedVoronoi");
        return(PB);
    }
    if (VipTestType(vol,S16BIT)!=OK)
    {
        VipPrintfError("Sorry, VipWatershedVoronoi is only implemented for S16BIT volume");
        VipPrintfExit("(voronoi)VipWatershedVoronoi");
        return(PB);
    }
    if (VipTestType(altitude,S16BIT)!=OK)
    {
        VipPrintfError("Sorry, altitude should be a S16BIT volume");
        VipPrintfExit("(voronoi)VipWatershedVoronoi");
        return(PB);
    }
    if (mVipVolBorderWidth(vol) < 1)
    {
        VipPrintfError("Sorry, VipWatershedVoronoi is only implemented with border");
        VipPrintfExit("(voronoi)VipWatershedVoronoi");
        return(PB);
    }
    if(VipTestEqualBorderWidth(vol,altitude)==PB)
    {
        VipPrintfError("Sorry, voronoi and altitude should have the same borderwidth");
        VipPrintfExit("(voronoi)VipWatershedVoronoi");
        return(PB);
    }

    printf("--------------------------------------------\n");
    printf("Watershed driven front propagation voronoi...\n");
    printf("--------------------------------------------\n");

    printf("Initialization...\n");
    fflush(stdout);
    
    VipSetBorderLevel( vol, outside ); /* already done before but security */

    buck = VipCreateFrontIntBucketVoronoiObject( vol, connectivity, VIP_FRONT, domain, outside);
    if(buck==PB) return(PB);
    nextbuck = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
    if(nextbuck==PB) return(PB);
    buck_immortals = VipAllocIntBucket(mVipMax(VIP_INITIAL_FRONT_SIZE,buck->n_points));
    if(buck_immortals==PB) return(PB);

    vcs = VipGetConnectivityStruct( vol, connectivity );

    first = VipGetDataPtr_S16BIT(vol);
    afirst = VipGetDataPtr_S16BIT(altitude);
    if(plan_hemi!=NULL) phfirst = VipGetDataPtr_S16BIT(plan_hemi);

    /*initialisation*/
    /*Mark all initial immortals and dwindle the front*/

    immortals = 0;
    interval = (float)(int_max - int_min + 1)/nb_interval;

    /*main loop*/
    for(m=0;m<nb_interval;m++)
    {
        loop = 0;
        count = 1;
        while((loop++<500)&&(count)&&(buck->n_points>0))
        {
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            printf("loop: %3d, numero_int: %2d, immortals: %3d", loop, m, immortals);
            fflush(stdout);
            
            count = 0;
            
            /*first scan: mark points in the gray level interval*/
            buckptr = buck->data;
            for(i=buck->n_points;i--;)
            {
                ptr = first + *buckptr;
                if(*ptr==VIP_FRONT)
                {
                    aptr = afirst + *buckptr;
                    if(*aptr>(float)(int_max - (m+1)*interval)) *ptr=VIP_CANDIDATE;
                }
                buckptr++;
            }
            
            /*second scan: mark points immortal (border) or inside a voronoi*/
            buckptr = buck->data;
            for(i=buck->n_points;i--;)
            {
                ptr = first + *buckptr;
                if(*ptr==VIP_CANDIDATE)
                {
                    for(dir=0;dir<vcs->nb_neighbors;dir++)
                    {
                        ptr_neighbor = ptr + vcs->offset[dir];
                        if(*ptr_neighbor==1 || *ptr_neighbor==2 || *ptr_neighbor==3)
                        {
                            if(*ptr==VIP_CANDIDATE)
                            {
                                if(plan_hemi!=NULL)
                                {
                                    phptr = phfirst + *buckptr;
                                    if(*ptr_neighbor==1 && *phptr==0) *ptr=*ptr_neighbor;
                                    else if (*ptr_neighbor==2 && *phptr==255) *ptr=*ptr_neighbor;
                                    else if (*ptr_neighbor==3) *ptr=*ptr_neighbor;
                                    count++;
                                }
                                else
                                {
                                    *ptr=*ptr_neighbor;
                                    count++;
                                }
                            }
                            else if((*ptr!=*ptr_neighbor) && (*ptr!=VIP_IMMORTAL))
                            {
                                *ptr=VIP_IMMORTAL;
                                immortals++;
                                if(buck_immortals->n_points==buck_immortals->size)
                                {
                                    if(VipIncreaseIntBucket(buck_immortals,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
                                }
                                buck_immortals->data[buck_immortals->n_points++]=*buckptr;
                                break;
                            }
                        }
                    }
                }
                buckptr++;
            }
            VipFillNextFrontFromOldFrontVoronoiObject(first,buck,nextbuck,vcs,VIP_FRONT,domain, outside);
            
            /*bucket permutation*/
            VipPermuteTwoIntBucket(&buck, &nextbuck);
            nextbuck->n_points = 0;
        }
    }
    printf("\n");
    
    VipCleanUpBorderFromImmortalsVoronoiObject( vol, buck_immortals, VIP_IMMORTAL, 0, CONNECTIVITY_26);
    
    VipFreeConnectivityStruct(vcs);
    VipFreeIntBucket(buck);
    VipFreeIntBucket(nextbuck);
    
    return(OK);
}

/*----------------------------------------------------------------------------*/
VipIntBucket *VipCreateFrontIntBucketVoronoiObject( Volume *vol, int connectivity, int front_value, int domain, int outside)
{
    (void)(outside);
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

    vcs = VipGetConnectivityStruct( vol, connectivity );

    ptr = VipGetDataPtr_S16BIT(vol);

    NbTotalPts = VipOffsetVolume(vol);

    buck = VipAllocIntBucket(VIP_INITIAL_FRONT_SIZE);

    for ( i=0; i<NbTotalPts; i++ )
    {
        if (*ptr==domain)
        {
            for (icon=0;icon<vcs->nb_neighbors;icon++)
            {
                voisin = ptr + vcs->offset[icon];
                if(*voisin==1 || *voisin==2 || *voisin==3)
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
int VipFillNextFrontFromOldFrontVoronoiObject(
Vip_S16BIT *first_vol_point,
VipIntBucket *buck,
VipIntBucket *nextbuck,
VipConnectivityStruct *vcs,
int front_value,
int domain,
int outside)
{
    int *buckptr;
    Vip_S16BIT *ptr, *ptr_neighbor;
    int i, dir;

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
        if(*ptr==front_value)
        {
            if(nextbuck->n_points==nextbuck->size)
            {
                if(VipIncreaseIntBucket(nextbuck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
            }
            nextbuck->data[nextbuck->n_points++]=*buckptr;
        }
        else if((*ptr==1) || (*ptr==2) || (*ptr==3))
        {
            for(dir=0;dir<vcs->nb_neighbors;dir++)
            {
                ptr_neighbor = ptr + vcs->offset[dir];
                if(*ptr_neighbor==domain)   /*non zero, not immortal, AND NOT ALREADY IN FRONT*/
                {
                    *ptr_neighbor = front_value;
                    if(nextbuck->n_points==nextbuck->size)
                    {
                        if(VipIncreaseIntBucket(nextbuck,VIP_FRONT_SIZE_INCREMENT)==PB) return(PB);
                    }
                    nextbuck->data[nextbuck->n_points++] = *buckptr + vcs->offset[dir];
                }
            }
        }
        else if(*ptr==outside)
        {
            VipPrintfWarning("This should be inside value");
        }
        buckptr++;
    }
    return(OK);
}

/*-------------------------------------------------------------------------*/
int VipCleanUpBorderFromImmortalsVoronoiObject( Volume *vol, VipIntBucket *buck, int immortals, int outside, int connectivity)
{
    (void)(immortals);
    (void)(outside);
    int *buckptr;
    Vip_S16BIT *first, *ptr, *ptr_neighbor;
    VipConnectivityStruct *vcs;
    int i, dir, count;
    int nb_vois1, nb_vois2, nb_vois3;

    if (VipVerifyAll(vol)==PB)
    {
        VipPrintfExit("VipCleanUpBorderFromImmortalsVoronoiObject");
        return(PB);
    }
    if (VipTestType(vol,S16BIT)!=OK)
    {
        VipPrintfError("Sorry, VipCleanUpBorderFromImmortalsVoronoiObject is only implemented for S16BIT volume");
        VipPrintfExit("VipCleanUpBorderFromImmortalsVoronoiObject");
        return(PB);
    }
    if(buck==NULL)
    {
        VipPrintfError("NULL bucket in VipCleanUpBorderFromImmortalsVoronoiObject");
        VipPrintfExit("VipCleanUpBorderFromImmortalsVoronoiObject");
        return(PB);
    }
    
    count = 0;
    
    vcs = VipGetConnectivityStruct( vol, connectivity );
    first = VipGetDataPtr_S16BIT(vol);
    
    buckptr = buck->data;
    for(i=buck->n_points;i--;)
    {
        nb_vois1 = 0;
        nb_vois2 = 0;
        nb_vois3 = 0;
        
        ptr = first + *buckptr;
        for(dir=0;dir<vcs->nb_neighbors;dir++)
        {
            ptr_neighbor = ptr + vcs->offset[dir];
            switch(*ptr_neighbor)
            {
                case 1:
                    nb_vois1++;
                    break;
                case 2:
                    nb_vois2++;
                    break;
                case 3:
                    nb_vois3++;
                    break;
                default:
                    break;
            }
        }
        if(nb_vois1>nb_vois2 && nb_vois1>nb_vois3) *ptr=1;
        else if(nb_vois2>nb_vois1 && nb_vois2>nb_vois3) *ptr=2;
        else if(nb_vois3>nb_vois1 && nb_vois3>nb_vois2) *ptr=3;
        else if(nb_vois1==nb_vois3 || nb_vois2==nb_vois3) *ptr=3;
        else
        {
            if(count%2==0) *ptr=1;
            else *ptr=2;
            count++;
        }
        buckptr++;
    }

    VipFreeConnectivityStruct(vcs);

    return(OK);
}
