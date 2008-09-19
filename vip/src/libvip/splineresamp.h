/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/splineresamp      * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 20/01/99
 * VERSION     : 1.4                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : Spline resampler
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



#ifndef VIP_SPLINERESAMP_H
#define VIP_SPLINERESAMP_H

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

#include <vip/volume.h>
#include <vip/depla.h>


/*------------------------------------------------------------------*/
extern Volume *VipSplineResamp(
Volume *vol,
Volume *template,
VipDeplacement *dep,
int degree);
/*------------------------------------------------------------------*/


#ifdef __cplusplus
  }
#endif

#endif /* VIP_SPLINERESAMP_H */
