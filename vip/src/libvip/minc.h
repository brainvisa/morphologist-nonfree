/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/minc.h       * TYPE     : Include
 * AUTHOR      : MANGIN J.F.          * CREATION : 11/2001
 * VERSION     : 2.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : IO MINC
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

#ifndef VIP_MINC_H
#define VIP_MINC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vip/volume.h>
#include <vip/TivoliIO.h>

/*----------------------------------------------------------------------------*/
extern Volume *VipReadMincVolumeWithBorder(char *name,int borderWidth);
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif
