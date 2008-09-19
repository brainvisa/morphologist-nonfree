/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/deriche.h        * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 04/01/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
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


#ifndef VIP_DERICHE_H
#define VIP_DERICHE_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <vip/bucket.h>
#include <vip/deriche/deriche_gen.h>
#include <vip/deriche/extedge_gen.h>

#define DERICHE_EXTREMA 20
#define DERICHE_NORM 21
#define DERICHE_EXTREMA_DIRECTION 22

#define EXTEDGE2D_ALL 11
#define EXTEDGE2D_ALL_EXCEPT_Y_TOP 12
#define EXTEDGE2D_ALL_EXCEPT_Y_BOTTOM 13

#define EXTEDGE3D_ALL 111
#define EXTEDGE3D_ALL_EXCEPT_Z_TOP 112
#define EXTEDGE3D_ALL_EXCEPT_Z_BOTTOM 113



/*----------------------------------------------------------------------------*/
extern int VipConvertStringToExtedgeMode(
char *thestring
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume  *VipDeriche2DGradientNorm(
  Volume *vol,
  float ALPHA,
  int how,
  int mode,
  float threshold);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume  *VipDeriche3DGradientX(
  Volume *vol,
  float ALPHA,
  int how);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume  *VipDeriche3DGradientY(
  Volume *vol,
  float ALPHA,
  int how);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume  *VipDeriche3DGradientZ(
  Volume *vol,
  float ALPHA,
  int how);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume  *VipDeriche3DSmoothing(
  Volume *vol,
  float ALPHA,
  int how);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int VipDeriche3DGradientNorm(
  Volume *vol,
  float alpha,
  int mode,
  float threshold);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int VipDeriche3DGradientNormAndReturnG3DBucket(
  Volume *vol,
  float alpha,
  int mode,
  float threshold,
  VipG3DBucket **gbuck);
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif /* VIP_DERICHE_H */
