/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipClusterArg        * TYPE     : Command line
 * AUTHOR      : RIVIERE D.           * CREATION : 12/12/2004
 * VERSION     : 2.6                  * REVISION :
 * LANGUAGE    : C/C++                * EXAMPLE  :
 * DEVICE      : 
 ****************************************************************************
 *
 * DESCRIPTION :  
 *
 ****************************************************************************
 *
 * USED MODULES : 
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_FOLD_ARG_MESH_AIMS_H
#define VIP_FOLD_ARG_MESH_AIMS_H

#include <vip/volume/structure.h>
#include <vip/volume/struct_volume.h>

#ifdef VIP_CARTO_VOLUME_WRAPPING

#ifdef __cplusplus
  extern "C" {
#endif

void meshAims( Volume* vol, const char* outfname );

#ifdef __cplusplus
  }
#endif

#endif
#endif

