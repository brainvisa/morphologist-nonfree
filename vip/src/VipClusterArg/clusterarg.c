/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipClusterArg        * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 22/10/99
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sparc
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

#include <vip/volume.h>
#include <vip/util.h>
#include <vip/connex.h>
#include <vip/distmap.h>
#include <vip/morpho.h>
#include <vip/depla.h>
#include <vip/activations.h>
#include <vip/splineresamp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <math.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol);
  VIP_DEC_VOLUME(template);  
  VIP_DEC_VOLUME(vol16); 
  VIP_DEC_VOLUME(volresamp);

  char *input = NULL;
  char *templatename = NULL;
  char output[VIP_NAME_MAXLEN] = "cluster";
  int readlib;
  int size = 0;
  char stringconnectivity[256] = "18";
  int connectivity;
  VipDeplacement *depla = NULL;
  VipDeplacement allocated_depla;
  char *deplaname = NULL;
  char *normaname = NULL;
  float C1[3], C2[3];
  char meshflag = 'y';


  int i;

  readlib = VIDA;

  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-input", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  input = argv[i];
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-size", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  size = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-connectivity", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  strcpy(stringconnectivity,argv[i]);
	} 
      else if (!strncmp (argv[i], "-transfo", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  deplaname = argv[i];
	} 
      else if (!strncmp (argv[i], "-normalization", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  normaname = argv[i];
	} 
      else if (!strncmp (argv[i], "-Template", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  templatename = argv[i];
	}
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipClusterArg");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-mesh", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  if(argv[i][0]=='y') meshflag='y';
	  else if(argv[i][0]=='n') meshflag = 'n';
	  else
	    {
	      VipPrintfError("meshflag: y/n choice");
	      VipPrintfExit("(commandline)VipClusterArg");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp(argv[i], "-help",2)) Help();
      else Usage();
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      VipPrintfError("input  argument is required by VipClusterArg");
      Usage();
    }
  if ( VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }

  if(!strcmp(output,"cluster")) strcpy(output,input);

  if(deplaname!=NULL)
      {
	  if(templatename==NULL)
	      {
		  VipPrintfError("You have to specify a template with the transformation");
		  Usage();
	      }
	  depla = VipReadDeplacement(deplaname);
	  if(depla==NULL) return(VIP_CL_ERROR);
      }
	  
  connectivity = VipConvertStringToConnectivity(stringconnectivity);
  if(connectivity==PB)
    {
      VipPrintfError("This connectivity is unknown...");
      Usage();
    }

  printf("Reading cluster image %s...\n",input);
  if (readlib == TIVOLI)
      vol = VipReadTivoliVolumeWithBorder(input,0);
  else
      vol = VipReadVolumeWithBorder(input,0);
  
  if(vol==NULL) return(VIP_CL_ERROR);

  if(!VipTestType(vol,S16BIT))
      {
	  vol16 = VipTypeConversionToS16BIT(vol,RAW_TYPE_CONVERSION);
	  if(!vol16) return(VIP_CL_ERROR);
	  vol16->shfj = vol->shfj;
	  vol->shfj = NULL;
	  VipFreeVolume(vol);
	  vol = vol16; 
      }

  VipSingleThreshold(vol,GREATER_THAN,1,BINARY_RESULT);

  if(size>0) VipConnexVolumeFilter(vol, connectivity, size, CONNEX_BINARY);

  if(templatename==NULL) templatename="/home/Panabase/demo/talairach/nanatRa";
  printf("\nReading template...\n");
  fflush(stdout);
  template = VipReadVolume(templatename);
  if(template==PB) return(VIP_CL_ERROR);


  if(depla==NULL)
    {
      depla = &allocated_depla;
      depla->r.xx = 1.0;
      depla->r.xy = 0.0;
      depla->r.xz = 0.0;
      depla->r.yy = 1.0;
      depla->r.yx = 0.0;
      depla->r.yz = 0.0;
      depla->r.zz = 1.0;
      depla->r.zx = 0.0;
      depla->r.zy = 0.0;
      if(vol->shfj!=NULL && template->shfj!=NULL)
	{
	  printf("From center: %f %f %f\n",vol->shfj->center.x,
		 vol->shfj->center.y,
		 vol->shfj->center.z);
	  printf("To   center: %f %f %f\n", template->shfj->center.x,
		 template->shfj->center.y,
		 template->shfj->center.z);
	  if(vol->shfj->spm_normalized==VTRUE && template->shfj->spm_normalized==VTRUE)
	    {
	      printf("Working with normalized data...\n");
	      /*flip x, y, z*/
	      C1[0]=(mVipVolSizeX(vol)-vol->shfj->center.x)*mVipVolVoxSizeX(vol);

	      C2[0]=(mVipVolSizeX(template)-template->shfj->center.x)*mVipVolVoxSizeX(template);
	  	  
	    }
	  else if(vol->shfj->spm_normalized==VFALSE && template->shfj->spm_normalized==VFALSE)
	    {
	      C1[0]=(vol->shfj->center.x-1)*mVipVolVoxSizeX(vol);   
	      C2[0]=(template->shfj->center.x-1)*mVipVolVoxSizeX(template);
	    }
	  else
	    {
	      VipPrintfError("Uncompatible normalization without deplacement provided");
	      exit(EXIT_FAILURE);
	    }
	  C1[1]=(mVipVolSizeY(vol)-vol->shfj->center.y)*mVipVolVoxSizeY(vol);
	  C1[2]=(mVipVolSizeZ(vol)-vol->shfj->center.z)*mVipVolVoxSizeZ(vol);
	  C2[1]=(mVipVolSizeY(template)-template->shfj->center.y)*mVipVolVoxSizeY(template);
	  C2[2]=(mVipVolSizeZ(template)-template->shfj->center.z)*mVipVolVoxSizeZ(template);
	  depla->t.x = C2[0]-C1[0];
	  depla->t.y = C2[1]-C1[1];
	  depla->t.z = C2[2]-C1[2];
	}
      else
	{
	  depla->t.x = - mVipVolVoxSizeX(template)/2.+mVipVolVoxSizeX(vol)/2.;
	  depla->t.y = - mVipVolVoxSizeY(template)/2.+mVipVolVoxSizeY(vol)/2.;
	  depla->t.z = - mVipVolVoxSizeZ(template)/2.+mVipVolVoxSizeZ(vol)/2.;
	}
    }
  printf("Translation: %f %f %f\n",depla->t.x,depla->t.y,depla->t.z);

  printf("Resampling volume for buckets dedicated to %s...\n", templatename);
  volresamp = VipSplineResamp(vol,template,depla,0);
  if(volresamp==PB) return(VIP_CL_ERROR);
  volresamp->shfj = template->shfj;
  template->shfj = NULL;
  VipFreeVolume(template);
  VipSingleThreshold(volresamp,GREATER_THAN,1,BINARY_RESULT);


  VipWriteClusterArg( volresamp, 0, connectivity, output,
		      normaname, meshflag);

  if (meshflag=='y')  VipComputeLabelledClusterTmtkTriangulation( volresamp, output);

  return 0;

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Create a graph of clusters for manipulation in Anatomist\n");
  (void)fprintf(stderr,"(contrast comparison, structural group analysis, etc)\n");
  (void)fprintf(stderr,"Usage: VipClusterArg\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-t[ransfo] {3D rotation/translation (default:none)}\n");
  (void)fprintf(stderr,"        [-n[ormalisation] {dep file (default:none)}\n");
  (void)fprintf(stderr,"        [-T[emplate] {volume (default:/home/Panabase/demo/talairach/nanatRa)}\n");
  (void)fprintf(stderr,"        [-o[utput] {arg name (default:\"input\")}]\n");
  (void)fprintf(stderr,"        [-s[ize] {int: min cluster size (default:0)}]\n");
  (void)fprintf(stderr,"        [-m[esh] {char: y/n (default:y)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  exit(-1);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Create a SPM cluster based attributed relational graph");
  (void)printf("\n");
  (void)printf("Example: VipClusterArg -i circuit_attention -s 5\n");
  (void)printf("Then load circuit_attention.arg in Anatomist\n");
  (void)printf("and put it in 2D and 3D visualization windows\n");
  (void)printf("Warning: put a normalized anatomical image in 2D window before clusters\n");
  (void)printf("Manipulate using a browser or a dedicated hierarchy\n");
  (void)printf("\n");
  (void)printf("        -i[nput] {image name}\n");
  (void)printf("You can get it from SPM during in the RESULT section from the WRITE button\n");
  (void)printf("\n");
  (void)printf("        [-t[ransfo] {3D rotation/translation (default:none)}\n");
  (void)printf("If you are not satisfied with the link between the contrast image\n");
  (void)printf("and the anatomical image that is computed from SPM center\n");
  (void)printf("you can provide your own translation (perhaps modifying\n");
  (void)printf("slightly the printed one...)\n");
  (void)printf("The transfo file format is the following:\n");
  (void)printf("tx ty tz\n");
  (void)printf("1. 0. 0.\n");
  (void)printf("0. 1. 0.\n");
  (void)printf("0. 0. 1.\n");
  (void)printf("If you are a wizard, you can even add a rotation matrix...\n");
  (void)printf("My advice relative to translation sign:\n");
  (void)printf("try first with large translation. Indeed according\n");
  (void)printf("to normalized or non normalized situation, image may be variously flipped\n");
  (void)printf("\n");
  (void)printf("        [-n[ormalisation] {ascii file (default:none)}\n");
  (void)printf("This flag is used to get Talairach coordinate when playing\n");
  (void)printf("with non normalized data.\n");
  (void)printf("This file is an ASCII/Anatomist version of SPM anatSXXXX_sn3d.mat\n");
  (void)printf("You get it with the command:\n");
  (void)printf("AnaImportSPMsn3dFile <SXXXX_002>\n");
  (void)printf("        [-T[emplate] {volume (default:/home/Panabase/demo/talairach/nanatRa)}\n");
  printf("Usually the T1-weighted anatomical volume on which you will visualize the clusters in 2D\n");
  printf("This volume gives image size and resolution\n");
  (void)printf("        [-o[utput] {arg name (default:\"input\")}]\n");
  (void)printf("        [-s[ize] {int: min cluster size in 18-connectivity (default:0)}]\n");
  (void)printf("        [-m[esh] {char: y/n (default:y)}]\n");
  printf("Trigger the mesh generation\n");
  printf("With large clusters, this can be very expensive\n");
  printf("In most situation, you can work only with voxels\n");
  printf("but Anatomist takes some time to yield the first visualization\n");
  (void)printf("        [-r[eadformat] {char: v, s or t (default:any)}]\n");
  (void)printf("        [-h[elp]\n");
  exit(-1);
}

/******************************************************/
