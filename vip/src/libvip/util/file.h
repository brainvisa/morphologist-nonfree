/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/fold_arg         * TYPE     : Header
 * AUTHOR      : MANGIN J-F           * CREATION : 02/03/1999
 * VERSION     : 1.4                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 ****************************************************************************
 *
 * DESCRIPTION : recuperation du format de graphe relationnel
 *               attribue sur les plis corticaux  de la these de JF Mangin
 *
 ****************************************************************************
 *
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_UTIL_FILE_H
#define VIP_UTIL_FILE_H

#ifdef __cplusplus
extern "C"
{
#endif


  int VipExists( const char* filename );
  int VipIsDirectory( const char* dirname );
  int VipIsSymlink( const char* path );
  int VipMkdir( const char* dirname );
  char VipFileSeparator();
  void VipBasename( const char* orig, char* dest );
  void VipDirname( const char* orig, char* dest );
  /* transform '\' into '/' */
  void VipUnixFilename( const char* orig, char* dest );
  const char *VipTmpDirectory();
  void VipUnlink( const char* file );
  void VipRename( const char* src, const char* dst );
  /* Concatenates two files: like "cat toadd >> dst" */
  void VipFileCat( const char* toadd, const char* dst );

#ifdef __cplusplus
}
#endif

#endif
