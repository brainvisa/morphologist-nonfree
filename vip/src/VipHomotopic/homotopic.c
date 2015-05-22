/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipHomotopic.c      * TYPE     : Command line
 * AUTHOR      : FISCHER C.          * CREATION : 08/03/2011
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
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
#include <vip/morpho.h>
#include <vip/skeleton.h>
#include <vip/pyramid.h>
#include <vip/histo.h>
#include <vip/deriche.h>
#include "vip/brain.h"

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    
    /*declarations and initializations*/
    
    VIP_DEC_VOLUME(vol);
    VIP_DEC_VOLUME(closing);
    VIP_DEC_VOLUME(mask);
    VIP_DEC_VOLUME(extedge);
    VIP_DEC_VOLUME(greylevel);
    VIP_DEC_VOLUME(copy_grey);
    Pyramid *pyr, *pyrlab;
    char *input = NULL;
    char mode = 'H';
    char output[VIP_NAME_MAXLEN] = "";
    char *classifname = NULL;
    Volume *classif = NULL;
    char *cortexname = NULL;
    Volume *cortex = NULL;
    char *squeletonname = NULL;
    Volume *squeleton = NULL;
    VipT1HistoAnalysis *hana = NULL;
    char *hananame = NULL;
    float mG = -1., mW = -1.;
    int i;
    char version = '2';
    float fclosingsize = 10.;
    int linside = 0;
    int loutside = 11;
    int readlib, writelib;
    int random_seed = time(NULL);
    
    readlib = VIDA;
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
      else if (!strncmp (argv[i], "-mode", 2)) 
        {
            if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
            else if(argv[i][0]=='H') mode = 'H';
            else if(argv[i][0]=='C') mode = 'C';
            else
            {
                VipPrintfError("This mode is unknown");
                VipPrintfExit("(commandline)VipHomotopic");
                return(VIP_CL_ERROR);
            }
        }
      else if (!strncmp(argv[i], "-help", 3))
        {
          /* This has been placed before -hana because otherwise -help is
             captured by the test for -hana. We cannot use -h for help, because
             the documentation has long described -h as a shortcut for
             -hana. */
          return Help();
        }
      else if (!strncmp (argv[i], "-hana", 2)) 
        {
          if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          hananame = argv[i];
        }
      else if (!strncmp (argv[i], "-classif", 3)) 
        {
            if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
            classifname = argv[i];
        }
      else if (!strncmp (argv[i], "-cortex", 3)) 
        {
            if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
            cortexname = argv[i];
        }
	    else if (!strncmp(argv[i], "-srand", 6)){
	    	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	    	  random_seed = atoi(argv[i]);
	    }
      else if (!strncmp (argv[i], "-squeleton", 2)) 
        {
            if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
            squeletonname = argv[i];
        }
      else if (!strncmp (argv[i], "-version", 2)) 
        {
            if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
            else if(argv[i][0]=='1') version = '1';
            else if(argv[i][0]=='2') version = '2';
            else
            {
                VipPrintfError("This version is unknown");
                VipPrintfExit("(commandline)VipHomotopic");
                return(VIP_CL_ERROR);
            }
        }
      else if (!strncmp (argv[i], "-fclosing", 3)) 
        {
          if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
          fclosingsize = atof(argv[i]);
        }
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipHomotopic");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  if(argv[i][0]=='t') writelib = TIVOLI;
	  else if(argv[i][0]=='v') writelib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipHomotopic");
	      return(VIP_CL_ERROR);
	    }
	}    
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL) 
    {
      VipPrintfError("input arg is required by VipHomotopic");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }
   if (cortexname==NULL && mode=='H') 
   {
       VipPrintfError("cortex arg is required by VipHomotopic in this mode");
       return(Usage());
   }
   if (classifname==NULL && mode=='C') 
   {
       VipPrintfError("classif arg is required by VipHomotopic in this mode");
       return(Usage());
   }

  srand(random_seed);
  
  printf("Reading %s...\n",input);
  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,1);
  else
    vol = VipReadVolumeWithBorder(input,1);
  if (vol==NULL) return (VIP_CL_ERROR);
  
  if(mode=='C')
  {
      printf("Reading classif...\n");
      classif = VipReadVolumeWithBorder(classifname,1);
      if(!classif) return(VIP_CL_ERROR);
      if(hananame==NULL)
      {
          VipPrintfError("With the Cortical mode, you have to provide an histogram analysis (-hana)\n");
          return Usage();
      }
      else
      {
          hana = VipReadT1HistoAnalysis(hananame);
          if(hana==PB)
          {
              VipPrintfError("Can not read this histogram analysis");
              return (VIP_CL_ERROR);
          }
          if(hana->white==NULL || hana->gray==NULL)
          {
              VipPrintfError("Can not use this poor histogram analysis (no gray/white modes)");
              return (VIP_CL_ERROR);
          }
      }
      
      printf("-------------------------------\n");
      printf("Computing greylevel image...\n");
      printf("-------------------------------\n");
      
      VipMerge(vol, classif, VIP_MERGE_ONE_TO_ONE, 0, 0);
      
      mG = hana->gray->mean;
      mW = hana->white->mean;
      hana->gray->mean = (int)(hana->gray->mean - 2.*hana->gray->sigma);
      hana->white->mean = (int)(0.3*hana->white->mean+0.7*hana->gray->mean);
      greylevel = VipGrayWhiteClassificationRegularisationForVoxelBasedAna(vol, hana, VFALSE, 10, 20, CONNECTIVITY_26);
      
      VipMerge(greylevel, classif, VIP_MERGE_ONE_TO_ONE, 200, 200);
      copy_grey = VipCreateSingleThresholdedVolume( greylevel, EQUAL_TO, 200, BINARY_RESULT );
      VipConnexVolumeFilter(copy_grey, CONNECTIVITY_6, -1, CONNEX_BINARY);
      VipCleaningConnectivity(copy_grey, CONNECTIVITY_6, 1);
      VipInvertBinaryVolume(copy_grey);
      VipCleaningConnectivity(copy_grey, CONNECTIVITY_6, 2);
      VipInvertBinaryVolume(copy_grey);
      
      VipChangeIntLabel( greylevel, 200, 100 );
      VipChangeIntLabel( greylevel, 100, 50 );
      VipMerge( greylevel, copy_grey, VIP_MERGE_ONE_TO_ONE, 255, 100 );
      
      VipFreeVolume(copy_grey);
      hana->gray->mean = (int)(mG - hana->gray->sigma);
      hana->white->mean = (int)(mW - hana->white->sigma);
      copy_grey = VipGrayWhiteClassificationRegularisationForVoxelBasedAna(vol, hana, VFALSE, 10, 20, CONNECTIVITY_26);
      
      VipMerge(copy_grey, classif, VIP_MERGE_ONE_TO_ONE, 200, 200);
      VipSingleThreshold( copy_grey, EQUAL_TO, 200, BINARY_RESULT );
      VipConnexVolumeFilter(copy_grey, CONNECTIVITY_6, -1, CONNEX_BINARY);
      VipCleaningConnectivity(copy_grey, CONNECTIVITY_6, 1);
      VipInvertBinaryVolume(copy_grey);
      VipCleaningConnectivity(copy_grey, CONNECTIVITY_6, 2);
      VipInvertBinaryVolume(copy_grey);
      
      VipMerge( greylevel, copy_grey, VIP_MERGE_ONE_TO_ONE, 255, 150 );
      
      VipChangeIntLabel( greylevel, 10, 0 );
      VipMerge(greylevel, classif, VIP_MERGE_ONE_TO_ONE, 200, 200);
      VipFreeVolume(copy_grey);
      
      printf("-------------------------------\n");
      printf("Computing closing of %s...\n",vol->name);
      printf("-------------------------------\n");
      closing = VipCopyVolume(greylevel,"closing");
      if (closing==PB) return (VIP_CL_ERROR);
      if (fclosingsize > 0) {
        if (VipClosing ( closing, CHAMFER_BALL_3D, fclosingsize ) == PB)
          return (VIP_CL_ERROR);
      }
      VipChangeIntLabel( closing, 255, 1 );
      VipMerge( closing, greylevel, VIP_MERGE_SAME_VALUES, 0, 0);
      VipFreeVolume(greylevel);
      
      printf("-------------------------------\n");
      printf("Computing pyramid and bounding box deflation...\n");
      printf("-------------------------------\n");
      
      pyr = VipPutVolumeInPyramidAndCreatePyramid( closing, 3, PYR_MAX );
      pyrlab = VipCreateBoundingBoxLabelPyramid( pyr, 3, 255, linside, loutside );
      
      printf("-------------------------------\n");
      printf("Grey/White interface detection...\n");
      printf("-------------------------------\n");

      VipHomotopicInsideDilation(pyrlab->image[0]->volume, pyr->image[0]->volume, 100, 255, linside, loutside, FRONT_RANDOM_ORDER);
      if(version=='2')
      {
          VipHomotopicGeodesicErosionFromOutside(pyrlab->image[0]->volume, 1, 255, linside, loutside);
      }
      VipCopyVolumeHeader( closing, pyrlab->image[0]->volume );
  }
  
  else if(mode=='H')
  {
      printf("Reading cortex...\n");
      cortex = VipReadVolumeWithBorder(cortexname,1);
      //need to reset the random seed here because something in VipReadVolumeWithBorder changes it
      srand(random_seed);  
      if(!cortex) return(VIP_CL_ERROR);
      if(squeletonname==NULL)
      {
          printf("You did not provide a skeleton image, sulci won't be very open...\n");
      }
      else
      {
          printf("Reading squeleton...\n");
          squeleton = VipReadVolumeWithBorder(squeletonname,1);
          if(!squeleton) return(VIP_CL_ERROR);
      
          printf("-------------------------------\n");
          printf("Masking %s with the sulcus...\n",vol->name);
          printf("-------------------------------\n");
          
          mask = VipCreateSingleThresholdedVolume( squeleton, GREATER_OR_EQUAL_TO, 19, BINARY_RESULT );
          extedge = VipCopyVolume( mask, "extedge" );
          VipExtedge( extedge, EXTEDGE3D_ALL, SAME_VOLUME );
          VipMerge( mask, extedge, VIP_MERGE_ONE_TO_ONE, 255, 0 );
          VipFreeVolume(extedge);
          extedge = VipCreateSingleThresholdedVolume( squeleton, EQUAL_TO, 80, BINARY_RESULT );
          
          VipMerge( mask, extedge, VIP_MERGE_ONE_TO_ONE, 255, 255 );
          VipMerge( vol, mask, VIP_MERGE_ONE_TO_ONE, 255, 0 );
          VipFreeVolume(extedge);
          VipFreeVolume(mask);
      }
      
      if(version=='2')
      {
          /* The homotopic erosion from inside erodes the cortex until it
             leaves an object that is one voxel thick. This object will be
             considered to be part of the CSF, but as in the version 2 of the
             cortex image this layer of one voxel in the CSF is no longer present
             we pre-dilate the cortex by one voxel toward the outside. */
          VipHomotopicGeodesicDilation(cortex, 1, 255, 11, 11, 0, FRONT_RANDOM_ORDER);        
      }
      
      printf("-------------------------------\n");
      printf("CSF/Grey interface detection using homotopic erosion of %s...\n", cortexname);
      printf("-------------------------------\n");

      VipHomotopicErosionFromInside( cortex, vol, 100, 255, linside, loutside );
  }
    
    printf("-------------------------\n");
    printf("Writing %s...\n",output);
    if (writelib == TIVOLI)
    {
        if(mode=='C')
        {
            if(VipWriteTivoliVolume(pyrlab->image[0]->volume,output)==PB) return(VIP_CL_ERROR);
        }
        else if(mode=='H')
        {
            if(VipWriteTivoliVolume(cortex,output)==PB) return(VIP_CL_ERROR);
        }
    }
    else
    {
        if(mode=='C')
        {
            if(VipWriteVolume(pyrlab->image[0]->volume,output)==PB) return(VIP_CL_ERROR);
        }
        else if(mode=='H')
        {
            if(VipWriteVolume(cortex,output)==PB) return(VIP_CL_ERROR);
        }
    }
    
    return(0);
}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
    (void)fprintf(stderr,"Usage: VipHomotopic\n");
    (void)fprintf(stderr,"        -i[nput] {MR image (bias corrected)}\n");
    (void)fprintf(stderr,"        -m[ode] {char: C[ortical interface] or H[emisphere surface]}\n");
    (void)fprintf(stderr,"        [-o[utput] {image name (default:\"cortex\")}]\n");
    (void)fprintf(stderr,"        [-h[ana] {histogram scale-space analysis (?.han) only used by the Cortical mode}]\n");
    (void)fprintf(stderr,"        [-cl[assif] {grey_white classification image only used by the Cortical mode}]\n");
    (void)fprintf(stderr,"        [-co[rtex] {cortex image name only used by the Hemisphere mode}]\n");
    (void)fprintf(stderr,"        [-s[keleton] {skeleton image name only used by the Hemisphere mode}]\n");
    (void)fprintf(stderr,"        [-v[ersion] {int: 1 or 2 (default: 2)}]\n");
    (void)fprintf(stderr,"        [-fc[losing] {float (mm) closing size for Cortical mode (default: 10)}]\n");
    (void)fprintf(stderr,"        [-r[eadformat] {char: v or t (default: v)}]\n");
    (void)fprintf(stderr,"        [-w[riteformat] {char: v or t (default: t)}]\n");
    (void)fprintf(stderr,"        [-he[lp]\n");
    return(VIP_CL_ERROR);
}
/*****************************************************/

