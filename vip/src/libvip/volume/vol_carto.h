/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/struct_volume.h  * TYPE     : Header
 * AUTHOR      : RIVIERE D.           * CREATION : 05/12/2004
 * VERSION     : 2.6                  * REVISION :
 * LANGUAGE    : C/C++                * EXAMPLE  :
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

#ifndef VIP_VOLUME_CARTO_H
#define VIP_VOLUME_CARTO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <vip/volume/structure.h>
#include <vip/volume/struct_volume.h>

#ifdef VIP_CARTO_VOLUME_WRAPPING

  extern void VipVolumeCartoAllocStruct( Volume *volume );
  /** returns OK if there was a carto::Volume in the provate structure and NO 
      if the regular Vip function has to do the deallocation */
  extern int VipVolumeCartoFreeStruct( Volume *volume );
  extern int VipVolumeCartoFreeData( Volume *volume );
  extern int VipVolumeCartoAllocData( Volume* volume );
  extern void VipVolumeCartoCopyStruct( Volume *volumeR, Volume *volumeW );
  extern int VipVolumeCartoTransferVolumeData( Volume *volumeR, 
                                               Volume *volumeW );
  extern Volume* VipVolumeCartoRead( const char* ima_name, int borderWidth, 
                                     int datatype, int frame );
  extern int VipVolumeCartoWrite( Volume* volume, const char* ima_name, 
                                  int format);
  extern int VipTestCartoImageFileExist( const char *ima_name );
  extern int VipVolumeCartoResizeBorder( Volume* volume, int borderWidth );
  // extern int VipTransferCartoVolumeData( Volume* volumeR, Volume* volumeW );

#endif

#ifdef __cplusplus
}
#endif

#endif /* VIP_VOLUME */

