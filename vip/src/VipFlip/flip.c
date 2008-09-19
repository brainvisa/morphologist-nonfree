/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipConvert.c         * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 11/1998
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
 *              / /   |   
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

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol);
  VIP_DEC_VOLUME(converted);  
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "flipped";
  int mode = VFALSE;
  int readlib, writelib;
  int testfile;

 
  int i;

  readlib = OK;
  writelib = OK;

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
      else if (!strncmp (argv[i], "-mode", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(!strcmp(argv[i],"xx")) mode = FLIP_XX;
	  else if(!strcmp(argv[i],"yy")) mode = FLIP_YY;
	  else if(!strcmp(argv[i],"zz")) mode = FLIP_ZZ;	  	  
	  else if(!strcmp(argv[i],"xy")) mode = FLIP_XY;	  	  
	  else if(!strcmp(argv[i],"xz")) mode = FLIP_XZ;	  	  
	  else if(!strcmp(argv[i],"yz")) mode = FLIP_YZ;
	  else if(!strcmp(argv[i],"xxzz")) mode = FLIP_XXZZ;
	  else if(!strcmp(argv[i],"yyzz")) mode = FLIP_YYZZ;
	  else
	    {
	      (void)fprintf(stderr,"Unknown mode: %s\n",argv[i]);
	      return(Usage());
	    }
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

  if (input==NULL || mode==VFALSE)
    {
      (void)fprintf(stderr,"input and mode arg are required by VipFlip\n");
      return(Usage());
    }
 
  if ((testfile=VipTestImageFileExist(input))==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
    }


  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,0);
  else if(readlib==SPM)
      vol = VipReadSPMVolumeWithBorder(input,0);
  else if (readlib==OK && (testfile==VIDA))
    {
      vol = VipReadVidaFloatVolumeWithBorder(input,0);
      if(vol==NULL)
	{
	  
	  VipPrintfWarning("Vida scaled reading failed, I try usual one...(no problem if input is in tivoli/GIS format)\n");
	  vol = VipReadVolumeWithBorder(input,0); 
	  if (vol==NULL)
	    {
	      VipPrintfError("This one fails also, sorry...\n");
	      return(Help());
	    }
	}
    }
  else 	  vol = VipReadVolumeWithBorder(input,0); 
  if (vol==NULL)
    {
      VipPrintfError("Reading problems, sorry...\n");
      return(VIP_CL_ERROR);
    }
  converted = VipFlipVolume( vol, mode);
  if(converted==PB) return(VIP_CL_ERROR);
 
  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(converted,output)==PB) return(VIP_CL_ERROR);
    }
  else if (writelib == SPM)
    {
      if(VipWriteSPMVolume(converted,output)==PB) return(VIP_CL_ERROR);
    }
  else if(writelib==OK)
    {
      if(VipWriteVolume(converted,output)==PB) return(VIP_CL_ERROR);
    }
  else
    {   
      if(VipWriteVidaScaledVolume(converted,output)==PB) return(VIP_CL_ERROR);
    }
  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipFlip\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        -m[ode] {xx,yy,zz,xy,xz,yz,xxzz,yyzz}\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"flipped\")}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v, t, s (default: v scaled)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, t, s (default: unscaled vida)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Flips volume either in a miror way or swaping coordinates");
  (void)printf("\n");
  (void)printf("Usage: VipFlip\n");
  (void)printf("        -i[nput] {image name}\n");
  (void)printf("        -m[ode] {xx,yy,zz,xy,xz,yz,xxzz,yyzz}\n");
  (void)printf("        [-o[utput] {image name (default:\"flipped\")}]\n");
  (void)printf("with r mode, the conversion is a simple cast\n");
  (void)printf("        [-r[eadformat] {char: v, s or t (default: scaled vida)}]\n");
  (void)printf("Forces the reading of VIDA or TIVOLI image file format\n");
  (void)printf("By default, the vida scaled reading to float volume is used\n");
  (void)printf("When -r v is asked, the vida reading to S16BIT is used\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default: unscaled vida)}]\n");
  (void)printf("Forces the writing of VIDA, TIVOLI or SPM image file format\n");
  (void)printf("the v choice forces scaled writing of float volume to vida format\n");
  (void)printf("the default mode do not scaled the volume\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
