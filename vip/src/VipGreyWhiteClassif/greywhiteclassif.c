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
#include <vip/morpho.h>
#include <vip/brain.h>
#include <vip/histo.h>
#include <vip/connex.h>
#include <vip/deriche.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  VIP_DEC_VOLUME(mask); 
  VIP_DEC_VOLUME(classif); 
  VIP_DEC_VOLUME(extedge); 
  char *input = NULL;
  char *maskname = NULL;
  char output[VIP_NAME_MAXLEN]="";
  int i;
  int readlib, writelib;
  float mG=-1., sG=-1., mW=-1., sW=-1.;
  VipT1HistoAnalysis *hana = NULL;
  char *hananame = NULL; 
  int niterations = 5;
  float KPOTTS = 20;
  char stringconnectivity[256] = "26";
  int connectivity;
  int label = 255;
  int exttogrey = VTRUE;

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
      else if (!strncmp (argv[i], "-mwhite", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  mW= atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-connectivity", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(stringconnectivity,argv[i]);
	} 
     else if (!strncmp (argv[i], "-hana", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  hananame = argv[i];
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-mgray", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  mG= atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-label", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  label = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-mask", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  maskname = argv[i];
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
      else if (!strncmp (argv[i], "-exttogrey", 2)) 
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

  printf("Selecting label %d...\n",label);
  VipSingleThreshold(mask,EQUAL_TO,label,BINARY_RESULT);

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
	  
  VipMaskVolume(vol,mask);
  VipFreeVolume(mask);

  classif = VipGrayWhiteClassificationRegularisationForVoxelBasedAna(vol, hana, VFALSE, 
								     niterations, KPOTTS, connectivity);
  VipChangeIntLabel(classif,VOID_LABEL,0); /*background, should be better to do that in the package*/
  if(exttogrey==VTRUE)
    {
      extedge = VipExtedge(classif,EXTEDGE3D_ALL,NEW_VOLUME);
      if(extedge==PB) return(VIP_CL_ERROR);
      VipMerge(classif,extedge,VIP_MERGE_ALL_TO_ONE,255,GRAY_LABEL);
      VipFreeVolume(extedge);
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
  (void)fprintf(stderr,"        -m[ask] {mask or Voronoi diagram}\n");
  (void)fprintf(stderr,"        -h[ana] {histogram scale-space analysis (?.han)}]\n");
  (void)fprintf(stderr,"        -e[xttogrey] {y/n default:y}\n");
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
  (void)printf("        -m[ask] {brain/hemisphere/cerebellum mask}\n");
  (void)printf(" a binary mask\n");
  (void)printf("        -h[ana] {histogram scale-space analysis name (?.han)}]\n");
  (void)printf(" the T1-MR histo analysis usually obtained with VipHistoAnalysis\n");
  (void)printf("        -e[xttogrey] {y/n default:y}\n");
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
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
