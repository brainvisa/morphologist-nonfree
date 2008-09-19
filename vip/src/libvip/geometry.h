/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/geometry.h       * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 8/12/98
 * VERSION     : 1.4                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : Invariants differentiels
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *              / /   |              |
 *****************************************************************************/



#ifndef VIP_GEOMETRY_H
#define VIP_GEOMETRY_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <math.h>
#include <vip/volume.h>
#include <vip/util.h>
#include <vip/geometry/geometry_gen.h>

#define MEAN_CURVATURE 17
#define GAUSSIAN_CURVATURE 18
#define MEAN_LVV 19
#define GAUSSIAN_LVV 20
#define PRINCIPAL_CURVATURE_1 22
#define PRINCIPAL_CURVATURE_2 23
#define LAPLACIAN 24
#define NORMALIZED_LAPLACIAN 25

#ifdef __cplusplus
  }
#endif

#endif /* VIP_GEOMETRY_H */
