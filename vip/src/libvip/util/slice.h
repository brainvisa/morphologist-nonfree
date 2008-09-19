/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/slice.h          * TYPE     : Header
 * AUTHOR      : POUPON F.            * CREATION : 12/02/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Extraction d'une coupe dans un volume
 *
 ****************************************************************************
 *
 * USED MODULES : vip/volume.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_SLICE_H
#define VIP_SLICE_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <vip/volume.h>

#define VIP_AXIAL_SLICE    100
#define VIP_CORONAL_SLICE  101
#define VIP_SAGITTAL_SLICE 102

/*----------------------------------------------------------------------------*/
extern Volume *VipGetAxialSlice(
  Volume *vol,
  int     slice
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume *VipGetCoronalSlice(
  Volume *vol,
  int     slice
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume *VipGetSagittalSlice(
  Volume *vol,
  int     slice
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern Volume *VipGetSlice(
  Volume *vol,
  int     slice,
  int     slice_type
);
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif
