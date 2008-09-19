/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipSplitBrain.c      * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 25/09/2000
 * VERSION     : 1.7                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun
 ****************************************************************************
 *
 * DESCRIPTION : Split brain into hemispheres + cerebellum (+brain stem)
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
#include <vip/util.h>
#include <vip/connex.h>
#include <vip/morpho.h>
#include <vip/histo.h>
#include <vip/distmap.h>
#include <vip/talairach.h>
#include <vip/brain.h>

/*----------------------------------------------------------------------------*/
static int VipGetLabelsFromTemplate(
  Vip3DBucket_S16BIT *buck,
  Volume *vol,
  Volume *template,
  VipHisto *labels,
  VipTalairach *tal);
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int VipWriteCoordInTemplate(
  Vip3DBucket_S16BIT *buck,
  Volume *vol,
  Volume *template,
  int value,
  VipTalairach *tal);
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int VipWriteCoordOverZBucketInVolume_S16BIT(
  Vip3DBucket_S16BIT *buck,
  Volume *vol,
  int value,
  int Z);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static int GetCutAlongTheLineBucket(  Volume *vol, Volume *template,
						      VipTalairach *tal,
				      int wthreshold, Vip3DBucket_S16BIT **buck);
/*----------------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  VIP_DEC_VOLUME(voronoi); 
  VIP_DEC_VOLUME(classif); 
  VIP_DEC_VOLUME(seed); 
  VIP_DEC_VOLUME(template); 
  VIP_DEC_VOLUME(brain); 
  char *ridgename = NULL;
  Volume *ridge=NULL;
  Vip3DBucket_S16BIT *cutbuck=NULL;
  char template_name[VIP_NAME_MAXLEN] = "unknown";
  char histo_name[VIP_NAME_MAXLEN] = "";
  char *input = NULL;
  char brainname[VIP_NAME_MAXLEN] = "brain";
  char output[VIP_NAME_MAXLEN] = "voronoi";
  int borderwidth = 1;
  int readlib, writelib;
  int CutUse=VTRUE;
  int i, cc;
  int xCA=128, yCA=113, zCA=70; 
  int xCP=128, yCP=141, zCP=70;
  int xP=128, yP=60, zP=20;
  int wthreshold = -1;
  float erosion_size = 2.;
  float wtcoef = 2.;
  char analyse = 'y';
  VipT1HistoAnalysis *ana=NULL;
  Vip3DBucket_S16BIT *cclist=NULL, *ccptr=NULL;
  Vip3DBucket_S16BIT *lhemi=NULL, *rhemi=NULL, *cereb=NULL;
  Vip3DBucket_S16BIT *largest1=NULL, *largest2=NULL, *largest3=NULL, *largest4=NULL, *largest5=NULL;
  char point_filename[VIP_NAME_MAXLEN]="";
  int ccsize=500;
  int ncc;
  int seedOK=VFALSE;
  int talset = VFALSE;
  int Zover=-1;
  VipTalairach tal;
  int template_filled = VFALSE;
  int write_seed = VFALSE;
  VipHisto *labels=NULL;
  int write_label[1000];
  int itsOK;
  int labelmax;
  int TemplateUse = VTRUE;
  char seedname[1024]="temp+seed";
  int select_seed, total_cc;
  float bary = 0.75;
  int Coefset = VFALSE;
  char walgo = 'r';

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
     else if (!strncmp (argv[i], "-hname", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(histo_name,argv[i]);
	} 
      else if (!strncmp (argv[i], "-template", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(template_name,argv[i]);
	}
      else if (!strncmp (argv[i], "-Ridge", 2)) 
       {
         if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
         ridgename = argv[i];
       }
      else if (!strncmp (argv[i], "-brain", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(brainname,argv[i]);
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-wthreshold", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  wthreshold = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-wseed", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') write_seed = VTRUE;
	  else if(argv[i][0]=='n') write_seed = VFALSE;
	  else
	    {
	      VipPrintfError("wseed: y/n choice!");
	      VipPrintfExit("(commandline)VipSplitBrain");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-walgo", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  walgo = argv[i][0];
	  if(walgo!='c' && walgo!='r' && walgo!='b' && walgo!='t') 
	    {
	      VipPrintfError("wseed: r/c/b/t choice!");
	      VipPrintfExit("(commandline)VipSplitBrain");
	      return(VIP_CL_ERROR);
	    }
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
      else if (!strncmp (argv[i], "-TemplateUse", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') TemplateUse = VTRUE;
	  else if(argv[i][0]=='n') TemplateUse = VFALSE;
	  else
	    {
	      VipPrintfError("TemplateUse: y/n choice!");
	      VipPrintfExit("(commandline)VipSplitBrain");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-ccsize", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  ccsize = atoi(argv[i]);
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
      else if (!strncmp (argv[i], "-Zover", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  Zover = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-Coef", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  wtcoef = atof(argv[i]);
	  Coefset = VTRUE;
	}
     else if (!strncmp (argv[i], "-CutUse", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') CutUse = VTRUE;
	  else if(argv[i][0]=='n') CutUse = VFALSE;
	  else
	    {
	      VipPrintfError("CutUse: y/n choice!");
	      VipPrintfExit("(commandline)VipSplitBrain");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-Bary", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  bary = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-erosion", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  erosion_size = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='s') readlib = SPM;
	  else if(argv[i][0]=='a') readlib = ANY_FORMAT;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipGetHead");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') writelib = TIVOLI;
	  else if(argv[i][0]=='s') writelib = SPM;
	  else if(argv[i][0]=='v') writelib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipSplitBrain");
	      return(VIP_CL_ERROR);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/
  

  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipSplitBrain");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
    }
  if (brainname==NULL)
    {
      VipPrintfError("input arg is required by VipSplitBrain");
      return(Usage());
    }
  if (VipTestImageFileExist(brainname)==PB)
    {
      (void)fprintf(stderr,"Can not open this brain image: %s\n",brainname);
      return(VIP_CL_ERROR);
    }

  if(Zover==-1) Zover = zCA+10;

  printf("Reading %s...\n",input);
  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,borderwidth);
  else if (readlib == SPM)
    vol = VipReadSPMVolumeWithBorder(input,borderwidth);
  else if (readlib == VIDA)
    vol = VipReadVidaVolumeWithBorder(input,borderwidth); 
  else
    vol = VipReadVolumeWithBorder(input,borderwidth);

  if(vol==NULL) return(VIP_CL_ERROR);

  printf("Reading %s...\n",brainname);
  if (readlib == TIVOLI)
    brain = VipReadTivoliVolumeWithBorder(brainname,borderwidth);
  else if (readlib == SPM)
    brain = VipReadSPMVolumeWithBorder(brainname,borderwidth);
  else if (readlib == VIDA)
    brain = VipReadVidaVolumeWithBorder(brainname,borderwidth); 
  else
    brain = VipReadVolumeWithBorder(brainname,borderwidth);

  if(brain==NULL) return(VIP_CL_ERROR);

  /*computing interhemispheric plane and Talairach stuff*/

  if(TemplateUse==VTRUE)
    {
      printf("Reading voronoi template %s...\n",template_name);
      template = VipReadVolume(template_name);
      if(template!=PB)
	{
	  template_filled = VTRUE;
	  labels = VipCreateHisto(0,VipGetVolumeMax(template));
	  if(!labels) return(VIP_CL_ERROR);
	}
    }

  if(GetCommissureCoordinates(brain, point_filename, &tal,
			      xCA, yCA, zCA, 
			      xCP, yCP, zCP, 
			      xP, yP, zP, talset)==PB) return(VIP_CL_ERROR);

  VipComputeTalairachTransformation(brain,&tal);
  /*VipWriteTalairachRefFile(input,&tal);*/
  /*
  VipPrintTalairach(&tal);
  */
  /*read or compute HistoAnalysis*/

  if(analyse=='y')
    {
      ana = VipGetT1HistoAnalysisCustomized(vol,5,0.5);
      if(ana==PB)
	{
	  printf("Histogram analysis problem, sorry...\n");
	  return(VIP_CL_ERROR);
	}
    }
  else if(analyse=='r')
    {
      if(!strcmp(histo_name,""))
	 strcpy(histo_name,input);
      printf("Reading histogram analysis: %s\n", histo_name);
      ana = VipReadT1HistoAnalysis( histo_name);
      if(ana==PB)
	{
	  printf("Histogram analysis read problem (file %s), sorry...\n",
		 histo_name);
	  return(VIP_CL_ERROR);
	}
    }
  if(wthreshold==-1)
    {
      if(ana==NULL)
	{
	  VipPrintfError("I need either white threshold or histogram analysis!");
	  return(VIP_CL_ERROR);
	}
      if (Coefset!=VTRUE && walgo=='b')
	{
	  wthreshold = (int)((1-bary) * ana->gray->mean + bary *ana->white->mean);
	  printf("Bary average between (%d-%f-%d)\n",
		 ana->gray->mean, bary, ana->white->mean );
	}
      else if (walgo=='c')
	{
	  wthreshold = ana->white->mean - wtcoef * ana->white->sigma;
	  printf("white matter mean: %d - %f * standard deviation: %d\n",ana->white->mean,
		 wtcoef, ana->white->sigma);
	}
    }
  if (walgo!='r')
    {
      printf("Used threshold for white matter: %d\n", wthreshold);
      fflush(stdout);
    }

  if(CutUse==VTRUE)
    {
      if(strcmp(point_filename,"") ||talset==VTRUE)
	{
	  if(GetCutAlongTheLineBucket(  vol, template,
					&tal, wthreshold, &cutbuck)==PB)
	    {
	      fprintf(stderr,"Problem with GetCutAlongTheLineBucket");
	      return(VIP_CL_ERROR);
	    }
	}
    }


  if(VipMaskVolume(vol,brain)==PB) return(VIP_CL_ERROR);
  VipFreeVolume(brain);
  brain = VipCopyVolume(vol,"brain");
  if(brain==PB) return(VIP_CL_ERROR);

  if(walgo=='r')
    {
      printf("Regularized classification\n");
      /*fucking stupid way I deal with .han file, I should modify the lib,
	but I wan that to work in Marseille NOW*/
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
      classif = VipGrayWhiteClassificationRegularisationForRobust( vol, ana, VTRUE);
      if(classif==PB) return(VIP_CL_ERROR);
      VipSetVolumeLevel(vol,0);
      VipMerge(vol,classif,VIP_MERGE_ONE_TO_ONE,WHITE_LABEL,255);
      VipFreeVolume(classif);
    }
  else
    {
      VipDoubleThreshold( vol, VIP_BETWEEN_OR_EQUAL_TO,wthreshold, ana->white->mean + 2 * ana->white->sigma, BINARY_RESULT );
      /*2005 if(VipSingleThreshold(vol, GREATER_THAN, wthreshold, BINARY_RESULT)==PB) return(VIP_CL_ERROR);*/
    }

  /* VipWriteTivoliVolume(vol,"seuil"); */
  printf("Keep only the largest 6 connected component...\n");
  if(VipConnexVolumeFilter(vol,CONNECTIVITY_6,-1,CONNEX_BINARY)==PB)
    return(VIP_CL_ERROR);


  printf("Opening of size 1.6mm ...\n");
  if(VipOpening(vol,CHAMFER_BALL_3D,1.6)==PB) return(VIP_CL_ERROR);
  printf("Closing of size 1.6mm ...\n");
  if(VipClosing(vol,CHAMFER_BALL_3D,1.6)==PB) return(VIP_CL_ERROR);
  /*
  printf("Opening of size 1 relatively to 18-connectivity...\n");
  if(VipConnectivityChamferOpening(vol,1,CONNECTIVITY_18,FRONT_PROPAGATION)==PB) return(VIP_CL_ERROR);
  printf("Closing of size 1 relatively to 18-connectivity...\n");
  if(VipConnectivityChamferClosing(vol,1,CONNECTIVITY_18,FRONT_PROPAGATION)==PB) return(VIP_CL_ERROR);
  */
  /*
  printf("Closing of size 1 relatively to 18-connectivity...\n");
  if(VipConnectivityChamferClosing(vol,1,CONNECTIVITY_18,FRONT_PROPAGATION)==PB) return(VIP_CL_ERROR);
  */
  if (cutbuck!=NULL)
    {
      printf("AutoDeleting %d white matter points from interhemispheric plane and pons\n",
	     cutbuck->n_points);
      VipWriteCoordBucketInVolume(cutbuck,vol,0);
      if(write_seed==VTRUE)  VipWriteTivoliVolume(vol,"cut");
    }
  printf("Let us try to split this brain correctly...\n");
  while(seedOK==VFALSE && (erosion_size>=2) && (erosion_size<10))
    {
      if(erosion_size<2)
	{
	  VipPrintfError("too small erosion");
	}
      if((seed = VipCopyVolume(vol,"seed")) ==PB) return(VIP_CL_ERROR);

      printf("erosion of size %f mm\n",erosion_size);
      if(VipErosion(seed,CHAMFER_BALL_3D,erosion_size)==PB) return(VIP_CL_ERROR);

      cclist = VipGetFiltered3DConnex(seed, CONNECTIVITY_26, ccsize);
      ccptr = cclist;
      ncc = 0;
      while(ccptr!=NULL)
	{
	  printf("cc(%d) highest point: x:%d, y:%d, z:%d\n",ccptr->size,ccptr->data[0].x,ccptr->data[0].y,ccptr->data[0].z);
	  ncc++;
	  ccptr = ccptr->next;
	}

      if(ncc<3)
	{
	  printf("Not enough cc\n");
	  printf("-------------\n");
	  erosion_size += 0.5;
	}
      else
	{	  
	  if(template_filled == VTRUE)
	    {
	      ccptr = cclist;
	      cc = 0;
	      select_seed = 0;
	      total_cc = 0;
	      while(ccptr!=NULL)
		{
		  VipGetLabelsFromTemplate(ccptr,seed,template,labels,&tal);
		  printf("cc: %d points, back: %d%%, right: %d%%, left: %d%%, cereb: %d%%\n",ccptr->n_points,
			 (int)(100*mVipHistoVal(labels,0))/ccptr->n_points,
			 (int)(100*mVipHistoVal(labels,1))/ccptr->n_points,
			 (int)(100*mVipHistoVal(labels,2))/ccptr->n_points,
			 (int)(100*mVipHistoVal(labels,3))/ccptr->n_points);
		  labelmax = 0;
		  for(i=0;i<=mVipHistoRangeMax(labels);i++)
		    {
		      if (mVipHistoVal(labels,i)>mVipHistoVal(labels,labelmax)) labelmax=i;
		    }
		  if (
		      ( ((100*mVipHistoVal(labels,labelmax))/ccptr->n_points) > 95 )
		      ||
			(((100*mVipHistoVal(labels,1))/ccptr->n_points
			  +(100*mVipHistoVal(labels,2))/ccptr->n_points)<5
			  && (labelmax==3 || 
			      (labelmax==0 && ((100*mVipHistoVal(labels,3))/ccptr->n_points)>20))
			      ))	
		    {
		      if (labelmax!=0) write_label[cc] = labelmax;
		      else write_label[cc]=3;
		      select_seed += ccptr->n_points;
		    }
		  else write_label[cc] = 0;
		  total_cc += ccptr->n_points;
		  ccptr=ccptr->next;
		  cc++;
		}
	      
	      for (i=0;i<=mVipHistoRangeMax(labels);i++)
		{
		  mVipHistoVal(labels,i) = 0;
		}
	      printf("color choice (R=1,L=2,C=3) : ");
	      for(cc=0;cc<ncc;cc++)
		{
		  mVipHistoVal(labels,write_label[cc]) = 1;
		  printf("%d ",write_label[cc]);
		}
	      printf("\n");
	      itsOK = 1;
	      for(i=1;i<=mVipHistoRangeMax(labels);i++) itsOK *=mVipHistoVal(labels,i);
	      if(itsOK == 1 && ((float)select_seed/(float)total_cc)>0.9)
		{
		  seedOK = VTRUE;
		}
	      else
		{
		  printf("--------------------Bad split-------------------\n");
		  printf("Perhaps CA, CP and InterHemispheric point are bad?\n (if you have not set it)\n");
		  printf("As an alternative, you can disable\n the use of the voronoi template (-T n)\n");
		  printf("-----Perhaps you can wait for the next one------\n");
		}
	      if(write_seed==VTRUE && TemplateUse==VTRUE)
		{
		  ccptr = cclist;
		  cc = 0;
		  while(ccptr!=NULL)
		    {
		      VipWriteCoordInTemplate(ccptr,brain,
					      template,
					      write_label[cc]*10,&tal);

		      ccptr = ccptr->next;
		      cc++;
		    }
		  strcat(seedname,"+");
		  VipWriteTivoliVolume(template,seedname);
		  printf("Reading voronoi template again %s...\n",template_name);
		  VipFreeVolume(template);
		  template = VipReadVolume(template_name);

		}
	    }
	  else
	    {
	      largest1 = VipGetNthBiggest3DBucket_S16BIT(cclist,1);
	      largest2 = VipGetNthBiggest3DBucket_S16BIT(cclist,2);
	      largest3 = VipGetNthBiggest3DBucket_S16BIT(cclist,3);
	      if(largest1->data[0].z>=largest2->data[0].z && largest1->data[0].z>=largest3->data[0].z)
		{
		  cereb = largest1;
		  largest1 = largest3;
		  largest3 = cereb;
		}
	      if(largest2->data[0].z>=largest1->data[0].z && largest2->data[0].z>=largest3->data[0].z)
		{
		  cereb = largest2;
		  largest2 = largest3;
		  largest3 = cereb;
		}
	      if(largest3->data[0].z>=largest2->data[0].z && largest3->data[0].z>=largest1->data[0].z)
		{
		  cereb = largest3;
		}
	      
	      if(ncc>3)
		{
		  largest4 = VipGetNthBiggest3DBucket_S16BIT(cclist,4);
		}
	      if(largest4!=NULL && largest4->data[0].z>largest3->data[0].z)
		{
		  cereb=largest4;
		}
	      if(ncc>4)
		largest5 = VipGetNthBiggest3DBucket_S16BIT(cclist,5);
	      if(largest5!=NULL && largest5->data[0].z>largest3->data[0].z)
		{
		  cereb=largest5;
		}

	      if(((abs(largest1->size-largest2->size)/largest1->size)<0.15)
		 && largest1->data[0].z<(zCA-10)
		 && largest2->data[0].z<(zCA-10)
		 && (largest1->data[0].z-largest2->data[0].z)<10
		 )
		{
		  if ((largest1->data[0].x<(xCA-10))
		      && (largest2->data[0].x>(xCA+10)))
		    {
		      lhemi = largest2;
		      rhemi = largest1;
		      seedOK = VTRUE;
		    }
		  else if ((largest2->data[0].x<(xCA-10))
			   && (largest1->data[0].x>(xCA+10)))
		    {
		      rhemi = largest2;
		      lhemi = largest1;
		      seedOK = VTRUE;
		    }
		  else
		    {
		      VipPrintfWarning("Hemisphere seeds are not symetric relative to xCA\n");
		      printf("Perhaps you have not set xCA (currently: %d) and all is OK, I go on...\n",xCA);
		      if ((largest1->data[0].x)<(largest2->data[0].x))
			{
			  lhemi = largest2;
			  rhemi = largest1;
			  seedOK = VTRUE;
			}
		      else 
			{
			  rhemi = largest2;
			  lhemi = largest1;
			  seedOK = VTRUE;
			}
		    }
		}
	    }
	  if(seedOK==VFALSE)
	    {
	      printf("Can not find hemisphere seeds\n\n");
	      erosion_size += 0.5;
	      VipFreeVolume(seed);
	    }
	}		    
    }

  if(seedOK==VFALSE)
    {
      return(VIP_CL_ERROR);
      VipPrintfError("Can not find hemisphere and cerebellum seeds");
    }
  
  VipSetVolumeLevel(seed,0);
  if(template_filled != VTRUE)
    {
      if(lhemi!=NULL)
	{
	  VipWriteCoordBucketInVolume(lhemi,seed,2);
	}
      if(rhemi!=NULL) 
	{
	  VipWriteCoordBucketInVolume(rhemi,seed,1);
	}
      if(cereb!=NULL)
	{
	  if(Zover==-1)
	    VipWriteCoordOverZBucketInVolume_S16BIT(cereb,seed,3,Zover);
	  else VipWriteCoordBucketInVolume(cereb,seed,3);
	}
      /*VipWriteTivoliVolume(seed,"toto");*/
    }
  else
    {
      ccptr = cclist;
      cc = 0;
      while(ccptr!=NULL)
	{
	  VipWriteCoordBucketInVolume(ccptr,seed,write_label[cc]);
	  if(write_seed==VTRUE  && TemplateUse==VTRUE) VipWriteCoordInTemplate(ccptr,brain,template,write_label[cc]*10,&tal);
	  ccptr = ccptr->next;
	  cc++;
	}
    }
  if(write_seed==VTRUE) VipWriteTivoliVolume(template,"temp+seed-final");


  VipChangeIntLabel( seed, 255, 0);
  if(write_seed==VTRUE)       
    VipWriteTivoliVolume(seed,"seed");

  printf("White matter core reconstruction...\n");
  if(ana!=NULL)
    {
      vol = VipCreateDoubleThresholdedVolume( brain, VIP_BETWEEN_OR_EQUAL_TO,(int)(0.5*ana->white->mean+0.5*ana->gray->mean),
                                              ana->white->mean + 2 * ana->white->sigma, BINARY_RESULT );

      /*2005 vol = VipCreateSingleThresholdedVolume(brain, GREATER_THAN, (int)(0.5*ana->white->mean+0.5*ana->gray->mean) , BINARY_RESULT);*/
      if(vol==PB) return(VIP_CL_ERROR);
    }
  else
    {
      vol = VipCreateSingleThresholdedVolume(brain, GREATER_THAN, wthreshold, BINARY_RESULT);
      if(vol==PB) return(VIP_CL_ERROR);
    }
  if(VipConnexVolumeFilter(vol,CONNECTIVITY_6,-1,CONNEX_BINARY)==PB)
    return(VIP_CL_ERROR);

  if (ridgename!=NULL)
    {
      ridge = VipReadVolume(ridgename);
      if(!ridge) return(VIP_CL_ERROR);
      VipMaskVolume(ridge,vol);
      printf("Opening of size 1.6mm ...\n");
        if(VipOpening(vol,CHAMFER_BALL_3D,1.6)==PB) return(VIP_CL_ERROR);
      VipMerge(vol,ridge,VIP_MERGE_ONE_TO_ONE, 255, 255);
    }


  VipMerge(vol,seed,VIP_MERGE_SAME_VALUES,0,0);

  /* VipWriteTivoliVolume(vol,"domain"); */

  voronoi = VipComputeCustomizedFrontPropagationGeodesicVoronoi (vol, 255, 0, 3, 3, 3, 50);

  VipFreeVolume(vol);

  if((ana->gray->mean + 1 * ana->gray->sigma)<(int)(0.5*ana->white->mean+0.5*ana->gray->mean))
    {
      printf("White matter complete reconstruction...\n");
      if(ana!=NULL)
	{
	  vol = VipCreateSingleThresholdedVolume(brain, GREATER_THAN, ana->gray->mean + 1 * ana->gray->sigma, BINARY_RESULT);
	  if(vol==PB) return(VIP_CL_ERROR);
	}
      else
	{
	  vol = VipCreateSingleThresholdedVolume(brain, GREATER_THAN, wthreshold, BINARY_RESULT);
	  if(vol==PB) return(VIP_CL_ERROR);
	}
      if(VipConnexVolumeFilter(vol,CONNECTIVITY_6,-1,CONNEX_BINARY)==PB)
	return(VIP_CL_ERROR);
    
      VipMerge(vol,voronoi,VIP_MERGE_SAME_VALUES,0,0);

      VipFreeVolume(voronoi);
    
      voronoi = VipComputeCustomizedFrontPropagationGeodesicVoronoi (vol, 255, 0, 3, 3, 3, 50);
      VipFreeVolume(vol);
    }
  printf("Brain complete reconstruction...\n");
  vol = VipCreateSingleThresholdedVolume(brain, GREATER_THAN, 1, BINARY_RESULT);
  if(vol==PB) return(VIP_CL_ERROR);

  if(VipConnexVolumeFilter(vol,CONNECTIVITY_6,-1,CONNEX_BINARY)==PB)
    return(VIP_CL_ERROR);

  VipMerge(vol,voronoi,VIP_MERGE_SAME_VALUES,0,0);

  VipFreeVolume(voronoi);

  voronoi = VipComputeCustomizedFrontPropagationGeodesicVoronoi (vol, 255, 0, 3, 3, 3, 50);
  VipFreeVolume(vol);

  vol=voronoi;

  /*
  if (cutbuck!=NULL)
    {
      printf("AutoDeleting %d white matter points from interhemispheric plane\n",
	     cutbuck->n_points);
      VipWriteCoordBucketInVolume(cutbuck,vol,0);
    }
  */
  printf("Writing %s...\n",output);
  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else if (writelib == SPM)
    {
      if(VipWriteTivoliVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else
    if(VipWriteVolume(vol,output)==PB) return(VIP_CL_ERROR);

  VipFreeVolume(vol);
  VipFree(vol);

  return(0);

}
/*----------------------------------------------------------------------------*/
int VipWriteCoordOverZBucketInVolume_S16BIT(
  Vip3DBucket_S16BIT *buck,
  Volume *vol,
  int value,
  int Z)
/*----------------------------------------------------------------------------*/
{
  Vip_S16BIT *ptr;
  VipOffsetStruct *vos;
  Vip3DPoint_S16BIT *ptr3d;
  int i;

   vos = VipGetOffsetStructure(vol);
   ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;

   ptr3d = buck->data;
   for(i=buck->n_points;i--;)
     {
       if(ptr3d->z>Z)
	 *(ptr + ptr3d->x + ptr3d->y * vos->oLine + ptr3d->z * vos->oSlice) = value;
       ptr3d++;
     }

  VipFree(vos);

  return(0);
}


/*----------------------------------------------------------------------------*/
static int GetCutAlongTheLineBucket(  Volume *vol, Volume *template,
  VipTalairach *tal, int wthreshold, Vip3DBucket_S16BIT **buck)
/*----------------------------------------------------------------------------*/
{
   Vip_S16BIT *ptr;
   int x,y,z;
  float input_point[3], output_point[3];
  int ox, oy, oz;
  VipOffsetStruct *vos;
  Volume *bidon;

  if(!vol || ! tal || !buck)
    {
      VipPrintfError("Empty arg in GetCutAlongTheLineBucket");
      return(PB);
    }
  bidon = template;
  bidon = NULL;

  vos = VipGetOffsetStructure(vol);
  ptr = VipGetDataPtr_S16BIT(vol) + vos->oFirstPoint;

  *buck = VipAlloc3DBucket_S16BIT(20000);
  if(!*buck) return(PB);

  /*corpus callosum*/
  x = 0;
  for (y=-50;y<50;y+=3)
    for (z=-50;z<30;z+=3)
      {
	input_point[0] = x;
	input_point[1] = y;
	input_point[2] = z;
	fflush(stdout);
	VipTalairachInverseTransformation(input_point,tal,output_point);
	ox = (int)(output_point[0]/mVipVolVoxSizeX(vol)+0.5);
	oy = (int)(output_point[1]/mVipVolVoxSizeY(vol)+0.5);
	oz = (int)(output_point[2]/mVipVolVoxSizeZ(vol)+0.5);
	fflush(stdout);
	if((ox>=0)&&(ox<mVipVolSizeX(vol))
	   &&(oy>=0)&&(oy<mVipVolSizeY(vol))
	   &&(oz>=0)&&(oz<mVipVolSizeZ(vol)))	  
	  {
	    if( *(ptr + ox + (oy * vos->oLine) + oz * (vos->oSlice)) >wthreshold)
	      {
		(*buck)->data[(*buck)->n_points].x = ox;
		(*buck)->data[(*buck)->n_points].y = oy;
		(*buck)->data[(*buck)->n_points].z = oz;
		(*buck)->n_points++;
		/*		printf("%d, %d, %d",ox,oy,oz);*/
	      }
	  }
	
      }

  /*pons*/
  
  for(x = -15;x<=15;x+=3)
  for (y=10;y<45;y+=3)
    for (z=10;z<23;z+=3)
      {
	input_point[0] = x;
	input_point[1] = y;
	input_point[2] = z;
	fflush(stdout);
	VipTalairachInverseTransformation(input_point,tal,output_point);
	ox = (int)(output_point[0]/mVipVolVoxSizeX(vol)+0.5);
	oy = (int)(output_point[1]/mVipVolVoxSizeY(vol)+0.5);
	oz = (int)(output_point[2]/mVipVolVoxSizeZ(vol)+0.5);
	fflush(stdout);
	if((ox>=0)&&(ox<mVipVolSizeX(vol))
	   &&(oy>=0)&&(oy<mVipVolSizeY(vol))
	   &&(oz>=0)&&(oz<mVipVolSizeZ(vol)))	  
	  {
	    if( *(ptr + ox + (oy * vos->oLine) + oz * (vos->oSlice)) >wthreshold)
	      {
		(*buck)->data[(*buck)->n_points].x = ox;
		(*buck)->data[(*buck)->n_points].y = oy;
		(*buck)->data[(*buck)->n_points].z = oz;
		(*buck)->n_points++;
	      }
	  }
	
      }
      
  for(x = -30;x<=30;x+=3)
  for (y=10;y<45;y+=3)
    for (z=25;z<31;z+=3)
      {
	input_point[0] = x;
	input_point[1] = y;
	input_point[2] = z;
	fflush(stdout);
	VipTalairachInverseTransformation(input_point,tal,output_point);
	ox = (int)(output_point[0]/mVipVolVoxSizeX(vol)+0.5);
	oy = (int)(output_point[1]/mVipVolVoxSizeY(vol)+0.5);
	oz = (int)(output_point[2]/mVipVolVoxSizeZ(vol)+0.5);
	fflush(stdout);
	if((ox>=0)&&(ox<mVipVolSizeX(vol))
	   &&(oy>=0)&&(oy<mVipVolSizeY(vol))
	   &&(oz>=0)&&(oz<mVipVolSizeZ(vol)))	  
	  {
	    if( *(ptr + ox + (oy * vos->oLine) + oz * (vos->oSlice)) >wthreshold)
	      {
		(*buck)->data[(*buck)->n_points].x = ox;
		(*buck)->data[(*buck)->n_points].y = oy;
		(*buck)->data[(*buck)->n_points].z = oz;
		(*buck)->n_points++;
	      }
	  }
	
      }
  

  VipDwindle3DBucket_S16BIT(*buck);

  return(OK);

}
/*----------------------------------------------------------------------------*/
static int VipWriteCoordInTemplate(
  Vip3DBucket_S16BIT *buck,
  Volume *vol,
  Volume *template,
  int value,
  VipTalairach *tal)
/*----------------------------------------------------------------------------*/
{
  Vip_S16BIT *ptr;
  VipOffsetStruct *vos;
  Vip3DPoint_S16BIT *ptr3d;
  int i;
  float input_point[3], output_point[3];
  int x, y, z;
  float xtCA;
  float ytCA;
  float ztCA=87.;

  if(!buck || !vol || !template || ! tal)
    {
      VipPrintfError("Empty arg in VipWriteCoordInTemplate");
      return(VIP_CL_ERROR);
    }

  xtCA = mVipVolSizeX(template)*mVipVolVoxSizeX(template)/2.;
  ytCA = mVipVolSizeY(template)*mVipVolVoxSizeY(template)/2.;

  vos = VipGetOffsetStructure(template);
  ptr = VipGetDataPtr_S16BIT( template ) + vos->oFirstPoint;

   ptr3d = buck->data;
   for(i=buck->n_points;i--;)
     {
       input_point[0] = ptr3d->x * mVipVolVoxSizeX(vol);
       input_point[1] = ptr3d->y * mVipVolVoxSizeY(vol);
       input_point[2] = ptr3d->z * mVipVolVoxSizeZ(vol);
       VipTalairachTransformation(input_point,tal,output_point);
       output_point[0] += xtCA;
       output_point[1] += ytCA;
       output_point[2] += ztCA;

       x = (int)(output_point[0]/mVipVolVoxSizeX(template)+0.5);
       y = (int)(output_point[1]/mVipVolVoxSizeX(template)+0.5);
       z = (int)(output_point[2]/mVipVolVoxSizeX(template)+0.5);
       if((x>=0)&&(x<mVipVolSizeX(template))
	  &&(y>=0)&&(y<mVipVolSizeY(template))
	  &&(z>=0)&&(z<mVipVolSizeZ(template)))	      
	 *(ptr + x + (y * vos->oLine) + z * (vos->oSlice)) = value;
       ptr3d++;
     }

  VipFree(vos);

  return(OK);
}
/*----------------------------------------------------------------------------*/
static int VipGetLabelsFromTemplate(
  Vip3DBucket_S16BIT *buck,
  Volume *vol,
  Volume *template,
  VipHisto *labels,
  VipTalairach *tal)
/*----------------------------------------------------------------------------*/
{
  Vip_S16BIT *ptr;
  VipOffsetStruct *vos;
  Vip3DPoint_S16BIT *ptr3d;
  int i;
  float input_point[3], output_point[3];
  int x, y, z;
  float xtCA=77;
  float ytCA=72;
  float ztCA=87.;
  int value;

  if(!buck || !vol || !template || !tal || !labels)
    {
      VipPrintfError("Empty arg in VipWriteCoordInTemplate");
      return(VIP_CL_ERROR);
    }

  
  vos = VipGetOffsetStructure(template);
  ptr = VipGetDataPtr_S16BIT( template ) + vos->oFirstPoint;

  for (i=mVipHistoRangeMin(labels);i<=mVipHistoRangeMax(labels);i++)
    {
      mVipHistoVal(labels,i) = 0;
    }
   
  ptr3d = buck->data;
  for(i=buck->n_points;i--;)
    {
      input_point[0] = ptr3d->x * mVipVolVoxSizeX(vol);
      input_point[1] = ptr3d->y * mVipVolVoxSizeY(vol);
      input_point[2] = ptr3d->z * mVipVolVoxSizeZ(vol);
      VipTalairachTransformation(input_point,tal,output_point);
      output_point[0] += xtCA;
      output_point[1] += ytCA;
      output_point[2] += ztCA;

      x = (int)(output_point[0]/mVipVolVoxSizeX(template)+0.5);
      y = (int)(output_point[1]/mVipVolVoxSizeX(template)+0.5);
      z = (int)(output_point[2]/mVipVolVoxSizeX(template)+0.5);
      if((x>=0)&&(x<mVipVolSizeX(template))
	 &&(y>=0)&&(y<mVipVolSizeY(template))
	 &&(z>=0)&&(z<mVipVolSizeZ(template)))	      
	{
	  value = *(ptr + x + (y * vos->oLine) + z * (vos->oSlice));
	  mVipHistoVal(labels,value)++;
	}
      ptr3d++;
    }

  VipFree(vos);

  return(OK);
}



/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipSplitBrain\n");
  (void)fprintf(stderr,"        -i[nput] {MRI (bias corrected) image name}\n");
  (void)fprintf(stderr,"        [-t[emplate] {voronoi in Talairach: (default: /home2/local/Panabase/demo/talairach/closedvoronoi)}]\n");
  (void)fprintf(stderr,"        [-T[emplateUse] {(y/n), default:y}]\n");
  (void)fprintf(stderr,"        [-R[idge] {White ridge image name (default: not used)}]\n");
  (void)fprintf(stderr,"        [-C[utUse] {(y/n), default:y}]\n");
  (void)fprintf(stderr,"        [-b[rain] {binary brain image name (default:brain)}]\n");
  (void)fprintf(stderr,"        [-o[utput] {splitted brain image name (default:voronoi)}]\n");
  (void)fprintf(stderr,"        [-wa[lgo] {algo r/b/c/t, default:r\n");
  (void)fprintf(stderr,"        [-wt[hreshold] {algo t: int (default :mW - Coef*sW)}]\n");
  (void)fprintf(stderr,"        [-ws[eed] {write seed image (y/n), default:n}]\n");
  (void)fprintf(stderr,"        [-Co[ef] {float (default :2)}]\n");
  (void)fprintf(stderr,"        [-B[ary] {float (default :0.75)}]\n");
  (void)fprintf(stderr,"        [-e[rosion] {float (default :2mm)}]\n");
  (void)fprintf(stderr,"        [-c[csize] {int (default :500)}]\n");
  (void)fprintf(stderr,"        [-P[oints] {AC,PC,IH coord filename (*.tal) (default:not used)}]\n");
  (void)fprintf(stderr,"        [-xA[C] {(int [0-xsize[) AC X coordinate}]\n");
  (void)fprintf(stderr,"        [-yA[C] {(int [0-ysize[) AC Y coordinate}]\n");
  (void)fprintf(stderr,"        [-zA[C] {(int [0-zsize[) AC Z coordinate}]\n");
  (void)fprintf(stderr,"        [-xP[C] {(int [0-xsize[) PC X coordinate}]\n");
  (void)fprintf(stderr,"        [-yP[C] {(int [0-ysize[) PC Y coordinate}]\n");
  (void)fprintf(stderr,"        [-zP[C] {(int [0-zsize[) PC Z coordinate}]\n");
  (void)fprintf(stderr,"        [-a[nalyse] {char y/n/r: automatic histogram analysis, default:y}]\n");
  (void)fprintf(stderr,"        [-hn[ame] {histo analysis, default:input}]\n");
  (void)fprintf(stderr,"        [-xI[nterHem] {(int [0-xsize[) Inter-hemis. point X coord.}]\n");
  (void)fprintf(stderr,"        [-yI[nterHem] {(int [0-ysize[) Inter-hemis. point Y coord.}]\n");
  (void)fprintf(stderr,"        [-zI[nterHem] {(int [0-zsize[) Inter-hemis. point Z coord.}]\n");
  (void)fprintf(stderr,"        [-Z[over] {int (default :zCA+10)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: a, s, v or t (default:any)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: s, v or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);
 
}
/*****************************************************/

static int Help()
{
  (void)printf("Usage: VipSplitBrain\n");
  (void)printf("Try to split the brain into 3 parts: hemispheres and cerebellum.\n");
  (void)printf("White matter is segmented first, then cleaned up to prevent cavities\n");
  (void)printf("from disturbing morphological processing. Then the standard\n");
  (void)printf("Erosion, seed selection, reconstruction skim is used.\n");
  (void)printf("The program tries several erosion sizes in order to find the smallest one\n");
  (void)printf("which minimizes morphological opening effects. When provided, a template\n");
  (void)printf("of the localization of the 3 parts in SPM talairach space is used to select seeds.\n");
  (void)printf("Otherwise, simple considerations are used.\n");
  (void)printf("You can improve the use of the template if you provide CA, CP, and\n");
  (void)printf("a third point of interhemispheric plane\n");
  (void)printf("        -i[nput] {MRI (bias corrected) image name}\n");
  (void)printf("        [-t[emplate] {voronoi in Talairach: (default: /home2/local/Panabase/demo/talairach/closedvoronoi)}]\n");
  (void)printf("a 3 color image with the usual spm bounding box\n");
  (void)printf("        [-T[emplateUse] {(y/n), default:y}]\n");
  (void)printf("Without template, the result is less reliable but often correct\n");
  (void)printf("        [-C[utUse] {(y/n), default:y}]\n");
  (void)printf("Delete some points in corpus callosum and stem from Talairach Coord\n");
  (void)printf("        [-R[idge] {White ridge image name (default: not used)}]\n");
  (void)printf("        [-b[rain] {binary brain image name (default:brain)}]\n");
  (void)printf("brain binary mask\n");
  (void)printf("        [-o[utput] {splitted brain image name (default:voronoi)}]\n");
  (void)printf("a 3 color label image (a voronoi diagram inside brain mask\n");
  (void)printf("        [-wa[lgo] {algo r/b/c/t, default:r\n");
  (void)printf("r: regulariwed grey white classification\n");
  (void)printf("b: barycenter, cf -B option\n");
  (void)printf("c: coef, cf -wt and -Coef options\n");
  (void)printf("t: direct input of the threshold\n");
  (void)printf("        [-wt[hreshold] {algo t: int (default :mW - Coef*sW)}]\n");
  (void)printf("The threshold providing white matter after masking by brain mask\n");
  (void)printf("mW: white matter mean grey level, sW: standard deviation\n");
  (void)printf("        [-ws[eed] {write seed image (y/n), default:n}]\n");
  (void)printf("To get an image of seeds and seeds in the template (to understand some problems\n");
  (void)printf("        [-Co[ef) {float (default :2)}]\n");
  (void)printf("multiplicative factor providing threshold for white matter\n");
  (void)printf("        [-B[ary] {float (default :0.75)}]\n");
  (void)printf("Factor for barycenter between gray matter (1-Bary) mean and white matter mean (Bary)\n");
  (void)printf("        [-e[rosion] {float (default :2mm)}]\n");
  (void)printf("Initial erosion size, then step by step increasing by 0.5mm\n");
  (void)printf("        [-c[csize] {int (default :500)}]\n");
  (void)printf("Connected component minima lsize to enter the seed selection process\n");
  (void)printf("        [-P[oints] {AC,PC,IH coord filename (*.tal) (default:not used)}]\n");
  (void)printf("Correct format for the commissure coordinate file toto.APC:\n");
  (void)printf("AC: 91 88 113\nPC: 91 115 109\nIH: 90 109 53\n");  
  (void)printf("        [-xA[C] {(int [0-xsize[) AC X coordinate}]\n");
  (void)printf("        [-yA[C] {(int [0-ysize[) AC Y coordinate}]\n");
  (void)printf("        [-zA[C] {(int [0-zsize[) AC Z coordinate}]\n");
  (void)printf("        [-xP[C] {(int [0-xsize[) PC X coordinate}]\n");
  (void)printf("        [-yP[C] {(int [0-ysize[) PC Y coordinate}]\n");
  (void)printf("        [-zP[C] {(int [0-zsize[) PC Z coordinate}]\n");
  (void)printf("        [-a[nalyse] {char y/n/r: automatic histogram analysis, default:y}]\n");
  (void)printf("y: do histogram analysis to get mw and sw, r: read input.han,\n");
  (void)printf("n: do simpler things\n");
  (void)printf("        [-hn[ame] {histo analysis, default:input )}]\n");
  (void)printf("read when -a r is set, this file has the .han extension (VipHistoAnalysis)\n");
  (void)printf("        [-xI[nterHem] {(int [0-xsize[) Inter-hemis. point X coord.}]\n");
  (void)printf("        [-yI[nterHem] {(int [0-ysize[) Inter-hemis. point Y coord.}]\n");
  (void)printf("        [-zI[nterHem] {(int [0-zsize[) Inter-hemis. point Z coord.}]\n");
  (void)printf("        [-Z[over] {int (default :zCA+10)}]\n");
  (void)printf("Without template, subset of the cerebellum part above Zover are deleted\n");
  (void)printf("in order to modify the competion between the 3 seeds for influence\n");
  (void)printf("        [-r[eadformat] {char: a, s, v or t (default:any)}]\n");
  (void)printf("        [-w[riteformat] {char: s, v or t (default:t)}]\n");
  (void)printf("        [-h[elp]\n");
  (void)printf("Some more information in:\n");
  (void)printf("Shape bottlenecks and Conservative Flow systems\n");
  (void)printf("J.- F. Mangin, J. Regis and V. Frouin\n");
  (void)printf("IEEE/SIAM MMBIA Workshop (Math. Methods in\n");
  (void)printf("Biomed. Image Analysis), San Francisco, IEEE Press\n");
  (void)printf("319-328, 1996\n");
  return(VIP_CL_ERROR);
}

/******************************************************/
