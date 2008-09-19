/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/distmap_static.h     * TYPE     : Header
 * AUTHOR      : MANGIN J.F.              * CREATION : 05/10/1996
 * VERSION     : 0.1                      * REVISION :
 * LANGUAGE    : C                        * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Informations privees du package distmap
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


#ifndef VIP_DISTMAP_STATIC_H
#define VIP_DISTMAP_STATIC_H

#define CHAMFER_DOMAIN 32500
#define CHAMFER_MASK_MAXSIZE 20
#define VIP_OUTSIDE_DOMAIN 32501


/*Warning: do not change these values, they are used for optimization purpose*/
#define SWEEPING_FORWARD 1
#define SWEEPING_BACKWARD 0

#define LONG_BUCKET 50000 /*initial length of a bucket*/
#define INCREMENT_BUCKET 200000 /*increment length of a bucket*/
/* since during front propagation we use a thick front, the maximum
number of bucket is limited */



extern float VIP_USED_DISTMAP_MULTFACT;

typedef struct distmap_mask_point {
		int x;
		int y;
		int z;
		int offset;
		int dist;
			   } DistmapMask_point;

typedef struct distmap_mask {
                int xcubesize;
                int ycubesize;
                int zcubesize;
  		int length;
		DistmapMask_point *first_point;
		     } DistmapMask;


/*-----------------------------------------------------*/
extern int CreateAndSplitCubicMask (
   Volume *vol,
   int xsize,
   int ysize,
   int zsize,
   DistmapMask **forward,
   DistmapMask **backward
);
/*-----------------------------------------------------*/

/*----------------------------------------------------------------*/
extern void FreeMask (
   DistmapMask *m
);
/*----------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
extern int ForwardSweepingWithBorder (
   Volume *vol,
   DistmapMask *mask,
   int borderlevel
);
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
extern int BackwardSweepingWithBorder (
   Volume *vol,
   DistmapMask *mask,
   int borderlevel
);
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
extern int ForwardSweepingVoronoiWithBorder (
   Volume *vol,
   Volume *label,
   DistmapMask *mask,
   int borderlevel
);
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
extern int BackwardSweepingVoronoiWithBorder (
   Volume *vol,
   Volume *label,
   DistmapMask *mask,
   int borderlevel
);
/*--------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
extern void VolumeChamferPreparation(
Volume *vol
);
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
extern void VolumeChamferPreparationGeodesic(
Volume *vol,
int domain,
int outside_domain
);
/*-------------------------------------------------------------------------*/

/*-----------------------------------------------------*/
extern DistmapMask *CreateSquareMask (
   Volume *vol,
   int xsize,
   int ysize,
   int zsize
);
/*-----------------------------------------------------*/

/*--------------------------------------------------------------------*/
extern int FrontPropagation (
   Volume *vol,
   DistmapMask *mask,
   int limit
);
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
extern int FrontPropagationVoronoi (
   Volume *vol,
   Volume *label,
   DistmapMask *mask
);
/*--------------------------------------------------------------------*/

/*-----------------------------------------------------*/
extern DistmapMask *CreateConnectivityMask (
   Volume *vol,
   int connectivity
);
/*-----------------------------------------------------*/

/*--------------------------------------------------------------------*/
int FrontPropagationConnectivity (
   Volume *vol,
   DistmapMask *mask,
   int limit
);
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
extern int FrontPropagationConnectivityVoronoi (
   Volume *vol,
   Volume *label,
   DistmapMask *mask
);
/*--------------------------------------------------------------------*/

#endif /* VIP_DISTMAP_STATIC_H */
