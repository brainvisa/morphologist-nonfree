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

#include <vip/distmap_static.h>

int CreateAndSplitConnectivityMask(Volume*,int,DistmapMask**,DistmapMask**);


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
