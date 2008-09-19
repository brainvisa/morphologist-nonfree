/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_distmap.c        * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 05/10/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : tout ce qui concerne les transformations de distances
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

float VIP_USED_DISTMAP_MULTFACT = VIP_USUAL_DISTMAP_MULTFACT;

int CreateAndSplitConnectivityMask(Volume*,int,DistmapMask**,DistmapMask**);


/*----------------------------------------------------------------------------*/
int VipComputeChamferDistanceMap (
  Volume *vol
)
{
  return(VipComputeCustomizedChamferDistanceMap( vol, 3, 3, 3, VIP_USUAL_DISTMAP_MULTFACT));
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int VipComputeCustomizedChamferDistanceMap (
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

  if(!VipVerifyAll(vol))
    {
      VipPrintfExit("(distmap)VipComputeCustomizedChamferDistanceMap");
      return(PB);
    }


  if(mult_factor<=0.)
    {
      VipPrintfError("Strange mult_factor (use VIP_USUAL_DISTMAP_MULTFACT)p");
      VipPrintfExit("(distmap)VipComputeCustomizedChamferDistanceMap");
      return(PB);

    }
  else VIP_USED_DISTMAP_MULTFACT = mult_factor;

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipComputeCustomizedChamferDistanceMap is only implemented for S16BIT volume");
      VipPrintfExit("(distmap)VipComputeCustomizedChamferDistanceMap");
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
  VolumeChamferPreparation( vol);

  if (CreateAndSplitCubicMask ( vol, xmask, ymask, zmask, &f, &b )==PB) 
    return(PB);
   
  (void)printf("Forward.....\n");
  nb_change = ForwardSweepingWithBorder( vol, f, VIP_OUTSIDE_DOMAIN);
  if(nb_change==0)
    {
      VipPrintfWarning("Strange image in VipComputeCustomizedChamferDistanceMap?");
    }
  (void)printf("Backward.....\n");
  (void)BackwardSweepingWithBorder( vol, b, VIP_OUTSIDE_DOMAIN);

  FreeMask( f );
  FreeMask( b);

  return(OK);

}

/*----------------------------------------------------------------------------*/
int VipComputeConnectivityChamferDistanceMap (
  Volume *vol,
  int connectivity
)
/*---------------------------------------------------------------------------*/


{
  DistmapMask *f, *b;
  int nb_change;

  if(!VipVerifyAll(vol))
    {
      VipPrintfExit("(distmap)VipComputeConnectivityChamferDistanceMap");
      return(PB);
    }


  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipComputeConnectivityChamferDistanceMap is only implemented for S16BIT volume");
      VipPrintfExit("(distmap)VipComputeConnectivityChamferDistanceMap");
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
  VolumeChamferPreparation( vol);

  if(CreateAndSplitConnectivityMask ( vol, connectivity, &f, &b )==PB) return(PB);
   
  (void)printf("Forward.....\n");
  nb_change = ForwardSweepingWithBorder( vol, f, VIP_OUTSIDE_DOMAIN);
  if(nb_change==0)
    {
      VipPrintfWarning("Strange image in VipComputeConnectivityChamferDistanceMap?");
    }
  (void)printf("Backward.....\n");
  (void)BackwardSweepingWithBorder( vol, b, VIP_OUTSIDE_DOMAIN);

  FreeMask( f );
  FreeMask( b);

  return(OK);

}

/*----------------------------------------------------------------------------*/
int VipComputeGeodesicChamferDistanceMap (
  Volume *vol,
  int domain,
  int outside_domain,
  int nbitermax
)
{
  return(VipComputeCustomizedGeodesicChamferDistanceMap( vol, domain, outside_domain, nbitermax,
						  3, 3, 3, VIP_USUAL_DISTMAP_MULTFACT));
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int VipComputeCustomizedGeodesicChamferDistanceMap (
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

  if(!VipVerifyAll(vol))
    {
      VipPrintfExit("(distmap)VipComputeCustomizedGeodesicChamferDistanceMap");
      return(PB);
    }
  if ((nbitermax>100)||(nbitermax<=0))
    {
      VipPrintfError("Strange maximum iteration number");
      VipPrintfExit("(distmap)VipComputeCustomizedGeodesicChamferDistanceMap");
      return(PB);
    }

  if(mult_factor<=0.)
    {
      VipPrintfError("Strange mult_factor (use VIP_USUAL_DISTMAP_MULTFACT)");
      VipPrintfExit("(distmap)VipComputeCustomizedGeodesicChamferDistanceMap");
      return(PB);
    }
  else VIP_USED_DISTMAP_MULTFACT = mult_factor;

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipComputeCustomizedGeodesicChamferDistanceMap is only implemented for S16BIT volume");
      VipPrintfExit("(distmap)VipComputeCustomizedGeodesicChamferDistanceMap");
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
  VolumeChamferPreparationGeodesic( vol, domain, outside_domain);

  CreateAndSplitCubicMask ( vol, xmask, ymask, zmask, &f, &b );

   
  changef = changeb = VTRUE;
  nbiter = 1;
  while( ((changeb!=VFALSE) || (changef!=VFALSE))&&(nbiter<=nbitermax))
    {
      (void)printf("Forward.....\n");
      changef = ForwardSweepingWithBorder( vol, f, VIP_OUTSIDE_DOMAIN);
      (void)printf("Backward.....\n");
      changeb = BackwardSweepingWithBorder( vol, b, VIP_OUTSIDE_DOMAIN);
      (void)printf("iter: %d, modif: %d\n", nbiter++, changeb + changef);
    }

  VipChangeIntLabel(vol,VIP_OUTSIDE_DOMAIN, outside_domain);
  FreeMask( f );
  FreeMask( b);

  return(OK);


}

/*----------------------------------------------------------------------------*/
int VipComputeConnectivityGeodesicChamferDistanceMap (
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

  if(!VipVerifyAll(vol))
    {
      VipPrintfExit("(distmap)VipComputeConnectivityGeodesicChamferDistanceMap");
      return(PB);
    }
  if ((nbitermax>100)||(nbitermax<=0))
    {
      VipPrintfError("Strange maximum iteration number");
      VipPrintfExit("(distmap)VipComputeConnectivityGeodesicChamferDistanceMap");
      return(PB);
    }

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipComputeConnectivityGeodesicChamferDistanceMap is only implemented for S16BIT volume");
      VipPrintfExit("(distmap)VipComputeConnectivityGeodesicChamferDistanceMap");
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
  VolumeChamferPreparationGeodesic( vol, domain, outside_domain);

  CreateAndSplitConnectivityMask ( vol, connectivity, &f, &b );
   
  changef = changeb = VTRUE;
  nbiter = 1;
  while( ((changeb!=VFALSE) || (changef!=VFALSE))&&(nbiter<=nbitermax))
    {
      (void)printf("Forward.....\n");
      changef = ForwardSweepingWithBorder( vol, f, VIP_OUTSIDE_DOMAIN);
      (void)printf("Backward.....\n");
      changeb = BackwardSweepingWithBorder( vol, b, VIP_OUTSIDE_DOMAIN);
      (void)printf("iter: %d, modif: %d\n", nbiter++, changeb + changef);
    }

  VipChangeIntLabel(vol,VIP_OUTSIDE_DOMAIN, outside_domain);
  FreeMask( f );
  FreeMask( b);

  return(OK);
}


/*-------------------------------------------------------------------------*/
void VolumeChamferPreparation(
Volume *vol
)
/*-------------------------------------------------------------------------*/

{
  VipOffsetStruct *vos;
  int ix, iy, iz;
  Vip_S16BIT *ptr;

  vos = VipGetOffsetStructure(vol);
  ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;

  for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
    {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	{
	  for ( ix = mVipVolSizeX(vol); ix-- ; )        /* loop on points */
	    {
	      if (*ptr) *ptr++ = 0;
	      else *ptr++ = CHAMFER_DOMAIN;
	    }
	  ptr += vos->oPointBetweenLine;  /*skip border points*/
	}
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
    }

  VipFree(vos);
}

/*-------------------------------------------------------------------------*/
void VolumeChamferPreparationGeodesic(
Volume *vol,
int domain,
int outside_domain
)
/*-------------------------------------------------------------------------*/

{
  VipOffsetStruct *vos;
  int ix, iy, iz;
  Vip_S16BIT *ptr;

  vos = VipGetOffsetStructure(vol);
  ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;

  for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
    {
      for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	{
	  for ( ix = mVipVolSizeX(vol); ix-- ; )        /* loop on points */
	    {
	      
	      if (*ptr==outside_domain) *ptr = VIP_OUTSIDE_DOMAIN;
	      else if (*ptr==domain) *ptr = CHAMFER_DOMAIN;	      
	      else *ptr = 0;
	    
	      ptr++;
	    }
	  ptr += vos->oPointBetweenLine;  /*skip border points*/
	}
      ptr += vos->oLineBetweenSlice; /*skip border lines*/
    }
	
  VipFree(vos);
}

/*----------------------------------------------------------------------------*/
int VipComputeFrontPropagationChamferDistanceMap (
  Volume *vol,
  int domain,
  int outside_domain,
  int limit_mode,
  float limit_value
)
/*---------------------------------------------------------------------------*/
{
  return(VipComputeCustomizedFrontPropagationChamferDistanceMap(vol,domain,outside_domain,
								limit_mode,limit_value,3,3,3,VIP_USUAL_DISTMAP_MULTFACT));
								
}
/*----------------------------------------------------------------------------*/
int VipComputeCustomizedFrontPropagationChamferDistanceMap (
  Volume *vol,
  int domain,
  int outside_domain,
  int limit_mode,
  float flimit_value,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor
)
/*---------------------------------------------------------------------------*/
{
  DistmapMask *m;
  int limit_value;

  if(!VipVerifyAll(vol))
    {
      VipPrintfExit("(distmap)VipComputeCustomizedFrontPropagationChamferDistanceMap");
      return(PB);
    }
  if ((limit_mode!=VIP_PUT_LIMIT_TO_INFINITE)&&(limit_mode!=VIP_PUT_LIMIT_TO_OUTSIDE)&&
      (limit_mode!=VIP_PUT_LIMIT_TO_LIMIT)&&(limit_mode!=VIP_NO_LIMIT_IN_PROPAGATION)
      &&(limit_mode!=VIP_DO_NOTHING))
    {
      VipPrintfError("Unkown limit mode");
      VipPrintfExit("(distmap)VipComputeCustomizedFrontPropagationChamferDistanceMap");
      return(PB);
    }

  if ((limit_mode!=VIP_NO_LIMIT_IN_PROPAGATION)&&(flimit_value<=0))
    {
      VipPrintfError("Negative or NULL limit value!");
      VipPrintfExit("(distmap)VipComputeCustomizedFrontPropagationChamferDistanceMap");
      return(PB);
    }

  if(mult_factor<=0.)
    {
      VipPrintfError("Strange mult_factor (use VIP_USUAL_DISTMAP_MULTFACT)");
      VipPrintfExit("(distmap)VipComputeCustomizedFrontPropagationChamferDistanceMap");
      return(PB);
    }
  else VIP_USED_DISTMAP_MULTFACT = mult_factor;

  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipComputeCustomizedFrontPropagationChamferDistanceMap is only implemented for S16BIT volume");
      VipPrintfExit("(distmap)VipComputeCustomizedFrontPropagationChamferDistanceMap");
      return(PB);
    }

  if (VipBorderWidthOf(vol)<=0)
    {
      VipPrintfInfo("Volume should have non nul borderwidth");
      VipPrintfExit("VipComputeCustomizedFrontPropagationChamferDistanceMap");
      return(PB);
    }
  else
    {
      VipSetBorderLevel( vol, VIP_OUTSIDE_DOMAIN );
    }
  VolumeChamferPreparationGeodesic( vol, domain, outside_domain);

  m = CreateSquareMask ( vol, xmask, ymask, zmask);

  if( (mVipVolBorderWidth(vol)<m->xcubesize)
      ||(mVipVolBorderWidth(vol)<m->ycubesize)
      ||(mVipVolBorderWidth(vol)<m->zcubesize)
      )
      {
      VipPrintfInfo("Volume should have borderwidth larger than chamfer mask dimensions");
      VipPrintfExit("VipComputeCustomizedFrontPropagationChamferDistanceMap");
      return(PB);
    }
   
  if (limit_mode==VIP_NO_LIMIT_IN_PROPAGATION)
    limit_value = 60000;
  else limit_value = (int)(flimit_value*VIP_USED_DISTMAP_MULTFACT);

  if(FrontPropagation(vol, m, limit_value)==PB) return(PB);
  
  if(limit_mode==VIP_PUT_LIMIT_TO_INFINITE)
    {
      VipChangeIntRange(vol,VIP_BETWEEN,limit_value,VIP_OUTSIDE_DOMAIN, CHAMFER_DOMAIN);
    }
  else if(limit_mode==VIP_PUT_LIMIT_TO_OUTSIDE)
    {
      VipChangeIntInfiniteRange(vol, GREATER_THAN,limit_value, outside_domain);
    }
  else if(limit_mode==VIP_PUT_LIMIT_TO_LIMIT)
    {
      VipChangeIntRange(vol,VIP_BETWEEN,limit_value,VIP_OUTSIDE_DOMAIN, limit_value);
      VipChangeIntLabel(vol,VIP_OUTSIDE_DOMAIN, outside_domain);
    }
  else if(limit_mode==VIP_DO_NOTHING)
    {
      /*private mode for fast morphology*/
    }
  else
    {
      VipChangeIntLabel(vol,VIP_OUTSIDE_DOMAIN, outside_domain);
    }
  FreeMask( m );

  return(OK);


}

/*----------------------------------------------------------------------------*/
int VipComputeFrontPropagationConnectivityDistanceMap (
  Volume *vol,
  int domain,
  int outside_domain,
  int limit_mode,
  int limit_value,
  int connectivity
)
/*---------------------------------------------------------------------------*/
{
  DistmapMask *m;

  if(!VipVerifyAll(vol))
    {
      VipPrintfExit("(distmap)VipComputeFrontPropagationConnectivityDistanceMap");
      return(PB);
    }
  if ((limit_mode!=VIP_PUT_LIMIT_TO_INFINITE)&&(limit_mode!=VIP_PUT_LIMIT_TO_OUTSIDE)&&
      (limit_mode!=VIP_PUT_LIMIT_TO_LIMIT)&&(limit_mode!=VIP_NO_LIMIT_IN_PROPAGATION)
      &&(limit_mode!=VIP_DO_NOTHING))
    {
      VipPrintfError("Unkown limit mode");
      VipPrintfExit("(distmap)VipComputeFrontPropagationConnectivityDistanceMap");
      return(PB);
    }

  if ((limit_mode!=VIP_NO_LIMIT_IN_PROPAGATION)&&(limit_value<=0))
    {
      VipPrintfError("Negative or NULL limit value!");
      VipPrintfExit("(distmap)VipComputeFrontPropagationConnectivityDistanceMap");
      return(PB);
    }


  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipComputeFrontPropagationConnectivityDistanceMap is only implemented for S16BIT volume");
      VipPrintfExit("(distmap)VipComputeFrontPropagationConnectivityDistanceMap");
      return(PB);
    }

  if (VipBorderWidthOf(vol)<=0)
    {
      VipPrintfInfo("Volume should have non nul borderwidth");
      VipPrintfExit("VipComputeFrontPropagationConnectivityDistanceMap");
      return(PB);
    }
  else
    {
      VipSetBorderLevel( vol, VIP_OUTSIDE_DOMAIN );
    }
  VolumeChamferPreparationGeodesic( vol, domain, outside_domain);

  m = CreateConnectivityMask ( vol, connectivity);
  VIP_USED_DISTMAP_MULTFACT = 1.;

  if( (mVipVolBorderWidth(vol)<m->xcubesize)
      ||(mVipVolBorderWidth(vol)<m->ycubesize)
      ||(mVipVolBorderWidth(vol)<m->zcubesize)
      )
      {
      VipPrintfInfo("Volume should have borderwidth larger than chamfer mask dimensions");
      VipPrintfExit("VipComputeFrontPropagationConnectivityDistanceMap");
      return(PB);
    }
   
  if (limit_mode==VIP_NO_LIMIT_IN_PROPAGATION)
    limit_value = 60000;

  /*Warning: could be optimized...*/
  if(FrontPropagationConnectivity(vol, m, limit_value)==PB) return(PB);
  
  if(limit_mode==VIP_PUT_LIMIT_TO_INFINITE)
    {
      VipChangeIntRange(vol,VIP_BETWEEN,limit_value,VIP_OUTSIDE_DOMAIN, CHAMFER_DOMAIN);
    }
  else if(limit_mode==VIP_PUT_LIMIT_TO_OUTSIDE)
    {
      VipChangeIntInfiniteRange(vol, GREATER_THAN,limit_value, outside_domain);
    }
  else if(limit_mode==VIP_PUT_LIMIT_TO_LIMIT)
    {
      VipChangeIntRange(vol,VIP_BETWEEN,limit_value,VIP_OUTSIDE_DOMAIN, limit_value);
      VipChangeIntLabel(vol,VIP_OUTSIDE_DOMAIN, outside_domain);
    }
  else if(limit_mode==VIP_DO_NOTHING)
    {
      /*private mode for fast morphology*/
    }
  else
    {
      VipChangeIntLabel(vol,VIP_OUTSIDE_DOMAIN, outside_domain);
    }
  FreeMask( m );

  return(OK);


}
