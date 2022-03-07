/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_morpho.c         * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 06/11/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : premier fichier du package de morpho math
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


/* NB: In this file, most of the treatments are implemented using chamfer
transform, using the S16BIT volume type, which has some consequences */

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

/*
#include <Vip_morpho_static.h>
*/

/*---------------------------------------------------------------------------*/
/*static int Check3DMorphoSizeConsistence*/
/*---------------------------------------------------------------------------*/
/*(
Volume *vol,
float size
);*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static int Check2DMorphoSizeConsistence
/*---------------------------------------------------------------------------*/
(
Volume *vol,
float size
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static int ImplementedMorphoMode
(
 int mode
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static int ImplementedMorphoMode
(
 int mode
)
/*----------------------------------------------------------------------------*/
{
  if( (mode==CHAMFER_BALL_3D)
      ||(mode==CHAMFER_BALL_2D) ) return(OK);
  else return(PB);
}

/*----------------------------------------------------------------------------*/
int VipDilation(
  Volume *vol,
  int mode,
  float size
)
/*---------------------------------------------------------------------------*/
{ 
  int status=0;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(morpho)VipDilation");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipDilation is only implemented for S16BIT volume");
      (void)fprintf(stderr,"Use VipCreateDilatedVolume for other types...\n");
      VipPrintfExit("(morpho)VipDilation");
      return(PB);
    }

  if(ImplementedMorphoMode(mode)==PB)
    {
      VipPrintfError("Unknown mode in VipDilation");
      VipPrintfExit("(morpho)VipDilation");
      return(PB);
    }
  
  switch(mode)
    {
    case CHAMFER_BALL_3D:
      status = VipCustomizedChamferDilation(vol,size,3,3,3,VIP_USUAL_DISTMAP_MULTFACT,FRONT_PROPAGATION);
      break;
    case CHAMFER_BALL_2D:
      status = VipCustomizedChamferDilation(vol,size,3,3,1,VIP_USUAL_DISTMAP_MULTFACT,FRONT_PROPAGATION);
      break;
    }

  return(status);
}

/*----------------------------------------------------------------------------*/
int VipCustomizedChamferDilation (
  Volume *vol,
  float size,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor,
  int mode
)
/*---------------------------------------------------------------------------*/
{ 

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(morpho)VipCustomizedChamferDilation");
      return(PB);
    }
 
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipCustomizedChamferDilation is only implemented for S16BIT volume");
      (void)fprintf(stderr,"Use VipCreateCustomizedChamferDilatedVolume for other types...\n");
      VipPrintfExit("(morpho)VipCustomizedChamferDilation");
      return(PB);
    }
  if( (mode!=FRONT_PROPAGATION)&&(mode!=CHAMFER_TRANSFORMATION))
    {
      VipPrintfError("Unknown mode in VipCustomizedChamferDilation");     
      VipPrintfExit("(morpho)VipCustomizedChamferDilation");
      return(PB);
    }
 if(Check2DMorphoSizeConsistence(vol,size)==PB)
   {
      VipPrintfExit("(morpho)VipCustomizedChamferDilation");
      return(PB);
   }
  
 if(mode==CHAMFER_TRANSFORMATION)
   {
     if (VipComputeCustomizedChamferDistanceMap (vol,xmask,ymask,zmask,mult_factor)==PB)
       return(PB);
   }
 else
   {
     if (VipComputeCustomizedFrontPropagationChamferDistanceMap (
	vol, 0, -60000, VIP_DO_NOTHING, size, xmask, ymask, zmask, mult_factor)==PB)
       return(PB);
   }

  if (VipSingleThreshold(vol,LOWER_OR_EQUAL_TO,(int)(size*mult_factor+0.5),BINARY_RESULT)==PB)
    return(PB);

  return(OK);
}

/*----------------------------------------------------------------------------*/
int VipCustomizedGeodesicChamferDilation (
  Volume *vol,
  int domain,
  int outside_domain,
  float size,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor
)
/*---------------------------------------------------------------------------*/
{ 

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(morpho)VipCustomizedGeodesicChamferDilation");
      return(PB);
    }
 
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipCustomizedGeodesicChamferDilation is only implemented for S16BIT volume");
      VipPrintfExit("(morpho)VipCustomizedGeodesicChamferDilation");
      return(PB);
    }
 
  if(Check2DMorphoSizeConsistence(vol,size)==PB)
    {
      VipPrintfExit("(morpho)VipCustomizedChamferDilation");
      return(PB);
    }
  

  if (VipComputeCustomizedFrontPropagationChamferDistanceMap (
							      vol, domain, outside_domain, VIP_DO_NOTHING, 
							      size, xmask, ymask, zmask, mult_factor)==PB)
    return(PB);
 

  if (VipSingleThreshold(vol,LOWER_OR_EQUAL_TO,(int)(size*mult_factor+0.5),BINARY_RESULT)==PB)
    return(PB);

  return(OK);
}

/*----------------------------------------------------------------------------*/
int VipConnectivityChamferDilation (
  Volume *vol,
  float size,
  int connectivity,
  int mode
)
/*---------------------------------------------------------------------------*/
{ 

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(morpho)VipConnectivityChamferDilation");
      return(PB);
    }
 
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipConnectivityChamferDilation is only implemented for S16BIT volume");
      VipPrintfExit("(morpho)VipConnectivityChamferDilation");
      return(PB);
    }
  if( (mode!=FRONT_PROPAGATION)&&(mode!=CHAMFER_TRANSFORMATION))
    {
      VipPrintfError("Unknown mode in VipConnectivityChamferDilation");     
      VipPrintfExit("(morpho)VipConnectivityChamferDilation");
      return(PB);
    }
  if(Check2DMorphoSizeConsistence(vol,size)==PB)
   {
      VipPrintfExit("(morpho)VipConnectivityChamferDilation");
      return(PB);
   }
   
  // To avoid issues in VipComputeFrontPropagationConnectivityDistanceMap when size is lower than 1
  if (size < 1.)
    size = 1.;
 
  if(mode==CHAMFER_TRANSFORMATION)
   {
     if (VipComputeConnectivityChamferDistanceMap (vol,connectivity)==PB)
       return(PB);
   }
  else
   {
     VipComputeFrontPropagationConnectivityDistanceMap (vol,0,60000,VIP_DO_NOTHING,size,connectivity);
   }

  if (VipSingleThreshold(vol,LOWER_OR_EQUAL_TO,(int)(size+0.01),BINARY_RESULT)==PB)
    return(PB);

  return(OK);
}
/*----------------------------------------------------------------------------*/
int VipErosion(
  Volume *vol,
  int mode,
  float size
)
/*---------------------------------------------------------------------------*/
{ 
  int status=0;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(morpho)VipErosion");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipErosion is only implemented for S16BIT volume");
      (void)fprintf(stderr,"Use VipCreateErodedVolume for other types...\n");
      VipPrintfExit("(morpho)VipErosion");
      return(PB);
    }

  if(ImplementedMorphoMode(mode)==PB)
    {
      VipPrintfError("Unknown mode in VipErosion");
      VipPrintfExit("(morpho)VipErosion");
      return(PB);
    }
  
  switch(mode)
    {
    case CHAMFER_BALL_3D:
      status = VipCustomizedChamferErosion(vol,size,3,3,3,VIP_USUAL_DISTMAP_MULTFACT,FRONT_PROPAGATION);
      break;
    case CHAMFER_BALL_2D:
      status = VipCustomizedChamferErosion(vol,size,3,3,1,VIP_USUAL_DISTMAP_MULTFACT,FRONT_PROPAGATION);
      break;
    }
  return(status);
}

