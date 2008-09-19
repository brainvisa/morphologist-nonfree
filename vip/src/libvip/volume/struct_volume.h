/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/struct_volume.h  * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 19/01/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : jfmangin@cea.fr
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *            29/07/98| POUPON F.    | Ajout des champs 'scale' et 'offset'
 *                    |              |  et VmdType dans la structure 
 *                    |              |  SHFJ_private_struct
 *            18/11/98| Frouin V.    | Ajout des pointeurs 'start_time' et
 *                    |              | 'dur_time' dans shfj_private-struct
 *****************************************************************************/

#ifndef VIP_STRUCT_VOLUME_H
#define VIP_STRUCT_VOLUME_H

#include <stdio.h>

/* volume private structure */
/* PLEASE, DO AS IF YOU DID NOT KNOW THIS STRUCTURE */
/*This encapsulation principle is crucial, we want to
be able to change it*/
/*Moreover, it is rather strange because it corresponds
to Tivoli volume structure*/

typedef struct shfj_private_struct {
  int unite; /*mode de calibration, cf. vidaio - Rozenn*/
  int VmdType; /* Type de donnee Vida : FX_SHORT_FX, FX_SHORT_FL ou 
		  FX_FLOAT_FL - F.P. */
  float scale; /* facteur d'echelle pour la quantification. Les facteurs sont
		  les memes pour toutes les coupes. cf. vidaio - F.P. */
  float offset; /* decalage pour la quantification. Les offsets sont les meme
		   pour toutes les coupes. cf. vidaio - F.P. */
  int   *start_time; /*Pointeur sur une suite a allouer d'entier 32 bits 
                       contenant le start time des frames des dyn series*/
  int   *dur_time;   /*Pointeur sur une suite a allouer d'entier 32 bits 
                       contenant le dura time des frames des dyn series*/
  int spm_normalized; /*ce flag indique que l'image lue au format SPM etait normalisee
			et a ete flippee en X, Y, Z*/
  struct {	/*origine du repere proportionnel pour format spm */
    /* jusqu'a present c'etait un int, mais on va prevoir un float,
       un jour ils seront peut-etre moins palmes...*/
		float x;						
		float y;
		float z;
  } center;
  int byte_swapping_actif;
} SHFJ_private_struct;

typedef struct enst_private_struct {
  int GIS_value; /*a faire...*/
} ENST_private_struct;

/* 2004/12 Denis
   Modif for wrapping a cartograph/aims volume
*/
/* Comment this macro to disable wrapping */
#define VIP_CARTO_VOLUME_WRAPPING

#ifdef VIP_CARTO_VOLUME_WRAPPING
struct VipVolumeCartoPrivate;
#endif

typedef struct {
	/* generic attributes : */
	long id; /*this value is not used by VIP but perhaps by TIVOLI */
	char name[NAME_MAXLEN+1];
	struct {	/*this substructure is used by TIVOLI when only a sub-image */
		char name[NAME_MAXLEN+1]; /* is read. Then the original volume */
		int x;							/*information is there*/
		int y;
		int z;
		int t;
	} ref; /* reference image */
	/* intrinsic caracteristics */
	int type;
	struct { /*NB: this values do not take into account the volume borderwidth*/
		int x; /* number of voxels per line */
		int y; /* number of lines */
		int z; /* number of slices */
		int t; /* number of images in the sequence */
	} size;
	struct {
		float x; /* in millimeters */
		float y; /* in millimeters */
		float z; /* in millimeters */
		float t; /* in second */
	} voxelSize;
	/* extrinsic caracteristic */
	int borderWidth;
	/* numerical values of the image contents */
	char *data;
	char ***tab;
	/* security flag */
	int state;
	size_t size3Db; /*What's this?*/	
#ifndef VIP_CARTO_VOLUME_WRAPPING
	/* memory mapping */
	char *swapfile;
#endif

        SHFJ_private_struct *shfj;
        ENST_private_struct *enst;

#ifdef VIP_CARTO_VOLUME_WRAPPING
        struct VipVolumeCartoPrivate *carto;
#endif

} Volume;

/*WARNING: I keep the ifdef in volume structure for compatibility
with TIVOLI but it seems rather dangerous. We have to think it over later*/

#endif /*VIP_STRUCT_VOLUME_H*/

