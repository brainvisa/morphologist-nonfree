/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipCylinder.c      * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 22/01/2000
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 ****************************************************************************
 *
 * DESCRIPTION : Electrode/ needle...
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


#include <vip/cylinder.h>

/*------------------------------------------------------------------*/
static void Usage();
static void Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  char *template = NULL;
  char output[VIP_NAME_MAXLEN] = "electrode";
  float x1 = 0.;
  float x2 = 0.;
  float y1 = 0.;
  float y2 = 0.;
  float z1 = 0.;
  float z2 = 0.;
  float radius = 0.5;
  float hatheight = -1;
  int i;
  float xvoxsize, yvoxsize,zvoxsize;



  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-template", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  template = argv[i];
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-x1", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  x1 = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-x2", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  x2 = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-radius", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  radius = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-hatheight", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  hatheight = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-y1", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  y1 = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-y2", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  y2 = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-z1", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  z1 = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-z2", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) Usage();
	  z2 = atof(argv[i]);
	}
      else if (!strncmp(argv[i], "-help",2)) Help();
      else Usage();
    }

  /*check that all required arguments have been given*/


  if  ( x1 == x2 && y1 == y2 && z1 == z2 )
    {
      VipPrintfError("The two points are identical!");
      Usage();
    }


  if (template!=NULL && VipTestImageFileExist(template)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",template);
      exit(0);
    }
 
 
  if(template!=NULL)
    {
      printf("Reading template %s...\n",template);
      vol = VipReadVolumeWithBorder(template,0);
      if(vol==PB) return(PB);
      xvoxsize = mVipVolVoxSizeX(vol);
      yvoxsize = mVipVolVoxSizeY(vol);
      zvoxsize = mVipVolVoxSizeZ(vol);
      VipFreeVolume(vol);
    }
  else
    {
      xvoxsize = 1.;
      yvoxsize = 1.;
      zvoxsize = 1.;
    }

  x1*=xvoxsize;
  x2*=xvoxsize;
  y1*=yvoxsize;
  y2*=yvoxsize;
  z1*=zvoxsize;
  z2*=zvoxsize;

  printf("Writing %s\n",output);
  if(hatheight<=0)
    {
      if(VipWriteCylinder(x1,y1,z1,x2,y2,z2,output,radius)==PB)
	Usage();
    }
  else
    {
      if(VipWriteHatCylinder(x1,y1,z1,x2,y2,z2,output,radius,hatheight)==PB)
	Usage();
    }

  return(EXIT_SUCCESS);

}
/*-----------------------------------------------------------------------------------------*/

static void Usage()
{
  (void)fprintf(stderr,"Usage: VipCylinder\n");
  (void)fprintf(stderr,"        [-t[emplate]] {coordinates in this image }\n");
  (void)fprintf(stderr,"        [-o[utput] {cylinder name (default:\"cylinders\")}]\n"); 
  (void)fprintf(stderr,"        -x1 {float }\n");
  (void)fprintf(stderr,"        -x2 {float }\n");
  (void)fprintf(stderr,"        -y1 {float }\n");
  (void)fprintf(stderr,"        -y2 {float }\n");
  (void)fprintf(stderr,"        -z1 {float }\n");
  (void)fprintf(stderr,"        -z2 {float }\n");
  (void)fprintf(stderr,"        [-r[adius] {float (default:0.5mm)}]\n");
  (void)fprintf(stderr,"        [-ha[theight] {float (default:0mm)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  exit(-1);

}
/*****************************************************/

static void Help()
{
 
  VipPrintfInfo("Create a triangulated  cylinder given by two extremities");
  printf("If a template is specified, extremities are specified as coordinates\n");
  printf("Otherwise, they are supposed to be in mm\n\n");
  (void)printf("Usage: VipCylinder\n");
  (void)printf("        [-t[emplate]] {coordinates in this image }\n");
  (void)printf("        [-o[utput] {cylinder name (default:\"cylinders\")}]\n"); 
  (void)printf("        -x1 {float }\n");
  (void)printf("        -x2 {float }\n");
  (void)printf("        -y1 {float }\n");
  (void)printf("        -y2 {float }\n");
  (void)printf("        -z1 {float }\n");
  (void)printf("        -z2 {float }\n");
  (void)printf("        [-r[adius] {float (default:0.5mm)}]\n");
  (void)printf("        [-ha[theight] {float (default:0mm)}]\n");
  (void)printf("        [-h[elp]\n");
  exit(-1);

}

/******************************************************/