/*----------------------------------------------------------------------------*/
int VipCustomizedChamferErosion (
  Volume *vol,
  float size,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor,
  int mode
)
/*---------------------------------------------------------------------------*/
{ 
  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(morpho)VipCustomizedChamferErosion");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipCustomizedChamferErosion is only implemented for S16BIT volume");
      (void)fprintf(stderr,"Use VipCreateCustomizedChamferErodedVolume for other types...\n");
      VipPrintfExit("(morpho)VipCustomizedChamferErosion");
      return(PB);
    }

  if(Check2DMorphoSizeConsistence(vol,size)==PB)
    {
      VipPrintfExit("(morpho)VipCustomizedChamferErosion");
      return(PB);
    }

  if( (mode!=FRONT_PROPAGATION)&&(mode!=CHAMFER_TRANSFORMATION))
    {
      VipPrintfError("Unknown mode in VipCustomizedChamferErosion");     
      VipPrintfExit("(morpho)VipCustomizedChamferErosion");
      return(PB);
    }

  if(VipInvertBinaryVolume(vol)==PB) return(PB);

  if(mode==CHAMFER_TRANSFORMATION)
    {
      if(VipComputeCustomizedChamferDistanceMap (vol,xmask,ymask,zmask,mult_factor)==PB) 
	return(PB);
    }
  else
    {
      if (VipComputeCustomizedFrontPropagationChamferDistanceMap (
	  vol, 0, -60000, VIP_DO_NOTHING, size, xmask, ymask, zmask, mult_factor)==PB)
       return(PB);
   }

  if(VipSingleThreshold(vol,GREATER_THAN,(int)(size*mult_factor+0.5),BINARY_RESULT)==PB)
    return(PB);

  return(OK);
}

/*----------------------------------------------------------------------------*/
int VipConnectivityChamferErosion (
  Volume *vol,
  float size,
  int connectivity,
  int mode
)
/*---------------------------------------------------------------------------*/
{ 
  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(morpho)VipConnectivityChamferErosion");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipConnectivityChamferErosion is only implemented for S16BIT volume");
      VipPrintfExit("(morpho)VipConnectivityChamferErosion");
      return(PB);
    }

  if(Check2DMorphoSizeConsistence(vol,size)==PB)
    {
      VipPrintfExit("(morpho)VipConnectivityChamferErosion");
      return(PB);
    }

  if( (mode!=FRONT_PROPAGATION)&&(mode!=CHAMFER_TRANSFORMATION))
    {
      VipPrintfError("Unknown mode in VipConnectivityChamferErosion");     
      VipPrintfExit("(morpho)VipConnectivityChamferErosion");
      return(PB);
    }

  if(VipInvertBinaryVolume(vol)==PB) return(PB);

  // To avoid issues in VipComputeFrontPropagationConnectivityDistanceMap when size is lower than 1
  if (size < 1.) size = 1.;

  if(mode==CHAMFER_TRANSFORMATION)
    {
      if (VipComputeConnectivityChamferDistanceMap (vol,connectivity)==PB)
	return(PB);
    }
  else
    {
      VipComputeFrontPropagationConnectivityDistanceMap (vol,0,60000,VIP_DO_NOTHING,size,connectivity);
    }

  if(VipSingleThreshold(vol,GREATER_THAN,(int)(size + 0.01),BINARY_RESULT)==PB)
    return(PB);

  return(OK);
}

/*----------------------------------------------------------------------------*/
int VipClosing(
  Volume *vol,
  int mode,
  float size
)
/*---------------------------------------------------------------------------*/
{ 
  int status=0;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(morpho)VipClosing");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipClosing is only implemented for S16BIT volume");
      (void)fprintf(stderr,"Use VipCreateClosedVolume for other types...\n");
      VipPrintfExit("(morpho)VipClosing");
      return(PB);
    }

  if(ImplementedMorphoMode(mode)==PB)
    {
      VipPrintfError("Unknown mode in VipClosing");
      VipPrintfExit("(morpho)VipClosing");
      return(PB);
    }
  
  switch(mode)
    {
    case CHAMFER_BALL_3D:
      status = VipCustomizedChamferClosing(vol,size,3,3,3,VIP_USUAL_DISTMAP_MULTFACT,FRONT_PROPAGATION);
      break;
    case CHAMFER_BALL_2D:
      status = VipCustomizedChamferClosing(vol,size,3,3,1,VIP_USUAL_DISTMAP_MULTFACT,FRONT_PROPAGATION);
      break;
    }

  return(status);
}

/*----------------------------------------------------------------------------*/
int VipCustomizedChamferClosing (
  Volume *vol,
  float size,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor,
  int mode
)
/*---------------------------------------------------------------------------*/
{ 
  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(morpho)VipCustomizedChamferClosing");
      return(PB);
    }
 
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipCustomizedChamferClosing is only implemented for S16BIT volume");
      (void)fprintf(stderr,"Use VipCreateCustomizedChamferClosedVolume for other types...\n");
      VipPrintfExit("(morpho)VipCustomizedChamferClosing");
      return(PB);
    }

  if(Check2DMorphoSizeConsistence(vol,size)==PB)
    {
      VipPrintfExit("(morpho)VipCustomizedChamferClosing");
      return(PB);
    }

  if( (mode!=FRONT_PROPAGATION)&&(mode!=CHAMFER_TRANSFORMATION))
    {
      VipPrintfError("Unknown mode in VipCustomizedChamferClosing");     
      VipPrintfExit("(morpho)VipCustomizedChamferClosing");
      return(PB);
    }

  if(mode==CHAMFER_TRANSFORMATION)
    {
      if(VipComputeCustomizedChamferDistanceMap (vol,xmask,ymask,zmask,mult_factor)==PB) 
	return(PB);
    }
  else
    {
      if (VipComputeCustomizedFrontPropagationChamferDistanceMap (
	  vol, 0, -60000, VIP_DO_NOTHING, size, xmask, ymask, zmask, mult_factor)==PB)
       return(PB);
   }


  if(VipSingleThreshold(vol,GREATER_THAN,(int)(size*mult_factor+0.5),BINARY_RESULT)==PB)
    return(PB);

  if(mode==CHAMFER_TRANSFORMATION)
    {
      if(VipComputeCustomizedChamferDistanceMap (vol,xmask,ymask,zmask,mult_factor)==PB) 
	return(PB);
    }
  else
    {
      if (VipComputeCustomizedFrontPropagationChamferDistanceMap (
	  vol, 0, -60000, VIP_DO_NOTHING, size, xmask, ymask, zmask, mult_factor)==PB)
       return(PB);
   }


  if(VipSingleThreshold(vol,GREATER_THAN,(int)(size*mult_factor+0.5),BINARY_RESULT)==PB)
    return(PB);

  return(OK);
}

/*----------------------------------------------------------------------------*/
int VipConnectivityChamferClosing (
  Volume *vol,
  float size,
  int connectivity,
  int mode
)
/*---------------------------------------------------------------------------*/
{ 
  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(morpho)VipConnectivityChamferClosing");
      return(PB);
    }
 
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipConnectivityChamferClosing is only implemented for S16BIT volume");
      VipPrintfExit("(morpho)VipCustomizedChamferClosing");
      return(PB);
    }

  if( (mode!=FRONT_PROPAGATION)&&(mode!=CHAMFER_TRANSFORMATION))
    {
      VipPrintfError("Unknown mode in VipConnectivityChamferClosing");     
      VipPrintfExit("(morpho)VipConnectivityChamferClosing");
      return(PB);
    }

  if(Check2DMorphoSizeConsistence(vol,size)==PB)
    {
      VipPrintfExit("(morpho)VipCustomizedChamferClosing");
      return(PB);
    }
    
  // To avoid issues in VipComputeFrontPropagationConnectivityDistanceMap when size is lower than 1
  if (size < 1.) size = 1.;

  if(mode==CHAMFER_TRANSFORMATION)
    {
      if (VipComputeConnectivityChamferDistanceMap (vol,connectivity)==PB)
	return(PB);
    }
  else
    {
      VipComputeFrontPropagationConnectivityDistanceMap (vol,0,60000,VIP_DO_NOTHING,size,connectivity);
    }

  if(VipSingleThreshold(vol,GREATER_THAN,(int)(size+0.01),BINARY_RESULT)==PB)
    return(PB);

  if(mode==CHAMFER_TRANSFORMATION)
    {
      if (VipComputeConnectivityChamferDistanceMap (vol,connectivity)==PB)
	return(PB);
    }
  else
    {
      VipComputeFrontPropagationConnectivityDistanceMap (vol,0,60000,VIP_DO_NOTHING,size,connectivity);
    }

  if(VipSingleThreshold(vol,GREATER_THAN,(int)(size+0.01),BINARY_RESULT)==PB)
    return(PB);

  return(OK);
}