static int Help()
{
    VipPrintfInfo("Homotopic segmentation of an object defined by the external hull");
    (void)printf("of the brain (the surface of a brain closing) and by the CSF/Grey interface or by the Grey/White interface\n");
    (void)printf("Usage: VipHomotopic\n");
    (void)printf("        -i[nput] {MR image (bias corrected)}\n");
    (void)printf("        -m[ode] {char: C[ortical interface] or H[emisphere surface]}\n");
    (void)printf("Cortical interface: compute a spherical cortical interface from the classification.\n");
    (void)printf("Hemisphere interface: compute a spherical hemisphere interface.\n");
    (void)printf("        [-h[ana] {histogram scale-space analysis name (?.han) only used by the Cortical mode}]\n");
    (void)printf(" the T1-MR histo analysis usually obtained with VipHistoAnalysis\n");
    (void)printf("        [-cl[assif] {grey_white classification image only used by the Cortical mode}]\n");
    (void)printf(" a grey_white classification obtained with VipGreyWhiteClassification\n");
    (void)printf("        [-co[rtex] {cortex image name only used by the Hemisphere mode}]\n");
    (void)printf(" a white matter segmentation obtained with VipHomotopic in Cortical mode\n");
    (void)printf("        [-s[keleton] {skeleton image name only used by the Hemisphere mode}]\n");
    (void)printf(" an image of the sulci obtained with VipSkeleton\n");
    (void)printf("        [-o[utput] {image name (default:\"homotopic\")}]\n");
    (void)printf("        [-v[ersion] {int, version for the cortical interface, 1 or 2 (default: 2)}]\n");
    (void)printf("        [-fc[losing] {float (mm) closing size for Cortical mode (default: 10)}]\n");
    (void)printf("        [-r[eadformat] {char: v or t (default: v)}]\n");
    (void)printf("        [-w[riteformat] {char: v or t (default: t)}]\n");
    (void)printf("        [-he[lp]\n");
    return(VIP_CL_ERROR);
}
/******************************************************/
