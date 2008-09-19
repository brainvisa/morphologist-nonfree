/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/gaussian.h        * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 07/12/98
 * VERSION     : 1.4                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : jfmangin@cea.fr
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


#ifndef VIP_GAUSSIAN_H
#define VIP_GAUSSIAN_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <vip/bucket.h>

#define NEW_FLOAT_VOLUME 707

/*---------------------------------------------------------------------------*/
extern Volume  *VipDeriche3DGaussian(
  Volume *vol,
  float sigma,
  int how);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern Volume  *VipDeriche2DAxiGaussian(
  Volume *vol,
  float sigmaTrans, float sigmaAxi,
  int how);
/*---------------------------------------------------------------------------*//*---------------------------------------------------------------------------*/
extern Volume  *VipDericheCustomGaussian(
  Volume *vol,
  float sx, float sy, float sz,
  int how);
/*---------------------------------------------------------------------------*/


#ifdef __cplusplus
  }
#endif

#endif /* VIP_GAUSSIAN_H */