/*----------------------------------------------------------------------------*/
int VipOpening(
  Volume *vol,
  int mode,
  float size
)
/*---------------------------------------------------------------------------*/
{ 
  int status=0;

  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(morpho)VipOpening");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipOpening is only implemented for S16BIT volume");
      (void)fprintf(stderr,"Use VipCreateOpenedVolume for other types...\n");
      VipPrintfExit("(morpho)VipOpening");
      return(PB);
    }

  if(ImplementedMorphoMode(mode)==PB)
    {
      VipPrintfError("Unknown mode in VipOpening");
      VipPrintfExit("(morpho)VipOpening");
      return(PB);
    }
  
  switch(mode)
    {
    case CHAMFER_BALL_3D:
      status = VipCustomizedChamferOpening(vol,size,3,3,3,VIP_USUAL_DISTMAP_MULTFACT,FRONT_PROPAGATION);
      break;
    case CHAMFER_BALL_2D:
      status = VipCustomizedChamferOpening(vol,size,3,3,1,VIP_USUAL_DISTMAP_MULTFACT,FRONT_PROPAGATION);
      break;
    }

  return(status);
}

/*---------------------------------------------------------------------------*/

int VipCustomizedChamferOpening (
  Volume *vol,
  float size,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor,
  int mode
)
/*---------------------------------------------------------------------------*/
{ 
  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(morpho)VipCustomizedChamferOpening");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipCustomizedChamferOpening is only implemented for S16BIT volume");
      (void)fprintf(stderr,"Use VipCreateCustomizedChamferOpenedVolume for other types...\n");
      VipPrintfExit("(morpho)VipCustomizedChamferOpening");
      return(PB);
    }

  if(Check2DMorphoSizeConsistence(vol,size)==PB)
   {
      VipPrintfExit("(morpho)VipCustomizedChamferOpening");
      return(PB);
    }

  if( (mode!=FRONT_PROPAGATION)&&(mode!=CHAMFER_TRANSFORMATION))
    {
      VipPrintfError("Unknown mode in VipCustomizedChamferOpening");     
      VipPrintfExit("(morpho)VipCustomizedChamferOpening");
      return(PB);
    }

  if(VipInvertBinaryVolume(vol)==PB) return(PB);

  if(mode==CHAMFER_TRANSFORMATION)
    {
      if(VipComputeCustomizedChamferDistanceMap (vol,xmask,ymask,zmask,mult_factor)==PB) 
	return(PB);
    }
  else
    {
      if (VipComputeCustomizedFrontPropagationChamferDistanceMap (
	  vol, 0, -60000, VIP_DO_NOTHING, size, xmask, ymask, zmask, mult_factor)==PB)
       return(PB);
   }

  if(VipSingleThreshold(vol,GREATER_THAN,(int)(size*mult_factor+0.5),BINARY_RESULT)==PB)
    return(PB);

  if(mode==CHAMFER_TRANSFORMATION)
    {
      if(VipComputeCustomizedChamferDistanceMap (vol,xmask,ymask,zmask,mult_factor)==PB) 
	return(PB);
    }
  else
    {
      if (VipComputeCustomizedFrontPropagationChamferDistanceMap (
	  vol, 0, -60000, VIP_DO_NOTHING, size, xmask, ymask, zmask, mult_factor)==PB)
       return(PB);
   }

  if(VipSingleThreshold(vol,LOWER_OR_EQUAL_TO,(int)(size*mult_factor+0.5),BINARY_RESULT)==PB)
    return(PB);

  return(OK);

}

/*---------------------------------------------------------------------------*/

int VipConnectivityChamferOpening (
  Volume *vol,
  float size,
  int connectivity,
  int mode
)
/*---------------------------------------------------------------------------*/
{ 
  if (VipVerifyAll(vol)==PB)
    {
      VipPrintfExit("(morpho)VipConnectivityChamferOpening");
      return(PB);
    }
  if (VipTestType(vol,S16BIT)!=OK)
    {
      VipPrintfError("Sorry,  VipConnectivityChamferOpening is only implemented for S16BIT volume");
      VipPrintfExit("(morpho)VipConnectivityChamferOpening");
      return(PB);
    }

  if( (mode!=FRONT_PROPAGATION)&&(mode!=CHAMFER_TRANSFORMATION))
    {
      VipPrintfError("Unknown mode in VipConnectivityChamferOpening");     
      VipPrintfExit("(morpho)VipConnectivityChamferOpening");
      return(PB);
    }

  if(Check2DMorphoSizeConsistence(vol,size)==PB)
   {
      VipPrintfExit("(morpho)VipConnectivityChamferOpening");
      return(PB);
    }

  if(VipInvertBinaryVolume(vol)==PB) return(PB);

  // To avoid issues in VipComputeFrontPropagationConnectivityDistanceMap when size is lower than 1
  if (size < 1.) size = 1.;
  
  if(mode==CHAMFER_TRANSFORMATION)
    {
      if (VipComputeConnectivityChamferDistanceMap (vol,connectivity)==PB)
	return(PB);
    }
  else
    {
      VipComputeFrontPropagationConnectivityDistanceMap (vol,0,60000,VIP_DO_NOTHING,size,connectivity);
    }

  if(VipSingleThreshold(vol,GREATER_THAN,(int)(size+0.01),BINARY_RESULT)==PB)
    return(PB);

   if(mode==CHAMFER_TRANSFORMATION)
    {
      if (VipComputeConnectivityChamferDistanceMap (vol,connectivity)==PB)
	return(PB);
    }
  else
    {
      VipComputeFrontPropagationConnectivityDistanceMap (vol,0,60000,VIP_DO_NOTHING,size,connectivity);
    }

  if(VipSingleThreshold(vol,LOWER_OR_EQUAL_TO,(int)(size+0.01),BINARY_RESULT)==PB)
    return(PB);

  return(OK);

}

/*---------------------------------------------------------------------------*/
static int Check2DMorphoSizeConsistence
/*---------------------------------------------------------------------------*/
(
Volume *vol,
float size
)
/*---------------------------------------------------------------------------*/
{
  if( (size>0) && ( size<mVipVolSizeX(vol)*mVipVolVoxSizeX(vol) ) &&
      (size>0) && ( size<mVipVolSizeY(vol)*mVipVolVoxSizeY(vol) ) )
    return (OK);
  else 
    {
      VipPrintfWarning("Morphological element size surprisingly large relatively to volume size");
      return(PB);
    }
}

/*---------------------------------------------------------------------------*/
/*static int Check3DMorphoSizeConsistence*/
/*---------------------------------------------------------------------------*/
/*(
Volume *vol,
float size
)*/
/*---------------------------------------------------------------------------*/
/*{
  if( (size>0) && ( size<mVipVolSizeX(vol)*mVipVolVoxSizeX(vol) ) &&
      (size>0) && ( size<mVipVolSizeY(vol)*mVipVolVoxSizeY(vol) ) &&
      (size>0) && ( size<mVipVolSizeZ(vol)*mVipVolVoxSizeZ(vol) ) )
    return (OK);
  else 
    {
      VipPrintfWarning("Morphological element size surprisingly large relatively to volume size");
      return(PB);
    }
}*/
