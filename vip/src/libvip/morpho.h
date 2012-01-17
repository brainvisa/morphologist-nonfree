/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/morpho.h         * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 05/10/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Diverses constantes et procedures liees aux cartes de 
 *               distance
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


#ifndef VIP_MORPHO_H
#define VIP_MORPHO_H

#ifdef __cplusplus
  extern "C" {
#endif

#define CHAMFER_BALL_2D 77
#define CHAMFER_BALL_3D 78

#define FRONT_PROPAGATION 81
#define CHAMFER_TRANSFORMATION 82

#define MINIMA 11
#define MAXIMA 12
#define STRICTMINIMA 13
#define STRICTMAXIMA 14

#define VIP_GEODESIC_DEPTH 17
#define VIP_BIRDFLY_DEPTH 18

/*NB: a good method should cope with morphomath methods*/
/*---------------------------------------------------------------------------*/
extern Volume *VipLocalExtrema(Volume *vol, int connectivity, int forbidden, int extrematype);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipComputeWatershedVoronoi(
  Volume *vol,
  Volume *label,
  int connectivity,
  int domain,
  int forbidden,
  int nseed,
  int hight_threshold
);
/*---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
extern Volume *VipSulcalRootsWatershedVoronoi(
				Volume *vol,
				Volume *saddle_point,
				int connectivity_extrema,
				int mode,
				int domain,
				int forbidden,
				int outside,
				int hight_threshold
);
/*---------------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
extern Volume *ConvertBrainToSaddlePoint(Volume *brain, float sigma, int bwidth, float threshold);
/*--------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
extern int VipDilation(
  Volume *vol,
  int mode,
  float size
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipConnectivityChamferDilation (
  Volume *vol,
  float size,
  int connectivity,
  int mode
);
/*---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
extern int VipCustomizedChamferDilation (
  Volume *vol,
  float size,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor,
  int mode
);
/*---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
extern int VipCustomizedGeodesicChamferDilation (
  Volume *vol,
  int domain,
  int outside_domain,
  float size,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipHomotopicInsideVolumeDilation ( 
  Volume *vol1,
  Volume *vol2,
  int nb_iteration,
  int domain,
  int outside_domain,
  int front_mode
);
/*-------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipErosion(
  Volume *vol,
  int mode,
  float size
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipConnectivityChamferErosion (
  Volume *vol,
  float size,
  int connectivity,
  int mode
);
/*---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
extern int VipCustomizedChamferErosion (
  Volume *vol,
  float size,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor,
  int mode
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipClosing(
  Volume *vol,
  int mode,
  float size
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipCustomizedChamferClosing (
  Volume *vol,
  float size,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor,
  int mode
);
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern int VipConnectivityChamferClosing (
  Volume *vol,
  float size,
  int connectivity,
  int mode
);
/*---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
extern int VipOpening(
  Volume *vol,
  int mode,
  float size
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

extern int VipConnectivityChamferOpening (
  Volume *vol,
  float size,
  int connectivity,
  int mode
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipCustomizedChamferOpening (
  Volume *vol,
  float size,
  int xmask,
  int ymask,
  int zmask,
  float mult_factor,
  int mode
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Volume *VipIntersectionDirectionalGrayLevelClosing(Volume *vol);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Volume *VipGrayLevelPlanDilation(Volume *vol, int connectivity, int forbidden);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Volume *VipGrayLevelPlanErosion(Volume *vol, int connectivity, int forbidden);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Volume *VipGrayLevelPlanDilationErosion(Volume *vol, int connectivity, int forbidden);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Volume *VipGrayLevelPlanErosionDilation(Volume *vol, int connectivity, int forbidden);
/*---------------------------------------------------------------------------*/


#ifdef __cplusplus
  }
#endif

#endif /* VIP_MORPHO_H */
