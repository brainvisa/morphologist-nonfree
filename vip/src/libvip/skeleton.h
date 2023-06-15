/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/skeleton.h       * TYPE     : Include
 * AUTHOR      : MANGIN J.F.          * CREATION : 12/04/97
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 *****************************************************************************
 *
 * DESCRIPTION : various skeletonization algorithms
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

#ifndef VIP_SKELETON_H
#define VIP_SKELETON_H

#ifdef __cplusplus
extern "C" {
#endif

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
#include <vip/bucket.h>
#include <vip/pyramid.h>
#include <vip/topology.h>
#include <vip/skeleton/skeleton_gen.h>

#define VIP_INITIAL_FRONT_SIZE 500000
#define VIP_FRONT_SIZE_INCREMENT 200000
#define VIP_FRONT -123

#define NON_SIMPLE_AND_NON_VOLUME_BECOME_IMMORTAL 77
#define SURFACES_BECOME_IMMORTAL 78
#define CURVES_BECOME_IMMORTAL 79
#define NOBODY_BECOME_IMMORTAL 80

#define FRONT_6CONNECTIVITY_DIRECTION 88

#define FRONT_RANDOM_ORDER 107
#define FRONT_RANDOM_AND_DEPTH 108

/*---------------------------------------------------------------------------*/
// int VipHomotopicErosionFromInside( Volume *vol, int nb_iteration,
// 				  int object, int inside, int outside, int front_mode);
/*-------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int VipHomotopicErosionFromInsideSnake( Volume *vol, Volume *graylevel, int nb_iteration,
				  int object, int inside, int outside,
					float KISING, float mG, float sigmaG, float mW, float sigmaW);
/*-------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
  extern int VipHomotopicInsideDilationSnakeRidge( Volume *ridge,Volume *vol, Volume *graylevel, int nb_iteration,
				  int object, int inside, int outside, int front_mode,
			       float KISING, float mG, float sigmaG, float mW, float sigmaW);
/*-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipHomotopicInsideDilationSnake( Volume *vol, Volume *graylevel, int nb_iteration,
				  int object, int inside, int outside, int front_mode,
			       float KISING, float mG, float sigmaG, float mW, float sigmaW);
/*-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipHomotopicInsideDilation( Volume *vol, Volume *mask, int nb_iteration,
				       int object, int inside, int outside, int front_mode );
/*-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipFoetusHomotopicInsideDilationSnake( Volume *vol, Volume *graylevel, int nb_iteration,
				  int object, int inside, int outside, int front_mode,
			       float KISING, float mG, float sigmaG, float mW, float sigmaW);
/*-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipHomotopicInsideSoftDilationSnake( Volume *vol, Volume *graylevel, int nb_iteration,
				  int object, int inside, int outside, int front_mode,
			       float KISING, float mG, float sigmaG, float mW, float sigmaW);
/*-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipHomotopicGeodesicErosionFromOutside( Volume *vol, int nb_iteration,
				  int object, int inside, int outside);
/*-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipHomotopicErosionFromInside( Volume *vol, Volume *graylevel, int nb_iteration,
				  int object, int inside, int outside );
/*-------------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
extern Pyramid *VipCreateBoundingBoxLabelPyramid(Pyramid *pdata, int nlevel, int object, int inside, int outside);
/*--------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipHomotopicOutsidePruning( Volume *vol, int inside, int outside );
/*---------------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
extern int VipHomotopicGeodesicDilation( Volume *vol, int nb_iteration,
				  int object, int domain, int forbiden, int outside, int front_mode );
/*--------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipHomotopicCurvePruning( Volume *vol, int inside, int outside );
/*---------------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*Points become immortals according to immortal_elixir*/
/*The front is managed according to front_mode*/
extern int VipHomotopicSkeleton( Volume *vol, int immortal_elixir,
                                 int front_mode, int keep_immortals );

/*--------------------------------------------------------------------*/
extern int VipWatershedHomotopicSkeleton( Volume *vol, Volume *altitude, int immortal_elixir, int inside, int outside);
/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
extern Volume *ConvertBrainToAltitude(
  Volume *brain,
  float sigma,
  float level0,
  float levelmax,
  float wave_erosion,
  int bwidth,
  float threshold);
/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
extern Volume *ConvertMeanCurvToAltitude(
  Volume *meancurv,
  float level0,
  float levelmax,
  float wave_erosion,
  int bwidth,
  float threshold);
/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
extern int VipFillNextFrontFromOldFront(
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  VipConnectivityStruct *vcs,
  int next_value,
  int front_value);
/*--------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*Put all non volume point in front*/
extern VipIntBucket *VipCreateFrontIntBucket( Volume *vol, int connectivity, int front_value);

/*-------------------------------------------------------------------------*/
/*Assume there is no volume point in front, remove all IMMORTALS (accordind to immortal_elixir) from front*/
extern int VipCleanUpFrontFromImmortals(
  Topology26Neighborhood *topo26,
  int immortal_elixir,
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  int *immortals,
  int front_value);

/*---------------------------------------------------------------------------*/
extern VipIntBucket *VipCreateFrontIntBucketForDilation(
  Volume *vol,
  int connectivity,
  int front_value,
  int object, 
  int domain,
  int front_mode);
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipFillNextFrontFromOldFrontForDilation(
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  VipConnectivityStruct *vcs,
  int next_value,
  int front_value,
  int object);
/*-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern VipIntBucket *VipCreateFrontIntBucketForErosionFromOutside( Volume *vol, int connectivity, int front_value,
				       int object, int outside);
/*---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern int VipFillNextFrontFromOldFrontForErosionFromOutside(
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  VipConnectivityStruct *vcs,
  int next_value,
  int front_value,
  int outside,
  int inside);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
extern int VipFillNextFrontFromOldFrontForErosion(
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  VipConnectivityStruct *vcs,
  int next_value,
  int front_value,
  int outside,
  int inside);
/*-------------------------------------------------------------------------*/

extern VipIntBucket *VipCreateFrontIntBucketHollowObject( Volume *vol, int connectivity, int front_value, int inside, int outside);

extern int VipCleanUpFrontFromImmortalsHollowObject(
  Topology26Neighborhood *topo26,
  int immortal_elixir,
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  int *immortals,
  int front_value,
  int inside,
  int outside);

extern int VipFillNextFrontFromOldFrontHollowObject(
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  VipConnectivityStruct *vcs,
  int front_value,
  int inside,
  int outside);
/*---------------------------------------------------------------------------*/
extern int VipRandomizeFrontOrder(VipIntBucket *front, int nloop);

extern int VipWatershedFrontPropagation( Volume *vol, Volume *altitude, Volume *plan_hemi, int int_min, int int_max, int domain, int outside, int nb_interval, int connectivity);

extern int VipFillNextFrontFromOldFrontVoronoiObject(
  Vip_S16BIT *first_vol_point,
  VipIntBucket *buck,
  VipIntBucket *nextbuck,
  VipConnectivityStruct *vcs,
  int front_value,
  int domain,
  int outside);

extern VipIntBucket *VipCreateFrontIntBucketVoronoiObject( Volume *vol, int connectivity, int front_value, int domain, int outside);

int
VipHomotopicWellComposedErosionFromOutside (Volume * vol,
                                            int nb_iteration,
                                            Vip_S16BIT object,
                                            Vip_S16BIT inside,
                                            Vip_S16BIT outside);
int
VipHomotopicWellComposedDilationTowardInside (Volume * vol,
                                              int nb_iteration,
                                              Vip_S16BIT object,
                                              Vip_S16BIT inside,
                                              Vip_S16BIT forbidden,
                                              Vip_S16BIT outside,
                                              int front_mode);


#ifdef __cplusplus
}
#endif

#endif
