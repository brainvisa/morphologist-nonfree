/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/voi.h            * TYPE     : Header
 * AUTHOR      : FROUIN V.            * CREATION : 14/04/1999
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC
 ****************************************************************************
 *
 * DESCRIPTION : Definition  de volume d'interet pour Vip.
 *               Ces notions sont orientees pour les statistiques.
 *
 ****************************************************************************
 *
 * USED MODULES :
 *                
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *            08/09/99|  V. Frouin   | Ajout d'une zone explicite pour la
 *                    |              | gestion des postitions entieres en vue
 *                    |              | du re-echantillonnage LinearResamp
 ****************************************************************************/

#ifndef VIP_VOI_H
#define VIP_VOI_H

#ifdef __cplusplus
  extern "C" {
#endif


#include <stddef.h>
#include <math.h>

#include <vip/volume.h>
#include <vip/util.h>
#include <vip/bucket.h>
#include <vip/depla.h>
#include <vip/nomen.h>



#define VROI_LOADED          0x00000001
#define VROI_RDY_FORARM      0x00000002
#define VROI_ARMED           0x00000004
#define VROI_RDY_FORITERATE  0x00000008
#define VROI_ITERATED        0x00000010
#define VROI_RDY_FORSTAT     0x00000020
#define VROI_STATED          0x00000040

#define VROI_RDY_FORCORR     0x00000080
#define VROI_CORRED          0x00000100

#define VROI_RDY_FOREDIT     0x00000200
#define VROI_EDITED          0x00000400


#define PVEC_CORR  1
#define UN_CORR    2


#define mVipRoiDepIsIdentity(dep) 0


		     
typedef struct
{
   float                      vol;
   float                      mean;
   float                      std;
   float                      min;
   float                      max;
} Statistics;


typedef struct
{
                             /* compatibilite avec les modules i/o des       */
                             /* necessite d'utiliser les signés...           */
  Vip3DBucket_S16BIT         **ListeDePaquetDePoint;
                             /* structure bucket contrepartie des depla-     */
                             /* cements entier pour linearresamp             */
  Vip3DBucket_U16BIT         **ListeDePaquetDeDeplaEntier; 
                             /* offset en terme de pointeurs                 */
  VipIntBucket               **ListeDePaquetDoffset;
                             /* liste de pointeurs sur les mesures           */
  float                      ***ListeDePaquetDeMesure;
                             /* */
                             /* */
                             /* Liste de stats struct 1 par temps et roi     */
                             /* Classement par temps puis roi                */
                             /* L'indice le plus rapide concerne les rois    */
  Statistics                 ***ListeDeStat;
  Statistics                 ***ListeDeStatCorr;
  int                        NbVoi;
  int                        *label;         /* Ersatz du roi-name */
  Volume                     *ReferenceImage;/* Image quantitative */
  Volume                     *VoiImage;      /* Image anatom de def des roi*/
  VipDeplacement             *deplacement;
  int                        Etat;

  Vip3DBucket_S16BIT         **traLDPDP;
  Vip3DBucket_U16BIT         **traLDPDDE;
  VipIntBucket               **traLDPDO;
  Statistics                 ***traLDS, ***traLDSC;
  Statistics                 **t_traLDS, **t_traLDSC;
  float                      ***traLDPDM;
  float                      **t_traLDPDM;
  
} VipVoi;


/*---------------------------------------------------------------------------*/

extern              VipVoi  *VipVoiReadFromVolume_S16BIT(
   Volume                      *image,
   int                         *label,
   int                         nb_label
);
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
extern              int      VipVoiGetFunctionalValueFromVida(
   VipVoi                      *roi,
   VipDeplacement              *dep,
   Volume                      *image
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern              int      VipVoiGetFunctionalValueFromVidaWithMC(
   VipVoi                      *roi,
   VipDeplacementVector        *depVector,
   Volume                      *image
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern               int      VipGetVoiStatus(
   VipVoi                      *roi
);
/*---------------------------------------------------------------------------*/

extern int VipVoiSetDeplacement(VipVoi *voi, VipDeplacement *dep);

/*---------------------------------------------------------------------------*/
extern               int      VipFreeVoi(
   VipVoi                      *roi
);
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
extern              int        VipVoiComputeStat(
   VipVoi                      *roi
);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern              int        VipVoiGTMCorrection(
   VipVoi                     *voi,
   VipMatrix_VDOUBLE           *gtm
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern             int        VipVoiStreamStat(
   VipVoi                      *roi,
   FILE                        *stream,
   VipNomenList                *nomen,
   int                         flag
);
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
extern VipVoi *VipVoiAllocate( Volume *label_vol, int *label, int nb );
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
extern int VipVoiAllocateBucket(
    VipVoi    *voi
    );
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
extern int VipVoiAllocateDepla(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
extern int VipVoiAllocateOffset(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipVoiAllocateStat(
    VipVoi     *voi
    );

/*---------------------------------------------------------------------------*/
extern int VipVoiAllocateStatCorr(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int VipVoiAllocateMes(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipVoiAllocateDeplaData(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipVoiResetDeplaData(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
extern int VipVoiAllocateOffsetData(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipVoiResetOffsetData(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipVoiAllocateStatData(
    VipVoi     *voi
    );

/*---------------------------------------------------------------------------*/
extern int VipVoiAllocateStatCorrData(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int VipVoiAllocateMesData(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Vip3DBucket_S16BIT *VipVoiFirstPtrBucket(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Vip3DBucket_U16BIT *VipVoiFirstPtrDepla(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern VipIntBucket *VipVoiFirstPtrOffset(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Statistics **VipVoiFirstPtrStat(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Statistics **VipVoiFirstPtrStatCorr(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern Statistics *VipVoiFirstPtrStatVector(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern Statistics *VipVoiFirstPtrStatCorrVector(
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern float **VipVoiFirstPtrMes( 
    VipVoi     *voi
    );

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern float *VipVoiFirstPtrMesVector( 
    VipVoi     *voi
    );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern Vip3DBucket_S16BIT *VipVoiNextPtrBucket(
    VipVoi *vo
    );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern Vip3DBucket_U16BIT *VipVoiNextPtrDepla(
    VipVoi *voi
    );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern VipIntBucket *VipVoiNextPtrOffset(
    VipVoi *voi
    );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern Statistics **VipVoiNextPtrStat(
    VipVoi *voi
    );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern Statistics **VipVoiNextPtrStatCorr(
    VipVoi             *voi
    );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern Statistics *VipVoiNextPtrStatVector(
    VipVoi *voi
    );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern Statistics *VipVoiNextPtrStatCorrVector(
    VipVoi *voi
    );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern float **VipVoiNextPtrMes( 
			 VipVoi             *voi
			 );
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern float *VipVoiNextPtrMesVector(
			      VipVoi *voi
			      );
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif
