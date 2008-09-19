/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipAlternatedSequentialFilter.c * TYPE     : Command line
 * AUTHOR      : POUPON F.                       * CREATION : 09/04/1999
 * VERSION     : 0.1                             * REVISION :
 * LANGUAGE    : C                               * EXAMPLE  :
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

/*###########################################################################

	I N C L U S I O N S

  ###########################################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <vip/volume.h>
#include <vip/util.h>
#include <vip/connex.h>
#include <vip/morpho.h>
#include <vip/distmap.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{     
  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol);
  VIP_DEC_VOLUME(temp);
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "smoothed";
  int xmasksize = 3;
  int ymasksize = 3;
  int zmasksize = 3;
  float size = 1.0, s1;
  float size2 = 0.0, s2;
  char stringconnectivity[256] = "6";
  int connectivity=0;
  int connectivity_activ = VTRUE;
  float multfactor = VIP_USUAL_DISTMAP_MULTFACT ;
  int borderwidth = 1;
  int maxxyz;
  char algo = 'f';
  int algodefine = FRONT_PROPAGATION;
  int readlib, writelib;
  char first = 'o';
  int i, maxImg=0, nitermax=5;
  int ncycle=1, j;

  readlib = VIDA;
  writelib = VIDA;

  /*loop on command line arguments*/
  for (i=1; i<argc; i++)
    {	
      if (!strncmp (argv[i], "-input", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  input = argv[i];
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-algo", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  algo = argv[i][0];
	}
      else if (!strncmp (argv[i], "-nitermax", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  nitermax = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-first", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  first = argv[i][0];
	  if ((first != 'o') && (first != 'c')) first = 'o';
	}
      else if (!strncmp (argv[i], "-size", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  size = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-Size", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  size2 = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-Ncycle", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  ncycle = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-xmasksize", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  xmasksize = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-ymasksize", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  ymasksize = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-zmasksize", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  zmasksize = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-connectivity", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  {
	    strcpy(stringconnectivity,argv[i]);
	    if (argv[i][0] == 'n') connectivity_activ = VFALSE;
	    else connectivity_activ = VTRUE;
	  }
	} 
      else if (!strncmp (argv[i], "-multfactor", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  multfactor = atof(argv[i]);
	}     
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if (argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else if(argv[i][0]=='s') readlib = SPM;
	  else
	    {
	      VipPrintfError("This format is not implemented for reading");
	      VipPrintfExit("(commandline)VipMorphologicalSmoothing");
	      return(PB);
	    }
	}
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if (argv[i][0]=='t') writelib = TIVOLI;
	  else if(argv[i][0]=='v') writelib = VIDA;
	  else if(argv[i][0]=='s') writelib = SPM;
	  else
	    {
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipMorphologicalSmoothing");
	      return(PB);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/
  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipMorphologicalSmoothing");
      return(Usage());
    }

  if (VipTestImageFileExist(input)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n", input);
      return(PB);
    }

  if (algo == 'f')
    {
      algodefine = FRONT_PROPAGATION;
    }
  else if (algo == 't')
    {
      algodefine = CHAMFER_TRANSFORMATION;
    }
  else
    {
      VipPrintfError("Unknown algorithm...");
      return(Usage());
    }

  if (size <= 0.0)
    {
      VipPrintfError("Structuring element size must be greater than 0 mm.");
      return(Usage());
    }

  if (size2 <= 0.0) size2 = size;

  if (ncycle < 1) ncycle = 1;

  if (connectivity_activ == VTRUE)
    {
      connectivity = VipConvertStringToConnectivity(stringconnectivity);
      if (connectivity == PB)
	{
	  VipPrintfError("This connectivity is unknown...");
	  return(Usage());
	}
    }

  maxxyz = mVipMax(xmasksize,ymasksize);
  maxxyz = mVipMax(zmasksize,maxxyz);
  borderwidth = (maxxyz-1)/2;

  /* Lecture */
  printf("Reading %s...\n",input);
  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input, borderwidth);
  else if (readlib == SPM)
    vol = VipReadSPMVolumeWithBorder(input, borderwidth);
  else
    vol = VipReadVolumeWithBorder(input, borderwidth);

  if (vol == NULL) 
    {
      VipPrintfError("Error while reading volume.");
      return(PB);
    }

  /* Fait une sauvegarde du volume de depart */
  temp = VipCopyVolume(vol, "");
  if (!temp)
    {
      VipPrintfError("Cannot duplicate the volume");
      if (vol) VipFreeVolume(vol);
      return(PB);
    }

  /* Binarisation */
  printf("Thresholding...\n");
  if ((VipSingleThreshold(temp, NOT_EQUAL_TO, 0, BINARY_RESULT)) == PB)
    {
      VipPrintfError("Error while thresholding");
      if (vol) VipFreeVolume(vol);
      if (temp) VipFreeVolume(temp);
      return(PB);
    }

  /* Filtrage morphologique alterne sequentiel */
  for (j=1; j<=ncycle; j++)
    {
      s1 = (float)j*size/(float)ncycle;
      s2 = (float)j*size2/(float)ncycle;

      if (connectivity_activ == VFALSE)
	{
	  if (first == 'o')
	    {
	      printf("Opening (with customized chamfer)\n");
	      if (VipCustomizedChamferOpening(temp, s1, xmasksize, 
					      ymasksize, zmasksize, 
					      multfactor, algodefine) == PB) 
		{
		  VipPrintfError("Error while opening");
		  if (vol) VipFreeVolume(vol);
		  if (temp) VipFreeVolume(temp);
		  return(PB);
		}
	      printf("Closing (with customized chamfer)\n");
	      if (VipCustomizedChamferClosing(temp, s2, xmasksize, 
					      ymasksize, zmasksize, 
					      multfactor, algodefine) == PB) 
		{
		  VipPrintfError("Error while closing");
		  if (vol) VipFreeVolume(vol);
		  if (temp) VipFreeVolume(temp);
		  return(PB);
		}
	    }
	  else 
	    {
	      printf("Closing (with customized chamfer)\n");
	      if (VipCustomizedChamferClosing(temp, s2, xmasksize, 
					      ymasksize, zmasksize, 
					      multfactor, algodefine) == PB) 
		{
		  VipPrintfError("Error while closing");
		  if (vol) VipFreeVolume(vol);
		  if (temp) VipFreeVolume(temp);
		  return(PB);
		}
	      printf("Opening (with customized chamfer)\n");
	      if (VipCustomizedChamferOpening(temp, s1, xmasksize, 
					      ymasksize, zmasksize, 
					      multfactor, algodefine) == PB) 
		{
		  VipPrintfError("Error while opening");
		  if (vol) VipFreeVolume(vol);
		  if (temp) VipFreeVolume(temp);
		  return(PB);
		}
	    }
	}
      else
	{
	  if (first == 'o')
	    {
	      printf("Opening (with connectivity chamfer)\n");
	      if (VipConnectivityChamferOpening(temp, s1, connectivity, 
						algodefine) == PB) 
		{
		  VipPrintfError("Error while opening");
		  if (vol) VipFreeVolume(vol);
		  if (temp) VipFreeVolume(temp);
		  return(PB);
		}
	      printf("Closing (with connectivity chamfer)\n");
	      if (VipConnectivityChamferClosing(temp, s2, connectivity, 
						algodefine) == PB)
		{
		  VipPrintfError("Error while closing");
		  if (vol) VipFreeVolume(vol);
		  if (temp) VipFreeVolume(temp);
		  return(PB);
		}
	    }
	  else
	    {
	      printf("Closing (with connectivity chamfer)\n");
	      if (VipConnectivityChamferClosing(temp, s2, connectivity, 
						algodefine) == PB)
		{
		  VipPrintfError("Error while closing");
		  if (vol) VipFreeVolume(vol);
		  if (temp) VipFreeVolume(temp);
		  return(PB);
		}
	      printf("Opening (with connectivity chamfer)\n");
	      if (VipConnectivityChamferOpening(temp, s1, connectivity, 
						algodefine) == PB) 
		{
		  VipPrintfError("Error while opening");
		  if (vol) VipFreeVolume(vol);
		  if (temp) VipFreeVolume(temp);
		  return(PB);
		}
	    }
	}
    }

  /* cherche le niveau maximum present pour changer le 0 en max+1 */
  maxImg = (int)VipGetVolumeMax(vol);
  if (maxImg >= 32767)
    {
      VipPrintfError("Cannot change the 0 label. Volume max is too high");
      if (vol) VipFreeVolume(vol);
      if (temp) VipFreeVolume(temp);
      return(PB);
    }

  maxImg++;
  printf("Replacing 0 by %d...\n", maxImg);
  if ((VipChangeIntLabel(vol, 0, maxImg)) == PB)
    {
      VipPrintfError("Error while changing label 0");
      if (vol) VipFreeVolume(vol);
      if (temp) VipFreeVolume(temp);
      return(PB);
    }

  /* masque l'image de depart par le resultat */
  printf("Applying mask...\n");
  if ((VipMaskVolume(vol, temp)) == PB)
    {
      VipPrintfError("Error while masking volume");
      if (vol) VipFreeVolume(vol);
      if (temp) VipFreeVolume(temp);
      return(PB);
    }
  VipFreeVolume(temp);

  /* diagramme de Voronoi */
  if (connectivity_activ == VFALSE)
    {
      if (algo == CHAMFER_TRANSFORMATION)
	{
	  printf("Computing Voronoi (with customized iterated geodesic)...\n");
	  temp = VipComputeCustomizedIteratedGeodesicVoronoi(vol, maxImg, 0, 
							     nitermax, 
							     xmasksize, 
							     ymasksize, 
							     zmasksize, 
							     multfactor);
	  if (!temp)
	    {
	      VipPrintfError("Error while computing Voronoi diagram");
	      if (vol) VipFreeVolume(vol);
	      return(PB);
	    }
	}
      else 
	{
	  printf("Computing Voronoi (with customized front propagation geodesic)...\n");
	  temp=VipComputeCustomizedFrontPropagationGeodesicVoronoi(vol,
								   maxImg,
								   0,
								   xmasksize,
								   ymasksize,
								   zmasksize,
								   multfactor);
	  if (!temp)
	    {
	      VipPrintfError("Error while computing Voronoi diagram");
	      if (vol) VipFreeVolume(vol);
	      return(PB);
	    }	  
	}
    }
  else
    {
      if (algo == CHAMFER_TRANSFORMATION)
	{
	  printf("Computing Voronoi (with connectivity iterated geodesic)...\n");
	  temp = VipComputeConnectivityIteratedGeodesicVoronoi(vol, maxImg, 0,
							       nitermax,
							       connectivity);
	  if (!temp)
	    {
	      VipPrintfError("Error while computing Voronoi diagram");
	      if (vol) VipFreeVolume(vol);
	      return(PB);
	    }
	}
      else
	{
	  printf("Computing Voronoi (with connectivity front propagation geodesic)...\n");
	  temp=VipComputeConnectivityFrontPropagationGeodesicVoronoi(vol, 
								     maxImg,
								     0,
								     connectivity);
	  if (!temp)
	    {
	      VipPrintfError("Error while computing Voronoi diagram");
	      if (vol) VipFreeVolume(vol);
	      return(PB);
	    }
	}
    }

  /* Sauvegarde */
  printf("Writing %s...\n", output);
  if (writelib == TIVOLI)
    {
      if (VipWriteTivoliVolume(temp, output) == PB) return(PB);
    }
  else if (writelib == SPM)
    {
      if (VipWriteSPMVolume(temp, output) == PB) return(PB);
    }
  else
    if (VipWriteVolume(temp, output) == PB) return(PB);

  VipFreeVolume(temp);
  VipFreeVolume(vol);

  return(EXIT_SUCCESS);
}

/*--------------------------------------------------------------------------*/
static int Usage()
{
  (void)fprintf(stderr,"Usage: VipAlternatedSequentialFilter\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t[-s[ize] {maximum opening size in mm (default:1 mm)}]\n");
  (void)fprintf(stderr,"\t\t[-S[ize] {maximum closing size in mm (default:same as opening size)}]\n");
  (void)fprintf(stderr,"\t\t[-N[cycle] {int (default:1)}]\n");
  (void)fprintf(stderr,"\t\t[-f[irst] {char: o or c (default:\"o\")}]\n");
  (void)fprintf(stderr,"\t\t[-a[lgo] {char: f or t (default:\"f\")}]\n");
  (void)fprintf(stderr,"\t\t[-n[itermax] {int (default:5)}]\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {image name (default:\"smoothed\")}]\n");
  (void)fprintf(stderr,"\t\t[-x[masksize] {int (default:3)}]\n");
  (void)fprintf(stderr,"\t\t[-y[masksize] {int (default:3)}]\n");
  (void)fprintf(stderr,"\t\t[-z[masksize] {int (default:3)}]\n");
  (void)fprintf(stderr,"\t\t[-m[ultfactor] {float (default:VIP_USUAL_DISTMAP_MULTFACT)}]\n");
  (void)fprintf(stderr,"\t\t[-c[onnectivity] {string:n/6/18/26/4/8/4s/8s/4c/8c (default:6)}]\n");
  (void)fprintf(stderr,"\t\t[-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-h[elp]\n");
  return(PB);
}

/*****************************************************/
static int Help()
{
  VipPrintfInfo("Succession of opening and closing on an object defined as all non zero points");
  (void)printf("The structuring element is a ball of radius size for a chamfer distance.\n");
  (void)printf("\n");
  (void)printf("Usage: VipAlternatedSequentialFilter\n");
  (void)printf("\t\t-i[nput] {image name} : object definition\n");
  (void)printf("\t\t[-s[ize] {maximum opening size in mm (default:1 mm)}]\n");
  (void)printf("\t\t[-S[ize] {maximum closing size in mm (default:same as opening size)}]\n");
  (void)printf("The maximum size corresponds to the last cycle structure element size.\n");
  (void)printf("For each iteration, the structure element size is Iter*SizeMax/Ncycle\n");
  (void)printf("\t\t[-N[cycle] number of cycle {int (default:1)}]\n");
  (void)printf("\t\t[-f[irst] {char: o or c (default:\"o\")}]\n");
  (void)printf("o:opening first,  c:closing first\n");
  (void)printf("\t\t[-a[lgo] {char: f or t (default:\"f\")}]\n");
  (void)printf("f:bounded thick front propagation, t: chamfer transformation\n");
  (void)printf("\t\t[-n[itermax] {int (default:5)}]\n");
  (void)printf("maximum number of iterations for Voronoi iterated chamfer transform\n");
  (void)printf("\t\t[-o[utput] {image name (default:\"smoothed\")}]\n");
  (void)printf("\t\t[-x[masksize] {int (default:3)}: chamfer mask xsize]\n");
  (void)printf("\t\t[-y[masksize] {int (default:3)}: chamfer mask ysize]\n");
  (void)printf("\t\t[-z[masksize] {int (default:3)}: chamfer mask zsize]\n");
  (void)printf("\t\t[-m[ultfactor] {float (default:VIP_USUAL_DISTMAP_MULTFACT)}]\n");
 (void)printf("\t\t[-c[onnectivity] {string:n/6/18/26/4/8/4s/8s/4c/8c (default:6)}]\n");
  (void)printf("\"n\" means not used. Suffixes \"s\" and \"c\" means sagittal and coronal orientations\n");
  (void)printf("\t\t[-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("\t\t[-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)printf("Forces the writing of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("\t\t[-h[elp]\n");
  return(PB);
}
