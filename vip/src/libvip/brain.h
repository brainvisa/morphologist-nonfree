/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_brain.h          * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 18/06/1997
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
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



#ifndef VIP_BRAIN_H
#define VIP_BRAIN_H

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
#include <vip/volume.h>
#include <vip/histo.h>
#include <vip/morpho.h>
#include <vip/talairach.h>


#define VOID_LABEL 10
#define GRAY_LABEL 100
#define BRAIN_LABEL 150
#define WHITE_LABEL 200
#define FAT_LABEL 220


/*---------------------------------------------------------------------------*/
int VipDilateInPartialVolume(Volume *vol, Volume *mask); 
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipDilateInPartialVolumeFar(Volume *vol, Volume *mask, int layer); 
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
VipHisto *VipCreateHistogram(Volume *vol, Volume *mask, Volume *edges, int connectivity);
// int VipCreateHistogram(Volume *vol, Volume *mask, Volume *edges, int connectivity);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipDilateVolumeBorder(Volume *vol, Volume *mask, int T_GRAY_CSF, int T_WHITE_FAT, int connectivity);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipVolumeEdges(Volume *edges, Vip_S16BIT *edges_ptr, int seuil);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipCreateBrainSeed(
Volume *vol,
Volume *var,
VipT1HistoAnalysis *ana,
int T_GRAY_WHITE,
int T_WHITE_FAT,
int SEUIL_VAR
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipPropagationConnectivity(
Volume *mask,
Volume *vol1,
Volume *vol2,
int mode,
int seuil_vol1,
int seuil_vol2,
int connectivity
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipPropagationWithRefluxConnectivity(
Volume *mask,
Volume *vol,
Volume *variance,
int distance,
int mode,
int seuil1,
int seuil2,
int connectivity
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipPropagationReflux(
Volume *mask,
Volume *vol1,
Volume *vol2,
int nb_iterations,
int mode,
int seuil_vol1,
int seuil_vol2,
int connectivity
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipGetBrain2010(
Volume *vol,
Volume *variance,
Volume *edges,
Volume *ridge,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float brain_erosion_size,
int variance_threshold,
int nb_iterations,
float CA[3],
float CP[3],
float P[3]
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipGetBrain2005(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float opening_size,
float max_cortical_thickness,
float white_erosion_size,
float white_recovering_size,
int nb_iterations,
Volume *ridge
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipPutOneSliceTwoZero(Volume *vol, int z);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipGetBrain(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
int VipGetBrainStandard(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float brain_erosion_size,
float brain_recovering_size,
int nb_iterations,
int iterative_erosion
);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int VipGetBrainCustomized(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float brain_erosion_size,
float brain_recovering_size,
float white_erosion_size,
float white_recovering_size,
int nb_iterations,
VipTalairach *tal,
int iterative_erosion
);
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
extern int VipGetBrainForRenderingDouble(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug
);
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
int VipFillWhiteCavities(Volume *vol, Volume *brainmask, VipT1HistoAnalysis *ana, int maxsize);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int VipGetBrainForRenderingFast(
Volume *vol,
int dumb,
int debug,
int Tlow,
int Thigh
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipGetBrainForRenderingFastCustomized(
Volume *vol,
int dumb,
int debug,
float brain_erosion_size,
float brain_dilation_size,
int Tlow,
int Thigh
);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int VipGetBrainForRenderingDoubleCustomized(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float white_erosion_size,
int white_recovering_size,
float brain_erosion_size,
int brain_recovering_size,
int nb_iterations
);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipGetBrainForRenderingSimple(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug
);

/*---------------------------------------------------------------------------*/
extern int VipGetBrainForRenderingSimpleCustomized(
Volume *vol,
VipT1HistoAnalysis *ana,
int dumb,
int debug,
float brain_erosion_size,
int brain_recovering_size,
int nb_iterations
);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern Volume *VipGrayWhiteClassificationRegularisation(Volume *vol,VipT1HistoAnalysis *ana,
int dumb);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern Volume *VipGrayWhiteClassificationRegularisationForVoxelBasedAna(Volume *vol,VipT1HistoAnalysis *ana,
int dumb, int nb_iterations, float KPOTTS, int connectivity);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern Volume *VipCSFBrainFatClassificationRegularisation(Volume *vol,VipT1HistoAnalysis *ana,
int dumb, int nb_iterations);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
Volume *VipCSFGrayWhiteFatClassificationRegularisationForRobustApproach(Volume *vol,VipT1HistoAnalysis *ana,
int dumb, int nb_iterations,int T_VOID_GRAY_LOW, int T_VOID_GRAY_HIGH,
int T_WHITE_FAT_LOW, int T_WHITE_FAT_HIGH, int T_GRAY_WHITE);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Volume *VipCSFGrayWhiteFatClassificationRegularisation(Volume *vol,VipT1HistoAnalysis *ana,
int dumb, int nb_iterations);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
Volume *VipGrayWhiteClassificationRegularisationForRobust(Volume *vol,VipT1HistoAnalysis *ana,
							  int dumb);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
Volume *VipGrayWhiteClassificationRegularisationForRobust2005(Volume *vol,VipT1HistoAnalysis *ana,
                                                              int T_VOID_GRAY_LOW, int T_VOID_GRAY_HIGH,
                                                              int T_GRAY_WHITE_LOW, int T_GRAY_WHITE_HIGH,
                                                              int T_WHITE_FAT, int dumb);
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* VIP_BRAIN_H */


