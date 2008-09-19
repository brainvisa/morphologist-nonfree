/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/vfilter.h        * TYPE     : Header
 * AUTHOR      : POUPON F.            * CREATION : 30/10/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Application d'un V-Filtre sur un volume - declarations
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

#ifndef VIP_VFILTER_H
#define VIP_VFILTER_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <vip/volume.h>
#include <vip/alloc.h>

#define OPTIMIZED_VFILTER     100
#define NON_OPTIMIZED_VFILTER 101

#define MAXVAR (double)(-1L)

/*---------------------------------------------------------------------------*/
extern Volume *VipVFilter(
   Volume *vol,
   int size
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Volume *VipNonOptimizedVFilter(
   Volume *vol,
   int size
);
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif
