/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipDilatation.c      * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 03/01/1997
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
#include <vip/connex.h>
#include <vip/morpho.h>
#include <vip/skeleton.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "openfold";
  float erosionsize = 2.;
  char stringconnectivity[256] = "26";
  int connectivity=0;
  int connectivity_activ = VFALSE;
  int borderwidth = 1;
  int readlib, writelib;
  char algo = 'e';
  int nbiter = 5;
  char *seedname = NULL;
  VIP_DEC_VOLUME(seed); 
  int threshold = -1;
  int i;
  char fill = 'n';
  float fclosingsize = 5.;
  float ferosionsize = 10.;
  

  readlib = VIDA;
  writelib = VIDA;

  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-input", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  input = argv[i];
	}
      else if (!strncmp (argv[i], "-seed", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  seedname = argv[i];
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-algo", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  algo = argv[i][0];
	}
      else if (!strncmp (argv[i], "-erosion", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  erosionsize = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-fclosing", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  fclosingsize = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-ferosion", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  ferosionsize = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-nbiter", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  nbiter = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-threshold", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  threshold = atoi(argv[i]);
	}
     else if (!strncmp (argv[i], "-connectivity", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  {
	    strcpy(stringconnectivity,argv[i]);
	    connectivity_activ = VTRUE;
	  }
	} 
      else if (!strncmp (argv[i], "-fill", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  fill = argv[i][0];
	  if(fill!='y' && fill!='n')
	    {
	      VipPrintfError("fill: y/n question!");
	      return(Usage());
	    }
	}
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipDilation");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') writelib = TIVOLI;
	  else if(argv[i][0]=='v') writelib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipDilation");
	      return(VIP_CL_ERROR);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipOpenFold");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }

  if(algo=='e')
    {
	if(erosionsize<=0.)
	    {
		VipPrintfError("Structuring element size is required by algo e");
		return(Usage());
	    }
	if(connectivity_activ==VTRUE)
	    {
		connectivity = VipConvertStringToConnectivity(stringconnectivity);
		if(connectivity==PB)
		    {
			VipPrintfError("This connectivity is unknown...");
			return(Usage());
		    }
	    }
    }
  else if (algo=='t')
    {
      if(threshold==-1)
	{
	  VipPrintfError("threshold is required by algo t...");
	  return(Usage());
	}
    }
  else if (algo=='i')
    {
      if(seedname==NULL)
	{
	  VipPrintfError("seed image name is required by algo i...");
	  return(Usage());
	}
    }
  else if(algo!='f')
    {
      VipPrintfError("Unknown algorithm");
      return(Usage());
    }




  printf("Reading %s...\n",input);
  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,borderwidth);
  else
    vol = VipReadVolumeWithBorder(input,borderwidth);

  if(vol==NULL) return(VIP_CL_ERROR);

  /*compute seed*/


  

  if(algo=='e')
    {
      seed = VipCopyVolume(vol,"seed");
      if(seed==PB) return(VIP_CL_ERROR);

      if(connectivity_activ==VFALSE)
	{      
	  if (VipErosion (seed, erosionsize, FRONT_PROPAGATION)==PB) return(VIP_CL_ERROR);
	}
      else
	{
	  if (VipConnectivityChamferErosion (seed, erosionsize, connectivity, FRONT_PROPAGATION)==PB) return(VIP_CL_ERROR);
	}
    }
  else if (algo=='i')
      {
	  seed = VipReadVolumeWithBorder(seedname,borderwidth);
	  if(seed==PB) return(VIP_CL_ERROR);
      }
  else if (algo=='t')
      {
	  seed = VipCreateSingleThresholdedVolume(vol,
						  GREATER_THAN, threshold, BINARY_RESULT);
	  if(seed==PB) return(VIP_CL_ERROR);
      }

  if(VipSingleThreshold(vol, GREATER_THAN, 1, BINARY_RESULT)==PB) return(VIP_CL_ERROR);

  if(algo!='f')
      {
	  if(VipMerge( vol, seed, VIP_MERGE_ONE_TO_ONE, 255, 499)==PB) return(VIP_CL_ERROR); 
	  
	  VipFreeVolume(seed);
	  
	  VipHomotopicGeodesicDilation( vol, nbiter, 499, 255, 0, 0, FRONT_RANDOM_AND_DEPTH);
      }
  
  if(fill=='y')
      {
	  seed = VipCopyVolume(vol, "fill");
	  if(seed==PB) return(VIP_CL_ERROR);
	  if(VipClosing(seed, CHAMFER_BALL_3D, fclosingsize)==PB) return(VIP_CL_ERROR);
	  if(VipErosion(seed, CHAMFER_BALL_3D, ferosionsize)==PB) return(VIP_CL_ERROR);
	  if(VipMerge( vol, seed, VIP_MERGE_ONE_TO_ONE, 255, 499)==PB) return(VIP_CL_ERROR);
	  VipFreeVolume(seed);
      }


  printf("Writing %s...\n",output);
  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else
    if(VipWriteVolume(vol,output)==PB) return(VIP_CL_ERROR);

  VipFreeVolume(vol);
  VipFree(vol);

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipOpenFold\n");
  (void)fprintf(stderr,"        -i[nput] {brain image name}\n");
  (void)fprintf(stderr,"        [-a[lgo] {string:e/t/i/f (default : e)}]\n");
  (void)fprintf(stderr,"        [-n[iter] {nb iter homotopic dilation (default : 5)}]\n");
  (void)fprintf(stderr,"        [-e[rosion] {algo e: erosion size (default : 2mm)}]\n");
  (void)fprintf(stderr,"        [-c[onnectivity] {algo e: (erosion) string:6/18/26/4/8/4s/8s/4c/8c (default:6)}]\n");
  (void)fprintf(stderr,"        [-t[hreshold] {algo t: int (default : not used)}]\n");
  (void)fprintf(stderr,"        [-s[eed] {algo i: image name (default : not used)}]\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"openfold\")}]\n");
  (void)fprintf(stderr,"        [-f[ill] {y/n, fill the fold depth (default: n)}]\n");
  (void)fprintf(stderr,"        [-fc[losing] {float (mm) closing size for f mode (default: 5)}]\n");
  (void)fprintf(stderr,"        [-fe[rosion] {float (mm) erosion size for f mode (default: 10)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Apply an homotopic dilation to a hard seed of the brain");
  (void)printf("The homotopic constraint prevent the merge of two neighboring gyri\n");
  (void)printf("In this way, we get better triangulation for 3D rendering\n");
  (void)printf("Or better cortex surface representations\n");
  (void)printf("\n");
  (void)printf("Usage: VipOpenFold\n");
  (void)printf("        -i[nput] {brain image name}\n");
  (void)printf(" a brain segmentation usually obtained with VipGetBrain\n");
  (void)printf(" for t algorithm, this image has to include MR gray level values\n");
  (void)printf("        [-a[lgo] {string:e/t/i/f (default : e)}]\n");
  (void)printf(" e: the hard seed is obtained from a simple binary erosion of the binary brain\n");
  (void)printf(" t: the hard seed related to white matter is obtained by a simple thresholding\n");
  (void)printf(" i: the hard seed is given by another image\n");
  (void)printf(" f: nothing happens apart from filling if trigered\n");
  (void)printf("        [-n[iter] {nb iter homotopic dilation (default : 5)}]\n");
  (void)printf(" the homotopic dilation follows a 6-connectivity related front based propagation\n");
  (void)printf("        [-e[rosion] {algo e : erosion size (default : 2mm)}]\n");
  (void)printf(" without c flag, this parameter is in mm, with c falg, it is connectivity related\n");
  (void)printf("        [-c[onnectivity] {(erosion) string:6/18/26/4/8/4s/8s/4c/8c (default:not used)}]\n");
  (void)printf("        [-t[hreshold] {algo t: int (default : not used)}]\n");
  (void)printf("        [-s[eed] {algo i: image name (default : not used)}]\n");
  (void)printf("        [-o[utput] {image name (default:\"openfold\")}]\n");
  (void)printf("        [-f[ill] {y/n, fill the fold depth (default: n)}]\n");
  (void)printf("This refinement consists in closing the brain, then eroding the closure,\n");
  (void)printf("and adding the erosion result to the segmented brain to fill in the depth.\n");
  (void)printf("This is done for 3D rendering optimization (less triangles)\n");
  (void)printf("        [-fc[losing] {float (mm) closing size for f mode (default: 5)}]\n");
  (void)printf("        [-fe[rosion] {float (mm) erosion size for f mode (default: 10)}]\n");
  (void)printf("        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)printf("        [-w[riteformat] {char: v or t (default:v)}]\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
