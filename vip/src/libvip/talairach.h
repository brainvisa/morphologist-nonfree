/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/talairach.h      * TYPE     : Header
 * AUTHOR      : POUPON F.            * CREATION : 07/01/1998
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Genere les fichier de transformation vers Talairach
 *
 ****************************************************************************
 *
 * USED MODULES : string.h - vip/volume.h - vip/talairach.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_TALAIRACH_H
#define VIP_TALAIRACH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include <vip/volume.h>
#include <vip/bucket.h>
#include <vip/math.h>

#define P_COR_0 0
#define P_COR_1 1
#define P_COR_2 2
#define P_COR_3 3
#define P_SAG_0 4
#define P_SAG_1 5
#define P_SAG_2 6
#define P_AXI_0 7
#define P_AXI_1 8
#define P_AXI_2 9
 
#define TALAIRACH_SIZEX 157.
#define TALAIRACH_SIZEY 189.
#define TALAIRACH_SIZEZ 136.

typedef struct
{
  float a;
  float b;
  float c;
  float d;
  int num; /** plane number **/
} VipPlane;

typedef struct
{ 
  Vip3DPoint_VFLOAT AC;   /** AC point coordinates **/
  Vip3DPoint_VFLOAT PC;   /** PC point coordinates **/
  Vip3DPoint_VFLOAT Hemi; /** Inter-hemispheric point coordinates **/

  Vip3DPoint_VFLOAT MinAxial;    /** Minimum axial plane coordinates **/
  Vip3DPoint_VFLOAT MaxAxial;    /** Maximim axial plane coordinates **/
  Vip3DPoint_VFLOAT MinSagittal; /** Minimum sagittal plane coordinates **/
  Vip3DPoint_VFLOAT MaxSagittal; /** Maximum sagittal plane coordinates **/
  Vip3DPoint_VFLOAT MinCoronal;  /** Minimum coronal plane coordinates **/
  Vip3DPoint_VFLOAT MaxCoronal;  /** Maximum coronal plane coordinates **/

  Vip3DPoint_VFLOAT ACPC_vector;  /** AC-PC plane vector **/
  Vip3DPoint_VFLOAT Hemi_vector;  /** Inter-hemispheric plane vector **/
  Vip3DPoint_VFLOAT Cross_vector; /** Third plane vector **/

  Vip3DPoint_VFLOAT Scale; /** Talairach scale factor **/
  float Translation[3];    /** Talairach translation vector **/
  float Rotation[3][3];    /** Talairach rotation matrix **/

  int MinBox[3]; /** Talairach box minimum (X,Y,Z) coordinates **/
  int MaxBox[3]; /** Talairach box maximum (X,Y,Z) coordinates **/

  VipPlane Plane[10]; /** 10 Talairach planes **/

  Vip3DPoint_VFLOAT VoxelGeometry; /** Voxel sizes (in mm) **/
} VipTalairach;

/*---------------------------------------------------------------------------*/
extern int GetCommissureCoordinates(Volume *vol, char *point_filename, VipTalairach *tal,
			     int xCA, int yCA, int zCA, 
			     int xCP, int yCP, int zCP, 
			     int xP, int yP, int zP,
				    int coord_provided);
/*---------------------------------------------------------------------------*/

void VipFillTalairachPlane(
  VipTalairach *tal,
  int num,
  float AA,
  float BB,
  float CC,
  float DD
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void VipInitTalairachPlanes(
  VipTalairach *tal,
  float *minA,
  float *maxA,
  float *minS,
  float *maxS,
  float *minC,
  float *maxC
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void Vip3PlanesIntersection(
  VipTalairach *tal,
  int num1,
  int num2,
  int num3,
  float *ptInter
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void VipTalairachRotation(
  float *pt, 
  VipTalairach *tal,
  float *res
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void VipTalairachInverseRotation(
  float *pt,
  VipTalairach *tal,
  float *res
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void VipTalairachTranslation(
  float *pt, 
  VipTalairach *tal,
  float *res
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void VipTalairachInverseTranslation(
  float *pt,
  VipTalairach *tal,
  float *res
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void VipTalairachTransformation(
  float *pt, 
  VipTalairach *tal,
  float *res
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void VipTalairachInverseTransformation(
  float *pt,
  VipTalairach *tal,
  float *res
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void VipTalairachBox(
  Volume *vol,
  VipTalairach *tal
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void VipTalairachRotationAndTranslation(
  VipTalairach *tal
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void VipComputeDeterminant(
  float *A, 
  float *B, 
  float *C, 
  float *sol
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void Vip3DPlanesResolution(
  float *a, 
  float *b, 
  float *c, 
  float *d, 
  float *x, 
  float *y, 
  float *z
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void VipInitTalairachVectors(
  VipTalairach *tal
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void VipComputeTalairachTransformation(
  Volume *vol,
  VipTalairach *tal
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipWriteTalairachRefFile(
  char *filename,
  VipTalairach *tal
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipWriteTalairachTalFile(
  char *filename,
  VipTalairach *tal
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipWriteTalairachFiles(
  char *filename,
  VipTalairach *tal
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipReadTalairachRefFile(
  char *filename,
  VipTalairach *tal
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipReadTalairachTalFile(
  char *filename,
  VipTalairach *tal
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipReadTalairachFiles(
  char *filename,
  VipTalairach *tal
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void VipPrintTalairach(
  VipTalairach *tal
);
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif
