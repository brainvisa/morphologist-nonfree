/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/struct_volume.h  * TYPE     : Header
 * AUTHOR      : RIVIERE D.           * CREATION : 05/12/2004
 * VERSION     : 2.6                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : 
 *****************************************************************************
 *
 * DESCRIPTION : jfmangin@cea.fr
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *****************************************************************************/

#ifndef VIP_VOLUME_CARTO_CPP_H
#define VIP_VOLUME_CARTO_CPP_H

#include <vip/volume/structure.h>
#include <vip/volume/struct_volume.h>

#ifdef VIP_CARTO_VOLUME_WRAPPING

#include <aims/data/data.h>

namespace vip
{
  template<typename T>
  extern carto::rc_ptr<carto::Volume<T> > 
  volumeFromVip( ::Volume* volume );
}

#endif
#endif

