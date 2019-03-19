/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/histo.h          * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 20/04/1997
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 *****************************************************************************
 *
 * DESCRIPTION : gestion des histogrammes (scalespace, Kmeans...)
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



#ifndef VIP_HISTO_H
#define VIP_HISTO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <math.h>

#include <vip/util.h>
#include <vip/alloc.h>				
#include <vip/volume.h>


  /*access to histogram: histo->val[greylevel] */
  /*Once again, we do not provide real encapsulation*/
  /*for optimization purpose*/
  /*Nevertheless, try to use provided methods as far as possible*/

  typedef struct Vip_histo {
    int *histo_ad;         /*buffer address*/
    int *val;              /*val = histo_ad + buf_zero_position, can be outside buffer*/
    int min;               /*first non zero value*/
    int max;               /*mast non zero value*/
    int buf_zero_position; /*val[0] = histo_ad[buf_zero_position] if 0 is inside buffer*/
    int buf_size;         /* buffer length */
    int range_max;          /*buf_max = buf_size - buf_zero_position -1*/
    int range_min;          /*buf_min = -buf_zero_position*/
  } VipHisto;

#include <vip/histo/histo_ss.h>

#define mVipHistoVal(h,g) ((h)->val[(g)])
#define mVipHistoRangeMin(h) ((h)->range_min)
#define mVipHistoRangeMax(h) ((h)->range_max)
  /*NB, if you want to be sure that min and max are uptodate,
    use the functions VipGetHistoMin and VipGetHistoMax*/
#define mVipHistoMin(h) ((h)->min)
#define mVipHistoMax(h) ((h)->max)

#define WRITE_HISTO_BINARY 0
#define WRITE_HISTO_ASCII 1




/*---------------------------------------------------------------*/
extern VipT1HistoAnalysis *VipAnalyseCascadesRidge(
SSCascade *clist,
Vip1DScaleSpaceStruct *volstruct,
float mean);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern VipHisto *VipGetHistoSurface( VipHisto *hin, Volume *vol);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern  VipHisto *VipGetForcedUndersampledHisto(VipHisto *hin, int *ratio);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern VipHisto *VipGetEntropyForcedUndersampledHisto(
VipHisto *hin,
float entropy,
int *ratio);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int VipGetHistoEntropy(
VipHisto *histo,
float *entropo);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int VipPourcentageLowerThanThreshold(Volume *vol, int lowthreshold, int pourcentage);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int VipCutRightFlatPartOfHisto(VipHisto *h, int cutvalue);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int VipTestHistoExists(char *name);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern VipHisto *VipGetUndersampledHisto(VipHisto *hin, int *ratio);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern VipHisto *VipGetPropUndersampledHisto(
VipHisto *hin,
int prop,
int *ratio,
int *power2,
int threshold,
int final_length);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern VipHisto *VipGetCumulHisto(VipHisto *hin);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern VipHisto *VipReadHisto(char *name);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int VipWriteHisto(VipHisto *histo, char *name, int mode);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern VipHisto *VipGetLeftExtendedHisto(VipHisto *hin, int length);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern VipHisto *VipGetRightExtendedHisto(VipHisto *hin, int length);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern VipHisto *VipCreateHisto(
int minvalue,
int maxvalue);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int VipFreeHisto(VipHisto *histo);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int VipSetHistoVal(
VipHisto *histo,
int greylevel,
int value);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int VipGetHistoVal(
VipHisto *histo,
int greylevel,
int *value);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int VipGetHistoMin(
VipHisto *histo,
int *min);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int VipGetHistoMax(
VipHisto *histo,
int *max);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern VipHisto *VipComputeVolumeHisto(Volume *vol);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern VipHisto *VipComputeCleanedUpVolumeHisto(Volume *vol, Volume *discard);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern Volume *VipComputeMeanVolume(Volume *vol);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern Volume *VipComputeVarianceVolume(Volume *vol);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern VipHisto *VipCreateThreeGaussianSumHisto( 
float m1,
float s1,
float h1,
float m2,
float s2,
float h2,
float m3,
float s3,
float h3);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
VipHisto *VipCreateForGaussianSumHisto( 
float m1,
float s1,
float h1,
float m2,
float s2,
float h2,
float m3,
float s3,
float h3,
float m4,
float s4,
float h4);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int  VipComputeStatInRidgeVolume(
Volume *vol,
Volume *thresholdedvol,
float *mean,
float *sigma,
int robust);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int VipComputeRobustStatInMaskVolume(
Volume *vol,
Volume *thresholdedvol,
float *mean,
float *sigma,
int robust);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int VipIterateToGetPropUndersampledRatio(
VipHisto *histo,
int *ratio,
int ratios[5][5],
int j);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int VipGetOtsuThreshold(Volume *vol);
/*---------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* VIP_HISTO_H */






