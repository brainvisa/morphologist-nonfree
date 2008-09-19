/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipSplineResamp.c    * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 20/01/99
 * VERSION     : 1.4                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
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

#include <vip/TivoliIO.h>
#include <vip/volume.h>
#include <vip/util.h>
#include <vip/depla.h>
#include <vip/splineresamp.h>

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

  Roi        roi;
  int usefloat = VFALSE;
  char *input = NULL;
  char *template = NULL;
  char *templateheader = NULL;
  char *deplacement = NULL;
  int depid = VFALSE;
  int depwrite = VFALSE;
  char depfilename[VIP_NAME_MAXLEN] = "deplacement";
  char output[VIP_NAME_MAXLEN] = "splineresampled";
  int readlib, writelib;
  VipDeplacement *dep;
  int i;
  int degree = 3;
  int dimx=-1, dimy=-1, dimz=-1;
  float voxx=-1, voxy=-1, voxz=-1;
  VipDeplacement id;

  readlib = ANY_FORMAT;
  writelib = TIVOLI;

  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-input", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  input = argv[i];
	}
      else if (!strncmp (argv[i], "-theader", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  templateheader = argv[i];
	}
      else if (!strncmp (argv[i], "-sx", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  voxx = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-sy", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  voxy = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-sz", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  voxz = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-dx", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  dimx = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-did", 3)) 
	{
	  depid = VTRUE;
	}
      else if (!strncmp (argv[i], "-dwrite", 3)) 
	{
	  depwrite = VTRUE;
	}   
      else if (!strncmp (argv[i], "-dy", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  dimy = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-dz", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  dimz = atoi(argv[i]);
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
      else if (!strncmp (argv[i], "-order", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  degree = atoi(argv[i]);
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
	  else if(argv[i][0]=='a') readlib = ANY_FORMAT;
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

  if (input==NULL)
    {
      (void)fprintf(stderr,"input arg is required by VipSplineResamp\n");
      return(Usage());
    }
  if (template==NULL && templateheader==NULL && (dimx<0 || dimy<0 ||dimz<0 ||  voxx<0 || voxy<0 || voxz<0))
    {
      (void)fprintf(stderr,"some template arg is required by VipSplineResamp\n");
      return(Usage());
    }
  if (deplacement==NULL && depid==VFALSE)
    {
      (void)fprintf(stderr,"deplacement arg is required by VipSplineResamp\n");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
    }
  if (template!=NULL && VipTestImageFileExist(template)==PB)
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

  if(template!=NULL)
    {
      voltemplate = VipReadVolumeWithBorder(template,0);
      if (voltemplate==PB)
	{
	  VipPrintfError("Template reading Problem...");
	  return(VIP_CL_ERROR);
	}
      VipFreeVolumeData(voltemplate);
    }
  else if(templateheader!=NULL)
    {
      voltemplate = readVolumeDim(  templateheader,&roi);
      if (voltemplate==PB)
	{
	  VipPrintfError("Template reading Problem...");
	  return(VIP_CL_ERROR);
	}
    }
  else if (!(dimx<0 || dimy<0 ||dimz<0 ||  voxx<0 || voxy<0 || voxz<0))
    {
      voltemplate = VipDeclare3DVolumeStructure(dimx,dimy,dimz,voxx,voxy,voxz,S16BIT,"template",0);
      if (voltemplate==PB)
	{
	  VipPrintfError("Template creating Problem...");
	  return(VIP_CL_ERROR);
	}
    }
  else
    {
      VipPrintfError("Template is mandatory");
      return(Usage());
    }

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

  
  
  if(deplacement!=NULL)
    {
      dep = VipReadDeplacement(deplacement);
      if (dep==PB)
	{
	  VipPrintfError("Deplacement reading Problem...");
	  return(VIP_CL_ERROR);
	}
    }
  else
    {
      id.r.xx = 1.0;
      id.r.xy = 0.0;
      id.r.xz = 0.0;
      id.r.yx = 0.0;
      id.r.yy = 1.0;
      id.r.yz = 0.0;
      id.r.zx = 0.0;
      id.r.zy = 0.0;
      id.r.zz = 1.0;
      id.t.x = (mVipVolVoxSizeX(vol)-mVipVolVoxSizeX(voltemplate))/2.;
      id.t.y = (mVipVolVoxSizeY(vol)-mVipVolVoxSizeY(voltemplate))/2.;
      id.t.z = (mVipVolVoxSizeZ(vol)-mVipVolVoxSizeZ(voltemplate))/2.;
      dep = &id;
      if(depwrite==VTRUE) 
	{
	  strcpy(depfilename,input);
	  strcat(depfilename,"_TO_template");
	  VipWriteDeplacement(dep,depfilename);
	}
    }

  volresample = VipSplineResamp(vol,voltemplate,dep,degree);
  if (volresample==PB)
    {
      VipPrintfError("Problem in VipSplineResamp...");
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
  (void)fprintf(stderr,"Usage: VipSplineResamp\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        -t[emplate] { image name}\n");
  (void)fprintf(stderr,"        -th[eader] {template header name (GIS format)}\n");
  (void)fprintf(stderr,"        [-sx] {float:template vox size x}\n");
  (void)fprintf(stderr,"        [-sy] {float:template vox size y}\n");
  (void)fprintf(stderr,"        [-sz] {float:template vox size z}\n");
  (void)fprintf(stderr,"        [-dx] {int:template dim x}\n");
  (void)fprintf(stderr,"        [-dy] {int:template dim y}\n");
  (void)fprintf(stderr,"        [-dz] {int:template dim z}\n");
  (void)fprintf(stderr,"        [-f[loat]]\n");
  (void)fprintf(stderr,"        -d[eplacement] {deplacement file (rotation or affine}\n");
  (void)fprintf(stderr,"        -d[id] {set deplacement to identity}\n");
  (void)fprintf(stderr,"        -d[write] {write the deplacement file as input_TO_template}\n");
  (void)fprintf(stderr,"        [-or[der] {int: [0..7] (default:3)}]\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"splineresampled\")}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: a, v, s or t (default:any)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Performs a spline based resampling of input volume according to template geometry");

  (void)printf("\n");
  (void)printf("Usage: VipSplineResamp\n");
  (void)printf("        -i[nput] {image name}\n");
  (void)printf("        -t[emplate] {image name}\n");
  (void)printf("        -th[eader] {template header name (GIS format)}\n");
  (void)printf("        [-sx] {float:template vox size x}\n");
  (void)printf("        [-sy] {float:template vox size y}\n");
  (void)printf("        [-sz] {float:template vox size z}\n");
  (void)printf("        [-dx] {int:template dim x}\n");
  (void)printf("        [-dy] {int:template dim y}\n");
  (void)printf("        [-dz] {int:template dim z}\n");
  (void)printf("        [-f[loat]]\n");
  (void)printf("The command works with float input volume (reading and writing do the same)\n");
  (void)printf("        -d[eplacement] {deplacement file (rotation or affine}\n");
  (void)printf("        -d[id] {set deplacement to identity}\n");
  (void)printf("Warning, in that case, if the voxel size is different a translation is used\n");
  (void)printf("to correct for the fact that our coordinate origin is the first voxel center\n");
  (void)printf("        -d[write] {write the deplacement file as input_TO_template}\n");
  (void)printf("        [-or[der] {int: [0..7] (default:3)}]\n");
  (void)printf("Spline interpolation order:\n");
  (void)printf("0 : nearest neighbor\n");
  (void)printf("1 : linear\n");
  (void)printf("2 : quadratic\n");
  (void)printf("3 : cubic\n");
  (void)printf("4 : quartic\n");
  (void)printf("5 : quintic\n");
  (void)printf("6 : galactic\n");
  (void)printf("7 : intergalactic\n");
  (void)printf("        [-o[utput] {image name (default:\"splineresampled\")}]\n");
  (void)printf("        [-r[eadformat] {char: a, v, s or t (default:any)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)printf("Forces the writing of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-h[elp]\n");
  (void)printf("For more information:\n");
  (void)printf("http://bigwww.epfl.ch/thevenaz/interpolation/index.html\n");
  (void)printf("M. Unser, Splines: A Perfect Fit for Signal and Image Processing\n");
  (void)printf("IEEE Signal Processing Magazine, vol. 16, no. 6, pp. 22-38, November 1999\n"); 
  return(VIP_CL_ERROR);

}

/******************************************************/
