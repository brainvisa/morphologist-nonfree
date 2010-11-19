/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipGetBrain.c   * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 08/01/98
 * VERSION     : 1.1                  * REVISION :
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


#include "vip/histo.h"
#include "vip/brain.h"
#include "vip/talairach.h"
#include "vip/bucket.h"
#include "vip/distmap.h"
#include "vip/connex.h"
/**/
#include <vip/deriche.h>
#include <vip/gaussian.h>

/*----------------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
static int Usage();
static int Help();
static int VipCheckBrainMask( Volume* vol );
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{     
  char *ridgename = NULL;
  char *variancename = NULL;
  char *edgesname = NULL;
  Volume *ridge=NULL;
  Volume *variance = NULL;
  Volume *edges = NULL;
  char *input = NULL;
  int readlib, writelib;
  float dscale = 0.5;
  int i;
  Volume *vol=NULL, *brain=NULL, *converter=NULL, *vol2=NULL;
  char analyse = 'y';
  int track = 5;
  float berosion = 2.1;
  float bdilation = 2.6;
  int bdilationflag = VFALSE;
  float werosion = 2.1;
  float brecover = 3.;
  float wrecover = 30.;
  int vthreshold = 14;
  VipT1HistoAnalysis *ana;
  char mode = 'S';
  char brainname[1024];
  int brainflag = VFALSE;
  char brainwrite = 'y';
  char closedname[256];
  int closedflag = VFALSE;
  int gmean = -123;
  int gsigma = -123;
  int wmean = -123;
  int wsigma = -123;
  int debugflag = VFALSE;
  int Tlow = -1;
  int Thigh = -1;
  char color = 'b';
  char close = 'n';
  char fillwhite = 'y';
  float closingsize = 10.;
  char stat = 'n';
  char statfilename[1024];
  char histofilename[256]="";
  FILE *statfile;
  int flag8bit = VFALSE; 
  int niterations = 1; 
  int Last = 3;
  int First = 1;
  char pathoname[256]="";
  Volume *patho = NULL;
  int xCA=0, yCA=0, zCA=0; 
  int xCP=0, yCP=0, zCP=0;
  int xP=0, yP=0, zP=0;
  char point_filename[VIP_NAME_MAXLEN]="";
  VipTalairach tal, *talptr=NULL, *coord=NULL;
  int talset = VFALSE;
  int layer = 0;
  char layeronly = 'n';
  /**/
  VipHisto *histo;
  char histoname[1024];
  char stripped_input[1024];
  char *root1, *root2;
  Volume *var = NULL;
  int Seuil_Gray_White = 0;
  /**/

  readlib = ANY_FORMAT;
  writelib = TIVOLI;
 
  strcpy(brainname,"brain_");
  strcpy(closedname,"closed_");

 /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-input", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  input = argv[i];
	}
     else if (!strncmp (argv[i], "-bname", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(brainname,argv[i]);
	  brainflag=VTRUE;
	} 
     else if (!strncmp (argv[i], "-Ridge", 2)) 
       {
         if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
         ridgename = argv[i];
       }
     else if (!strncmp (argv[i], "-Variance", 2)) 
       {
         if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
         variancename = argv[i];
       }
     else if (!strncmp (argv[i], "-Edges", 2)) 
       {
         if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
         edgesname = argv[i];
       }
     else if (!strncmp (argv[i], "-hname", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(histofilename,argv[i]);
	} 
     else if (!strncmp (argv[i], "-patho", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(pathoname,argv[i]);
	} 
      else if (!strncmp (argv[i], "-Cname", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(closedname,argv[i]);
	  closedflag=VTRUE;
	}
     else if (!strncmp (argv[i], "-track", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  track = atoi(argv[i]);
	}
     else if (!strncmp (argv[i], "-Tlow", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  Tlow = atoi(argv[i]);
	}
     else if (!strncmp (argv[i], "-Thigh", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  Thigh = atoi(argv[i]);
	}
     else if (!strncmp (argv[i], "-niter", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  niterations = atoi(argv[i]);
	}
     else if (!strncmp (argv[i], "-First", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  First = atoi(argv[i]);
	}
     else if (!strncmp (argv[i], "-Last", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  Last = atoi(argv[i]);
	}
     else if (!strncmp (argv[i], "-gmean", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  gmean = atoi(argv[i]);
	}
     else if (!strncmp (argv[i], "-gsigma", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  gsigma = atoi(argv[i]);
	}
     else if (!strncmp (argv[i], "-wmean", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  wmean = atoi(argv[i]);
	}
     else if (!strncmp (argv[i], "-wsigma", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  wsigma = atoi(argv[i]);
	}
     else if (!strncmp (argv[i], "-berosion", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  berosion = atof(argv[i]);
	}
     else if (!strncmp (argv[i], "-bdilation", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  bdilation = atof(argv[i]);
	  bdilationflag = VTRUE;
	}
     else if (!strncmp (argv[i], "-werosion", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  werosion = atof(argv[i]);
	}
     else if (!strncmp (argv[i], "-brecover", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  brecover = atof(argv[i]);
	}
    else if (!strncmp (argv[i], "-brain", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='n') brainwrite = 'n';
	  else if(argv[i][0]=='y') brainwrite = 'y';
	  else
	    {
	      VipPrintfError("brain option is a y/n switch");
	      VipPrintfExit("(commandline)VipGetBrain");
	      return(VIP_CL_ERROR);
	    }
	}
     else if (!strncmp (argv[i], "-wrecover", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  wrecover = atof(argv[i]);
	}
     else if (!strncmp (argv[i], "-vthreshold", 3))
     {
         if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
         vthreshold = atof(argv[i]);
     }
     else if (!strncmp (argv[i], "-mode", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  else if(argv[i][0]=='s') mode = 's';
	  else if(argv[i][0]=='S') mode = 'S';
	  else if(argv[i][0]=='R') mode = 'R';
	  else if(argv[i][0]=='f') mode = 'f';
	  else if(argv[i][0]=='5') mode = '5';
	  else if(argv[i][0]=='v') mode = 'v';
	  else if(argv[i][0]=='V') mode = 'V';
	  else if(argv[i][0]=='r') mode = 'r';
	  else
	    {
	      VipPrintfError("This mode is unknown");
	      VipPrintfExit("(commandline)VipGetBrain");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-debug", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='n') debugflag = VFALSE;
	  else if(argv[i][0]=='y') debugflag = VTRUE;
	  else
	    {
	      VipPrintfError("debug option is a y/n switch");
	      VipPrintfExit("(commandline)VipGetBrain");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-Csize", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  closingsize = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-Close", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='n') close = 'n';
	  else if(argv[i][0]=='y') close = 'y';
	  else
	    {
	      VipPrintfError("close option is a y/n switch");
	      VipPrintfExit("(commandline)VipGetBrain");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-fill", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='n') fillwhite = 'n';
	  else if(argv[i][0]=='y') fillwhite = 'y';
	  else
	    {
	      VipPrintfError("fill option is a y/n switch");
	      VipPrintfExit("(commandline)VipGetBrain");
	      return(VIP_CL_ERROR);
	    }
	}
       else if (!strncmp (argv[i], "-lonly", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='n') layeronly = 'n';
	  else if(argv[i][0]=='y') { layeronly = 'y'; layer=1; }
	  else
	    {
	      VipPrintfError("lonly option is a y/n switch");
	      VipPrintfExit("(commandline)VipGetBrain");
	      return(VIP_CL_ERROR);
	    }
	}
        else if (!strncmp (argv[i], "-layer", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  layer = atoi(argv[i]);

	  if(layer<0 || layer>5) 
	    {
	      VipPrintfError("layer option: 0 to 5 iterations");
	      VipPrintfExit("(commandline)VipGetBrain");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-color", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='b') color = 'b';
	  else if(argv[i][0]=='g') color = 'g';
	  else
	    {
	      VipPrintfError("color option is a g/b switch");
	      VipPrintfExit("(commandline)VipGetBrain");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-Points", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(point_filename,argv[i]);
	}   
      else if (!strncmp (argv[i], "-xA", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  xCA = atoi(argv[i]);
	  talset = VTRUE;
	}
      else if (!strncmp (argv[i], "-yA", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  yCA = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-zA", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  zCA = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-xP", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  xCP = atoi(argv[i]);
	  talset = VTRUE;
	}
      else if (!strncmp (argv[i], "-yP", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  yCP = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-zP", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  zCP = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-xI", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  xP = atoi(argv[i]);
	  talset = VTRUE;
	}
      else if (!strncmp (argv[i], "-yI", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  yP = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-zI", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  zP = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-dscale", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  dscale = atof(argv[i]);
	}  
      else if (!strncmp (argv[i], "-analyse", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='n') analyse = 'n';
	  else if(argv[i][0]=='y') analyse = 'y';
	  else if(argv[i][0]=='r') analyse = 'r';
	  else
	    {
	      VipPrintfError("analyse option is a y/n/r switch");
	      VipPrintfExit("(commandline)VipGetBrain");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-stat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='n') stat = 'n';
	  else if(argv[i][0]=='y') stat = 'y';
	  else
	    {
	      VipPrintfError("stat option is a y/n switch");
	      VipPrintfExit("(commandline)VipGetBrain");
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
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipGetBrain");
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
	      VipPrintfExit("(commandline)VipGetBrain");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/


  if(analyse == 'n' && mode!='f' && (gmean<=0 || gsigma<=0 || wmean<=0 || wsigma<=0))
    {
      VipPrintfError("You have to give gray and white matter statistics if you bypass histogram analysis");
      return(VIP_CL_ERROR);
    }

  
  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipGetBrain");
      return(Usage());
    }
  if(dscale>0.5)
    {
      VipPrintfError("The maximum dscale allowed for stability is 0.5");
      return(Usage());
    }

  if (VipTestImageFileExist(input)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
    }
  else
    {
      printf("Reading volume...\n");
      if(readlib==SPM)
	  {
	      vol = VipReadSPMVolumeWithBorder(input,1);
	  }
      else if (readlib==TIVOLI)
	  {
	      vol = VipReadTivoliVolumeWithBorder(input,1);/*
	      VipFreeVolume(vol);
	      test = malloc(10000);
	      vol = VipReadTivoliVolumeWithBorder(input,1);*/
	  }
      else if (readlib == VIDA)
	vol = VipReadVidaVolumeWithBorder(input,1);
      else
	  {
	      vol = VipReadVolumeWithBorder(input,1);
	  }
      if(vol==PB) return(VIP_CL_ERROR);
    }

  if(bdilationflag == VFALSE) bdilation = berosion + 0.5;
  
  
  if(mVipVolType(vol)==U8BIT)
      {
	  converter = VipTypeConversionToS16BIT(vol,RAW_TYPE_CONVERSION);
	  if(converter==PB) return(VIP_CL_ERROR);
	  flag8bit = VTRUE;
	  VipFreeVolume(vol);
	  vol = converter;
      }
  
  
  if(First!=0 || Last!=0)
      {
	  printf("Warning (especially SPM user)...\n");
	  printf("For robustness, putting %d first slices and %d last slices to zero\n",First,Last);
	  printf("The goal is to get rid of bad effects of MR ghost on morphological opening\n");
	  printf("If you are using SPM normalized images, put First and Last flag to zero\n");
      }
  for(i=0;i<First;i++)
      VipPutOneSliceTwoZero(vol,i);
  for(i=0;i<Last;i++)
      VipPutOneSliceTwoZero(vol,mVipVolSizeZ(vol)-i-1);
   
  if(brainwrite=='n' && close=='n')
      {
	  VipPrintfError("No output image selected (brain or closed brain)");
	  return(Usage());
      }
  if(brainflag==VFALSE) strcat(brainname,input);
  if(closedflag==VFALSE) strcat(closedname,input);
  
  if (layeronly=='y') analyse='n';
  if(analyse=='y')
     {
       ana = VipGetT1HistoAnalysisCustomized(vol,track,dscale);
       if(ana==PB)
	   {
	       printf("Histogram analysis problem, sorry...\n");
	       return(VIP_CL_ERROR);
	   }
       if(ana->gray==PB && mode=='p') 
	 {
	     VipPrintfWarning("No gray/white distinction, switching to normal segmentation mode");
	   mode='n';
	 }
       if(stat=='y')
	   {
	       if(ana==PB || ana->gray==PB)
		{
		    VipPrintfError("No gray/white distinction, can not create .stat file");
		    return(VIP_CL_ERROR);
		}
	       strcpy(statfilename,input);
	       strcat(statfilename,".stat");
	       statfile = fopen(statfilename,"w");
	       if(statfile==PB)
		   {
		       VipPrintfError("Can not write there");
		       return(VIP_CL_ERROR);
		   }
	       fprintf(statfile,"0 0\n%d %d\n%d %d\n",
		       ana->gray->mean, ana->gray->sigma,
		       ana->white->mean, ana->white->sigma);
	       fclose(statfile);
	   }
     } 
  else if(analyse=='r')
    {
      if(!strcmp(histofilename,""))
	 strcpy(histofilename,input);
      printf("Reading histogram analysis: %s\n", histofilename);
      ana = VipReadT1HistoAnalysis( histofilename);
      if(ana==PB)
	   {
	       printf("Histogram analysis read problem (file %s), sorry...\n",
		      histofilename);
	       return(VIP_CL_ERROR);
	   }
      if(ana->gray)
	{
	  ana->gray->left_sigma = ana->gray->sigma;
	  ana->gray->right_sigma = ana->gray->sigma;
	}
      if(ana->white)
	{
	  ana->white->left_sigma = ana->white->sigma;
	  ana->white->right_sigma = ana->white->sigma;
	}
    }
  else if (layeronly=='n')
    {
        ana = VipCalloc(1,sizeof(VipT1HistoAnalysis),"(commandLine)(VipGetBrain");
	if(ana==PB) return(VIP_CL_ERROR);
	ana->gray = VipAllocSSObject();
	if(ana->gray==PB) return(VIP_CL_ERROR);
	ana->white = VipAllocSSObject();
	if(ana->white==PB) return(VIP_CL_ERROR);
    }
  
  if(layeronly=='n')
  {
  	if(gmean>0 && ana->gray) ana->gray->mean = gmean;
 	 if(wmean>0 && ana->white) ana->white->mean = wmean;
  	if(gsigma>0 && ana->gray)
    {
      ana->gray->left_sigma = gsigma;
      ana->gray->sigma = gsigma;
      ana->gray->right_sigma = gsigma;
    }
  	if(wsigma>0 && ana->white)
    {
      ana->white->left_sigma = wsigma;
      ana->white->sigma = wsigma;
      ana->white->right_sigma = wsigma;
    }
  

    if(mode=='f')
    {
      if(Tlow<=0) 
	{
	  if(ana && ana->gray)
	    {
	      if(ana->sequence==MRI_T1_SPGR && ana->partial_volume_effect==VFALSE)
		Tlow = ana->gray->mean - (int)(2.5*ana->gray->left_sigma);
	      else
		Tlow = ana->gray->mean - (2*ana->gray->left_sigma);
	    }
	  else if(ana && ana->brain)
	    {
		Tlow = ana->brain->mean - (int)(2.5*ana->brain->left_sigma);	     
	    }
	  else
	    {
	      VipPrintfError("No low threshold and no automatic histogram analysis...");
	      return(VIP_CL_ERROR);
	    }
	}
      if(Thigh<=0) 
	{
	  if(ana && ana->white)
	    {
		Thigh = ana->white->mean + (4*ana->white->left_sigma);	      
	    }
	  else if(ana && ana->brain)
	    {
		Thigh = ana->brain->mean + (4*ana->brain->left_sigma);	      
	    }
	  else
	    {
	      VipPrintfError("No high threshold and no automatic histogram analysis...");
	      return(VIP_CL_ERROR);
	    }
	}
    }
  
    if(strcmp(pathoname,""))
    {
      printf("Reading pathology mask...\n");
      patho = VipReadVolumeWithBorder(pathoname,1);
      if(patho==PB) return(VIP_CL_ERROR);
      if( VipMerge( vol, patho, VIP_MERGE_ALL_TO_ONE, 0, ana->white->mean )== PB) return(VIP_CL_ERROR);
      /*put to white rather than grey, because of meninge stuff*/
      VipFreeVolume(patho);
    }

    if(ana && (strcmp(point_filename,"")||talset==VTRUE))
    {
      vol2=NULL;
      if(GetCommissureCoordinates(vol, point_filename, &tal,
			      xCA, yCA, zCA, 
			      xCP, yCP, zCP, 
			      xP, yP, zP, talset)==PB) return(VIP_CL_ERROR);

      coord = &tal;
      xCA = (int)(coord->AC.x); yCA = (int)(coord->AC.y); zCA = (int)(coord->AC.z);
      xCP = (int)(coord->PC.x); yCP = (int)(coord->PC.y); zCP = (int)(coord->PC.z);
      VipComputeTalairachTransformation(vol,&tal);
      talptr = &tal;
    }

    if (ridgename!=NULL)
    {
      if(mode!='5')
        VipPrintfWarning("Ridge image useless without mode 5");
      else
        {
          ridge = VipReadVolume(ridgename);
          if(!ridge) return(VIP_CL_ERROR);
        }
    }
    if (variancename!=NULL)
    {
      if(mode!='V')
        VipPrintfWarning("Variance image useless without mode 2010");
      else
        {
          variance = VipReadVolumeWithBorder(variancename,1);
          if(!variance) return(VIP_CL_ERROR);
        }
    }
    if (edgesname!=NULL)
    {
      if(mode!='V')
        VipPrintfWarning("Edges image useless without mode 2010");
      else
        {
          edges = VipReadVolumeWithBorder(edgesname,1);
          if(!edges) return(VIP_CL_ERROR);
        }
    }

  switch(mode)
    {
    case 'V': if(VipGetBrain2010(vol,variance,edges,ana,NO,debugflag,berosion,vthreshold,niterations,xCP,yCA,yCP)==PB) return(VIP_CL_ERROR);
      break;    
    case '5': if(VipGetBrain2005(vol,ana,NO,debugflag,1.6,5,berosion,10,niterations,ridge)==PB) return(VIP_CL_ERROR);
      break;    
    case 'S': if(VipGetBrainStandard(vol,ana,NO,debugflag,berosion,brecover,niterations,VTRUE)==PB) return(VIP_CL_ERROR);
      break;    
    case 's': if(VipGetBrainStandard(vol,ana,NO,debugflag,berosion,brecover,niterations,VFALSE)==PB) return(VIP_CL_ERROR);
      break;
    case 'R': if(VipGetBrainCustomized(vol,ana,NO,debugflag,berosion,brecover,werosion,wrecover,niterations,talptr,VTRUE)==PB) return(VIP_CL_ERROR);
      break;    
    case 'r': if(VipGetBrainCustomized(vol,ana,NO,debugflag,berosion,brecover,werosion,wrecover,niterations,talptr,VFALSE)==PB) return(VIP_CL_ERROR);
      break;
    case 'f': if(VipGetBrainForRenderingFastCustomized(vol,NO,debugflag,berosion,bdilation,Tlow,Thigh)==PB) return(VIP_CL_ERROR);
      break;
    default:
      VipPrintfError("Not implemented yet\n");
      return(VIP_CL_ERROR);
    }
 }	
    
    /*2009 Try to add a hack to fill up some  partial volume voxels in order to get result stable to the variability
      of the histogram analysis*/

//    if (layer>0)
//    {
// 	printf("Reading volume once again for partial volume tuning...\n");
// 	vol2 = VipReadVolumeWithBorder(input,1);
//      if(layeronly=='y')
//      {
// 		vol = VipReadVolumeWithBorder(brainname,1); 
// 		VipWriteVolume(vol,"brain");
//         	VipWriteVolume(vol2,"nobias");
// 	}
// 	if(VipDilateInPartialVolumeFar(vol2, vol,layer)==PB) return(VIP_CL_ERROR);
//      VipSingleThreshold( vol, GREATER_OR_EQUAL_TO,  1, BINARY_RESULT );
//    }


   /**/
   strcpy(histoname, "/volatile/cfischer/Histo/adni/");
   root2 = input;
   root1 = input;
   while(root2!=NULL)
   {
	root2 = strstr(root1,"/");
	if(root2!=NULL) root1 = root2+1;
   }
   strcpy(stripped_input,root1);
   strcat(histoname, stripped_input);

   if (layer>0)
   {
	printf("Reading volume once again for partial volume tuning...\n");
	vol2 = VipReadVolumeWithBorder(input,1);

// 	var = VipReadVolumeWithBorder("/volatile/cfischer/base/prtocole1/AB070075/t1mri/default_acquisition/default_analysis/variance_AB070075.ima", 1);
	VipSingleThreshold( variance, LOWER_THAN, 20, BINARY_RESULT );
// 	VipExtedge(variance,EXTEDGE2D_ALL,SAME_VOLUME);
// 	VipConnexVolumeFilter( variance, CONNECTIVITY_6, 100, CONNEX_BINARY );
// 	VipDilation(variance,CHAMFER_BALL_2D,5);
	VipWriteVolume(variance,"skinlayerbis");
   }

//    histo = VipCreateHistogram(vol2, vol3, edges, mc);
//    if(histo==PB) printf("Erreur\n");
//    printf("Writing histogram\n");
//    if(VipWriteHisto(histo,histoname,WRITE_HISTO_ASCII)==PB)
//    VipPrintfWarning("I can not write the histogram but I am going further");
   /**/
 

  if(fillwhite=='y' && ana &&layeronly!='y')
    {
      if(vol2==NULL)
	{
	  printf("Reading volume once again for masking...\n");
	  vol2 = VipReadVolumeWithBorder(input,1);
	}
      if(vol2==PB) return(VIP_CL_ERROR);	 
      printf("Filling spurious cavities in white matter \n");
      fflush(stdout);
      VipSingleThreshold( vol, GREATER_OR_EQUAL_TO,  1, BINARY_RESULT );
      if(VipFillWhiteCavities(vol2,vol,ana,500)==PB) return(VIP_CL_ERROR);
    }
  if(vol2!=NULL)
    {
      VipFreeVolume(vol2);		    
      vol2 = NULL; 
    }

  /* check if the mask is empty or fills the whole volume */
  if( VipCheckBrainMask( vol ) == PB )
    return VIP_CL_ERROR;

  if(close=='y')
    {
      if(color=='g')
	{
	  brain = VipCopyVolume(vol,"copy");
	  if (brain==PB) return(VIP_CL_ERROR);
	  printf("Applying closing of size %3.1fmm to the brain segmentation\n",closingsize);
	  if (VipClosing(vol,CHAMFER_BALL_3D,closingsize)==PB) return(VIP_CL_ERROR);
	  if( VipMerge( vol, brain, VIP_MERGE_ONE_TO_ONE, 255, 128 )== PB) return(VIP_CL_ERROR);
	  VipFreeVolume(brain);
	  brain = NULL;
	  if(flag8bit==VTRUE)
	      {
		  converter = VipTypeConversionToU8BIT(vol,RAW_TYPE_CONVERSION);
		  if(converter==PB) return(VIP_CL_ERROR);
		  VipFreeVolume(vol);
		  vol = converter;
	      }
	  printf("Reading volume once again for masking...\n");
	  brain = VipReadVolumeWithBorder(input,1);
	  if(brain==PB) return(VIP_CL_ERROR);	 
	  VipMaskVolumeLevel(brain,vol,128);
	  VipMerge(brain,vol,VIP_MERGE_ONE_TO_ONE,255,1);
	  VipFreeVolume(vol);
	  vol = NULL;
	  printf("Writing %s...\n",closedname);
	  if(flag8bit==VTRUE || writelib==TIVOLI) VipWriteTivoliVolume( brain,closedname );
	  else if (writelib==SPM)
	      VipWriteSPMVolume( brain,closedname );
	  else VipWriteVolume( brain,closedname );
	  if(brainwrite=='y')
	      {
		  VipChangeIntLabel(brain,1,0);
		  printf("Writing %s...\n",brainname);
		  if(flag8bit==VTRUE|| writelib==TIVOLI) VipWriteTivoliVolume( brain,brainname );
		  else if (writelib==SPM)
		      VipWriteSPMVolume( brain,brainname );
		  else VipWriteVolume( brain,brainname );
	      }
	}
      else
	  {
	      if(brainwrite=='y')
		  {
		      printf("Writing %s...\n",brainname);
		      if(flag8bit==VTRUE)
			  {
			      converter = VipTypeConversionToU8BIT(vol,RAW_TYPE_CONVERSION);
			      if(converter==PB) return(VIP_CL_ERROR);
			      VipWriteTivoliVolume( converter,brainname );
			      VipFreeVolume(converter);
			      converter = NULL;
			  }
		      else if (writelib==SPM)
			  VipWriteSPMVolume( vol,brainname );	
		      else if (writelib==TIVOLI)
			  VipWriteTivoliVolume( vol,brainname );	
		      else VipWriteVolume( vol,brainname );
		  }
	      printf("Applying closing of size %3.1fmm to the brain segmentation\n",closingsize);
	      if (VipClosing(vol,CHAMFER_BALL_3D,closingsize)==PB) return(VIP_CL_ERROR);
	      printf("Writing %s...\n",closedname);
	      if(flag8bit==VTRUE)
		  {
		      converter = VipTypeConversionToU8BIT(vol,RAW_TYPE_CONVERSION);
		      if(converter==PB) return(VIP_CL_ERROR);
		      VipFreeVolume(vol);
		      vol = converter;
		      VipWriteTivoliVolume( vol,closedname );
		  }
	      else if (writelib==SPM)
		  VipWriteSPMVolume( vol,closedname );	
	      else if (writelib==TIVOLI)
		  VipWriteTivoliVolume( vol,closedname );
	      else VipWriteVolume( vol,closedname );
	  }  
    }
  else
      {
	  if(flag8bit==VTRUE)
	      {
		  converter = VipTypeConversionToU8BIT(vol,RAW_TYPE_CONVERSION);
		  if(converter==PB) return(VIP_CL_ERROR);
		  VipFreeVolume(vol);
		  vol = converter;
	      }
	  if(color=='g')
	      {
		  printf("Reading volume %s once again for masking...\n",input);
		  brain = VipReadVolumeWithBorder(input,1);
		  VipMaskVolume(brain,vol);			      	  
		  VipFreeVolume(vol);
		  vol = NULL;
		  
		  printf("Writing %s...\n",brainname);
		  if(flag8bit==VTRUE || writelib==TIVOLI) VipWriteTivoliVolume( brain,brainname );		     
		  else if (writelib==SPM)
		      VipWriteSPMVolume( brain,brainname );	
		  else VipWriteVolume( brain,brainname );
	      }
	  else
	      {
		  printf("Writing %s...\n",brainname);
		  if(flag8bit==VTRUE || writelib==TIVOLI) VipWriteTivoliVolume( vol,brainname );
		  else if (writelib==SPM)
		      VipWriteSPMVolume( vol,brainname );
		  else VipWriteVolume( vol,brainname );
	      }
      }
  
   

  
  return(0);
  
}


/*---------------------------------------------------------------------------*/

int VipCheckBrainMask( Volume* vol )
{
  VipOffsetStruct *vos;
  int ix, iy, iz; /*, it;*/
  short *ptr;
  unsigned long nonnull, size;
  float ratio;
  vos = VipGetOffsetStructure(vol);
  ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
  nonnull = 0;
  size = mVipVolSizeZ(vol) * mVipVolSizeY(vol) * mVipVolSizeX(vol);

  /*for( it = mVipVolSizeT(vol); it-- ; ) */     /* loop on volumes */
  for( iz = mVipVolSizeZ(vol); iz-- ; )        /* loop on slices */
    for( iy = mVipVolSizeY(vol); iy-- ; )    /* loop on lines */
      for( ix = mVipVolSizeX(vol); ix-- ; )
      {
        if( *ptr != 0 )
          nonnull++;
        ptr++;
      }
  ratio = ((double) nonnull) / size;
  if( ratio < 0.02 || ratio > 0.9 )
  {
    VipPrintfError( "Brain mask extraction failed: the result mask is empty "
        "or fills the whole volume\n" );
    return PB;
  }
  return OK;
}

/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipGetBrain\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-b[rain] {char y/n: write segmented brain, default:y}]\n");
  (void)fprintf(stderr,"        [-bn[ame] {brain image name, default:brain_input}]\n");
  (void)fprintf(stderr,"        [-L[ast] {int (default:3)}]\n");
  (void)fprintf(stderr,"        [-F[irst] {int (default:1)}]\n");
  (void)fprintf(stderr,"        [-R[idge] {White ridge image name (default: not used)}]\n");
  (void)fprintf(stderr,"        [-V[ariance] {Variance image name (default: not used)}]\n");
  (void)fprintf(stderr,"        [-E[dges] {Edges image name (default: not used)}]\n");
  (void)fprintf(stderr,"        [-m[ode] {char: V[2010], 5[2005], S[tandard], R[obust], s[tandard], r[obust], f[ast], default:S}]\n");
  (void)fprintf(stderr,"        [-n[iter] {int nb iteration of classif. regularization (def:1)}]\n"); 
  (void)fprintf(stderr,"        [-p[atho] {pathology binary mask, default:no}]\n");
  (void)fprintf(stderr,"        [-f[ill] {char y/n: fill white cavities ,default:y}]\n");
  (void)fprintf(stderr,"        [-l[ayer] {int nb of extension into partial volume (def:0, max 2)}]\n");
  (void)fprintf(stderr,"        [-lo[nly] {char y/n default:n, with yes do only layer processing (reading mask)]\n");
  (void)fprintf(stderr,"        [-c[olor] {char b/g: binary/graylevel, default:b}]\n");
  (void)fprintf(stderr,"        [-Cl[ose] {char y/n: close the brain ,default:n}]\n");
  (void)fprintf(stderr,"        [-Cs[ize] {float: closing size ,default:10mm}]\n");
  (void)fprintf(stderr,"        [-Cn[ame] {closed brain image name, default:closed_input}]\n");
  (void)fprintf(stderr,"        [-de[bug] {char y/n: debug mode, default:n}]\n");
  (void)fprintf(stderr,"        [-be[rosion] {float: brain erosion size (default:2.1mm)}]\n");
  (void)fprintf(stderr,"        [-bd[ilation] {float: brain dilation size (fast mode) (default:be+0.5mm)}]\n");  
  (void)fprintf(stderr,"        [-br[ecover] {float: brain recovering size (default:3.mm)}]\n"); 
  (void)fprintf(stderr,"        [-we[rosion] {float: white matter erosion size (default:2.1mm)}]\n"); 
  (void)fprintf(stderr,"        [-wr[ecover] {float: white matter  recovering size (default:30.mm)}]\n");
  (void)fprintf(stderr,"        [-a[nalyse] {char y/n/r: automatic histogram analysis, default:y}]\n");
  (void)fprintf(stderr,"        [-hn[ame] {histo analysis, default:input}]\n");
  (void)fprintf(stderr,"        [-gm[ean] {int : gray matter mean (default:automatic)}]\n"); 
  (void)fprintf(stderr,"        [-gs[igma] {int : gray matter standard deviation (default:automatic)}]\n"); 
  (void)fprintf(stderr,"        [-wm[ean] {int : white matter mean (default:automatic)}]\n"); 
  (void)fprintf(stderr,"        [-ws[igma] {int : white matter standard deviation (default:automatic)}]\n");
  (void)fprintf(stderr,"        [-Tl[ow] {int : low threshold (default:automatic)}]\n"); 
  (void)fprintf(stderr,"        [-Th[igh] {int : high threshold (default:automatic)}]\n");  
  (void)fprintf(stderr,"        [-d[scale] {float: (default:0.5)}]\n"); 
  (void)fprintf(stderr,"        [-t[rack] {int (default:5)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: a, v, s or t (default:any)}]\n");
  (void)fprintf(stderr,"        [-P[oints] {AC,PC,IH coord filename (*.tal) (default:not used)}]\n");
  (void)fprintf(stderr,"        [-xA[C] {(int [0-xsize[) AC X coordinate}]\n");
  (void)fprintf(stderr,"        [-yA[C] {(int [0-ysize[) AC Y coordinate}]\n");
  (void)fprintf(stderr,"        [-zA[C] {(int [0-zsize[) AC Z coordinate}]\n");
  (void)fprintf(stderr,"        [-xP[C] {(int [0-xsize[) PC X coordinate}]\n");
  (void)fprintf(stderr,"        [-yP[C] {(int [0-ysize[) PC Y coordinate}]\n");
  (void)fprintf(stderr,"        [-zP[C] {(int [0-zsize[) PC Z coordinate}]\n");
  (void)fprintf(stderr,"        [-xI[nterHem] {(int [0-xsize[) Inter-hemis. point X coord.}]\n");
  (void)fprintf(stderr,"        [-yI[nterHem] {(int [0-ysize[) Inter-hemis. point Y coord.}]\n");
  (void)fprintf(stderr,"        [-zI[nterHem] {(int [0-zsize[) Inter-hemis. point Z coord.}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Performs brain segmentation using T1-histogram scale space analysis and morphomath");
  (void)printf("Note that the goal is to get a mask of the brain to further segment hemispheres");
  (void)printf("Therefore, the cerebellum may be eroded or connected to some other tissues,");
  (void)printf("You may get rid of that problem with some non default values, I hope...");
  (void)printf("\n");
  (void)printf("Usage: VipGetBrain\n");
  (void)printf("        -i[nput] {image name}\n");
  (void)printf("        [-b[rain] {char y/n: write segmented brain, default:y}]\n");
  (void)printf("        [-bn[ame] {brain image name, default:brain_input}]\n");
  (void)printf("        [-R[idge] {White ridge image name (default: not used)}]\n");
  (void)printf("        [-V[ariance] {Variance image name (default: not used)}]\n");
  (void)printf("        [-E[dges] {Edges image name (default: not used)}]\n");
  (void)printf("        [-m[ode] {char: V[2010], 5[2005], S[tandard], R[obust], s[tandard], r[obust], f[ast], default:S}]\n");
  (void)printf("Standard: iterative approach to choose best erosion size...\n");
  (void)printf("standard: fixed erosion size...\n");
  (void)printf("Robust: try to deal with meninge, sinus, Gibbs effect + iterative erosion...\n");
  (void)printf("robust: try to deal with meninge, sinus, Gibbs effect + fixed erosion size...\n");
  (void)printf("fast: erosion/dilatation...\n");
  (void)printf("        [-p[atho] {pathology binary mask, default:no}]\n");
  (void)printf("add a mask of the lesion to the binary mask before erosion to get brain seed\n");
  (void)printf("This can solve bad morphological process\n");
  (void)printf("        [-n[iter] {int nb iteration of classif. regularization (def:1)}]\n"); 
  (void)printf("The binary classification is regularized using Markov random field and\n");
  (void)printf("ICM minimization (nb iter). This maily prevents the opening effect\n");
  (void)printf("of small cavities\n");
  (void)printf("fast: two thresholds, erosion, largest cc, dilation\n");
  (void)printf("normal: classification + regularisation, erosion, largest cc, dilation + recovering\n"); 
  (void)printf("patho: classification + regularisation, white matter segmentation, whole brain segm.\n");
  (void)printf("       this mode is dedicated to pathologies leading to high signal inside the brain\n");
  (void)printf("        [-c[olor] {char b/g: binary/graylevel, default:b}]\n");
  (void)printf("        [-L[ast] {int (default:3)}]\n");
  (void)printf("        [-F[irst] {int (default:1)}]\n");
  (void)printf("put Last last slices to zero and First first slices to zero\n");
  (void)printf("The aim is to get rid of MR ghost effect disturbing morphomath\n");
  (void)printf("BE carefull, the default is related to SHFJ ordering (the neck is in last slices)\n");
  (void)printf("WARNING: if your image is spm normalized, put this setting to zero!!!\n");
  (void)printf("        [-l[ayer] {int nb of extension into partial volume (def:0, max 2)}]\n");
  (void)printf("Add layers of voxels (6-neighbors) if it is improving contrast at mask contour\n");
  (void)printf("        [-lo[nly] {char y/n default:n, with yes do only layer processing (reading mask)]\n");
  (void)printf("This is for postprocessing of bad quality mask images\n");
  (void)printf("        [-f[ill] {char y/n: fill white cavities ,default:y}]\n");
  (void)printf("fill in 6-connected cavities of mask which are mainly above white matter mean\n");
  (void)printf("        [-C[lose] {char y/n: close the brain , default:n}]\n");
  (void)printf("if the 'g' color mode is active, the closing adds gray level 1\n");
  (void)printf("        [-C[size] {float: closing size , default:10mm}]\n");
  (void)printf("        [-Cn[ame] {closed brain image name, default:closed_input}]\n");
  (void)printf("        [-de[bug] {char y/n: debug mode, default:n}]\n");
  (void)printf("Write some images, WARNING: the white matter image should not be used as segmentation\n");
  (void)printf("    indeed, it is overregularised for robustness of the morphological processing\n");
  (void)printf("        [-be[rosion] {float: brain erosion size (default:2.mm)}]\n"); 
  (void)printf("It should be noted that erosion size is dedicated to standard use of the best mode}]\n"); 
  (void)printf("With the other procedures, you may have to increase it a bit to get more robustness,}]\n"); 
  (void)printf("It should also be noted that with a lower erosion size, you could improve}]\n"); 
  (void)printf("the final result with atrophic brains but with a lower robustness relatively to the whole process}]\n"); 
  (void)printf("        [-bd[ilation] {float: brain dilation size (fast mode) (default:be+0.5mm)}]\n");  
  (void)printf("        [-br[ecover] {float: brain recovering size (default:3.mm)}]\n"); 
  (void)printf("dilation of size (erosion+recover) followed by erosion of size (recover)\n"); 
  (void)printf("        [-we[rosion] {float: white matter erosion size (default:2.1mm)}]\n"); 
  (void)printf("        [-wr[ecover] {float: white matter  recovering size (default:30.mm)}]\n");
  (void)printf("dilation of size (erosion+recover) followed by erosion of size (recover)\n");
  (void)printf("This parameter are stranges because they are dedicated to the robust approach\n");
  (void)printf("where a failure like taking extra white matter tissue) is not problematic\n");
  (void)printf("while missing some white matter may be more problematic (but not catastrophic)\n");
  (void)printf("        [-a[nalyse] {char y/n/r: automatic histogram analysis, default:y}]\n"); 
  (void)printf("y: automatic, r: read input.his, n: use -gm,-gs,-wm,-ws\n");
  (void)printf("        [-hn[ame] {histo analysis, default:input )}]\n");
  (void)printf("read when -a r is set, this file has the .han extension (VipHistoAnalysis)\n");
  (void)printf("        [-gm[ean] {int : gray matter mean (default:automatic)}]\n"); 
  (void)printf("        [-gs[igma] {int : gray matter standard deviation (default:automatic)}]\n"); 
  (void)printf("        [-wm[ean] {int : white matter mean (default:automatic)}]\n"); 
  (void)printf("        [-ws[igma] {int : white matter standard deviation (default:automatic)}]\n"); 
  (void)printf("        [-Tl[ow] {int : low threshold (default:automatic)}]\n"); 
  (void)printf("        [-Th[igh] {int : high threshold (default:automatic)}]\n"); 
  (void)printf("For fast segmentation, brain tissues = ]Tlow,Thigh[ range\n"); 
  (void)printf("        [-d[scale] {float: (default:0.5)}]\n");
  (void)printf("Scale discretization (0.5 is the maximum step for stability)\n"); 
  (void)printf("        [-t[rack] {int (default:5)}]\n"); 
  (void)printf("Track singularities reaching at least this scale\n"); 
  (void)printf("        [-s[tat] {char y/n: write old input.stat file (default:'n')}]\n");
  (void)printf("This file is used by an old topologically deformable model stuff\n"); 
  (void)printf("dedicated to g/w interface detection, it should not be used by somebody else...\n"); 
  (void)printf("simple Talairach normalisation may be used to improve sinus deletion\n");
  (void)printf("        [-P[oints] {AC,PC,IH coord filename (*.tal) (default:not used)}]\n");
  (void)printf("Correct format for the commissure coordinate file toto.APC:\n");
  (void)printf("AC: 91 88 113\nPC: 91 115 109\nIH: 90 109 53\n");  
  (void)printf("        [-xA[C] {(int [0-xsize[) AC X coordinate}]\n");
  (void)printf("        [-yA[C] {(int [0-ysize[) AC Y coordinate}]\n");
  (void)printf("        [-zA[C] {(int [0-zsize[) AC Z coordinate}]\n");
  (void)printf("        [-xP[C] {(int [0-xsize[) PC X coordinate}]\n");
  (void)printf("        [-yP[C] {(int [0-ysize[) PC Y coordinate}]\n");
  (void)printf("        [-zP[C] {(int [0-zsize[) PC Z coordinate}]\n");
  (void)printf("        [-xI[nterHem] {(int [0-xsize[) Inter-hemis. point X coord.}]\n");
  (void)printf("        [-yI[nterHem] {(int [0-ysize[) Inter-hemis. point Y coord.}]\n");
  (void)printf("        [-zI[nterHem] {(int [0-zsize[) Inter-hemis. point Z coord.}]\n");
  (void)printf("        [-r[eadformat] {char: a, v, s or t (default:any)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, TIVOLI or ANY image file format\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)printf("Forces the writing of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-h[elp]\n");
  printf("More information in:\n"); 
  printf("Robust brain segmentation using histogram\n");
  printf("scale-space analysis and mathematical morphology\n");
  printf("J.-F. Mangin, O. Coulon, and V. Frouin\n");
  printf("MICCAI, MIT, LNCS-1496, Springer Verlag\n");
  printf("1230-1241, 1998\n");

  return(VIP_CL_ERROR);

}


