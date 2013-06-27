/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipSkeleton.c        * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 12/04/1997
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
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
#include <vip/skeleton.h>
#include <vip/morpho.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol);
  VIP_DEC_VOLUME(meancurvature);
  VIP_DEC_VOLUME(altitude);
  VIP_DEC_VOLUME(saddle);
  VIP_DEC_VOLUME(volridge);
  VIP_DEC_VOLUME(voronoi);
  char *input = NULL;
  char *geometry = NULL;
  char classif = VTRUE;
  char soutput[VIP_NAME_MAXLEN] = "skeleton";
  char voutput[VIP_NAME_MAXLEN] = "rootsvoronoi";
  int readlib, writelib;
  int  immortal_flag = NON_SIMPLE_AND_NON_VOLUME_BECOME_IMMORTAL;
  char *meancurvname = NULL;
  int readmeancurv = VFALSE;
  float mcsigma = 1.;
  float gcsigma = 2.;
  float mcthreshold = 0.2;
  float gcthreshold = -0.05;
  char *prune = "co";
  int linside = 0;
  int loutside = 11;
  char algo = 's';
  int watershed = VTRUE;
  float erosion = 0.1;
  float lzero = 0.3;
  float lup = 1.0;
  char voronoiflag = VTRUE;
  int bwidth = 1;
  int skeletonization = VTRUE;
  int wprune = 3;
  int random_seed = time(NULL);
