/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/nomen.h          * TYPE     : Header
 * AUTHOR      : Frouin V.            * CREATION : 20/09/1999
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC
 *****************************************************************************
 *
 * DESCRIPTION : header for Nomenclature management
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


#ifndef VIP_NOMEN_H
#define VIP_NOMEN_H

#ifdef __cplusplus
  extern "C" {
#endif

/* general includes -------------------------------------------------------  */


/* application includes ---------------------------------------------------  */
#include <stdio.h>
#include <vip/nomen/nomen_def.h>
#include <vip/volume.h>

/* extern declaration   ---------------------------------------------------  */
extern VipNomenList *VipCreateNomenList( );
extern VipNomenList *VipCreateNomenListFromVolume ( Volume *vol );
extern VipNomenList *VipCreateRefNomenList( );
extern int           VipFreeNomenList(  VipNomenList *list);

extern int        VipMergeNomenList( VipNomenList *list, VipNomenList *ref);
extern int        VipPrintNomenList( FILE *fp, VipNomenList *ref);
extern int        VipGetNumberNomenList( VipNomenList *list );
extern int        VipGetLabelNomenList( VipNomenList *list, int number, int *retval);
extern int        VipGetNameNomenList( VipNomenList *list, int number, char *retval);
extern int        VipAddNomenList( VipNomenList *list, int label, char * name);
extern int        VipRemoveNomenListByLabel( VipNomenList *list, int label);
extern char *VipGetNameNomenListByLabel( VipNomenList *list, int label);


#ifdef __cplusplus
  }
#endif

#endif
