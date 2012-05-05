/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/volume.h         * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 16/01/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Entrees sorties tivoli + fonction d'acces aux champs tivoli
 *               Creation de 'vip/volume.h' fortement inspire des 
 *               entrees/sorties de la librairie Tivoli du département image 
 *               de Telecom Paris de maniere a pouvoir utiliser le meme 
 *               format d'images en memoire. Le but escompte est la 
 *               possibilite de link simultane avec Tivoli (ENST) et 
 *               Vip (Volumic Image Processing SHFJ).
 *               Les principales differences resident dans la gestion du 
 *               format du SHFJ dedie aux images fonctionnelles, la gestion
 *               des series dynamiques (images 4D) et la mise en oeuvre de
 *               procedures generiques (plutot que des switch comme dans 
 *               Tivoli) dans le cas ou la gestion du type des images est
 *               necessaire. Cette derniere difference se manifeste par un 
 *               split du fichier volume.c initial en un fichier de 
 *               procedure "type independant" et un fichier generique 
 *               volume.gen .
 *
 *               En outre, de maniere a reduire les probabilites de conflits
 *               entre les deux librairies, les diverses fonctions destinees
 *               a la gestion de cette structure de donnee ont ete dotees du
 *               prefixe "Vip" (ainsi que generalement d'une majuscule
 *               supplementaire), les alias des divers types de donnees ont
 *               ete dotes du prefixe "vip/"(le suffixe _t a ete supprime).
 *	
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


#ifndef VIP_VOLUME_H
#define VIP_VOLUME_H

#ifdef __cplusplus
  extern "C" {
#endif

extern int VIP_EXIT_IN_PRINTFEXIT;

typedef struct {
	long oFirstPoint;
        long oLastPoint;
	long oLine;
	long oPointBetweenLine;
	long oSlice;
	long oLineBetweenSlice;
	long oVolume;
	long oSliceBetweenVolume;
} VipOffsetStruct;

#include <vip/volume/structure.h>
#include <vip/util.h>
#include <vip/vida.h>
#include <vip/volume/volume_gen.h>
#include <vip/volume/subvolume_gen.h>


/* Format d'images geres*/

#define ANY_FORMAT 99
#define VIDA 100 /*analyse format with huge modifications*/
#define TIVOLI 101
#define SPM 102 /*analyse format with some modifications*/
#define MINC 103 /*BIC Montreal Neurological Institute*/

/* used in numerous procedures */
#define SAME_VOLUME 0
#define NEW_VOLUME 1

    /*  flag for (Volume*)->state  */

#define STATE_DECLARED	1	/* structure declared */
#define STATE_ALLOCATED	2	/* memory allocated for data */
#define STATE_FILLED	3	/* significant data */
#define STATE_FREED	4	/* structure freed */	


/* macro for volume declaration: */

#define VIP_DEC_VOLUME(aVolumePtr) Volume *aVolumePtr = NULL

/* this macro should be used to assure that Volume pointers
are set to NULL when they are not allocated*/


extern int VipMemoryMapping; /*this global flag decide if the VipReadVolume
Procedures are doing Memory mapping*/


  /* Seuls les donnees temporaires chargees en memoires sont mises */
  /* en memory mapping                                             */
  /* L'argument qui verouille cet aspect est essentiellement le    */
  /* Mecanisme avec Border Width.                                  */
  /* Il faudrai cependant etudier un mapping persistent pour les   */
  /* avec Border Width null et Format FX_SHORT_XX (vida) ou TIVOLI */

typedef struct swapfileNode {
	struct swapfileNode *next;
	const char *swapfile;
} swapfileNode;


/*----------------------------------------------------------------------------*/
extern Volume *VipAllocVolumeStructure();
  /*----------------------------------------------------------------------------*/


/*==============================================*
 |                                              |
 |testing functions (return 0 (PB) on failure)  |
 |                                              |
 *==============================================*/
 



/*----------------------------------------------------------------------------*/
	extern
	int			VipTestExist
/*
	test if volume contains significant information (caracteristics and/or
	data) ; this function is called by the following ones
*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipTestExistN
/*
	idem than VipTestExist for a list of volumes
*/
/*----------------------------------------------------------------------------*/
(
	int			n, ...
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipTestType
/*
	test if the volume type is the one specified
*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			type
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipTestTypeN
/*
	test if the volume type is one of the list of n
*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			n, ...
);




/*----------------------------------------------------------------------------*/
	extern
	int			VipTestEqual4DSize
/*
	test if both volumes have the same dimensions (size.x .y .z and .t)
*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume1,
	Volume	    *volume2
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipTestEqual3DSize
/*
	test if both volumes have the same dimensions (size.x .y and .z)
*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume1,
	Volume	    *volume2
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipTestEqual2DSize
/*
	test if both volumes have the same dimensions (size.x and .y)
*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume1,
	Volume	    *volume2
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipTestEqualType
/*
	test if both volumes have the same type
*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume1,
	Volume	    *volume2
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipTestEqualBorderWidth
/*
	test if both volumes have the same border width
*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume1,
	Volume	    *volume2
);


/*----------------------------------------------------------------------------*/
extern	char		    *VipTypeName
/* provide string with type name*/
/*----------------------------------------------------------------------------*/
(
	int			type
);



/*----------------------------------------------------------------------------*/
	extern
	int			VipVerifyAll
/*
	verify that the volume contains correct information 
*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipTestState
/* check if <volume> is in state <state>*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			state
);

/*----------------------------------------------------------------------------*/
extern	int			VipTestStateN
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			n, ...
);


/*----------------------------------------------------------------------------*/
	extern int			VipVerify2Dformat
/* check is volume is a unsigned 8bit 2D image */
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*=====================*
 |                     |
 |  getting functions  |
 |                     |
 *=====================*/

/*Macros to access volume fields*/

#define mVipVolName(vol) (vol)->name
#define mVipVolSizeX(vol) (vol)->size.x
#define mVipVolSizeY(vol) (vol)->size.y
#define mVipVolSizeZ(vol) (vol)->size.z
#define mVipVolSizeT(vol) (vol)->size.t
#define mVipVolType(vol) (vol)->type
#define mVipVolVoxSizeX(vol) (vol)->voxelSize.x
#define mVipVolVoxSizeY(vol) (vol)->voxelSize.y
#define mVipVolVoxSizeZ(vol) (vol)->voxelSize.z
#define mVipVolVoxSizeT(vol) (vol)->voxelSize.t
#define mVipVolBorderWidth(vol) (vol)->borderWidth
#define mVipVolShfjUnite(vol) (vol)->shfj->unite
#define mVipVolShfjVmdType(vol) (vol)->shfj->VmdType
#define mVipVolShfjScale(vol) (vol)->shfj->scale
#define mVipVolShfjOffset(vol) (vol)->shfj->offset

/*----------------------------------------------------------------------------*/
	
extern 	char			*VipGetVolumeName

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipGet3DSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int		    *sizeX,
	int		    *sizeY,
	int		    *sizeZ
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipGet4DSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int		    *sizeX,
	int		    *sizeY,
	int		    *sizeZ,
	int		    *sizeT
);

/*----------------------------------------------------------------------------*/
	extern
long        VipGet3DSizeInVoxels

/*----------------------------------------------------------------------------*/
(
		                 Volume * volume
);

/*----------------------------------------------------------------------------*/
	extern 
long        VipGet3DSizeInBytes

/*----------------------------------------------------------------------------*/
(
		                 Volume * volume
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipSizexOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*----------------------------------------------------------------------------*/
	extern	
	int			VipSizeyOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*----------------------------------------------------------------------------*/
	extern	
	int			VipSizezOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);

/*----------------------------------------------------------------------------*/
	extern	
	int			VipSizetOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipTypeOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipGet3DVoxelSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float	    *voxelSizeX,
	float	    *voxelSizeY,
	float	    *voxelSizeZ
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipGet4DVoxelSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float	    *voxelSizeX,
	float	    *voxelSizeY,
	float	    *voxelSizeZ,
	float	    *voxelSizeT
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipBorderWidthOf

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);





/*=====================*
 |                     |
 |  setting functions  |
 |                     |
 *=====================*/


  extern
  int			VipSetShfjUnite

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			unite
);

/*----------------------------------------------------------------------------*/

  extern
  int			VipSetShfjVmdType

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			vmdtype
);

/*----------------------------------------------------------------------------*/

  extern
  int			VipSetShfjScale

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float			scale
);

/*----------------------------------------------------------------------------*/

  extern
  int			VipSetShfjOffset

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float			offset
);

/*----------------------------------------------------------------------------*/

  extern
  int			VipShfjUnite

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);

/*----------------------------------------------------------------------------*/

  extern
  int			VipShfjVmdType

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);

/*----------------------------------------------------------------------------*/


  extern
  float			VipShfjScale

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);

/*----------------------------------------------------------------------------*/

  extern
  float			VipShfjOffset

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);

/*----------------------------------------------------------------------------*/


  extern
  int                 VipShfjDurationTime

/*----------------------------------------------------------------------------*/
(
        Volume      *volume, int frameNumber
);

/*----------------------------------------------------------------------------*/

  extern
  int                 VipShfjStartTime

/*----------------------------------------------------------------------------*/
(
        Volume      *volume, int frameNumber
);

/*----------------------------------------------------------------------------*/


	extern
	int			VipSetState

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			state
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipSetVolumeName

/*----------------------------------------------------------------------------*/
(
	Volume		*volume,
	char		*name
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipSet3DSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nx,
	int			ny,
	int			nz
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipSet4DSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nx,
	int			ny,
	int			nz,
	int 			nt
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipSetSizeX

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nx
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipSetSizeY

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			ny
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipSetSizeZ

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nz
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipSetSizeT

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			nt
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipSet3DVoxelSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float		dx,
	float		dy,
	float		dz
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipSet4DVoxelSize

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	float		dx,
	float		dy,
	float		dz,
	float 	dt
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipSetType

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			type
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipSetBorderWidth

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			borderWidth
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipResizeBorder

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	int			borderWidth
);







/*====================*
 |                    |
 |  offset functions  |
 |                    |
 *====================*/


/*----------------------------------------------------------------------------*/
	extern
	VipOffsetStruct *VipGetOffsetStructure

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*----------------------------------------------------------------------------*/
	extern
	long			VipOffsetFirstPoint

/*offset to first image point from data ptr*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);

/*----------------------------------------------------------------------------*/
	extern
	long			VipOffsetLastPoint

/*offset to last image point from data ptr*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*----------------------------------------------------------------------------*/
	extern
	long			VipOffsetLine

/*offset to next line*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*----------------------------------------------------------------------------*/
	extern
	long			VipOffsetPointBetweenLine

/*offset from last image point of one line to first image point of next line*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*----------------------------------------------------------------------------*/
	extern
	long			VipOffsetSlice

/* offset to next slice */
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*----------------------------------------------------------------------------*/
	extern
	long			VipOffsetLineBetweenSlice

/*offset from last image point of a slice to first image point of next slice*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*----------------------------------------------------------------------------*/
	extern
	long			VipOffsetVolume

/*offset to next volume */
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*----------------------------------------------------------------------------*/

long			VipOffsetSliceBetweenVolume

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipSizeofType

/* return byte size of volume type */
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);



/**********************************/
/*Procedures for volume management*/
/**********************************/

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipDeclare3DVolumeStructure
	
/*----------------------------------------------------------------------------*/
(
	int			xSize,
	int			ySize,
	int			zSize,
	float		xVoxelSize,
	float		yVoxelSize,
	float		zVoxelSize,
	int			type,
	char    *name,
	int			borderWidth
);

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipDeclare4DVolumeStructure
	
/*----------------------------------------------------------------------------*/
(
	int			xSize,
	int			ySize,
	int			zSize,
	int			tSize,
	float		xVoxelSize,
	float		yVoxelSize,
	float		zVoxelSize,
	float		tVoxelSize,
	int			type,
	char    *name,
	int			borderWidth
);

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipDuplicateVolumeStructure

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	char    *name
);


/*----------------------------------------------------------------------------*/
        extern
        void      *VipCopyVolumeHeader

/*----------------------------------------------------------------------------*/
(
        Volume      *sourceVolume,
        Volume      *destVolume
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipAllocateVolumeData
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);





/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipCreate3DVolume
	
/*----------------------------------------------------------------------------*/
(
	int			xSize,
	int			ySize,
	int			zSize,
	float		xVoxelSize,
	float		yVoxelSize,
	float		zVoxelSize,
	int			type,
	char    *name,
	int			borderWidth
);

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipCreate4DVolume
	
/*----------------------------------------------------------------------------*/
(
	int			xSize,
	int			ySize,
	int			zSize,
	int			tSize,
	float		xVoxelSize,
	float		yVoxelSize,
	float		zVoxelSize,
	float		tVoxelSize,
	int			type,
	char    *name,
	int			borderWidth
);

