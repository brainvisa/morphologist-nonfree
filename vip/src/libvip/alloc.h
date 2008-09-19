/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/alloc.h          * TYPE     : Header
 * AUTHOR      : POUPON F.            * CREATION : 22/11/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Definitions des routines d'allocation 1D, 2D, 3D et 4D
 *               fpoupon@cea.fr
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

#ifndef VIP_ALLOC_H
#define VIP_ALLOC_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include <vip/util.h>
#include <vip/alloc/alloc_gen.h>

#define mVipVectorSize(vec)    ((vec)->size)
#define mVipMatrixSizeX(mat)   ((mat)->xsize)
#define mVipMatrixSizeY(mat)   ((mat)->ysize)
#define mVipTable3DSizeX(tab3) ((tab3)->xsize)
#define mVipTable3DSizeY(tab3) ((tab3)->ysize)
#define mVipTable3DSizeZ(tab3) ((tab3)->zsize)
#define mVipTable4DSizeX(tab4) ((tab4)->xsize)
#define mVipTable4DSizeY(tab4) ((tab4)->ysize)
#define mVipTable4DSizeZ(tab4) ((tab4)->zsize)
#define mVipTable4DSizeT(tab4) ((tab4)->tsize)

/*---------------------------------------------------------------------------*/
extern void *VipMalloc(
  size_t  size,
  char   *message
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern void *VipCalloc(
  size_t  nobj,
  size_t  size,
  char   *message
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern void *VipRealloc(
  void   *p,
  size_t  size,
  char   *message
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipFree(
  void *ptr
);
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif
