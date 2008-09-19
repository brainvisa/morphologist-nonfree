/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/linresamp.h      * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 02/02/1997
 * VERSION     : 1.0                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : PC Linux
 *****************************************************************************
 *
 * DESCRIPTION : Trilinear resampler
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



#ifndef VIP_LINRESAMP_H
#define VIP_LINRESAMP_H

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
#include <vip/bucket.h>
#include <vip/depla.h>

#include <vip/linresamp/linresamp_gen.h>

#define VIP_LINRESAMP_ZERO 0
#define VIP_LINRESAMP_MINI 1

/*------------------------------------------------------------------*/
extern Volume *VipLinearResamp(
		Volume *vol,
		Volume *templatev,
		VipDeplacement *dep)
/*------------------------------------------------------------------*/
;    


#ifdef __cplusplus
  }
#endif

#endif /* VIP_LINRESAMP_H */
