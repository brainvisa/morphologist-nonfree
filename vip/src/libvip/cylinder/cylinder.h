/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_cylinder.h          * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 22/01/2000
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION :
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

#ifndef VIP_CYLINDER_H
#define VIP_CYLINDER_H

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

/*---------------------------------------------------------------------------*/
int VipWriteCylinder(float x1, float y1, float z1, 
		     float x2, float y2, float z2,
		     char *name, float radius);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipWriteHatCylinder(float x1, float y1, float z1, 
		     float x2, float y2, float z2,
		     char *name,
		     float radius,
		     float height);
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* VIP_CYLINDER_H */
