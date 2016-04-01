/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipCSFSeg.c          * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 22/01/2014
 * VERSION     : 1.0                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Laptop
 ****************************************************************************
 *
 * DESCRIPTION : 
 *
 ****************************************************************************
 *
 * USED MODULES : 
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/




/*##############################################################################

        I N C L U S I O N S

##############################################################################*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <vip/volume.h>
#include <vip/util.h>
#include <vip/morpho.h>
#include <vip/brain.h>
#include <vip/histo.h>
#include <vip/connex.h>
#include <vip/deriche.h>
#include <vip/gaussian.h>
#include <vip/geometry.h>
#include <vip/distmap.h>

/*--------------------------------------------------------------------------*/
static int Usage();
static int Help();
/*--------------------------------------------------------------------------*/


int main(int argc, char *argv[])
{
    /*declarations and initializations*/
    VIP_DEC_VOLUME(vol);
    VIP_DEC_VOLUME(mask);
    VIP_DEC_VOLUME(classif);
    VIP_DEC_VOLUME(edges);
    VIP_DEC_VOLUME(hemi);
    VIP_DEC_VOLUME(white);
    VIP_DEC_VOLUME(csf);
    VIP_DEC_VOLUME(outside);
    VIP_DEC_VOLUME(copy);
    VIP_DEC_VOLUME(copy2);
    VIP_DEC_VOLUME(copy3);
    VIP_DEC_VOLUME(copy4);
    VIP_DEC_VOLUME(variance);
    VIP_DEC_VOLUME(deriche);
    VIP_DEC_VOLUME(vent_rh);
    VIP_DEC_VOLUME(vent_lh);
    VIP_DEC_VOLUME(gaussian);
    VIP_DEC_VOLUME(meancurv);
    VIP_DEC_VOLUME(mc);
    VIP_DEC_VOLUME(csfinwhite);
    char *input = NULL;
    char *maskname = NULL;
    char output[VIP_NAME_MAXLEN]="csf";
    VipT1HistoAnalysis *hana = NULL;
    char *hananame = NULL;
    VipHisto *histo;
    char point_filename[VIP_NAME_MAXLEN]="";
    VipOffsetStruct *vos;
    Vip_S16BIT *ptr, *ptr_var;
    int ix, iy, iz;
    int i;
    float little_opening_size;
    float mean=0., sigma=0.;
    float vol_brain=0., vol_vent=0.;
    float ratio;
    VipTalairach tal, *coord=NULL;
    Volume *plan_hemi=NULL;
    float CA[3], CP[3], P[3], d[3];
    float ptPlanHemi[3], pt=0.;
    int readlib, writelib;
    int random_seed = time(NULL);
    
    readlib = ANY_FORMAT;
    writelib = TIVOLI;
    
    /*loop on command line arguments*/
    for (i=1;i<argc;i++)
    {
        if (!strncmp (argv[i], "-input", 2)) 
        {
            if (++i >= argc || !strncmp(argv[i],"-", 1)) return Usage();
            input = argv[i];
        }
        else if (!strncmp (argv[i], "-output", 2)) 
        {
            if (++i >= argc || !strncmp(argv[i],"-", 1)) return Usage();
            strcpy(output,argv[i]);
        }
        else if (!strncmp (argv[i], "-mask", 2)) 
        {
            if (++i >= argc || !strncmp(argv[i],"-", 1)) return Usage();
            maskname = argv[i];
        }
        else if (!strncmp (argv[i], "-hana", 2)) 
        {
            if (++i >= argc || !strncmp(argv[i],"-", 1)) return Usage();
            hananame = argv[i];
        }
        else if (!strncmp (argv[i], "-Points", 2)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          strcpy(point_filename, argv[i]);
        }  
        else if (!strncmp (argv[i], "-readformat", 2)) 
        {
            if(++i >= argc || !strncmp(argv[i],"-", 1)) return(Usage());
            if(argv[i][0]=='t') readlib = TIVOLI;
            else if(argv[i][0]=='v') readlib = VIDA;
            else if(argv[i][0]=='a') readlib = ANY_FORMAT;
            else if(argv[i][0]=='s') readlib = SPM;
            else
            {
                (void)fprintf(stderr,"This format is not implemented for reading\n");
                Usage();
            }
        }
        else if (!strncmp (argv[i], "-writeformat", 2)) 
        {
            if(++i >= argc || !strncmp(argv[i],"-", 1)) return Usage();
            if(argv[i][0]=='t') writelib = TIVOLI;
            else if(argv[i][0]=='v') writelib = VIDA;
            else if(argv[i][0]=='s') writelib = SPM;
            else
            {
                VipPrintfError("This format is not implemented for writing");
                VipPrintfExit("(commandline)VipCSFSeg");
                return(VIP_CL_ERROR);
            }
        }
        else if (!strncmp(argv[i], "-srand", 6))
        {
            if (++i >= argc || !strncmp(argv[i],"-", 1)) return(Usage());
            random_seed = atoi(argv[i]);
        }
        else if (!strncmp(argv[i], "-help", 2)) return Help();
        else return(Usage());
    }
    
    /*check that all required arguments have been given*/
    if (input==NULL) 
    {
        VipPrintfError("input arg is required by VipCSFSeg");
        return(Usage());
    }
    if (VipTestImageFileExist(input)==PB)
    {
        (void)fprintf(stderr,"Can not open this image: %s\n", input);
        return(VIP_CL_ERROR);
    }
    if (maskname==NULL) 
    {
        VipPrintfError("mask arg is required by VipCSFSeg");
        return(Usage());
    }
    if (VipTestImageFileExist(maskname)==PB)
    {
        (void)fprintf(stderr,"Can not open this image: %s\n", maskname);
        return(VIP_CL_ERROR);
    }
   
    srand(random_seed);
    
    /*reading of the inputs*/
    printf("Reading %s...\n", input);
    if (readlib == TIVOLI)
        vol = VipReadTivoliVolumeWithBorder(input, 1);
    else if (readlib == SPM)
        vol = VipReadSPMVolumeWithBorder(input, 1);
    else
        vol = VipReadVolumeWithBorder(input, 1);

    if (vol==NULL) return (VIP_CL_ERROR);
    
    printf("Reading %s...\n", maskname);
    if (readlib == TIVOLI)
        mask = VipReadTivoliVolumeWithBorder(maskname, 1);
    else if (readlib == SPM)
        mask = VipReadSPMVolumeWithBorder(maskname, 1);
    else
        mask = VipReadVolumeWithBorder(maskname, 1);
    
    if (mask==NULL) return (VIP_CL_ERROR);
    
    hana = VipReadT1HistoAnalysis(hananame);
    if (hana==PB)
    {
        VipPrintfError("Can not read this histogram analysis");
        return (VIP_CL_ERROR);
    }
    if (hana->white==NULL || hana->gray==NULL)
    {
        VipPrintfError("Can not use this poor histogram analysis (no gray/white modes)");
        return (VIP_CL_ERROR);
    }
    
    /*execution*/
    
    /*Calcul de la variance. Au niveau des frontières des ventricules, la
      variance est élévé.*/
//     variance = VipComputeVarianceVolume(vol);
//     if (variance==PB) return(VIP_CL_ERROR);
//     copy = VipCopyVolume(mask, "mask_cl");
//     if (VipExtRay(copy, EXTEDGE3D_ALL, SAME_VOLUME)==PB) return(PB);
//     VipSingleThreshold(copy, EQUAL_TO, 0, BINARY_RESULT);
//     if (VipErosion(copy, CHAMFER_BALL_3D, 2.)==PB) return(PB);
//     VipMaskVolume(variance, copy);
//     VipWriteVolume(variance, "variance");
    
    /*Calcul des contours avec un filtre deriche. Au niveau des frontières des
      ventricules, les contours sont élévés.*/
//     deriche = VipCopyVolume(vol, "deriche");
//     if (deriche==PB) return(VIP_CL_ERROR);
//     if (VipDeriche3DGradientNorm(deriche, 2., DERICHE_EXTREMA, 0.)==PB) return(VIP_CL_ERROR);
//     VipMaskVolume(deriche, copy);
//     VipWriteVolume(deriche, "deriche");
//     
//     vos = VipGetOffsetStructure(deriche);
//     ptr = VipGetDataPtr_S16BIT(deriche) + vos->oFirstPoint;
//     ptr_var = VipGetDataPtr_S16BIT(variance) + vos->oFirstPoint;
//     for ( iz=0; iz<mVipVolSizeZ(deriche); iz++ )    /* loop on slices */
//     {
//         for ( iy=0; iy<mVipVolSizeY(deriche); iy++ )    /* loop on lines */
//         {
//             for ( ix=0; ix<mVipVolSizeX(deriche); ix++ )    /* loop on points */
//             {
//                 if ((*ptr)!=0) *ptr *= (*ptr_var)/1000.;
//                 ptr++;
//                 ptr_var++;
//             }
//             ptr = ptr + vos->oPointBetweenLine;    /*skip border points*/
//             ptr_var = ptr_var + vos->oPointBetweenLine;    /*skip border points*/
//         }
//         ptr = ptr + vos->oLineBetweenSlice;    /*skip border lines*/
//         ptr_var = ptr_var + vos->oLineBetweenSlice;    /*skip border lines*/
//     }
//     VipWriteVolume(deriche, "deriche_var");
//     VipFreeVolume(copy);
    
    
    little_opening_size = 0.9;
    if(mVipVolVoxSizeX(mask)>little_opening_size) little_opening_size=mVipVolVoxSizeX(mask)+0.1;
    if(mVipVolVoxSizeY(mask)>little_opening_size) little_opening_size=mVipVolVoxSizeY(mask)+0.1;
    if(mVipVolVoxSizeZ(mask)>little_opening_size) little_opening_size=mVipVolVoxSizeZ(mask)+0.1;
    printf("little_opening_size=%f\n", little_opening_size), fflush(stdout);
    
    /*--Creation of the hemispheric plan--*/
    if(GetCommissureCoordinates(vol, point_filename, &tal,
                                128, 113, 70,
                                128, 141, 70,
                                128, 60, 20, VFALSE)==PB) return(VIP_CL_ERROR);
    coord = &tal;
    CA[0] = (int)(coord->AC.x); CA[1] = (int)(coord->AC.y); CA[2] = (int)(coord->AC.z);
    CP[0] = (int)(coord->PC.x); CP[1] = (int)(coord->PC.y); CP[2] = (int)(coord->PC.z);
    P[0] = (int)(coord->Hemi.x); P[1] = (int)(coord->Hemi.y); P[2] = (int)(coord->Hemi.z);
    
    plan_hemi = VipCreateSingleThresholdedVolume(vol, GREATER_OR_EQUAL_TO, 0, BINARY_RESULT);

    d[0] = d[1] = d[2] = 1000.0;
    Vip3DPlanesResolution(CA, CP, P, d, &(ptPlanHemi[0]), &(ptPlanHemi[1]), &(ptPlanHemi[2]));

    vos = VipGetOffsetStructure( plan_hemi );
    ptr = VipGetDataPtr_S16BIT( plan_hemi ) + vos->oFirstPoint;
    for ( iz=0; iz<mVipVolSizeZ(plan_hemi); iz++ )   /* loop on slices */
    {
        for ( iy=0; iy<mVipVolSizeY(plan_hemi); iy++ )  /* loop on lines */
        {
            for ( ix=0; ix<mVipVolSizeX(plan_hemi); ix++ )   /* loop on points */
            {
                pt = ptPlanHemi[0]*(ix) + ptPlanHemi[1]*(iy) + ptPlanHemi[2]*(iz) - 1000.0;
                if(-25<(int)(pt) && (int)(pt)<25) *ptr = 0;
                ptr++;
            }
            ptr = ptr + vos->oPointBetweenLine;  /*skip border points*/
        }
        ptr = ptr + vos->oLineBetweenSlice; /*skip border lines*/
    }
//     VipInvertBinaryVolume(plan_hemi);
    VipWriteVolume(plan_hemi, "plan_hemi");
    
    
    /*--Fisrt estimation of the CSF--*/
    copy = VipCreateSingleThresholdedVolume(mask, GREATER_THAN, 0, BINARY_RESULT);
    vol_brain = VipGetNumberLabelPoints(copy, 255);
    copy3 = VipCopyVolume(copy, "brain"); //TEST
    if (VipClosing(copy, CHAMFER_BALL_3D, 20.)==PB) return(PB);
    VipWriteVolume(copy, "brain_cl");
    outside = VipCopyVolume(copy, "outside");
    if (VipExtRay(copy3, EXTEDGE2D_ALL_EXCEPT_Y_TOP, SAME_VOLUME)==PB) return(VIP_CL_ERROR); //TEST
    VipWriteVolume(copy3, "brain_ext"); //TEST
    VipFreeVolume(copy3); //TEST
    
    VipMaskVolume(vol, copy);
    classif = VipCSFGrayWhiteFatClassificationRegularisationForRobustApproach(vol, hana, NO, 1, hana->gray->mean-3.5*hana->gray->sigma, hana->gray->mean-2.5*hana->gray->sigma, hana->white->mean+4*hana->white->sigma, hana->white->mean+10*hana->white->sigma, 0.5*hana->gray->mean + 0.5*hana->white->mean);
    VipSingleThreshold(classif, EQUAL_TO, 150, BINARY_RESULT);
    VipWriteVolume(classif, "classif_av");
    
    copy3 = VipCopyVolume(classif, "mask_cl");
    if (VipExtRay(classif, EXTEDGE3D_ALL, SAME_VOLUME)==PB) return(VIP_CL_ERROR);
    VipSingleThreshold(classif, EQUAL_TO, 0, BINARY_RESULT);
    VipMerge(classif, copy3, VIP_MERGE_ONE_TO_ONE, 255, 0); 
//     VipInvertBinaryVolume(classif);
//     VipMaskVolume(classif, copy); 
    VipWriteVolume(classif, "classif");
    
    VipComputeRobustStatInMaskVolume(vol, classif, &mean, &sigma, VFALSE);
    printf("mean: %f, sigma: %f\n", mean, sigma), fflush(stdout);
    csf = VipCreateDoubleThresholdedVolume(vol, VIP_BETWEEN_OR_EQUAL_TO, mVipMax(1, (int)(mean-2.*sigma)), (int)(mean), BINARY_RESULT);
    VipWriteVolume(csf, "csf_av");
    //Test
    copy2 = VipCopyVolume(csf, "copy_csf");
//     VipConnectivityChamferClosing(copy2, 1, CONNECTIVITY_6, FRONT_PROPAGATION);
//     VipChangeIntLabel(copy2, 255, GRAY_LABEL);
//     VipChangeIntLabel(copy2, 0, 101);
    VipRegularisation(copy2, 255);
    VipWriteVolume(copy2, "csf_av_reg2");
    VipFreeVolume(copy2);
    
    
    /*---Ventricles Segmentation---*/
    /*Segmentation de la matiere blanche, les ventricules sont à l'interieur de celle ci.*/
    white = VipGrayWhiteClassificationRegularisationForVoxelBasedAna(vol, hana, VFALSE, 10, 20, CONNECTIVITY_6);
    VipSingleThreshold(white, EQUAL_TO, 200, BINARY_RESULT);

    /*Selecting both hemisphere*/
    printf("Selecting label %d...\n", 1);
    hemi = VipCopyVolume(mask, "copy_mask");
//     VipSingleThreshold(hemi, EQUAL_TO, 1, BINARY_RESULT);
    VipDoubleThreshold(hemi, VIP_BETWEEN_OR_EQUAL_TO, 1, 2, BINARY_RESULT);
    
    /*WM from both hemisphere*/
    VipMaskVolume(white, hemi);
    if(VipConnexVolumeFilter(white, CONNECTIVITY_6, -1, CONNEX_BINARY)==PB) return(PB);
    VipWriteVolume(white, "white");
    
    /*Creation of a wall along the y axis to protect the ventricles.*/
//     vos = VipGetOffsetStructure(white);
//     ptr = VipGetDataPtr_S16BIT(white) + vos->oFirstPoint + mVipVolSizeX(white)-1;
//     for ( iz=0; iz<mVipVolSizeZ(white); iz++ )   /* loop on slices */
//     {
//         for ( iy=0; iy<mVipVolSizeY(white); iy++ )  /* loop on lines */
//         {
//             *ptr = 255;
//             ptr = ptr + vos->oPointBetweenLine + mVipVolSizeX(white);  /*skip border points*/
//         }
//         ptr = ptr + vos->oLineBetweenSlice; /*skip border lines*/
//     }
    copy3 = VipCopyVolume(white, "white");
    if (VipExtRay(copy3, EXTEDGE3D_ALL, SAME_VOLUME)==PB) return(VIP_CL_ERROR);
    VipSingleThreshold(copy3, EQUAL_TO, 0, BINARY_RESULT);
    if (VipConnexVolumeFilter(copy3, CONNECTIVITY_26, -1, CONNEX_BINARY)==PB) return(PB);
    
    /*Extraction of the csf in the closed WM*/
    copy2 = VipCreateSingleThresholdedVolume(mask, EQUAL_TO, 0, BINARY_RESULT);
    VipMaskVolume(copy2, copy3);
    
    VipFreeVolume(copy3);
  
    /*Keep only the components in the center of the brain.*/
    copy3 = VipCopyVolume(mask, "mask_cl");
    if (VipClosing(copy3, CHAMFER_BALL_3D, 20.)==PB) return(PB);
    if (VipErosion(copy3, CHAMFER_BALL_3D, 15.)==PB) return(PB);
    VipWriteVolume(copy3, "brain_cl_er");
    VipMaskVolume(copy2, copy3);
    VipWriteVolume(copy2, "extray");
    csfinwhite = VipCopyVolume(copy2, "extray_white");
    VipFreeVolume(copy);
    VipFreeVolume(copy3);
//     copy3 = VipCopyVolume(copy2, "vent_av");
    
//     if (VipConnexVolumeFilter(copy2, CONNECTIVITY_26, -1, CONNEX_BINARY)==PB) return(PB);
    
    /*Mask this components by the profound CSF*/
    VipMaskVolume(copy2, csf);
    /*Right Hemisphere*/
    copy = VipCreateSingleThresholdedVolume(mask, EQUAL_TO, 1, BINARY_RESULT);
    if (VipClosing(copy, CHAMFER_BALL_3D, 20.)==PB) return(PB);
    vent_rh = VipCopyVolume(copy2, "vent_rh");
    VipMaskVolume(vent_rh, copy);
    if (VipConnexVolumeFilter(vent_rh, CONNECTIVITY_6, -1, CONNEX_BINARY)==PB) return(PB);
    VipFreeVolume(copy);
    //**If the ventricles is big, mask by a opened csf. (Seuil sur un rapoort volume du cerveau ventricules ou volume direct ?
    vol_vent = VipGetNumberLabelPoints(vent_rh, 255);
    ratio = 100*vol_vent/vol_brain;
    printf("vol_vent: %f, ratio: %f\n", vol_vent, ratio), fflush(stdout);
    if (ratio > 0.8)
    {
        copy = VipCopyVolume(csf, "csf_op");
        if (VipOpening(copy, CHAMFER_BALL_3D, little_opening_size)==PB) return(PB);
        VipWriteVolume(copy, "csf_av_op");
        VipMaskVolume(vent_rh, copy);
        if (VipConnexVolumeFilter(vent_rh, CONNECTIVITY_6, -1, CONNEX_BINARY)==PB) return(PB);
        VipFreeVolume(copy);
    }
    /*Left Hemisphere*/
    copy = VipCreateSingleThresholdedVolume(mask, EQUAL_TO, 2, BINARY_RESULT);
    if (VipClosing(copy, CHAMFER_BALL_3D, 20.)==PB) return(PB);
    vent_lh = VipCopyVolume(copy2, "vent_lh");
    VipMaskVolume(vent_lh, copy);
    if (VipConnexVolumeFilter(vent_lh, CONNECTIVITY_6, -1, CONNEX_BINARY)==PB) return(PB);
    VipFreeVolume(copy);
    //**If the ventricles is big, mask by a opened csf. (Seuil sur un rapoort volume du cerveau ventricules ou volume direct ?
    vol_vent = VipGetNumberLabelPoints(vent_lh, 255);
    ratio = 100*vol_vent/vol_brain;
    printf("vol_vent: %f, ratio: %f\n", vol_vent, ratio), fflush(stdout);
    if (ratio > 0.8)
    {
        copy = VipCopyVolume(csf, "csf_op");
        if (VipOpening(copy, CHAMFER_BALL_3D, little_opening_size)==PB) return(PB);
        VipWriteVolume(copy, "csf_av_op");
        VipMaskVolume(vent_lh, copy);
        if (VipConnexVolumeFilter(vent_lh, CONNECTIVITY_6, -1, CONNEX_BINARY)==PB) return(PB);
        VipFreeVolume(copy);
    }
    /*Union of the both sides*/
    if (VipMerge(vent_rh, vent_lh, VIP_MERGE_ONE_TO_ONE, 255, 255)==PB) return(PB);
    VipWriteVolume(vent_rh, "ventricules");
//     //**Grandir dans le csf non ouvert pour les gros ventricules.
//     if (ratio > 1.)
//     {
//         if (VipMerge(copy2, vent_rh, VIP_MERGE_ONE_TO_ONE, 255, 512)==PB) return(PB);
//         VipChangeIntLabel(copy2, 0, -2);
//         if (VipComputeFrontPropagationChamferDistanceMap(copy2, 255, -2, VIP_NO_LIMIT_IN_PROPAGATION, 0)==PB) return(PB);
//         VipWriteVolume(copy2, "distmap_av2");
//         
//     }
 
    
    //A voir si ça sert:
//     if (VipClosing(vent_rh, CHAMFER_BALL_3D, 5.)==PB) return(PB);
    //Peut être faire une dilation dans le csf et pas a l'aveugle.
//     if (VipDilation(vent_rh, CHAMFER_BALL_3D, 1.5*little_opening_size)==PB) return(PB);
//     VipWriteVolume(vent_rh, "ventricules_cl");
//     
    /*Distance map of the ventricles seed in the CSF*/
    copy = VipCopyVolume(csf, "csf");
    VipMaskVolume(vent_rh, copy);
    if (VipMerge(copy, vent_rh, VIP_MERGE_ONE_TO_ONE, 255, 512)==PB) return(PB);
//     VipFreeVolume(vent_rh);
//     vent_rh = NULL;
    VipChangeIntLabel(copy, 0, -2);
    if (VipComputeFrontPropagationChamferDistanceMap(copy, 255, -2, VIP_NO_LIMIT_IN_PROPAGATION, 1)==PB) return(PB);
    VipWriteVolume(copy, "distmap_av");
    copy3 = VipCreateSingleThresholdedVolume(copy, GREATER_OR_EQUAL_TO, (int)(50*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT);
    VipSingleThreshold(copy, GREATER_THAN,  0, BINARY_RESULT);
    if (VipMerge(copy, copy3, VIP_MERGE_ONE_TO_ONE, 255, 512)== PB) return(PB);
    VipFreeVolume(copy3);
    copy3 = NULL;
    VipChangeIntLabel(copy, 0, -100);
    if (VipComputeFrontPropagationChamferDistanceMap(copy, 255, -100, VIP_NO_LIMIT_IN_PROPAGATION, 0)==PB) return(PB);
    VipWriteVolume(copy, "distmap");
    if (VipSingleThreshold(copy, GREATER_OR_EQUAL_TO, 32500, BINARY_RESULT)==PB) return(PB);
    VipMaskVolume(copy, plan_hemi);
    if (VipMerge(vent_rh, copy, VIP_MERGE_ONE_TO_ONE, 255, 255)==PB) return(PB);
//     if (VipConnexVolumeFilter(copy, CONNECTIVITY_6, -1, CONNEX_BINARY)==PB) return(PB);
    VipFreeVolume(copy);
    
    
/*Etape intermediaire*/   
//     if (VipClosing(vent_rh, CHAMFER_BALL_3D, 5.)==PB) return(PB);
// //     if (VipDilation(vent_rh, CHAMFER_BALL_3D, 1.5*little_opening_size)==PB) return(PB);
//     VipMaskVolume(vent_rh, csf);
//     VipWriteVolume(vent_rh, "ventricules_cl2");
//     
//     
//     /*Creation of the meancurvature image*/
//     VipFreeVolume(vol);
//     vol = VipReadVolumeWithBorder(input, 1);
//     gaussian = VipDeriche3DGaussian(vol, 1., NEW_FLOAT_VOLUME);
//     meancurv = Vip3DGeometry(gaussian, MEAN_CURVATURE);
//     VipFreeVolume(gaussian);
//     VipWriteVolume(meancurv, "meancurv");
//     VipSingleFloatThreshold(meancurv, GREATER_OR_EQUAL_TO, 0.4, BINARY_RESULT);
//     mc = VipTypeConversionToS16BIT(meancurv, RAW_TYPE_CONVERSION);
//     VipWriteVolume(mc, "meancurv_th");
//     
//     copy4 = VipCopyVolume(csf, "csf_cl");
//     if (VipClosing(copy4, CHAMFER_BALL_3D, little_opening_size)==PB) return(PB);
//     if (VipErosion(outside, CHAMFER_BALL_3D, little_opening_size)==PB) return(PB);
//     VipInvertBinaryVolume(outside);
//     if (VipConnexVolumeFilter(outside, CONNECTIVITY_6, -1, CONNEX_BINARY)==PB) return(PB);
//     if (VipMerge(copy4, outside, VIP_MERGE_ONE_TO_ONE, 255, 255)==PB) return(PB);
//     VipMaskVolume(mc, copy4);
//     VipFreeVolume(copy4);
// //     VipMaskVolume(copy4, copy3);
// //     if (VipMerge(mc, copy4, VIP_MERGE_ONE_TO_ONE, 255, 255)==PB) return(PB);
// //     VipFreeVolume(copy3);
//     
//     copy3 = VipCopyVolume(vent_rh, "vent");
// //     if (VipMerge(copy3, mc, VIP_MERGE_ONE_TO_ONE, 0, 0)==PB) return(PB);
//     if (VipMerge(mc, copy3, VIP_MERGE_ONE_TO_ONE, 255, 512)==PB) return(PB);
//     VipFreeVolume(copy3);
// //     VipHysteresisThresholding(mc, CONNECTIVITY_26, 0, CONNEX_BINARY, 0, 510, HYSTE_NUMBER, 1);
//     VipResizeBorder( mc, 1 );
//     VipChangeIntLabel(mc, 0, -2);
//     if (VipComputeFrontPropagationChamferDistanceMap(mc, 255, -2, VIP_NO_LIMIT_IN_PROPAGATION, 1)==PB) return(PB);
//     VipResizeBorder( mc, 0 );
//     VipWriteVolume(mc, "mc_vent");
//     copy3 = VipCreateSingleThresholdedVolume(mc, GREATER_OR_EQUAL_TO, (int)(60*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT);
//     //Test pour recuperer les morceaux non connexes
//     copy4 = VipCreateSingleThresholdedVolume(mc, EQUAL_TO, 32500, BINARY_RESULT);
//     //1ere fin de test
//     VipSingleThreshold(mc, GREATER_THAN,  0, BINARY_RESULT);
//     if (VipMerge(mc, copy3, VIP_MERGE_ONE_TO_ONE, 255, 512)== PB) return(PB);
//     VipFreeVolume(copy3);
//     copy3 = NULL;
//     VipResizeBorder( mc, 1 );
//     VipChangeIntLabel(mc, 0, -100);
//     if (VipComputeFrontPropagationChamferDistanceMap(mc, 255, -100, VIP_NO_LIMIT_IN_PROPAGATION, 0)==PB) return(PB);
//     VipResizeBorder( mc, 0 );
//     VipWriteVolume(mc, "mc_dist");
//     copy3 = VipCreateSingleThresholdedVolume(mc, EQUAL_TO, 32500, BINARY_RESULT);
//     VipSingleThreshold(mc, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT);
//     if (VipMerge(vent_rh, copy3, VIP_MERGE_ONE_TO_ONE, 255, 255)==PB) return(PB);
//     VipFreeVolume(copy3);
//     copy3 = NULL;
//     
//     //Test pour recuperer les morceaux non connexes
//     ////Masquer par la MB fermée
//     VipMaskVolume(copy4, csfinwhite);
//     VipWriteVolume(copy4, "cnc");
//     ////Masquer par la MB fermée erodée
//     copy3 = VipCreateSingleThresholdedVolume(mask, EQUAL_TO, 1, BINARY_RESULT);
//     if (VipClosing(copy3, CHAMFER_BALL_3D, 20.)==PB) return(PB);
//     if (VipErosion(copy3, CHAMFER_BALL_3D, 15.)==PB) return(PB);
//     copy = VipCreateSingleThresholdedVolume(mask, EQUAL_TO, 2, BINARY_RESULT);
//     if (VipClosing(copy, CHAMFER_BALL_3D, 20.)==PB) return(PB);
//     if (VipErosion(copy, CHAMFER_BALL_3D, 15.)==PB) return(PB);
//     if (VipMerge(copy, copy3, VIP_MERGE_ONE_TO_ONE, 255, 255)==PB) return(PB);
//     VipFreeVolume(copy3);
//     VipWriteVolume(copy, "hemi_cl_er");
//     VipMaskVolume(copy4, copy);
//     VipWriteVolume(copy4, "cnc2");
//     
//     ////Les ventricules doivent être proche de la matière blanche.
//     //Peut être refaire le masque de MB en étant plus souple sur le seuil bas.
//     if (VipDilation(white, CHAMFER_BALL_3D, little_opening_size)==PB) return(PB);
//     VipWriteVolume(white, "white_dl");
//     if (VipMerge(white, copy4, VIP_MERGE_ONE_TO_ONE, 0, 0)==PB) return(PB);
//     if (VipMerge(copy4, white, VIP_MERGE_ONE_TO_ONE, 255, 512)==PB) return(PB);
//     VipHysteresisThresholding(copy4, CONNECTIVITY_26, 0, CONNEX_BINARY, 0, 510, HYSTE_NUMBER, 1);
//     if (VipConnexVolumeFilter(copy4, CONNECTIVITY_26, 2, CONNEX_BINARY)==PB) return(PB);
//     if (VipMerge(mc, copy4, VIP_MERGE_ONE_TO_ONE, 255, 512)==PB) return(PB);
//     VipResizeBorder( mc, 1 );
//     VipChangeIntLabel(mc, 0, -2);
//     if (VipComputeFrontPropagationChamferDistanceMap(mc, 255, -2, VIP_NO_LIMIT_IN_PROPAGATION, 1)==PB) return(PB);
//     VipResizeBorder( mc, 0 );
//     VipWriteVolume(mc, "cnc_dist");
//     copy3 = VipCreateSingleThresholdedVolume(mc, GREATER_OR_EQUAL_TO, (int)(20*VIP_USUAL_DISTMAP_MULTFACT), BINARY_RESULT);
//     VipSingleThreshold(mc, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT);
//     if (VipMerge(mc, copy3, VIP_MERGE_ONE_TO_ONE, 255, 512)== PB) return(PB);
//     VipFreeVolume(copy3);
//     copy3 = NULL;
//     VipResizeBorder( mc, 1 );
//     VipChangeIntLabel(mc, 0, -100);
//     if (VipComputeFrontPropagationChamferDistanceMap(mc, 255, -100, VIP_NO_LIMIT_IN_PROPAGATION, 0)==PB) return(PB);
//     VipResizeBorder( mc, 0 );
//     VipWriteVolume(mc, "cnc_dist2");
//     copy3 = VipCreateSingleThresholdedVolume(mc, EQUAL_TO, 32500, BINARY_RESULT);
//     VipSingleThreshold(mc, GREATER_OR_EQUAL_TO,  0, BINARY_RESULT);
//     if (VipMerge(vent_rh, copy3, VIP_MERGE_ONE_TO_ONE, 255, 255)==PB) return(PB);
//     VipFreeVolume(copy3);
//     copy3 = NULL;
//     
// //     VipChangeIntLabel(white, 255, 512);
// //     if (VipMerge(white, copy4, VIP_MERGE_ONE_TO_ONE, 255, 255)==PB) return(PB);
// //     VipChangeIntLabel(white, 0, -2);
// //     if (VipComputeFrontPropagationChamferDistanceMap(white, 255, -2, VIP_NO_LIMIT_IN_PROPAGATION, 1)==PB) return(PB);
// //     VipWriteVolume(copy4, "cnc_dist");
//     //Fin de test
//     
//     /*Dilation of the ventricles in the csf*/
//     if (VipClosing(vent_rh, CHAMFER_BALL_3D, 5.)==PB) return(PB);
// //     if (VipDilation(vent_rh, CHAMFER_BALL_3D, 1.5*little_opening_size)==PB) return(PB);
//     VipMaskVolume(vent_rh, csf);
//     VipWriteVolume(vent_rh, "ventricules_cl3");
//     
//     
//     /*Dilation of the ventricles in the brain mask*/
//     copy3 = VipCopyVolume(hemi, "hemi_cl");
//     if (VipClosing(copy3, CHAMFER_BALL_3D, 20.)==PB) return(PB);
//     VipMaskVolume(vol, copy3);
//     classif = VipCSFGrayWhiteFatClassificationRegularisationForRobustApproach(vol, hana, NO, 1, hana->gray->mean-2.5*hana->gray->sigma, hana->gray->mean-1.7*hana->gray->sigma, hana->white->mean+3*hana->white->sigma, hana->white->mean+5*hana->white->sigma, 0.5*hana->gray->mean + 0.5*hana->white->mean);
//     VipSingleThreshold(classif, EQUAL_TO, 150, BINARY_RESULT);
//     VipWriteVolume(classif, "classif3");
//     VipFreeVolume(copy3);
//     
//     copy3 = VipCreateSingleThresholdedVolume(classif, EQUAL_TO, 0, BINARY_RESULT);
//     if (VipMerge(copy3, mask, VIP_MERGE_ONE_TO_ONE, 1, 0)==PB) return(PB);
//     if (VipMerge(copy3, mask, VIP_MERGE_ONE_TO_ONE, 2, 0)==PB) return(PB);
//     VipWriteVolume(copy3, "copy3");
//     
//     if (VipMerge(copy3, vent_rh, VIP_MERGE_ONE_TO_ONE, 255, 512)==PB) return(PB);
//     VipChangeIntLabel(copy3, 0, -2);
//     if (VipComputeFrontPropagationConnectivityDistanceMap(copy3, 255, -2, VIP_NO_LIMIT_IN_PROPAGATION, 1, CONNECTIVITY_6)==PB) return(PB);
//     VipWriteVolume(copy3, "vent_dist");
//     copy = VipCreateSingleThresholdedVolume(copy3, GREATER_OR_EQUAL_TO, 30, BINARY_RESULT);
//     VipSingleThreshold(copy3, GREATER_THAN,  0, BINARY_RESULT);
//     if (VipMerge(copy3, copy, VIP_MERGE_ONE_TO_ONE, 255, 512)== PB) return(PB);
//     VipFreeVolume(copy);
//     copy = NULL;
//     VipChangeIntLabel(copy3, 0, -100);
//     if (VipComputeFrontPropagationConnectivityDistanceMap(copy3, 255, -100, VIP_NO_LIMIT_IN_PROPAGATION, 1, CONNECTIVITY_6)==PB) return(PB);
//     VipWriteVolume(copy3, "vent_dist2");
//     if (VipSingleThreshold(copy3, GREATER_OR_EQUAL_TO, 31, BINARY_RESULT)==PB) return(PB);
//     if (VipMerge(vent_rh, copy3, VIP_MERGE_ONE_TO_ONE, 255, 255)==PB) return(PB);
    /*Fin Etape intermediaire*/
    
    /*---End of the Ventricles Segmentation---*/
    

    VipFreeVolume(classif);
    
    VipComputeRobustStatInMaskVolume(vol, csf, &mean, &sigma, VFALSE);
    printf("mean: %f, sigma: %f\n", mean, sigma), fflush(stdout);
    classif = VipCSFGrayWhiteFatClassificationRegularisationForRobustApproach(vol, hana, NO, 1, (int)(mean+sigma), (int)(mean+2*sigma), hana->white->mean+4*hana->white->sigma, hana->white->mean+10*hana->white->sigma, 0.5*hana->gray->mean + 0.5*hana->white->mean);
    VipWriteVolume(classif, "classif_ap");
    
    
    
    
    
    
    
    
    
    
    
    
    /*writing of the outputs*/
    printf("-------------------------\n");
    printf("Writing %s...\n",output);
    if (writelib == TIVOLI)
    {
        if(VipWriteTivoliVolume(vent_rh, output)==PB) return(VIP_CL_ERROR);
    }
    else if (writelib == SPM)
    {
        if(VipWriteSPMVolume(vent_rh, output)==PB) return(VIP_CL_ERROR);
    }
    else
    {
        if(VipWriteVolume(vent_rh, output)==PB) return(VIP_CL_ERROR);
    }
    
    return(0);
}


/*--------------------------------------------------------------------------*/
static int Usage()
{
  (void)fprintf(stderr,"Usage: VipCSFSeg\n");
  (void)fprintf(stderr,"        -i[nput] {MR image (bias corrected)}\n");
  (void)fprintf(stderr,"        -o[utput] {}\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: a, v, s or t (default:a)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s  or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-srand {int (default: time}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);
}

static int Help()
{
 
  VipPrintfInfo("CSF Segmentation\n");
  (void)printf("\n");
  (void)printf("Usage: VipGreyWhiteClassif\n");
  (void)printf("        -i[nput] {MR image (bias corrected)}\n");
  (void)printf("        -o[utput] {}\n");
  (void)printf("        [-r[eadformat] {char: a, v, s or t (default:a)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, TIVOLI or ANY image file format\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)printf("Forces the writing of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-srand {int (default: time}]\n");
  (void)printf("Initialization of the random seed, useful to get reproducible results\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);
}
/*--------------------------------------------------------------------------*/
  
  
  