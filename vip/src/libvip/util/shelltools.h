/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME :                      * TYPE     : Header
 * AUTHOR      :                      * CREATION : 
 * VERSION     :                      * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 ****************************************************************************
 *
 * DESCRIPTION : 
 *
 ****************************************************************************
 *
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_UTIL_SHELLTOOLS_H
#define VIP_UTIL_SHELLTOOLS_H

#include <vip/util/strlist.h>

#ifdef __cplusplus
extern "C"
{
#endif

  enum VipCopyFlags
    {
      VipNone = 0, 
      VipRecursive = 1, 
      VipPreserve = 2, 
      VipSymlinks = 4, 
      VipArchive = VipRecursive | VipPreserve | VipSymlinks, 
    };

  /* shell-like filenames expansion */
  VipStringList* VipShellExpand( const char* pattern );
  VipStringList* VipListDirectory( const char* path );
  void VipCp( const char* srcpatt, const char* dst, int flags );
  void VipMv( const char* srcpatt, const char* dst );
  void VipRm( const char* pattern, int flags );

#ifdef __cplusplus
}
#endif

#endif

