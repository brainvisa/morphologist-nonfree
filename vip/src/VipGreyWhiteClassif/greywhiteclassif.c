/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipGreyWhiteClassif.c      * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 13/01/2002
 * VERSION     : 2.1                  * REVISION :
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
#include <vip/skeleton.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol);
  VIP_DEC_VOLUME(mask);
  VIP_DEC_VOLUME(edges);
  VIP_DEC_VOLUME(classif);
  VIP_DEC_VOLUME(white);
  VIP_DEC_VOLUME(copy);
  VIP_DEC_VOLUME(copy2);
  VIP_DEC_VOLUME(copy3);
  VIP_DEC_VOLUME(extedge);
  VIP_DEC_VOLUME(ventricles);
  VIP_DEC_VOLUME(mc);
  VIP_DEC_VOLUME(sulcus);
  VIP_DEC_VOLUME(vsulcus);
  char *input = NULL;
  char *maskname = NULL;
  char output[VIP_NAME_MAXLEN]="";
  char algo = 'N';
  int i, l;
  int readlib, writelib;
  float mG=-1., sG=-1., mW=-1., sW=-1.;
  VipT1HistoAnalysis *hana = NULL;
  char *hananame = NULL;
  VipHisto *histo;
  int histo_val=0, interval=0;
  int histo_min=100000, moyenne_pics=0;
  float ratio=0.;
  int niterations = 5;
  float KPOTTS = 20;
  char stringconnectivity[256] = "26";
  int connectivity;
  int label = 255;
  int exttogrey = VTRUE;
  float mean=0, sigma=0;
  VipOffsetStruct *vos;
  Vip_S16BIT *ptr, *ptr1, *ptr2, *cptr;
  int ix, iy, iz;
  float little_opening_size;
  char point_filename[VIP_NAME_MAXLEN]="";
  float CA[3], CP[3], P[3], d[3];
  float ptPlanHemi[3];
  float pt = 0.;
  VipTalairach tal, *coord=NULL;
  char *edgesname = NULL;
  int random_seed = time(NULL);

  readlib = ANY_FORMAT;
  writelib = TIVOLI;

  /*loop on command line arguments*/

  for (i=1;i<argc;i++)
    {
      if (!strncmp (argv[i], "-input", 2)) 
        {
          if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          input = argv[i];
        }
      else if (!strncmp (argv[i], "-output", 2)) 
        {
          if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          strcpy(output,argv[i]);
        }
      else if (!strncmp (argv[i], "-algo", 2)) 
        {
          if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          else if (argv[i][0]=='R') algo = 'R';
          else if (argv[i][0]=='N') algo = 'N';
          else
          {
              VipPrintfError("This algo is unknown");
              VipPrintfExit("(commandline)VipGreyWhiteClassif");
              return(VIP_CL_ERROR);
          }
        }
      else if (!strncmp (argv[i], "-hana", 2)) 
        {
          if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          hananame = argv[i];
        }
      else if (!strncmp (argv[i], "-mask", 2)) 
        {
          if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          maskname = argv[i];
        }
      else if (!strncmp (argv[i], "-label", 2)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          label = atoi(argv[i]);
        }
      else if (!strncmp (argv[i], "-Points", 2)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          strcpy(point_filename,argv[i]);
        }
      else if (!strncmp (argv[i], "-edges", 3)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          edgesname = argv[i];
        }
      else if (!strncmp (argv[i], "-mwhite", 3)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          mW= atof(argv[i]);
        }
      else if (!strncmp (argv[i], "-mgray", 3)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          mG= atof(argv[i]);
        }
      else if (!strncmp (argv[i], "-sgray", 3)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          sG= atof(argv[i]);
        }
      else if (!strncmp (argv[i], "-swhite", 3)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          sW= atof(argv[i]);
        }
      else if (!strncmp (argv[i], "-connectivity", 2)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          strcpy(stringconnectivity,argv[i]);
        }
      else if (!strncmp (argv[i], "-niterations", 2)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          niterations= atoi(argv[i]);
        }
      else if (!strncmp (argv[i], "-KPOTTS", 2)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          KPOTTS= atof(argv[i]);
        } 
      else if (!strncmp (argv[i], "-exttogrey", 3)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          if(argv[i][0]=='y') exttogrey = VTRUE;
          else if(argv[i][0]=='n') exttogrey = VFALSE;
          else
            {
              VipPrintfError("elsetogrey y/n choice");
              return(VIP_CL_ERROR);
            }
        }
      else if (!strncmp (argv[i], "-readformat", 2)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
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
          if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          if(argv[i][0]=='t') writelib = TIVOLI;
          else if(argv[i][0]=='v') writelib = VIDA;
          else if(argv[i][0]=='s') writelib = SPM;
          else
            {
              VipPrintfError("This format is not implemented for writing");
              VipPrintfExit("(commandline)VipDilation");
              return(VIP_CL_ERROR);
            }
        }
      else if (!strncmp(argv[i], "-srand", 6)) {
      	    	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
      	    	  random_seed = atoi(argv[i]);
      }
      else if (!strncmp(argv[i], "-help",2)) return Help();
      else return(Usage());
    }

  /*check that all required arguments have been given*/


  if (input==NULL) 
    {
      VipPrintfError("input arg is required by VipGreyWhiteClassif");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }
  if (maskname==NULL) 
    {
      VipPrintfError("mask arg is required by VipGreyWhiteClassif");
      return(Usage());
    }
  if (VipTestImageFileExist(maskname)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",maskname);
      return(VIP_CL_ERROR);
   }
  connectivity = VipConvertStringToConnectivity(stringconnectivity);
  if(connectivity==PB)
    {
      VipPrintfError("This connectivity is unknown...");
      return(Usage());
    }

  if (!strcmp(output,"")) 
    {
      strcpy(output,maskname);
      strcat(output,"_greywhite");
    }

  srand(random_seed);

  printf("Reading %s...\n",input);
  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,1);
  else if (readlib == SPM)
    vol = VipReadSPMVolumeWithBorder(input,1);
  else
    vol = VipReadVolumeWithBorder(input,1);

  if (vol==NULL) return (VIP_CL_ERROR);

  printf("Reading %s...\n",maskname);
  if (readlib == TIVOLI)
    mask = VipReadTivoliVolumeWithBorder(maskname,1);
  else if (readlib == SPM)
    mask = VipReadSPMVolumeWithBorder(maskname,1);
  else
    mask = VipReadVolumeWithBorder(maskname,1);
  
  if (mask==NULL) return (VIP_CL_ERROR);
  
  if(algo=='N')
  {
      printf("Reading %s...\n",edgesname);
      if (readlib == TIVOLI)
          edges = VipReadTivoliVolumeWithBorder(edgesname,1);
      else if (readlib == SPM)
          edges = VipReadSPMVolumeWithBorder(edgesname,1);
      else
          edges = VipReadVolumeWithBorder(edgesname,1);
      
      if (edges==NULL) return (VIP_CL_ERROR);
  }

  if (hananame==NULL)
  {
      if (mG < 0 || sG < 0 || mW < 0 || sW < 0)
      {
          VipPrintfError("Without histogram analysis (-hana), you have to provide 4 statistical parameters\n");
          return Usage();
      }
  }
  else
  {
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
  }

  if (mG > 0)
      hana->gray->mean = (int)(mG+0.5);
  if (sG > 0)
      hana->gray->sigma = (int)(sG+0.5);
  if (mW > 0)
      hana->white->mean = (int)(mW+0.5);
  if (sW > 0)
      hana->white->sigma = (int)(sW+0.5);

  if(algo=='N')
  {
      copy = VipDeriche3DGaussian( vol, 1., NEW_FLOAT_VOLUME );
      mc = Vip3DGeometry( copy, MEAN_CURVATURE );
      VipFreeVolume(copy);
  }
  
  printf("Selecting label %d...\n",label);
  copy = VipCopyVolume(mask, "copy_mask");
  VipSingleThreshold( copy, EQUAL_TO, label, BINARY_RESULT );
  VipMaskVolume( vol, copy );
  VipFreeVolume(copy);
  
  if(algo=='R')
  {
      classif = VipGrayWhiteClassificationRegularisationForVoxelBasedAna(vol, hana, VFALSE, niterations, KPOTTS, connectivity);
      
      VipChangeIntLabel( classif, VOID_LABEL, 0 ); /*background, should be better to do that in the package*/
      
      if(exttogrey==VTRUE)
      {
          extedge = VipExtedge(classif, EXTEDGE3D_ALL, NEW_VOLUME);
          if(extedge==PB) return(VIP_CL_ERROR);
          VipMerge(classif, extedge, VIP_MERGE_ALL_TO_ONE, 255, GRAY_LABEL);
          VipFreeVolume(extedge);
      }
  }
  else if(algo=='N')
  {
      little_opening_size = 0.5;
      if(mVipVolVoxSizeX(vol)>little_opening_size) little_opening_size=mVipVolVoxSizeX(vol)+0.1;
      if(mVipVolVoxSizeY(vol)>little_opening_size) little_opening_size=mVipVolVoxSizeY(vol)+0.1;
      if(mVipVolVoxSizeZ(vol)>little_opening_size) little_opening_size=mVipVolVoxSizeZ(vol)+0.1;
      
      
      printf("Determination of the quality of the image's contrast...\n");
      histo = VipComputeVolumeHisto(vol);
      if (histo==PB) return(VIP_CL_ERROR);
      
      interval = (hana->white->mean - hana->gray->mean)/8;
      l = hana->white->mean-1;
      while(l>hana->gray->mean)
      {
          histo_val = 0;
          for(i=l; i>l-interval; i--)
          {
              histo_val += histo->val[i];
          }
          histo_val /= interval;
          if(histo_val<histo_min)
          {
              histo_min = histo_val;
              if(l<(0.25*hana->white->mean + 0.75*hana->gray->mean))
              {
                  histo_min = histo->val[(hana->white->mean+hana->gray->mean)/2];
                  break;
              }
          }
          l-=interval;
      }
      moyenne_pics = (histo->val[hana->white->mean] + histo->val[hana->gray->mean])/2;
      ratio = (float)(moyenne_pics)/(float)(histo_min);
      printf("min: %d, moyenne_pics: %d, ratio: %.3f\n", histo_min, moyenne_pics, ratio);
      fflush(stdout);
      
      
      /*Creation of the mask of the ventricles*/
      printf("Segmentation of the venticles...\n");
      if(GetCommissureCoordinates(vol, point_filename, &tal,
                                  CA[0], CA[1], CA[2], 
                                  CP[0], CP[1], CP[2], 
                                  P[0], P[1], P[2], VTRUE)==PB) return(VIP_CL_ERROR);
      coord = &tal;
      CA[0] = (int)(coord->AC.x); CA[1] = (int)(coord->AC.y); CA[2] = (int)(coord->AC.z);
      CP[0] = (int)(coord->PC.x); CP[1] = (int)(coord->PC.y); CP[2] = (int)(coord->PC.z);
      P[0] = (int)(coord->Hemi.x); P[1] = (int)(coord->Hemi.y); P[2] = (int)(coord->Hemi.z);
      
      ventricles = VipCopyVolume(mask, "ventricles");
      VipSingleThreshold( ventricles, EQUAL_TO, label, BINARY_RESULT );
      
      d[0] = d[1] = d[2] = 1000.0;
      Vip3DPlanesResolution(CA, CP, P, d, &(ptPlanHemi[0]), &(ptPlanHemi[1]), &(ptPlanHemi[2]));
      //         printf("a = %f, b = %f, c = %f, d = %f\n", ptPlanHemi[0], ptPlanHemi[1], ptPlanHemi[2], d[0]), fflush(stdout);
      
      vos = VipGetOffsetStructure( ventricles );
      ptr = VipGetDataPtr_S16BIT( ventricles ) + vos->oFirstPoint + (int)(CP[1]-50)*vos->oLine + (int)(CA[2]-40)*vos->oSlice;
      for ( iz=(int)(CA[2]-40); iz<CP[2]; iz++ )   /* loop on slices */
      {
          for ( iy=(int)(CP[1]-50); iy<(int)(CP[1]+20); iy++ )  /* loop on lines */
          {
              for ( ix=0; ix<mVipVolSizeX(ventricles); ix++ )   /* loop on points */
              {
                  pt = ptPlanHemi[0]*(ix) + ptPlanHemi[1]*(iy) + ptPlanHemi[2]*(iz) - 1000.0;
                  if(-30<(int)(pt) && (int)(pt)<10) *ptr = 255;
                  ptr++;
              }
              ptr = ptr + vos->oPointBetweenLine;  /*skip border points*/
          }
          ptr = ptr + vos->oLineBetweenSlice + (int)(mVipVolSizeY(ventricles)-70)*vos->oLine; /*skip border lines*/
      }
      
      VipExtedge(ventricles, EXTEDGE3D_ALL, SAME_VOLUME);
      if(VipConnexVolumeFilter( ventricles, CONNECTIVITY_26, -1, CONNEX_BINARY )==PB) return(PB);
      if( VipDilation( ventricles, CHAMFER_BALL_3D, 5. )== PB) return(PB);
      copy = VipCopyVolume(mask, "copy_mask");
      VipSingleThreshold( copy, EQUAL_TO, label, BINARY_RESULT );
      VipMerge(ventricles, copy, VIP_MERGE_ONE_TO_ONE, 255, 255);
      VipFreeVolume(copy);
      
      VipSingleThreshold(ventricles, EQUAL_TO, 0, BINARY_RESULT );
      
      copy = VipCopyVolume(mask, "copy_mask");
      VipExtedge(copy, EXTEDGE3D_ALL, SAME_VOLUME);
      if(VipConnexVolumeFilter( copy, CONNECTIVITY_26, -1, CONNEX_BINARY )==PB) return(PB);
      if( VipDilation( copy, CHAMFER_BALL_3D, 15 )== PB) return(PB);
      VipInvertBinaryVolume(copy);
      if(VipConnexVolumeFilter( copy, CONNECTIVITY_26, -2, CONNEX_BINARY )==PB) return(PB);
      VipMaskVolume( ventricles, copy );
      VipFreeVolume(copy);
      
      if(VipConnexVolumeFilter( ventricles, CONNECTIVITY_26, -2, CONNEX_BINARY )==PB) return(PB);
      copy = VipCopyVolume(mask, "hemi");
      VipSingleThreshold(copy, EQUAL_TO, label, BINARY_RESULT );
      VipInvertBinaryVolume(copy);
      copy2 = VipCopyVolume(copy, "clean_background");
      if(VipConnexVolumeFilter( copy2, CONNECTIVITY_26, -1, CONNEX_BINARY )==PB) return(PB);
      VipMerge(copy, copy2, VIP_MERGE_ONE_TO_ONE, 255, 0);
      VipMerge(ventricles, copy, VIP_MERGE_ONE_TO_ONE, 255, 255);
      VipFreeVolume(copy);
      VipFreeVolume(copy2);
      
      if( VipDilation( ventricles, CHAMFER_BALL_3D, 1.5*little_opening_size )== PB) return(PB);
      
      //Selection d'un hemisphere
      VipSingleThreshold(mask, EQUAL_TO, label, BINARY_RESULT );
      
      
      /*Classification with the histogramme analysis*/
      printf("Classification of the input volume based on histo analysis ...\n");
      classif = VipGrayWhiteClassificationForVoxelBasedAna(vol, hana, 0.5);
      
      
      extedge = VipCreateSingleThresholdedVolume(classif, EQUAL_TO, 11, BINARY_RESULT );
      VipMerge(extedge, ventricles, VIP_MERGE_ONE_TO_ONE, 255, 0);
      VipCleaningConnectivity(extedge, CONNECTIVITY_6, 1);
      VipDilation(extedge, CHAMFER_BALL_3D, 1.5 );
      white = VipCreateSingleThresholdedVolume(classif, GREATER_OR_EQUAL_TO, 200, BINARY_RESULT );
      VipMerge(white, extedge, VIP_MERGE_ONE_TO_ONE, 0, 0);
      VipMerge(classif, white, VIP_MERGE_ONE_TO_ONE, 255, 100);
      VipFreeVolume(extedge);
      VipFreeVolume(white);
      
      
      printf("Creation of the hemisphere's external border image...\n");
      extedge = VipCopyVolume(mask, "extedge");
      VipMerge(extedge, ventricles, VIP_MERGE_ALL_TO_ONE, 255, 255);
      if(VipClosing( extedge, CHAMFER_BALL_3D, 4. )== PB) return(PB);
      if(VipSingleThreshold(extedge, EQUAL_TO, 0, BINARY_RESULT )==PB) return(PB);
      if(VipDilation( extedge, CHAMFER_BALL_3D, 2.5 )== PB) return(PB);
      
      white = VipCreateSingleThresholdedVolume(classif, GREATER_OR_EQUAL_TO, 200, BINARY_RESULT );
      VipMerge(white, extedge, VIP_MERGE_ONE_TO_ONE, 0, 0);
      VipMerge(classif, white, VIP_MERGE_ONE_TO_ONE, 255, 100);
      VipFreeVolume(white);
      
      sulcus = VipCopyVolume(mc, "copy_mc");
      VipSingleFloatThreshold(sulcus,GREATER_OR_EQUAL_TO,0.6,BINARY_RESULT);
      copy = VipTypeConversionToS16BIT( sulcus , RAW_TYPE_CONVERSION);
      white = VipCreateSingleThresholdedVolume(classif, GREATER_OR_EQUAL_TO, 200, BINARY_RESULT);
      VipMerge(white, classif, VIP_MERGE_ONE_TO_ONE, 100, 255);
      if(VipConnexVolumeFilter( white, CONNECTIVITY_6,-1, CONNEX_BINARY )==PB) return(PB);
      if(VipOpening( white, CHAMFER_BALL_3D, 1.*little_opening_size )== PB) return(PB);
      VipMerge(copy, white, VIP_MERGE_ONE_TO_ONE, 255, 0);
      VipFreeVolume(sulcus);
      sulcus = VipCreateSingleThresholdedVolume(white, GREATER_THAN, 512, BINARY_RESULT);
      VipMerge(sulcus, copy, VIP_MERGE_ONE_TO_ONE, 255, 255);
      VipFreeVolume(copy);
      VipFreeVolume(white);
      
      
      printf("Creation of the gyri's edges...\n");
      copy = VipCopyVolume(mask, "copy_hemi");
      VipDilation(copy, CHAMFER_BALL_3D, 1.5*little_opening_size);
      VipMerge(copy, ventricles, VIP_MERGE_ONE_TO_ONE, 255, 0);
      VipMaskVolume(edges, copy);
      VipFreeVolume(copy);
      
      copy = VipCopyVolume(edges, "copy_edges");
      copy2 = VipCopyVolume(mask, "copy_mask");
      VipMerge(copy2, ventricles, VIP_MERGE_ONE_TO_ONE, 255, 255);
      VipErosion(copy2, CHAMFER_BALL_3D, 1.*little_opening_size);
      VipMerge(copy, copy2, VIP_MERGE_ONE_TO_ONE, 255, 0);
      VipFreeVolume(copy2);
      
      
      /*Creation of the white matter core*/
      printf("Computing white matter core ...\n");
      white = VipCreateSingleThresholdedVolume(classif, EQUAL_TO, 201, BINARY_RESULT);
      VipMerge(white, extedge, VIP_MERGE_ONE_TO_ONE, 255, 0);
      if(VipDilation( copy, CHAMFER_BALL_3D, 2.*little_opening_size )== PB) return(PB);
      VipMerge(white, copy, VIP_MERGE_ONE_TO_ONE, 255, 0);
      VipFreeVolume(copy);
      
      if(VipConnexVolumeFilter( white, CONNECTIVITY_26, -1, CONNEX_BINARY )==PB) return(PB);
      VipInvertBinaryVolume(white);
      if(VipConnexVolumeFilter( white, CONNECTIVITY_6, -1, CONNEX_BINARY )==PB) return(PB);
      VipInvertBinaryVolume(white);
      VipCleaningConnectivity(white, CONNECTIVITY_6, 1);

      //mise a gris des bords de l'hemisphere
      copy = VipCopyVolume(extedge, "extedge_copy");
      VipMerge(copy, classif, VIP_MERGE_ONE_TO_ONE, 11, 0);
      VipMerge(classif, copy, VIP_MERGE_ONE_TO_ONE, 255, 101);
      VipFreeVolume(copy);

      VipMerge(white, classif, VIP_MERGE_ONE_TO_ONE, 11, 11);
      VipChangeIntLabel(white, 255, 512);
      VipChangeIntLabel(white, 0, 255);
      VipChangeIntLabel(white, 512, 0);
      VipHomotopicErosionFromInsideSnakeNeighbourhood(white, vol, classif, 60, 255, 0, 11, 1., hana->gray->sigma, hana->white->sigma, 201);
      
      
      sulcus = VipCopyVolume(mc, "copy_mc");
      VipSingleFloatThreshold(sulcus,GREATER_OR_EQUAL_TO,0.5,BINARY_RESULT);
      copy = VipTypeConversionToS16BIT( sulcus , RAW_TYPE_CONVERSION);
      copy2 = VipCreateSingleThresholdedVolume(classif, GREATER_OR_EQUAL_TO, 200, BINARY_RESULT);
      VipMerge(copy2, classif, VIP_MERGE_ONE_TO_ONE, 100, 255);
      if(VipConnexVolumeFilter( copy2, CONNECTIVITY_6,-1, CONNEX_BINARY )==PB) return(PB);
      if(VipOpening( copy2, CHAMFER_BALL_3D, 1.*little_opening_size )== PB) return(PB);
      VipMerge(copy, copy2, VIP_MERGE_ONE_TO_ONE, 255, 0);
      VipFreeVolume(sulcus);
      sulcus = VipCreateSingleThresholdedVolume(copy2, GREATER_THAN, 512, BINARY_RESULT);
      VipMerge(sulcus, copy, VIP_MERGE_ONE_TO_ONE, 255, 255);
      VipFreeVolume(copy2);
      VipFreeVolume(copy);
      
      copy = VipCopyVolume(mask, "copy_mask");
      if(VipClosing(copy, CHAMFER_BALL_3D, 5. )== PB) return(PB);
      VipMaskVolume(sulcus, copy);
      VipComputeRobustStatInMaskVolume(vol, sulcus, &mean, &sigma, VFALSE);
      printf("sulcus_mean: %f, sulcus_sigma: %f\n", mean, sigma);
      fflush(stdout);
      copy2 = VipCreateSingleThresholdedVolume(vol, LOWER_OR_EQUAL_TO, mean+0.3*sigma, BINARY_RESULT);
      if(VipConnexVolumeFilter( copy2, CONNECTIVITY_6, -1, CONNEX_BINARY )==PB) return(PB);
      VipMerge(sulcus, copy2, VIP_MERGE_ONE_TO_ONE, 255, 255);
      VipMerge(sulcus, copy, VIP_MERGE_ONE_TO_ONE, 0, 255);
      VipFreeVolume(copy);

      vsulcus = VipCopyVolume(sulcus, "sulcus_ventr");
      VipMerge(vsulcus, ventricles, VIP_MERGE_ONE_TO_ONE, 255, 0);
      if(VipConnexVolumeFilter( vsulcus, CONNECTIVITY_18, -1, CONNEX_BINARY )==PB) return(PB);
      
      VipFreeVolume(mc);
      
      //Cleaning white matter
      copy = VipCreateSingleThresholdedVolume(classif, GREATER_OR_EQUAL_TO, 200, BINARY_RESULT);
      VipCleaningConnectivity(copy, CONNECTIVITY_6, 1);
      copy2 = VipCopyVolume(vsulcus, "copy_sulcus");
//      VipConnectivityChamferDilation(copy, 1, CONNECTIVITY_26, FRONT_PROPAGATION);
//      VipConnectivityChamferDilation(copy, 1, CONNECTIVITY_6, FRONT_PROPAGATION);
      if(VipDilation( copy2, CHAMFER_BALL_3D, 2.5 )== PB) return(PB);
      VipMerge(copy2, copy, VIP_MERGE_ONE_TO_ONE, 0, 0);
      VipMerge(copy, copy2, VIP_MERGE_ONE_TO_ONE, 255, 512);
      
      VipChangeIntLabel(classif, 201, 100);
      VipChangeIntLabel(classif, 200, 100);
      VipMerge(classif, copy, VIP_MERGE_ONE_TO_ONE, 255, 201);
      VipMerge(classif, copy, VIP_MERGE_ONE_TO_ONE, 512, 200);
      
      //Premiere croissance
      copy = VipCreateSingleThresholdedVolume(white, EQUAL_TO, 0, BINARY_RESULT);
      VipHomotopicErosionFromInsideSnakeNeighbourhood(white, vol, classif, 60, 255, 0, 11, 1., hana->gray->sigma, hana->white->sigma, WHITE_LABEL);
      VipSingleThreshold(white, EQUAL_TO, 0, BINARY_RESULT);
      
      copy2 = VipCopyVolume(white, "white_thin");
      VipMerge(white, copy, VIP_MERGE_ONE_TO_ONE, 255, 0);
      if( VipClosing( copy2, CHAMFER_BALL_3D, 1.4*little_opening_size )== PB) return(PB);
      copy3 = VipCopyVolume(copy2, "opening");
      if( VipOpening( copy3, CHAMFER_BALL_3D, 1.4*little_opening_size )== PB) return(PB);
      VipDilation(copy3, CHAMFER_BALL_3D, 1.*little_opening_size);
      
      VipMerge(copy2, copy3, VIP_MERGE_ONE_TO_ONE, 255, 0);
      VipMerge(copy2, white, VIP_MERGE_ONE_TO_ONE, 0, 0);
      VipMerge(white, copy2, VIP_MERGE_ONE_TO_ONE, 255, 512);
      VipFreeVolume(copy2);
      
      VipMerge(copy, white, VIP_MERGE_ONE_TO_ONE, 512, 255);
      
      copy2 = VipCopyVolume(vsulcus, "copy_sulcus2");
      if(VipDilation( copy2, CHAMFER_BALL_3D, 2.5 )== PB) return(PB);
//      VipConnectivityChamferDilation(copy2, 1, CONNECTIVITY_26, FRONT_PROPAGATION);
//      VipConnectivityChamferDilation(copy2, 1, CONNECTIVITY_6, FRONT_PROPAGATION);
      VipMerge(white, copy2, VIP_MERGE_ONE_TO_ONE, 255, 0);
      VipHysteresisThresholding(white, CONNECTIVITY_26, 0, CONNEX_BINARY, 0, 511, HYSTE_NUMBER, 5);

      VipMerge(white, copy, VIP_MERGE_ONE_TO_ONE, 255, 512);
      VipGrayWhiteClassificationForVoxelBasedNeighbourhood(vol, white, copy, VFALSE, 10, 0, CONNECTIVITY_6, 5);
      VipSingleThreshold(copy, EQUAL_TO, 255, BINARY_RESULT);
      if(VipConnexVolumeFilter(copy, CONNECTIVITY_6, -1, CONNEX_BINARY)==PB) return(PB);
      VipCleaningConnectivity(copy, CONNECTIVITY_6, 1);

      white = VipCopyVolume( copy, "white" );

      if(ratio>1.4)
      {
          VipMerge(copy, classif, VIP_MERGE_ONE_TO_ONE, 11, 11);
          VipChangeIntLabel(copy, 255, 512);
          VipChangeIntLabel(copy, 0, 255);
          VipChangeIntLabel(copy, 512, 0);
	  VipHomotopicErosionFromInsideSnakeNeighbourhood(copy, vol, classif, 1, 255, 0, 11, 1., hana->gray->sigma, hana->white->sigma, 1);
          
          VipChangeIntLabel(classif, 201, 200);
          VipMerge(classif, white, VIP_MERGE_ONE_TO_ONE, 255, 201);
          VipMerge(white, sulcus, VIP_MERGE_ONE_TO_ONE, 255, 11);
          VipMerge(white, classif, VIP_MERGE_ONE_TO_ONE, 11, 11);
          VipGrayWhiteClassificationForVoxelBasedNeighbourhood(vol, classif, white, VFALSE, 1, 0, CONNECTIVITY_6, 4);
          
          vos = VipGetOffsetStructure( white );
          ptr1 = VipGetDataPtr_S16BIT( white ) + vos->oFirstPoint;
          ptr2 = VipGetDataPtr_S16BIT( copy ) + vos->oFirstPoint;
          cptr = VipGetDataPtr_S16BIT( classif ) + vos->oFirstPoint;
          
          for ( iz = mVipVolSizeZ(white); iz-- ; )               /* loop on slices */
          {
              for ( iy = mVipVolSizeY(white); iy-- ; )            /* loop on lines */
              {
                  for ( ix = mVipVolSizeX(white); ix-- ; )/* loop on points */
                  {
                      if(*ptr1==255)
                      {
                          if(*ptr2==255 && *cptr<200) *ptr1 = 0;
                      }
                      ptr1++;
                      ptr2++;
                      cptr++;
                  }
                  ptr1 += vos->oPointBetweenLine;  /*skip border points*/
                  ptr2 += vos->oPointBetweenLine;  /*skip border points*/
                  cptr += vos->oPointBetweenLine;  /*skip border points*/
              }
              ptr1 += vos->oLineBetweenSlice; /*skip border lines*/
              ptr2 += vos->oLineBetweenSlice; /*skip border lines*/
              cptr += vos->oLineBetweenSlice; /*skip border lines*/
          }
          
//         VipSingleThreshold(white, GREATER_THAN, 11, BINARY_RESULT);
//         VipMerge(white, classif, VIP_MERGE_ONE_TO_ONE, 11, 11);
//         VipMerge(white, classif, VIP_MERGE_ONE_TO_ONE, 101, 11);
//         VipGrayWhiteClassificationForVoxelBasedNeighbourhood(vol, classif, white, VFALSE, 1, 0, CONNECTIVITY_6, 4);
        
        VipMerge(white, sulcus, VIP_MERGE_ONE_TO_ONE, 255, 11);
        VipMerge(white, classif, VIP_MERGE_ONE_TO_ONE, 11, 11);
        VipChangeIntLabel(white, 255, 512);
        VipChangeIntLabel(white, 0, 255);
        VipChangeIntLabel(white, 512, 0);
        VipHomotopicErosionFromInsideSnakeNeighbourhood(white, vol, classif, 1, 255, 0, 11, 1., hana->gray->sigma, hana->white->sigma, 1);
        VipSingleThreshold(white, EQUAL_TO, 0, BINARY_RESULT);
        VipCleaningConnectivity(white, CONNECTIVITY_6, 1);
      }
      
      VipSingleThreshold(white, EQUAL_TO, 255, BINARY_RESULT);
      
      VipInvertBinaryVolume(white);
      VipConnexVolumeFilter(white, CONNECTIVITY_6, -1, CONNEX_BINARY);
      VipCleaningConnectivity(white, CONNECTIVITY_6, 2);
      VipInvertBinaryVolume(white);
      
      VipCleaningTopo(vol, white, classif, VFALSE, 50, 1, 0);

      VipInvertBinaryVolume(white);
      VipConnexVolumeFilter(white, CONNECTIVITY_6, -1, CONNEX_BINARY);
      VipInvertBinaryVolume(white);
      
      VipChangeIntLabel(white, 255, 200);
      VipMerge(classif, white, VIP_MERGE_ONE_TO_ONE, 200, 201);
      VipMerge(white, classif, VIP_MERGE_ONE_TO_ONE, 11, 11);
      VipChangeIntLabel(white, 0, 100);
      VipChangeIntLabel(white, 11, 0);
      
      classif = VipCopyVolume(white, "grey_white");
  }
  
  printf("-------------------------\n");
  printf("Writing %s...\n",output);
  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(classif,output)==PB) {
        return(VIP_CL_ERROR);
      }
    }
  else if (writelib == SPM)
    {
      if(VipWriteSPMVolume(classif,output)==PB) {
        return(VIP_CL_ERROR);
      }
    }
  else
    if(VipWriteVolume(classif,output)==PB) {
      return(VIP_CL_ERROR);
    }
  
  return(0);
}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipGreyWhiteClassif\n");
  (void)fprintf(stderr,"        -i[nput] {MR image (bias corrected)}\n");
  (void)fprintf(stderr,"        -a[lgo] {char: R[egularisation], N[eighbourhood], default: N}\n");
  (void)fprintf(stderr,"        -m[ask] {mask or Voronoi diagram}\n");
  (void)fprintf(stderr,"        -h[ana] {histogram scale-space analysis (?.han)}\n");
  (void)fprintf(stderr,"        -ed[ges] {edges image name}\n");
  (void)fprintf(stderr,"        -P[oints] {AC,PC,IH coord filename (*.tal)}\n");
  (void)fprintf(stderr,"        -ex[ttogrey] {y/n default:y}\n");
  (void)fprintf(stderr,"        [-l[abel] {int (default:255)}]\n");
  (void)fprintf(stderr,"        [-n[iterations] {int (default:5)}]\n");
  (void)fprintf(stderr,"        [-K[Potts] {float (default:20.)}]\n");
  (void)fprintf(stderr,"        [-c[onnectivity] {string:6/18/26/4/8/4s/8s/4c/8c (default:26)}]\n");
  (void)fprintf(stderr,"        [-o[utput] {classif (default:\"input_greywhite\")}]\n");
  (void)fprintf(stderr,"        [-mg[ray] {float (default:?.han)}]\n");
  (void)fprintf(stderr,"        [-sg[ray] {float (default:?.han)}]\n");
  (void)fprintf(stderr,"        [-mw[hite] {float (default:?.han)}]\n");
  (void)fprintf(stderr,"        [-sw[hite] {float (default:?.han)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: a, v, s or t (default:a)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s  or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-srand {int (default: time}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Grey white classif without topological constraints\n");
  (void)printf("\n");
  (void)printf("Usage: VipGreyWhiteClassif\n");
  (void)printf("        -i[nput] {MR image (bias corrected)}\n");
  (void)printf("        -a[lgo] {algo R/N, default: N\n");
  (void)printf("R: classification with regularisation\n");
  (void)printf("N: classification with local propagation\n");
  (void)printf("        -m[ask] {brain/hemisphere/cerebellum mask}\n");
  (void)printf(" a binary mask\n");
  (void)printf("        -h[ana] {histogram scale-space analysis name (?.han)}\n");
  (void)printf(" the T1-MR histo analysis usually obtained with VipHistoAnalysis\n");
  (void)printf("        -ed[ges] {edges image name}\n");
  (void)printf("        -P[oints] {AC,PC,IH coord filename (*.tal)}\n");
  (void)printf("Correct format for the commissure coordinate file toto.APC:\n");
  (void)printf("AC: 91 88 113\nPC: 91 115 109\nIH: 90 109 53\n");
  (void)printf("        -ex[ttogrey] {y/n default:y}\n");
  (void)printf(" The points that can be reached by a straight ray from image border are set to grey, a priori knowledge to deal with various MR artefacts\n");
  (void)printf(" This adds spurious grey voxel in corpus callosum, but improves brain parcellation elsewhere\n");
  (void)printf("        [-l[abel] {int (default:255)}]\n");
  (void)printf(" Label defining the actual mask from the Mask/Voronoi image\n");
  (void)printf("        [-n[iterations] {int (default:5)}]\n");
  (void)printf(" Number of ICM iteration for regularisation\n");
  (void)printf("        [-K[Potts] {float (default:20.)}]\n");
  (void)printf(" Potts model weight multiplied by the number of interacting neigbors\n");
  (void)printf("        [-c[onnectivity] {string:6/18/26/4/8/4s/8s/4c/8c (default:26)}]\n");
  (void)printf("Suffixes \"s\" and \"c\" means sagittal and coronal orientations\n");
  (void)printf("        [-o[utput] {classif (default:\"input_greywhite\")}]\n");
  (void)printf("        [-mg[ray] {float (default:?.han)}]\n");
  (void)printf("        [-sg[ray] {float (default:?.han)}]\n");
  (void)printf("        [-mw[hite] {float (default:?.han)}]\n");
  (void)printf("        [-sw[hite] {float (default:?.han)}]\n");
  (void)printf("        [-r[eadformat] {char: a, v, s or t (default:a)}]\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)printf("        [-srand {int (default: time}]\n");
  (void)printf("Initialization of the random seed, useful to get reproducible results\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
