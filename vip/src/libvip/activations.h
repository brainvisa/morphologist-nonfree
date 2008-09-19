/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/activations.h       * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 20/10/99
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : contrast SPM to graph
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *              / /   |              |
 *****************************************************************************/



#ifndef VIP_ACTIVATIONS_H
#define VIP_ACTIVATIONS_H

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

/*-------------------------------------------------------------------------*/
extern int VipComputeLabelledClusterTmtkTriangulation( Volume *labeling, char *name);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
extern int  VipWriteClusterArg( Volume *vol,
int size,   int connectivity,
char *name, char *normaname, char meshflag);
/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif /* VIP_ACTIVATIONS_H */
