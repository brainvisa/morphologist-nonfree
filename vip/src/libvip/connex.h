/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/connex.h         * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 05/10/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Diverses constantes et procedures liees a la connexite
 *               jfmangin@cea.fr
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



#ifndef VIP_CONNEX_H
#define VIP_CONNEX_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <vip/volume.h>
#include <vip/bucket.h>
#include <vip/connex/3Dconnex_gen.h>

#define CONNECTIVITY_4 4
#define CONNECTIVITY_4_SAGITTAL 40
#define CONNECTIVITY_4_CORONAL  41
#define CONNECTIVITY_6 6
#define CONNECTIVITY_8 8
#define CONNECTIVITY_8_SAGITTAL 80
#define CONNECTIVITY_8_CORONAL  81
#define CONNECTIVITY_18 18
#define CONNECTIVITY_26 26
#define CONNECTIVITY_2x26 124
#define CONNECTIVITY_3x26 342
#define CONNECTIVITY_4x26 728


#define CONNEX_BINARY 30
#define CONNEX_GREYLEVEL 31
#define CONNEX_LABEL 32
#define CONNEX_ORDER 33

#define HYSTE_NUMBER 40
#define HYSTE_PERCENT 41

typedef struct {
	int connectivity;
	int nb_neighbors;
	int *offset;
	Vip3DPoint_S16BIT *xyz_offset;
	} VipConnectivityStruct;
  
/*----------------------------------------------------------------------------*/
int VipTestCoordBucketConnectedToLabel(
  Vip3DBucket_S16BIT *buck,
  Volume *vol,
  VipConnectivityStruct *vcs,
  int label);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int VipConvertStringToConnectivity(
char *thestring
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern VipConnectivityStruct *VipGetConnectivityStruct(
Volume *volume,
int connectivity);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern void VipFreeConnectivityStruct(
  VipConnectivityStruct *vcs
);
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
Vip3DBucket_S16BIT *VipGetFiltered3DConnex(
  Volume *vol,
  int connectivity,
  int how);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
Vip3DBucket_S16BIT *VipGetDualFiltered3DConnex(
  Volume *vol,
  int connectivity,
  int how);
  /* returns the list of connected comp. which are deleted by the corresponding
VipGetFiltered3DConnex*/
  /* if how>0, it means all cc with strictly less than how points will be removed
     if how<0, it means only the -how th will be preserved */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
Vip3DBucket_S16BIT *VipGetHysteresisFiltered3DConnex(
  Volume *vol,
  int connectivity,
  int how,
  float threshold,
  int mode,
  int param);
  /* if how>0, it means all cc with strictly less than how points will be removed
     if how<0, it means only the -how th will be preserved */
/* follows a low thresholding. If mode==HYSTE_NUMBER, all cc with at least
"param" points over threshold will be kept, if mode==HYSTE_PERCENT,
all cc with percent points over threshold will be kept */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
Vip3DBucket_S16BIT *VipGetDualHysteresisFiltered3DConnex(
  Volume *vol,
  int connectivity,
  int how,
  float threshold,
  int mode,
  int param); 
  /* returns the list of connected comp. which are deleted by the corresponding
VipGetHysteresisFiltered3DConnex*/
  /* if how>0, it means all cc with strictly less than how points will be removed
     if how<0, it means only the -how th will be preserved */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int VipHysteresisPercentThresholding(
  Volume *vol,
  int connectivity,
  int how,
  int connex_mode,
  int low_percent,
  int high_percent,
  int mode,
  int param);
/*VipHysteresisThresholding with percent of max*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int VipHysteresisThresholding(
  Volume *vol,
  int connectivity,
  int how,
  int connex_mode,
  float low_threshold,
  float high_threshold,
  int mode,
  int param);
  /* if how>0, it means all cc with strictly less than how points will be removed
     if how<0, it means only the -how th will be preserved */
/*  If mode==HYSTE_NUMBER, all cc with at least
"param" points over high_threshold will be kept, if mode==HYSTE_PERCENT,
all cc with percent points over high_threshold will be kept */  
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int VipConnexVolumeFilter(
  Volume *vol,
  int connectivity,
  int how,
  int mode);
  
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif /* VIP_CONNEX_H */
