/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipLinearResamp.c    * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 02/02/1997
 * VERSION     : 1.0                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : PC Linux
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

#include <vip/linresamp.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  VIP_DEC_VOLUME(voltemplate); 
  VIP_DEC_VOLUME(volresample); 

  int usefloat = VFALSE;
  char *input = NULL;
  char *template = NULL;
  char *deplacement = NULL;
  char output[VIP_NAME_MAXLEN] = "linresampled";
  int  outval = VIP_LINRESAMP_ZERO;
  int readlib, writelib;
  VipDeplacement *dep;
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
      else if (!strncmp (argv[i], "-template", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  template = argv[i];
	}
      else if (!strncmp (argv[i], "-float", 2)) 
	{
	  usefloat = VTRUE;
	} 
      else if (!strncmp (argv[i], "-deplacement", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  deplacement = argv[i];
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
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
	       return(Usage());
	    }
	}
       else if (!strncmp (argv[i], "-label", 2))
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          if(argv[i][0]=='0') outval = VIP_LINRESAMP_ZERO;
          else if(argv[i][0]=='m') outval = VIP_LINRESAMP_MINI;
          else
            {
              (void)fprintf(stderr,"label switch is given and value should be '0' or 'm'.\n");
              return(Usage());
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

  if (outval != VIP_LINRESAMP_ZERO && outval != VIP_LINRESAMP_MINI)
    {
      (void)fprintf(stderr,"label switch is given and value should be '0' or 'm'.\n");
      return(VIP_CL_ERROR); 
    }
  if (input==NULL)
    {
      (void)fprintf(stderr,"input arg is required by VipLinearResamp\n");
      return(Usage());
    }
  if (template==NULL)
    {
      (void)fprintf(stderr,"template arg is required by VipLinearResamp\n");
      return(Usage());
    }
  if (deplacement==NULL)
    {
      (void)fprintf(stderr,"deplacement arg is required by VipLinearResamp\n");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
    }
  if (VipTestImageFileExist(template)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",template);
      return(VIP_CL_ERROR);
    }


  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,0);
  else if (readlib == SPM)
    vol = VipReadSPMVolumeWithBorder(input,0);
  else if(usefloat==VFALSE)
    vol = VipReadVolumeWithBorder(input,0);
  else
    vol = VipReadVidaFloatVolumeWithBorder(input,0);


  if (vol==PB)
    {
      VipPrintfError("Input reading Problem...");
      return(VIP_CL_ERROR);
    }

  printf("Reading template information:\n");

  voltemplate = VipReadVolumeWithBorder(template,0);
  if (voltemplate==PB)
    {
      VipPrintfError("Template reading Problem...");
      return(VIP_CL_ERROR);
    }
  VipFreeVolumeData(voltemplate);

  /*
  voltemplate = VipReadVidaHeader(template);
  if (voltemplate==PB)
    {
      VipPrintfError("Template reading Problem...");
      return(VIP_CL_ERROR);
    }
  */
  /*with this fucking vidaio, the whole image
    has to be present*/
  /*A general format header reader should be developed*/

  
  

  dep = VipReadDeplacement(deplacement);
  if (dep==PB)
    {
      VipPrintfError("Deplacement reading Problem...");
      return(VIP_CL_ERROR);
    }

  volresample = VipLinearResampCustom( vol,voltemplate,dep, outval );
  if (volresample==PB)
    {
      return(VIP_CL_ERROR);
    }
 
  
  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(volresample,output)==PB) return(VIP_CL_ERROR);
    }
  else if (writelib == SPM)
    {
      if(VipWriteSPMVolume(volresample,output)==PB) return(VIP_CL_ERROR);
    }
  else if(usefloat==VFALSE)
    {
      if(VipWriteVolume(volresample,output)==PB) return(VIP_CL_ERROR);
    }
  else
    {
      if(VipWriteVidaScaledVolume(volresample,output)==PB) return(VIP_CL_ERROR);
    }

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipLinearResamp\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        -t[emplate] {header or image name}\n");
  (void)fprintf(stderr,"        [-f[loat]]\n");
  (void)fprintf(stderr,"        -d[eplacement] {deplacement file (rotation or affine}\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"linresampled\")}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-l[abel] {char: 0 or m (default:0)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Performs a trilinear resampling of input volume according to template geometry");

  (void)printf("\n");
  (void)printf("Usage: VipLinearResamp\n");
  (void)printf("        -i[nput] {image name}\n");
  (void)printf("        -t[emplate] {header or image name}\n");
  (void)printf("        [-f[loat]]\n");
  (void)printf("The command works with float input volume (reading and writing do the same)\n");
  (void)printf("        -d[eplacement] {deplacement file (rotation or affine}\n");
  (void)printf("        [-o[utput] {image name (default:\"linresampled\")}]\n");
  (void)printf("        [-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)printf("Forces the writing of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-l[abel] {char: 0 or m (default:0)}]\n");
  (void)printf("Forces the value of samples of the resample image outside to be set to 0(default) or minimmum of the image to resample\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
