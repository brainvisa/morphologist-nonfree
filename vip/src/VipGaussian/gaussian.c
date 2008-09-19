/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipGaussian.c         * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 7/11/98
 * VERSION     : 1.4                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
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
#include <vip/gaussian.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol);
  VIP_DEC_VOLUME(result);  
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "gaussian";
  float sigma = -1.;
  int outputfloat = VFALSE;
  /* temporary stuff */
  int readlib, writelib;
 
  int i;

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
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-sigma", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  sigma = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-float", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') outputfloat = VTRUE;
	  else if(argv[i][0]=='n') outputfloat = VFALSE;
	  else
	    {
	      VipPrintfError("Please answer y or n to flag -f");
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
	      VipPrintfExit("(commandline)VipDeriche");
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
	      VipPrintfExit("(commandline)VipDeriche");
	      return(VIP_CL_ERROR);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipGaussian");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
    }


  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,0);
  else
    vol = VipReadVolumeWithBorder(input,0);

  if(vol==NULL) return(VIP_CL_ERROR);

  if(sigma<=0.) sigma = mVipMax(mVipVolVoxSizeX(vol),mVipVolVoxSizeY(vol));

  if(outputfloat==VTRUE)
      {
	 result = VipDeriche3DGaussian( vol, sigma, NEW_FLOAT_VOLUME);
      }
  else
      {
	  result = VipDeriche3DGaussian( vol, sigma, SAME_VOLUME);
      }

  if(result==PB) return(VIP_CL_ERROR);

  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(result,output)==PB) return(VIP_CL_ERROR);
    }
  else if (outputfloat==VTRUE)
      {
	  if(VipWriteVidaScaledVolume(result,output)==PB) return(VIP_CL_ERROR);
      }
  else  if(VipWriteVolume(result,output)==PB) return(VIP_CL_ERROR);

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipGaussian\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-s[igma] {float (mm)  (default:largest pixel size)}]\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"gaussian\")}]\n");
  (void)fprintf(stderr,"        [-f[loat] {y/n (default:n)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Computes convolution by a 3D gaussian using Deriche recursive filters");
  (void)printf("Sigma is adapted to the voxel size in each direction\n");
  (void)printf("\n");
  (void)printf("Usage: VipGaussian\n");
  (void)printf("        -i[nput] {image name}\n");
  (void)printf("        [-s[igma] {float (mm)  (default:largest pixel size)}]\n");
  (void)printf("        [-o[utput] {image name (default:\"gaussian\")}]\n");
  (void)printf("        [-f[loat] {y/n (default:n)}]\n");
  (void)printf("if n: output keeps the type of input, y: output is in float\n");
  (void)printf("        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)printf("        [-w[riteformat] {char: v or t (default:v)}]\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