//   VipT1HistoAnalysis *hana = NULL;
//   char *hananame = NULL;


  int i;

  readlib = VIDA;
  writelib = VIDA;

  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {
      if (!strncmp(argv[i], "-srand", 6)){
        if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          random_seed = atoi(argv[i]);
        }
     else if (!strncmp (argv[i], "-immortality", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  {
	    algo = argv[i][0];
	  }
	} 
      else if (!strncmp (argv[i], "-input", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  input = argv[i];
	}
      else if (!strncmp (argv[i], "-soutput", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(soutput,argv[i]);
	}
      else if (!strncmp (argv[i], "-voutput", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(voutput,argv[i]);
	}    
      else if (!strncmp (argv[i], "-prune", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  {
	    prune = argv[i];
	  }
	}
//       else if (!strncmp (argv[i], "-hana", 2)) 
//         {
//           if (++i >= argc || !strncmp(argv[i],"-",1)) return Usage();
//           hananame = argv[i];
//         }
      else if (!strncmp (argv[i], "-lzero", 3)) 
	{
	  if(++i >= argc ) return(Usage());
	  lzero = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-lup", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  lup = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-erosion", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  erosion = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-linside", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  linside = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-wprune", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  wprune = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-loutside", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  loutside = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-readmc", 7)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          readmeancurv = VTRUE;
          meancurvname = argv[i];
        }
      else if (!strncmp (argv[i], "-mcsigma", 4)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  mcsigma = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-gcsigma", 4)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  gcsigma = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-mcthreshold", 4)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  mcthreshold = atof(argv[i]);
	}  
      else if (!strncmp (argv[i], "-gcthreshold", 4)) 
	{
	  gcthreshold = atof(argv[++i]);
	}
      else if (!strncmp (argv[i], "-geometry", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  geometry = argv[i];
	}
      else if (!strncmp (argv[i], "-skeleton", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='w') watershed = VTRUE;
	  else if(argv[i][0]=='s') watershed = VFALSE;
	  else if(argv[i][0]=='0')
	    {
	      skeletonization = VFALSE;
	      watershed = VFALSE;
	    }
	  else
	    {
	      VipPrintfError("Please answer w, s or 0 to flag -sk");
	      return(Usage());
	    }
	}
      else if (!strncmp (argv[i], "-voronoi", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') voronoiflag = VTRUE;
	  else if(argv[i][0]=='n') voronoiflag = VFALSE;
	  else
	    {
	      VipPrintfError("Please answer y or n to flag -v");
	      return(Usage());
	    }
	}
      else if (!strncmp (argv[i], "-classif", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') classif = VTRUE;
	  else if(argv[i][0]=='n') classif = VFALSE;
	  else
	    {
	      VipPrintfError("Please answer y or n to flag -c");
	      return(Usage());
	    }
	}
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipSkeleton");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') writelib = TIVOLI;
	  else if(argv[i][0]=='v') writelib = VIDA;
	  else
	    {
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipSkeleton");
	      return(VIP_CL_ERROR);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipSkeleton");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
   }

  if(watershed==VTRUE || voronoiflag==VTRUE)
      {
	  if(geometry==NULL)
	      {
		  VipPrintfError("geometry arg is required by the -sk w and -v y flags");
		  return(Usage());
	      }
	  if (VipTestImageFileExist(geometry)==PB)
	    {
	      (void)fprintf(stderr,"Can not open this image: %s\n",geometry);
	      return(VIP_CL_ERROR);
	    }
      }

  srand(random_seed);
  
  if(algo=='a') immortal_flag = NON_SIMPLE_AND_NON_VOLUME_BECOME_IMMORTAL;
  else if(algo=='c') immortal_flag = CURVES_BECOME_IMMORTAL;
  else if(algo=='s') immortal_flag = SURFACES_BECOME_IMMORTAL;
  else if(algo=='n') immortal_flag = NOBODY_BECOME_IMMORTAL;
  else
    {
      VipPrintfError("Unknown immortality mode");
      return(Help());
    }


  if(watershed==VFALSE && (skeletonization==VTRUE || strcmp(prune,"0") || classif==VTRUE))
    {
      printf("Reading %s...\n",input);
      if (readlib == TIVOLI)
	vol = VipReadTivoliVolumeWithBorder(input,bwidth);
      else
	vol = VipReadVolumeWithBorder(input,bwidth);
      if(vol==NULL) return(VIP_CL_ERROR);   
    }

  if(watershed==VTRUE && immortal_flag!=PB)
  {
      printf("===============================\n");
      printf("WATERSHED BASED SKELETONIZATION\n");
      printf("===============================\n");
      
      if(readmeancurv==VTRUE)
      {
          if(VipTestImageFileExist(meancurvname)==PB)
          {
              printf("Can not open this image: %s\n", meancurvname);
              return(VIP_CL_ERROR);
          }
          else
          {
              printf("Reading meancurvature image %s...\n", meancurvname);
              meancurvature = VipReadVolumeWithBorder(meancurvname, 0);
              printf("----------------------------------\n");
              altitude = ConvertMeanCurvToAltitude(meancurvature,
                                                   lzero, lup, erosion,
                                                   bwidth, mcthreshold);
              if(altitude==PB) return(VIP_CL_ERROR);
              VipFreeVolume(meancurvature);
          }
      }
      else
      {
          printf("Reading geometry image %s...\n", geometry);
          volridge = VipReadVolumeWithBorder(geometry, 0);
          printf("----------------------------------\n");
          altitude = ConvertBrainToAltitude(volridge, mcsigma,
                                            lzero, lup, erosion,
                                            bwidth,mcthreshold);
          if(altitude==PB) return(VIP_CL_ERROR);
          VipFreeVolume(volridge);
      }

//             if(hananame==NULL)
//           {
//               VipPrintfError("With the Cortical mode, you have to provide an histogram analysis (-hana)\n");
//               return Usage();
//           }
//           else
//           {
//               hana = VipReadT1HistoAnalysis(hananame);
//               if(hana==PB)
//               {
//                   VipPrintfError("Can not read this histogram analysis");
//                   return (VIP_CL_ERROR);
//               }
//               if(hana->white==NULL || hana->gray==NULL)
//               {
//                   VipPrintfError("Can not use this poor histogram analysis (no gray/white modes)");
//                   return (VIP_CL_ERROR);
//               }
//           }

      printf("--------------------\n");
      printf("Reading %s...\n",input);
      if (readlib == TIVOLI)
          vol = VipReadTivoliVolumeWithBorder(input,bwidth);
      else
          vol = VipReadVolumeWithBorder(input,bwidth);
      printf("--------------------\n");
      if(vol==NULL) return(VIP_CL_ERROR);
      
      /*hana->gray->mean = (int)(hana->gray->mean - hana->gray->sigma);
      hana->white->mean = (int)(hana->white->mean - hana->white->sigma);
      greywhite = VipGrayWhiteClassificationRegularisationForVoxelBasedAna( volridge, hana, VFALSE, 10, 20, CONNECTIVITY_26 );
      
      VipMerge( greywhite, vol, VIP_MERGE_ONE_TO_ONE, 0, 200 );
      VipSingleThreshold( greywhite, EQUAL_TO, 200, BINARY_RESULT );
      VipConnexVolumeFilter( greywhite, CONNECTIVITY_6, -1, CONNEX_BINARY );
      VipCleaningConnectivity( greywhite, CONNECTIVITY_6, 1 );
      VipInvertBinaryVolume( greywhite );
      VipCleaningConnectivity( greywhite, CONNECTIVITY_6, 2 );
      VipInvertBinaryVolume( greywhite );
      
      mask = VipCreateSingleThresholdedVolume( volridge, NOT_EQUAL_TO, 0, BINARY_RESULT  );
      VipSingleThreshold( altitude, GREATER_OR_EQUAL_TO, -1, BINARY_RESULT  );
      VipMerge( mask, altitude, VIP_MERGE_ONE_TO_ONE, 255, 0 );
      VipMerge( altitude, mask, VIP_MERGE_ONE_TO_ONE, 255, -11 );
      VipMerge( altitude, greywhite, VIP_MERGE_ONE_TO_ONE, 255, -111 );
      VipWriteVolume( altitude, "altitude" );
      VipChangeIntLabel( altitude, 255, 15);*/
      
      if(VipWatershedHomotopicSkeleton( vol, altitude, immortal_flag, linside, loutside) == PB) return(VIP_CL_ERROR);
      VipFreeVolume(altitude);
  }
  else if(skeletonization==VTRUE)
      {

	printf("===============\n");
	printf("SKELETONIZATION\n");
	printf("===============\n");

	  if(immortal_flag!=PB)
	      if (VipHomotopicSkeleton (vol, immortal_flag, FRONT_6CONNECTIVITY_DIRECTION)==PB)
		  return(VIP_CL_ERROR);
      }

  if(!strcmp(prune,"c") || !strcmp(prune,"co"))
      {
	 if( VipHomotopicCurvePruning( vol, linside, loutside )==PB) return(VIP_CL_ERROR);
      }
      
  if(!strcmp(prune,"o") || !strcmp(prune,"co"))
      {
	 if( VipHomotopicOutsidePruning( vol, linside, loutside )==PB) return(VIP_CL_ERROR);
      }

  if(classif==VTRUE)
      {
	printf("----------------------------------------------------\n");
	printf("Topological Classification of the skeleton points...\n");
	printf("----------------------------------------------------\n");

	if(VipTopologicalClassificationForTwoLabelComplement(vol,linside,loutside)==PB) return(VIP_CL_ERROR);
	  
      }
 
  if((skeletonization==VTRUE || strcmp(prune,"0") || classif==VTRUE))
    {
      printf("Writing skeleton %s...\n",soutput); 

      if (writelib == TIVOLI)
	{
	  if(VipWriteTivoliVolume(vol,soutput)==PB) return(VIP_CL_ERROR);
	}
      else
	if(VipWriteVolume(vol,soutput)==PB) return(VIP_CL_ERROR);
    
      VipFreeVolume(vol);
    }

  if(voronoiflag==VTRUE)
      {

	printf("=======================\n");
	printf("WATERSHED BASED VORONOI\n");
	printf("=======================\n");

	bwidth = 3; /*extremality test*/

	printf("Reading geometry image %s...\n",geometry);
	volridge = VipReadVolumeWithBorder(geometry,0);
	saddle = ConvertBrainToSaddlePoint( volridge, 
					     gcsigma,
					     bwidth,gcthreshold);
	if(saddle==PB) return(VIP_CL_ERROR);
	VipFreeVolume(volridge);

	printf("----------------------------------\n");
	printf("Reading %s...\n",input);
	if (readlib == TIVOLI)
	  vol = VipReadTivoliVolumeWithBorder(input,bwidth);
	else
	  vol = VipReadVolumeWithBorder(input,bwidth);
	
	if(vol==NULL) return(VIP_CL_ERROR);
	voronoi = VipSulcalRootsWatershedVoronoi(vol,saddle, CONNECTIVITY_2x26,VIP_GEODESIC_DEPTH,
						 255,0,11,100*wprune); 
	if(voronoi==PB) return(VIP_CL_ERROR);
	vol = voronoi;
      

	printf("----------------------------------\n");
	printf("Writing voronoi %s...\n",voutput); 

	if (writelib == TIVOLI)
	  {
	    if(VipWriteTivoliVolume(voronoi,voutput)==PB) return(VIP_CL_ERROR);
	  }
	else
	  if(VipWriteVolume(voronoi,voutput)==PB) return(VIP_CL_ERROR);
      }
  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipSkeleton\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-so[utput] {image name (default:\"skeleton\")}]\n");
  (void)fprintf(stderr,"        [-vo[utput] {image name (default:\"rootsvoronoi\")}]\n");
  (void)fprintf(stderr,"        [-sk[eleton] {w/s/0  (default:w)}]\n");
  (void)fprintf(stderr,"        [-im[mortality] {string:a/s/c/n (default:s)}]\n");
  (void)fprintf(stderr,"        [-v[oronoi] {y/n  (default:y)}]\n");
  (void)fprintf(stderr,"        [-p[rune] {string:c/o/co/0 (default:co)}]\n");
  (void)fprintf(stderr,"        [-wp[rune] {int:minimum catchment bassin depth (default:3mm)}]\n");
  (void)fprintf(stderr,"        [-li[nside] {int:label<290 (default:0)}]\n");
  (void)fprintf(stderr,"        [-lo[utside] {int:label<290 (default:11)}]\n");
  (void)fprintf(stderr,"        [-c[lassification] {y/n (default:y)}]\n");
  (void)fprintf(stderr,"        [-g[eometry] {image name for ridge and saddle points (default:nothing)}]\n");
  (void)fprintf(stderr,"        [-lz[ero] {(float) lowest mean curvature for watershed (default:0.3)}]\n");
  (void)fprintf(stderr,"        [-lu[p] {(float) highest mean curvature for watershed(default:1)}]\n");
  (void)fprintf(stderr,"        [-e[rosion] {(float) wave erosion for watershed (default:0.1)}]\n");
  (void)fprintf(stderr,"        [-readmc {mean_curvature image name (default:nothing)}]\n");
  (void)fprintf(stderr,"        [-mcs[igma] {float (mm)  (default:1mm)}]\n");
  (void)fprintf(stderr,"        [-gcs[igma] {float (mm)  (default:2mm)}]\n");
  (void)fprintf(stderr,"        [-mct[hreshold] {float (default:0.2)}]\n");
  (void)fprintf(stderr,"        [-gct[hreshold] {float (default:-0.05)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-srand {int (default: time}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Compute a 3D skeleton of the object (usually dedicated to cortical folds)");
  (void)printf("Default modes are dedicated to the cortex (input comes from VipHomotopicSnake)\n");
  (void)printf("In that case, a voronoi related to sulcal root is also proposed.\n");
  (void)printf("\n");
  (void)printf("Usage: VipSkeleton\n");
  (void)printf("        -i[nput] {image name} : object definition\n");
  (void)printf("        [-so[utput] {image name (default:\"skeleton\")}]\n");
  (void)printf("        [-vo[utput] {image name (default:\"rootsvoronoi\")}]\n");
  (void)printf("        [-sk[eleton] {w/s/0  (default:w)}]\n");
  (void)printf("w: the skeletonization is coupled with the watershed idea\n");
  (void)printf("   this mode is dedicated to the cortical fold extraction\n");
  (void)printf("   this mode requires an image to compute its mean curvature as a ridge detector\n");
  (void)printf("   The aim is an homotopic skeleton following the ridges\n");
  (void)printf("s: standard homotopic skeletomization\n");
  (void)printf("0: no skeletonization\n");
  (void)printf("        [-im[mortality] {string:a/s/c/n (default:s)}]\n");
  (void)printf("property required to be preserved during the homotopic skeletonization\n");
  (void)printf("a: homotopic with full preservation of points, curves and surfaces\n");
  (void)printf("s: homotopic with full preservation of points and surfaces\n");
  (void)printf("c: homotopic with full preservation of points and curves\n");
  (void)printf("n: homotopic with no preservation\n");
  (void)printf("        [-v[oronoi] {y/n  (default:y)}]\n");
  (void)printf("Computes a voronoi of the input object corresponding to a sulcal root based parcellisation\n");
  (void)printf("this mode requires an image to compute its Gaussian curvature as a saddle point detector\n");
  (void)printf("This saddle points represent the anatomical plis de passage\n");
  (void)printf("        [-p[rune] {string:c/o/co/0 (default:co)}]\n");
  (void)printf("c: homotopic pruning of curves\n");
  (void)printf("o: homotopic pruning of outside 6-neighbors\n");
  (void)printf("co: c & o\n");
  (void)printf("0: no pruning\n");
  (void)printf("        [-wp[rune] {int:minimum catchment bassin depth (default:3mm)}]\n"); 
  (void)printf("        [-li[nside] {int:label<290 (default:0)}]\n");
  (void)printf("        [-lo[utside] {int:label<290 (default:11)}]\n");
  (void)printf("        [-c[lassification] {y/n (default:y)}]\n");
  (void)printf("this flag trigered a final topological classification of the skeleton\n");
  (void)printf("        [-g[eometry] {image name (default:nothing)}]\n");
  (void)printf("--------------------------Mean curvature:-----------------------------------------\n");
  (void)printf("with watershed flag trigered, the watershed idea is related to the isophote mean\n");
  (void)printf("curvature of this image, which is usually a MR image with segmented brain\n");
  (void)printf("but could be the input binary image or a distance map computed from it\n");
  (void)printf("In this context, mean curvature is used as a ridge detector\n");
  (void)printf("During the first iterations, points whose mc < mcthreshold can not become immortals\n");
  (void)printf("This trick helps the skeletonization to get rid of small non significative simple surfaces\n");
  (void)printf("The skeletonization itself follows an immersion like process begining above mcthreshold\n");
  (void)printf("--------------------------Gaussian curvature:-----------------------------------------\n");
  (void)printf("with voronoi flag trigered, this image is used to compute isophote Gaussian curvature\n");
  (void)printf("Then points of the gray/white interface whose gc<gcthreshold are marked\n");
  (void)printf("The cortical folds are filled in at this location to reduce depth\n");
  (void)printf("        [-lz[ero] {(float) lowest mean curvature for watershed (default:0.3)}]\n");
  (void)printf("        [-lu[p] {(float) highest mean curvature for watershed(default:1)}]\n");
  (void)printf("        [-e[rosion] {(float) wave erosion for watershed (default:0.1)}]\n");
  (void)printf("        [-readmc {mean_curvature image name (default:nothing)}]\n");
  (void)printf("        [-mcs[igma] {float (mm)  (default:1mm)}]\n");
  (void)printf("the sigma of the Gaussian smoothing before mean curvature computation\n");
  (void)printf("        [-gcs[igma] {float (mm)  (default:2mm)}]\n");
  (void)printf("the sigma of the Gaussian smoothing before Gaussian curvature computation\n");
  (void)printf("        [-mct[hreshold] {float (default:0.2)}]\n");
  (void)printf("        [-gct[hreshold] {float (default:-0.05)}]\n");
  (void)printf("cf. geometry help\n");
  (void)printf("        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)printf("        [-w[riteformat] {char: v or t (default:v)}]\n");
  (void)printf("       [-srand {int (default: time}]\n");
  (void)printf("Initialization of the random seed, useful to get reproducible results\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
