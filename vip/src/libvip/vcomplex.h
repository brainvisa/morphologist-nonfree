/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/vcomplex.h       * TYPE     : Header
 * AUTHOR      : POUPON F.            * CREATION : 20/11/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : eclarations liees au calcul sur des complexes
 *               cpoupon@cea.fr
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


#ifndef VIP_COMPLEX_H
#define VIP_COMPLEX_H

#ifdef __cplusplus
  extern "C" {
#endif 

#include <math.h>
#include <float.h>
#include <vip/util.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef PI
#define PI M_PI
#endif

#include <vip/vcomplex/vcomplex_gen.h>

#define mVipRealPart(c)       (c).re
#define mVipImaginaryPart(c)  (c).im

#ifdef __cplusplus
  }
#endif

#endif