/*----------------------------------------------------------------------------*/

	int			VipTransferVolumeData 
/*
VipTransferVolumeData ( fromVolume, toVolume );
Copies the contents of 'fromVolume' to 'toVolume' (when
enough place is present).  It does not copy the borders.
*/
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volumeR,
	Volume	    *volumeW
);

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipCopyVolume
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	char    *name
);



/*----------------------------------------------------------------------------*/
	
	int			VipAddBorder
	
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);




/*----------------------------------------------------------------------------*/
	extern
	int			VipFreeVolume

/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);


/*----------------------------------------------------------------------------*/
        extern
	int			VipFreeVolumeData
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume
);

/*----------------------------------------------------------------------------*/
/*==========================*
 |                          |
 |  input-output functions  |
 |                          |
 *==========================*/


/*----------------------------------------------------------------------------*/
	int	    VipTestImageFileExist				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name
);
/*----------------------------------------------------------------------------*/

extern	int	    VipGetImageFormat

/*give image format: VIDA, TIVOLI or PB*/				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name
);


/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipReadVolume

/* Read a volume from disk with 0 borderwidth */
/* if VIDA format: the volume is systematically return rescaled in S16BIT,
To get perfect rescaling in FLOAT, use VipReadVidaFloatVolume */	
/* if TIVOLI format, the volume is read according to Tivoli image type*/
/*----------------------------------------------------------------------------*/
(
	const char    *name
);

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipReadVolumeWithBorder

/* Read a volume from disk with positive or null borderwidth */
/* if VIDA format: the volume is systematically return rescaled in S16BIT,
To get perfect rescaling in FLOAT, use VipReadVidaFloatVolume */	
/* if TIVOLI format, the volume is read according to Tivoli image type*/
/*----------------------------------------------------------------------------*/
(
	const char    *name,
	int		borderWidth
);


/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipReadTivoliVolume

/*switch toward Tivoli I/O*/			
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name
);

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipReadTivoliVolumeWithBorder

/*switch toward Tivoli I/O*/			
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name,
	int			borderWidth
);

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipReadSPMVolume

/*switch toward SPM I/O*/			
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name
);

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipReadSPMVolumeWithBorder

/*switch toward SPM I/O*/			
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name,
	int			borderWidth
);

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipReadVidaVolume
/* the volume is systematically return rescaled in S16BIT,
To get perfect rescaling in FLOAT, use VipReadVidaFloatVolume */				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name
);

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipReadVidaVolumeWithBorder
/* the volume is systematically return rescaled in S16BIT,
To get perfect rescaling in FLOAT, use VipReadVidaFloatVolume */				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name,
	int			borderWidth
);


/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipReadVidaFloatVolumeWithBorder
/* the volume is return in Float Perfectly scaled */				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name,
	int			borderWidth
);

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipReadVidaFloatVolume
/* the volume is return in Float Perfectly scaled */				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name
);

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipReadVidaFloatFrame
/* the volume is return in Float Perfectly scaled */				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name,
        int     fr1,
        int     fr2
);

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipReadVidaFrame
/* the volume is systematically return rescaled in S16BIT,
To get perfect rescaling in FLOAT, use VipReadVidaFloatFrame */				
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name,
        int     fr1,
        int     fr2
);

/*----------------------------------------------------------------------------*/
	extern
	Volume	    *VipReadVidaHeader
/* return header of vida image */
/*----------------------------------------------------------------------------*/
(
	const char    *ima_name
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipWriteVolume
/* The volume is systematically writen in VIDA without scale factors*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	const char    *ima_name
);


/*----------------------------------------------------------------------------*/
	extern
	int			VipWriteVidaScaledVolume
/* The volume is writen in VIDA format with optimal scale factors*/
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	const char    *ima_name
);

/*----------------------------------------------------------------------------*/
	extern
	int			VipWriteTivoliVolume
/* The volume is writen in Tivoli format */
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	const char    *ima_name
);

/*----------------------------------------------------------------------------*/
int VipWriteSPMVolume
/*----------------------------------------------------------------------------*/
(
	Volume	    *volume,
	const char    *ima_name
);
/*----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*/
        extern
        int VipGetRTConfigLong
(
		const char *name,
		long *value
);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
	extern
	char* VipGetRTConfigString
(
		const char *name
);
/*----------------------------------------------------------------------------*/



#ifdef __cplusplus
  }
#endif

#endif /* VIP_VOLUME_H */
