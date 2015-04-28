/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipHomotopicSnake.c      * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 27/07/1999
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
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
#include <vip/morpho.h>
#include <vip/skeleton.h>
#include <vip/pyramid.h>
#include <vip/histo.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  VIP_DEC_VOLUME(closing); 
  Pyramid *pyr, *pyrlab;
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "cortex";
  int i;
  float fclosingsize = 5.;
  int linside = 0;
  int loutside = 11;  
  int readlib, writelib;
  float mG=-1., sG=-1., mW=-1., sW=-1.;
  VipT1HistoAnalysis *hana = NULL;
  char *hananame = NULL; 
  int pressure = 0;
  float newsG, newsW;
  char *ridgename = NULL;
  Volume *ridge=NULL;

  readlib = VIDA;
  writelib = TIVOLI;

  /*loop on command line arguments*/

  for (i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-input", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  input = argv[i];
	}
      else if (!strncmp(argv[i], "-help", 3))
        {
          /* This has been placed before -hana because otherwise -help is
             captured by the test for -hana. We cannot use -h for help, because
             the documentation has long described -h as a shortcut for
             -hana. */
          return Help();
        }
      else if (!strncmp (argv[i], "-hana", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  hananame = argv[i];
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-fclosing", 3)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  fclosingsize = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-linside", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  linside = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-mgray", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  mG= atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-sgray", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  sG= atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-mwhite", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  mW= atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-swhite", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  sW= atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-pressure", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  pressure= atof(argv[i]);
	  if(pressure<0 || pressure>150)
	    {
	      VipPrintfError("pressure should be in [0:150]");
	      return Usage();
	    }
	}
      else if (!strncmp (argv[i], "-Ridge", 2)) 
       {
         if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
         ridgename = argv[i];
       }
      else if (!strncmp (argv[i], "-loutside", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  loutside = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipHomotopicSnake");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
	  if(argv[i][0]=='t') writelib = TIVOLI;
	  else if(argv[i][0]=='v') writelib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipHomotopicSnake");
	      return(VIP_CL_ERROR);
	    }
	}    
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL) 
    {
      VipPrintfError("input arg is required by VipHomotopicSnake");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }


  printf("===================================================\n");
  printf("Detecting grey/white interface\n");
  printf("===================================================\n");


  printf("Reading %s...\n",input);
  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,1);
  else
    vol = VipReadVolumeWithBorder(input,1);

  if (vol==NULL) return (VIP_CL_ERROR);
  if (ridgename!=NULL)
    {
      printf("Reading white ridge...\n");
      ridge = VipReadVolumeWithBorder(ridgename,1);
      if(!ridge) return(VIP_CL_ERROR);
    }

  if (hananame==NULL)
      {
	  if (mG < 0 || sG < 0 || mW < 0 || sW < 0)
	      {
		  VipPrintfError("Without histogram analysis (-hana), you have to provide 4 statistical parameters\n");
		  return Usage();
	      }
      }
  else
      {
	  hana = VipReadT1HistoAnalysis(hananame);
	  if (hana==PB)
	      {
		  VipPrintfError("Can not read this histogram analysis");
		  return (VIP_CL_ERROR);
	      }
	  if (hana->white==NULL || hana->gray==NULL)
	      {
		  VipPrintfError("Can not use this poor histogram analysis (no gray/white modes)");
		  return (VIP_CL_ERROR);
	      }
      }
  if (mG < 0)
    mG = hana->gray->mean ;
  if (sG < 0)
    sG = hana->gray->sigma ;
  if (mW < 0)
    mW = hana->white->mean ;
  if (sW < 0)
    sW = hana->white->sigma ;	 
  
	  
  printf("-------------------------------\n");
  printf("Computing closing of %s...\n",vol->name);
  printf("-------------------------------\n");
  closing = VipCopyVolume(vol,"closing");
  if (closing==PB) return (VIP_CL_ERROR);
  if (VipClosing (closing, CHAMFER_BALL_3D, fclosingsize)== PB) return (VIP_CL_ERROR);
  VipChangeIntLabel(closing,255,1);
  VipMerge(closing,vol,VIP_MERGE_SAME_VALUES,0,0);     
  VipFreeVolume(vol);

  printf("-------------------------------\n");
  printf("Computing pyramid and bounding box deflation...\n");
  printf("-------------------------------\n");

  pyr = VipPutVolumeInPyramidAndCreatePyramid( closing, 3, PYR_MAX );

  pyrlab = VipCreateBoundingBoxLabelPyramid( pyr, 3, 255, linside, loutside );

  if(ridge==NULL)
  {
      /*this first dilation is to be robust to some vasculature or meninge staying alive
        in the segmentation, this could be improve for similar problems inside Sylvian valley*/

      printf("----------------------------------------------------------------------------\n");
      printf("Small homotopic dilation to skip potential remaining vasculature or meninge...\n");
      VipHomotopicGeodesicDilation( pyrlab->image[0]->volume, 2,
				  255, linside, linside, loutside, FRONT_RANDOM_ORDER );

      /*To prevent the erosion from going back into that area*/
      VipMerge(pyr->image[0]->volume,pyrlab->image[0]->volume,VIP_MERGE_ONE_TO_ONE,255,mG-2*sG);
      /*nothing forbiden*/
      printf("-------------------------------\n");
      printf("Gray/white interface detection using homotopic snake...\n");
      printf("-------------------------------\n"); 
      printf("Statistical parameters:\n");
      printf("gray matter: mean: %.3f, sigma:%.3f\n",mG,sG);
      printf("white matter: mean: %.3f, sigma:%.3f\n",mW,sW);

	   
      VipHomotopicInsideDilationSnake(pyrlab->image[0]->volume,pyr->image[0]->volume,
                                      100, 255, linside, loutside, FRONT_RANDOM_ORDER,
                                      1., mG, sG, mW, sW);

      newsG = sG/2.+ sG*(float)(100-pressure)/200.;
      newsW = sW+ sW*(float)(pressure)/200.;
      if(pressure!=0)
        {
          printf("Adding pressure in white matter: sG=%.3f, sW=%.3f\n",newsG,newsW);
        }
      VipHomotopicErosionFromInsideSnake(pyrlab->image[0]->volume,pyr->image[0]->volume,
                                         100, 255, linside, loutside,
                                         1., mG, newsG, mW, newsW);
  }
  else
  {
     /*this first dilation is to be robust to some vasculature or meninge staying alive
        in the segmentation, this could be improve for similar problems inside Sylvian valley*/
      printf("----------------------------------------------------------------------------\n");
      printf("Small homotopic dilation to skip potential remaining vasculature, meninge, Gibbs ringing...\n");
      VipHomotopicGeodesicDilation( pyrlab->image[0]->volume, 2,
				  255, linside, linside, loutside, FRONT_RANDOM_ORDER );
      
      /*To prevent the erosion from going back into that area*/
      VipMerge(pyr->image[0]->volume,pyrlab->image[0]->volume,VIP_MERGE_ONE_TO_ONE,255,mG-2*sG);
      
      /*nothing forbiden*/
      printf("-------------------------------\n");
      printf("Gray/white interface detection using homotopic snake...\n");
      printf("-------------------------------\n"); 
      printf("Statistical parameters:\n");
      printf("gray matter: mean: %.3f, sigma:%.3f\n",mG,sG);
      printf("white matter: mean: %.3f, sigma:%.3f\n",mW,sW);

      VipHomotopicInsideDilationSnakeRidge(ridge,pyrlab->image[0]->volume,pyr->image[0]->volume,
                                           100, 255, linside, loutside, FRONT_RANDOM_ORDER,
                                           1., mG, sG, mW, sW);
      
      newsG = sG/2.+ sG*(float)(100-pressure)/200.;
      newsW = sW+ sW*(float)(pressure)/200.;
      if(pressure!=0)
        {
          printf("Adding pressure in white matter: sG=%.3f, sW=%.3f\n",newsG,newsW);
        }
      VipHomotopicErosionFromInsideSnake(pyrlab->image[0]->volume,pyr->image[0]->volume,
                                         100, 255, linside, loutside,
                                         1., mG, newsG, mW, newsW);
  }

  printf("-------------------------\n");
  printf("Writing %s...\n",output);
  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(pyrlab->image[0]->volume,output)==PB) return(VIP_CL_ERROR);
    }
  else
    if(VipWriteVolume(pyrlab->image[0]->volume,output)==PB) return(VIP_CL_ERROR);
  
  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipHomotopicSnake\n");
  (void)fprintf(stderr,"        -i[nput] {segmented brain image name}\n");
  (void)fprintf(stderr,"        -h[ana] {histogram scale-space analysis (?.han)}]\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"cortex\")}]\n");
  (void)fprintf(stderr,"        [-fc[losing] {float (mm) closing size for f mode (default: 5)}]\n");
  (void)fprintf(stderr,"        [-R[idge] {White ridge image name (default: not used)}]\n");
  (void)fprintf(stderr,"        [-li[nside] {int:label<290 (default:0)}]\n");
  (void)fprintf(stderr,"        [-lo[utside] {int:label<290 (default:11)}]\n");
  (void)fprintf(stderr,"        [-p[ressure] {int [0:100] (default:0)}]\n");
  (void)fprintf(stderr,"        [-mg[ray] {float (default:?.ana)}]\n");
  (void)fprintf(stderr,"        [-sg[ray] {float (default:?.ana)}]\n");
  (void)fprintf(stderr,"        [-mw[hite] {float (default:?.ana)}]\n");
  (void)fprintf(stderr,"        [-sw[hite] {float (default:?.ana)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-he[lp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Homotopic segmentation of an object defined by the external hull");
  (void)printf("of the brain (the surface of a brain closing) and by the gray/white interface\n");
  (void)printf("The result is endowed with a sphere homotopy and is supposed to be used by VipSkeleton\n");
  (void)printf("\n");
  (void)printf("Usage: VipHomotopicSnake\n");
  (void)printf("        -i[nput] {brain image name}\n");
  (void)printf(" a brain (or hemisphere) segmentation usually obtained with VipGetBrain\n");
  (void)printf("        -h[ana] {histogram scale-space analysis name (?.han)}]\n");
  (void)printf(" the T1-MR histo analysis usually obtained with VipHistoAnalysis\n");
  (void)printf("        [-o[utput] {image name (default:\"homotopic\")}]\n");
  (void)printf("        [-fc[losing] {float (mm) closing size for f mode (default: 5)}]\n");
  (void)printf("        [-R[idge] {White ridge image name (default: not used)}]\n");
  (void)printf("        [-li[nside] {int:label<290 (default:0)}]\n");
  (void)printf("        [-lo[utside] {int:label<290 (default:11)}]\n");
  (void)printf("        [-p[ressure] {int [0:150] (default:0)}]\n");
  (void)printf("Partial volume effect and topological constraints may prevent the snake to perfectly recover the highest parts of gyri, especially if the slice thickness is larger than 1 mm. In such case, adding some pressure inside the baloon lead to better results. The 0 to 100 parameter allows you to move the interface definition from (Grey-mean+2Gsigma/White-mean-2Wsigma) to (Grey-mean+1*Gsigma/White-mean-3*Wsigma). No pressure is used by default because for some cases, the grey matter of the primary motor area, which is often whiter than standard grey matter because of more myelin around motor axon (cf thalamus), is taken for white matter, which leads to a bad detection of central sulcus.\n");
  (void)printf("        [-mg[ray] {float (default:?.ana)}]\n");
  (void)printf("        [-sg[ray] {float (default:?.ana)}]\n");
  (void)printf("        [-mw[hite] {float (default:?.ana)}]\n");
  (void)printf("        [-sw[hite] {float (default:?.ana)}]\n");
  (void)printf("        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)printf("        [-w[riteformat] {char: v or t (default:t)}]\n");
  (void)printf("        [-he[lp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
