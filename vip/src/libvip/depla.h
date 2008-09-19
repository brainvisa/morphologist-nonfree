/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/depla.h          * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 15/01/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Diverses constantes et procedures liees aux rotations affines
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



#ifndef VIP_DEPLA_H
#define VIP_DEPLA_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include <vip/util.h>
#include <vip/alloc.h>				
#include <vip/volume.h>
#include <vip/bucket.h>

typedef struct viprotation {
		double xx;
		double xy;
		double xz;
		double yx;
		double yy;
		double yz;
		double zx;
		double zy;
		double zz;
			} VipRotation;

typedef struct deplacement {
		Vip3DPoint_VFLOAT t;
		VipRotation r;
			} VipDeplacement;

typedef struct  {
                int count;
                VipDeplacement *dep;
			} VipDeplacementVector;

#ifndef M_PI /* defini dans <math.h> seulement sur Sun, Borland, etc. */
#define M_PI 3.14159265358979323846
#endif
/*-------------------------------------------------------------------*/
VipDeplacement *VipReadDeplacement(
char *name);
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
VipDeplacementVector *VipReadDeplacementVector(
char *name);
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
int VipWriteDeplacement(
	VipDeplacement *dep,
	char *name);
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
float VipGetRotationAngle(
	VipDeplacement *dep);
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
int VipApplyDeplacement(
	VipDeplacement *d,
	Vip3DPoint_VFLOAT *in,
        Vip3DPoint_VFLOAT *out);
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
int VipApplyRotation(
	VipRotation *r,
	Vip3DPoint_VFLOAT *in,
        Vip3DPoint_VFLOAT *out);
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
VipDeplacement *VipInitialiseDeplacement(
	Vip3DPoint_VFLOAT *translat);
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
int VipDeplaFirstEgalDeplaSecond( 
	VipDeplacement *first,
	VipDeplacement *second);
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
int VipFillDeplacement(
  Vip3DPoint_VFLOAT *tra,
  VipRotation *rot,
  VipDeplacement *dep);
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
int VipFillRotation(
	float xangle,
	float yangle,
	float zangle,
	VipRotation *rot); 
	/*Remplis la structure rotation (qui doit etre allouee prealablement)
	 avec une rotation combinaison de 3 rotations autour des axes*/
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
int VipMultiplyRotations( 
	VipRotation *g,
	VipRotation *d,
	VipRotation *out); /* g=left, d=right ...*/
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
int VipPrintRotation( 
	VipRotation *rot);
/*-------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
int VipInverseDeplacement(
VipDeplacement *dep,
VipDeplacement *invdep);
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
int VipMultDeplacement(
	VipDeplacement *dep,
	VipDeplacement *left,
	VipDeplacement *right);
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
int VipScaleDeplacement(
	VipDeplacement *dep,
	Volume *from,
	Volume *to);
/*----------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif /* VIP_DEPLA_H */






