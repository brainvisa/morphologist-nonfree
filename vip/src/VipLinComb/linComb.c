/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipLinComb.c         * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 06/11/1996
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
#include <vip/util.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol1); 
  VIP_DEC_VOLUME(vol2); 
  VIP_DEC_VOLUME(result); 

  int usefloat = VFALSE;
  char *input1 = NULL;
  char *input2 = NULL;
  char output[VIP_NAME_MAXLEN] = "lincomb";
  int mean_activ = VFALSE;
  int readlib, writelib;
  float a1 = 0;
  int a1_filled = VFALSE;
  float a2 = 0;
  int a2_filled = VFALSE;
  float sum;

  int i;

  readlib = VIDA;
  writelib = VIDA;

  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-i1", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  input1 = argv[i];
	}
      else if (!strncmp (argv[i], "-i2", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  input2 = argv[i];
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-a1", 3)) 
	{
	  a1 = atof(argv[++i]);
	  a1_filled = VTRUE;
	}
      else if (!strncmp (argv[i], "-a2", 3)) 
	{
	  a2 = atof(argv[++i]);
	  a2_filled = VTRUE;
	}
      else if (!strncmp (argv[i], "-mean", 2)) 
	{
	  mean_activ = VTRUE;
	} 
      else if (!strncmp (argv[i], "-float", 2)) 
	{
	  usefloat = VTRUE;
	} 
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else if(argv[i][0]=='s') readlib = SPM;
	  else
	    {
	      (void)fprintf(stderr,"This format is not implemented for reading\n");
	      Usage();
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
	      (void)fprintf(stderr,"This format is not implemented for writing\n");
	      return(Usage());
	    }    
	}
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input1==NULL || input2==NULL)
    {
      (void)fprintf(stderr,"input and mask args are required by VipLinComb\n");
      return(Usage());
    }
  if (a1_filled==VFALSE || a2_filled==VFALSE)
    {
      (void)fprintf(stderr,"a1 and a2 args are required by VipLinComb\n");
      return(Usage());
    }
  if (VipTestImageFileExist(input1)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input1);
      return(VIP_CL_ERROR);
    }
  if (VipTestImageFileExist(input2)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input2);
      return(VIP_CL_ERROR);
    }

  if (readlib == TIVOLI)
    vol1 = VipReadTivoliVolumeWithBorder(input1,0);
  else if (readlib == SPM)
    vol1 = VipReadSPMVolumeWithBorder(input1,0);
  else if(usefloat==VFALSE)
    vol1 = VipReadVolumeWithBorder(input1,0);
  else
    vol1 = VipReadVidaFloatVolumeWithBorder(input1,0);

  if (readlib == TIVOLI)
    vol2 = VipReadTivoliVolumeWithBorder(input2,0);
  else if (readlib == SPM)
    vol2 = VipReadSPMVolumeWithBorder(input2,0);
  else if(usefloat==VFALSE)
    vol2 = VipReadVolumeWithBorder(input2,0);
  else
    vol2 = VipReadVidaFloatVolumeWithBorder(input2,0);
 
  if ((vol1==PB)||(vol2==PB))
    {
      VipPrintfError("Reading Problem...");
      return(VIP_CL_ERROR);
    }
 
  if((mean_activ==VTRUE)&&((a1<=0.)||(a2<=0.)))
    {
      VipPrintfError("mean mode requires positive a1 and a2!");
      return(Help());
    }
  if(mean_activ==VTRUE)
  {
    sum = a1 + a2;
    a1 /= sum;
    a2 /= sum;
  }
    
  result = VipLinearCombination (a1, vol1, a2, vol2);
  if(result==PB) return(VIP_CL_ERROR);
  
  
  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(result,output)==PB) return(VIP_CL_ERROR);
    }
  else if (writelib == SPM)
    {
      if(VipWriteSPMVolume(result,output)==PB) return(VIP_CL_ERROR);
    }
  else if(usefloat==VFALSE)
    {
      if(VipWriteVolume(result,output)==PB) return(VIP_CL_ERROR);
    }
  else
    {
      if(VipWriteVidaScaledVolume(result,output)==PB) return(VIP_CL_ERROR);
    }

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipLinComb\n");
  (void)fprintf(stderr,"        -a1 {float}\n");
  (void)fprintf(stderr,"        -i1 {image name}\n");
  (void)fprintf(stderr,"        -a2 {float}\n");
  (void)fprintf(stderr,"        -i2 {image name}\n");
  (void)fprintf(stderr,"        [-f[loat]]\n");
  (void)fprintf(stderr,"        [-m[ean]]\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"lincomb\")}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Computes the linear combination a1*i1+a2*i2");


  (void)printf("\n");
  (void)printf("Usage: VipLinComb\n");
  (void)printf("        -a1 {float}\n");
  (void)printf("        -i1 {image name}\n");
  (void)printf("        -a2 {float}\n");
  (void)printf("        -i2 {image name}\n");
  (void)printf("        [-f[loat]]\n");
  (void)printf("The command works with float volumes (reading and writing do the same)\n");
  (void)printf("        [-m[ean]]\n");
  (void)printf("for a1>0, and a2>0, computes a1*i1+a2*i2/(a1+a2)\n");
  (void)printf("        [-o[utput] {image name (default:\"lincomb\")}]\n");
  (void)printf("        [-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)printf("Forces the writing of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
