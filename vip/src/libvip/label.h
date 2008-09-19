/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/label.h          * TYPE     : Header
 * AUTHOR      : POUPON F.            * CREATION : 14/11/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Recupere le(s) contour(s) a partir d'un label ou d'une
 *               liste de labels
 *
 ****************************************************************************
 *
 * USED MODULES : math.h - vip/volume.h - vip/util.h - vip/connex.h
 *                vip/alloc.h - vip/bucket.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_LABEL_H
#define VIP_LABEL_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <math.h>

#include <vip/volume.h>
#include <vip/util.h>
#include <vip/connex.h>
#include <vip/alloc.h>
#include <vip/bucket.h>

/*----------------------------------------------------------------------------*/
extern Vip3DBucket_S16BIT *VipGetSingleLabelContour(
  Volume *vol,
  int     label,
  int     connexite
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume *VipGetMultipleLabelContourBucketsAndVolume(
  Volume *vol,
  Vip3DBucketVector_S16BIT *bucket,
  int *label,
  int *label_col,
  int  nb_label,
  int  connexite
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume *VipGetMultipleLabelContourVolume(
   Volume *vol,
   int    *label,
   int    *label_col,
   int     nb_label,
   int     connexite
);
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif
