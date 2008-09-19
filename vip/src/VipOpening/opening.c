/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipOpening.c         * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 03/01/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
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
#include <vip/distmap.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "opened";
  int xmasksize = 3;
  int ymasksize = 3;
  int zmasksize = 3;
  float size = 0.;
  char stringconnectivity[256] = "6";
  int connectivity=0;
  int connectivity_activ = VFALSE;
  float multfactor = VIP_USUAL_DISTMAP_MULTFACT ;
  int borderwidth = 1;
  int maxxyz;
  char algo = 'f';
  int algodefine = FRONT_PROPAGATION;
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
     else if (!strncmp (argv[i], "-algo", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  algo = argv[i][0];
	}
      else if (!strncmp (argv[i], "-size", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  size = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-xmasksize", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  xmasksize = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-ymasksize", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  ymasksize = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-zmasksize", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  zmasksize = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-connectivity", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  {
	    strcpy(stringconnectivity,argv[i]);
	    connectivity_activ = VTRUE;
	  }
	} 
      else if (!strncmp (argv[i], "-multfactor", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  multfactor = atof(argv[i]);
	}     
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else if(argv[i][0]=='s') readlib = SPM;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipOpening");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') writelib = TIVOLI;
	  else if(argv[i][0]=='v') writelib = VIDA;
	  else if(argv[i][0]=='s') writelib = SPM;
	  else
	    {
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipOpening");
	      return(VIP_CL_ERROR);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipOpening");
      return(Usage());
    }
if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }

  if(algo=='f')
    {
      algodefine = FRONT_PROPAGATION;
    }
  else if (algo=='t')
    {
      algodefine = CHAMFER_TRANSFORMATION;
    }
  else
    {
      VipPrintfError("Unknown algo");
      return(Usage());
    }

  if(size<=0.)
    {
      VipPrintfError("Structuring element size is required by VipOpening (mm.)");
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

  maxxyz = mVipMax(xmasksize,ymasksize);
  maxxyz = mVipMax(zmasksize,maxxyz);
  borderwidth = (maxxyz-1)/2;

  printf("Reading %s...\n",input);
  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,borderwidth);
  else if (readlib == SPM)
    vol = VipReadSPMVolumeWithBorder(input,borderwidth);
  else
    vol = VipReadVolumeWithBorder(input,borderwidth);


  if(vol==NULL) return(VIP_CL_ERROR);

 if(connectivity_activ==VFALSE)
    {
      if (VipCustomizedChamferOpening (vol, size, xmasksize, ymasksize, zmasksize, multfactor, algodefine)==PB) return(VIP_CL_ERROR);
    }
  else
    {
      if (VipConnectivityChamferOpening (vol, size, connectivity, algodefine)==PB) return(VIP_CL_ERROR);
    }
  
  printf("Writing %s...\n",output);

  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else if (writelib == SPM)
    {
      if(VipWriteSPMVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else
    if(VipWriteVolume(vol,output)==PB) return(VIP_CL_ERROR);

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipOpening\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        -s[ize] {opening size in mm.}\n");
  (void)fprintf(stderr,"        [-a[lgo] {char: f or t (default:\"f\")}]\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"opened\")}]\n");
  (void)fprintf(stderr,"        [-x[masksize] {int (default:3)}]\n");
  (void)fprintf(stderr,"        [-y[masksize] {int (default:3)}]\n");
  (void)fprintf(stderr,"        [-z[masksize] {int (default:3)}]\n");
  (void)fprintf(stderr,"        [-m[ultfactor] {float (default:VIP_USUAL_DISTMAP_MULTFACT)}]\n");
  (void)fprintf(stderr,"        [-c[onnectivity] {string:6/18/26/4/8/4s/8s/4c/8c (default:not used)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Dilates the object defined as all non zero points");
  (void)printf("The structuring element is a ball of radius size for a chamfer distance.\n");
  (void)printf("\n");
  (void)printf("Usage: VipOpening\n");
  (void)printf("        -i[nput] {image name} : object definition\n");
  (void)printf("        -s[ize] {opening size in mm.}\n");
  (void)printf("        [-a[lgo] {char: f or t (default:\"f\")}]\n");
  (void)printf("f:bounded thick front propagation, t: chamfer transformation\n");
  (void)printf("        [-o[utput] {image name (default:\"opened\")}]\n");
  (void)printf("        [-x[masksize] {int (default:3)}: chamfer mask xsize]\n");
  (void)printf("        [-y[masksize] {int (default:3)}: chamfer mask ysize]\n");
  (void)printf("        [-z[masksize] {int (default:3)}: chamfer mask zsize]\n");
  (void)printf("        [-m[ultfactor] {float (default:VIP_USUAL_DISTMAP_MULTFACT)}]\n");
 (void)printf("        [-c[onnectivity] {string:6/18/26/4/8/4s/8s/4c/8c (default:26)}]\n");
  (void)printf("Suffixes \"s\" and \"c\" means sagittal and coronal orientations\n");
  (void)printf("        [-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)printf("Forces the writing of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
