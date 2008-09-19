/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/bucket.h         * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 15/01/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION :
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

#ifndef VIP_BUCKET_H
#define VIP_BUCKET_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <vip/util.h>
#include <vip/volume.h>
#include <vip/alloc.h>

#include <vip/bucket/bucket_struct_gen.h>
#include <vip/bucket/bucket_gen.h>

typedef struct {
   Vip3DPoint_S16BIT p;
   Vip2DPoint_VFLOAT g2D;
               } VipG2DPoint;

typedef struct {
   Vip3DPoint_S16BIT p;
   Vip3DPoint_VFLOAT g3D;
               } VipG3DPoint;

typedef struct vipG3DBucket
{
  VipG3DPoint *data;
  int size; /*allocated size*/
  int n_points; /*filled locations*/
  struct vipG3DBucket *next; /*listes chainees*/
} VipG3DBucket;

typedef struct vipG2DBucket
{
  VipG2DPoint *data;
  int size; /*allocated size*/
  int n_points; /*filled locations*/
  struct vipG2DBucket *next; /*listes chainees*/
} VipG2DBucket;


typedef struct vipintBucket{
                int *data;
                int size; /*allocated size*/
                int n_points; /*filled locations*/
                struct vipintBucket *next;/*  struct vipintBucket *next; listes chainees*/
                           } VipIntBucket;

typedef struct vips16bitptrBucket{
                Vip_S16BIT **data;
                int size; /*allocated size*/
                int n_points; /*filled locations*/
                struct vips16bitptrBucket *next;/*  struct vipintBucket *next; listes chainees*/
                           } VipS16BITPtrBucket;

#include <vip/bucket/bucket_write_gen.h>



/*------------------------------------------------------------------*/
extern VipG2DBucket *VipAllocG2DBucket(int size);
/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
extern int VipDwindleG2DBucket(VipG2DBucket *vec);
/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
extern int VipFreeG2DBucket(VipG2DBucket *vec);
/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
extern int VipFreeG2DBucketList(VipG2DBucket *list);
/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
extern VipG3DBucket *VipAllocG3DBucket(int size);
/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
extern int VipDwindleG3DBucket(VipG3DBucket *vec);
/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
extern int VipFreeG3DBucket(VipG3DBucket *vec);
/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
extern int VipFreeG3DBucketList(VipG3DBucket *list);
/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
extern int VipSaveG3DBucketList(VipG3DBucket *list, char *name);
/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
extern int VipSaveG2DBucketList(VipG2DBucket *list, char *name);
/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
extern VipG2DBucket *VipReadG2DFile(char *name);
/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
extern VipG3DBucket *VipReadG3DFile(char *name);
/*------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipPermuteTwoIntBucket( VipIntBucket **b1, VipIntBucket **b2 );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern VipIntBucket *VipAllocIntBucket(int size);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern VipS16BITPtrBucket *VipAllocS16BITPtrBucket(int size);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern VipIntBucket *VipMallocIntBucket(int size);

/*---------------------------------------------------------------------------*/
extern int VipFreeIntBucketList(VipIntBucket *vec);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipFreeIntBucket(VipIntBucket *vec);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipFreeS16BITPtrBucket(VipS16BITPtrBucket *vec);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipIncreaseIntBucket(VipIntBucket *vec, int increment);
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif
