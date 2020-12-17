/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipBiasCorrection.c         * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 29/12/98
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
#include <vip/histo.h>
#include <vip/connex.h>
#include <vip/deriche.h>

#define F2D_REGULARIZED_FIELD 22
#define F3D_REGULARIZED_FIELD 33

#define MEAN_QUADRATIC_VARIATION 321
#define STANDARD_DEVIATION 432
#define GEOMETRY 543

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/
extern int  VipComputeStatInMaskVolume(Volume *vol, Volume *thresholdedvol, float *mean, float *sigma);

/*---------------------------------------------------------------------------*/
extern Volume *VipComputeBiasFieldMultiGrid(int mode, int dumb, Volume *vol, float undersampling, 
				     float Kentropy,
				     float Kregularization, float Koffset,
				     float amplitude,
				     float Tinit, float geom,int fieldtype,
				     int nb_sample_proba, float increment,
					    int ngrid, float RegulZTuning);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern Volume *VipResampleField(Volume *field, Volume *ima);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern int VipComputeUnbiasedVolume(Volume *field, Volume *ima);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Volume *VipComputeCompressedVolume(Volume *vol, int compression);
/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol);
  /*VIP_DEC_VOLUME(deriche);*/
  VIP_DEC_VOLUME(voltemp);
  VIP_DEC_VOLUME(thresholdedvol);
  VIP_DEC_VOLUME(converter);
  VIP_DEC_VOLUME(compressed);
  VIP_DEC_VOLUME(result);
  VIP_DEC_VOLUME(fullresult);    
  char *input = NULL;
  char fieldname[VIP_NAME_MAXLEN] = "field";
  int writefield = VTRUE;
  char output[VIP_NAME_MAXLEN] = "nobias";
  float amplitude = 1.1;
  /* temporary stuff */
  int readlib, writelib;
  int compression = 0;
  int compressionset = VFALSE;
  /*int tauto = VFALSE;*/
  int thresholdlow = 15;
  int thresholdlowset = VFALSE;
  int thresholdhigh = 100000;
  int thresholdhighset = VFALSE;
  int nInc =2;
  float Inc = 1.03;
  float Kentropy=1.;
  float Kregul=20.;
  float Koffset=0.5;
  float sampling = 16.;
  float temperature = 10.;
  float geom = 0.97;
  int dumb = VTRUE;
  VipHisto *histo, *comphisto;
  int ratio;
  int fieldtype = F3D_REGULARIZED_FIELD;
  int ngrid = 2;
  int init_threshold=0;
  float RegulZTuning=1.;
  int variance_threshold = -1;
  int variance_pourcentage = -1;
  int deriche_edges = -1;
  float lemax, tlow, thigh;
  int connectivity=CONNECTIVITY_26;
  Volume *edges=NULL;
  Volume *discard=NULL;
  int mode = MEAN_QUADRATIC_VARIATION;
  /*float max_gradient;*/
  /*float mean=0;*/
  /*float sigma = 0;*/
  /*VipHisto *histo_edges;*/
  int i;
  /*int threshold_edges = 2;*/

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
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-field", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(fieldname,argv[i]);
	}
      else if (!strncmp (argv[i], "-amplitude", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  amplitude = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-nInc", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  nInc = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-Increment", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  Inc = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-Grid", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  ngrid = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-Dimfield", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='2') fieldtype = F2D_REGULARIZED_FIELD;
	  else if(argv[i][0]=='3') fieldtype = F3D_REGULARIZED_FIELD;
	  else  
	    {
	      VipPrintfError("The Dimfield option has to be either 2 or 3");
	      VipPrintfExit("(commandline)VipBiasCorrection");
	      return(VIP_CL_ERROR);
	    }
	} 
      else if (!strncmp (argv[i], "-Temperature", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  temperature = atof(argv[i]);
	}
     else if (!strncmp (argv[i], "-geom", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  geom = atof(argv[i]);
	}
     else if (!strncmp (argv[i], "-ZregulTuning", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  RegulZTuning = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-Kentropy", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  Kentropy = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-Kregul", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  Kregul = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-Koffset", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  Koffset = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-compression", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  compression = atoi(argv[i]);
	  compressionset = VTRUE;
	}
      else if (!strncmp (argv[i], "-tlow", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  thresholdlow = atoi(argv[i]);
	  thresholdlowset = VTRUE;
	}
      else if (!strncmp (argv[i], "-thigh", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  thresholdhigh = atoi(argv[i]);
	  thresholdhighset = VTRUE;
	}
  /*
      else if (!strncmp (argv[i], "-tauto", 2)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          if(argv[i][0]=='y') tauto = VTRUE;
          else if(argv[i][0]=='n') tauto = VFALSE;
          else
            {
              VipPrintfError("y/n choice!");
              VipPrintfExit("tauto: (commandline)VipBiasCorrection");
              return(VIP_CL_ERROR);
            }
        }*/
      else if (!strncmp (argv[i], "-vtariance", 3)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          variance_threshold = atoi(argv[i]);
        }
      else if (!strncmp (argv[i], "-vpourcentage", 3)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          variance_pourcentage = atoi(argv[i]);
        }
      else if (!strncmp (argv[i], "-edges", 2)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          if(argv[i][0]=='2') deriche_edges = 2;
          else if(argv[i][0]=='3') deriche_edges = 3;
          else
            {
              VipPrintfError("2/3 choice!");
              VipPrintfExit("2D/3D edges: (commandline)VipBiasCorrection");
              return(VIP_CL_ERROR);
            }
        }
      else if (!strncmp (argv[i], "-sampling", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  sampling = atof(argv[i]);
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
	      VipPrintfExit("(commandline)VipBiasCorrection");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-mode", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='m') mode = MEAN_QUADRATIC_VARIATION;
	  else if(argv[i][0]=='s') mode = STANDARD_DEVIATION;
	  else if(argv[i][0]=='g') mode = GEOMETRY;
	  else
	    {
	      VipPrintfError("This mode is not implemented (choose among m, s)");
	      VipPrintfExit("(commandline)VipBiasCorrection");
	      return(VIP_CL_ERROR);
	    }
	}
     else if (!strncmp (argv[i], "-dumb", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') dumb=VTRUE;
	  else if(argv[i][0]=='n') dumb = VFALSE;
	  else
	    {
	      VipPrintfError("dumb: y/n choice");
	      VipPrintfExit("(commandline)VipBiasCorrection");
	      return(VIP_CL_ERROR);
	    }
	}
     else if (!strncmp (argv[i], "-Fwrite", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') writefield=VTRUE;
	  else if(argv[i][0]=='n') writefield = VFALSE;
	  else
	    {
	      VipPrintfError("Fwrite: y/n choice");
	      VipPrintfExit("(commandline)VipBiasCorrection");
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
	      VipPrintfExit("(commandline)VipBiasCorrection");
	      return(VIP_CL_ERROR);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else
	{
	  printf("Bad argument...\n");
	  return(Usage());
	}
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipBiasCorrection");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
    }

  readlib = readlib; /* compilation warning... */
  vol = VipReadVolumeWithBorder(input,0);

  if(vol==NULL) return(VIP_CL_ERROR);

  if(mVipVolType(vol)==U8BIT)
      {
	  converter = VipTypeConversionToS16BIT(vol,RAW_TYPE_CONVERSION);
	  if(converter==PB) return(VIP_CL_ERROR);
	  VipFreeVolume(vol);
	  vol = converter;
      }

  if (fieldtype == F2D_REGULARIZED_FIELD ) printf("2D field regularization\n");
  if (fieldtype == F3D_REGULARIZED_FIELD ) printf("3D field regularization\n");

 
  /*discard background before computing actual compression*/
  if(compressionset==VFALSE)
      {
	if(thresholdlowset==VFALSE)
	  {         
	    printf("Computing threshold oriented compression factor:");
	    histo = VipComputeVolumeHisto(vol);
	    ratio = 0;
	    comphisto = VipGetPropUndersampledHisto( histo, 95, &ratio, &compression, 0, 60);
	    VipFreeHisto(histo);
	    VipFreeHisto(comphisto);
	    printf("%d\n",ratio);
	    init_threshold = thresholdlow << compression ;
	  }
	else init_threshold = thresholdlow;
	printf("Initial threshold %d\n",init_threshold);
	thresholdedvol = VipCreateSingleThresholdedVolume( vol,
						     GREATER_THAN, init_threshold,GREYLEVEL_RESULT); 
	if(!thresholdedvol) return(VIP_CL_ERROR);
	printf("Computing actual compression factor:");
	histo = VipComputeVolumeHisto(thresholdedvol);
	VipFreeVolume(thresholdedvol);
	ratio = 0;
	/* discard potential modes higher than white matter*/
	comphisto = VipGetPropUndersampledHisto( histo, 95, &ratio, &compression, thresholdlow, 60 );
	printf("%d\n",ratio);
	VipFreeHisto(comphisto);
	VipFreeHisto(histo);	  
      }
  if (compression !=0)
      {
	  printf("Compressing volume (forgetting %d bits)...",compression);
	  fflush(stdout);
      }
  
  /* compress anyway because of the following threshold*/
  compressed = VipComputeCompressedVolume( vol, compression);
  if(compressed == PB) return(VIP_CL_ERROR);
  if (compression !=0)
    {
      printf("\n");
    }
  /* 
  if(VipWriteTivoliVolume(compressed,"compressed")==PB) return(VIP_CL_ERROR);
  */
  printf("Computing Bias with Kentropy = %f, Kregularization = %f, Koffset = %f\n",
	 Kentropy,Kregul,Koffset);
  if(thresholdlowset==VTRUE)
      thresholdlow = thresholdlow >> compression ;
  if(thresholdhighset==VTRUE)
      thresholdhigh = thresholdhigh >> compression ;
  printf("Low threshold: %d, High threshold: %d (after compression)\n", thresholdlow, thresholdhigh);
  VipDoubleThreshold(compressed,VIP_BETWEEN,thresholdlow,thresholdhigh,GREYLEVEL_RESULT);

  /*july 20, 2004, Try some refinement, in parallel with histo analysis, JFM*/

  if(variance_threshold!=-1 || variance_pourcentage!=-1 || deriche_edges!=-1)
    {
      if (deriche_edges!=-1)
        {
          if (deriche_edges==2)
            {
              edges = VipDeriche2DGradientNorm(vol, 2.5, NEW_VOLUME, DERICHE_EXTREMA, 0.);
              connectivity = CONNECTIVITY_8;
            }
          else
            {
              edges = VipCopyVolume(vol,"edges");
              if (!edges) return(VIP_CL_ERROR);
              if(VipDeriche3DGradientNorm(edges, 2.5, DERICHE_EXTREMA, 0.)==PB) return(VIP_CL_ERROR);
            }
          lemax = VipGetVolumeMax(edges); /*NB choix un peu arbitraire mais pas tres important*/
          tlow = 0.04*lemax;
          thigh = 0.08*lemax;
          if(VipHysteresisThresholding(edges,connectivity,SAME_VOLUME,CONNEX_BINARY,tlow,thigh,HYSTE_NUMBER,1)==PB) return(VIP_CL_ERROR);
        }
      if (variance_threshold!=-1 || variance_pourcentage!=-1)
        {                            
          voltemp = VipReadVolumeWithBorder(input,1);

          if(voltemp==NULL) return(VIP_CL_ERROR);

          discard = VipComputeVarianceVolume(voltemp);
          VipFreeVolume(voltemp);
          if (discard==PB) return(VIP_CL_ERROR);
          if (variance_pourcentage!=-1)
            {
              variance_threshold = VipPourcentageLowerThanThreshold(discard, 5, variance_pourcentage);
            }
          printf("Threshold on local deviations: %d\n", variance_threshold);
          VipSingleThreshold( discard, GREATER_OR_EQUAL_TO, variance_threshold, BINARY_RESULT );
          if (edges!=NULL)
            {
              VipMerge(discard,edges,VIP_MERGE_ALL_TO_ONE,0,255);
              VipFreeVolume(edges);
            }
        }
      else discard = edges;
      VipInvertBinaryVolume(discard);
      VipMaskVolume(compressed,discard);
      VipFreeVolume(discard);
    }



  result = VipComputeBiasFieldMultiGrid(mode, dumb,compressed,sampling,Kentropy,Kregul,Koffset,amplitude,temperature,geom,fieldtype,nInc,Inc,ngrid,RegulZTuning);

  if(result==PB) return(VIP_CL_ERROR);

  VipFreeVolume(compressed);
  /* debug  vol = compressed;*/

  fullresult = VipResampleField(result, vol);
  VipFreeVolume(result);
  
  if(fullresult==PB) return(VIP_CL_ERROR);
  if(writefield==VTRUE)
      {
	  if(VipWriteVolume(fullresult,fieldname)==PB) return(VIP_CL_ERROR);
      }

  printf("Correcting volume...\n");
  /*  vol = VipReadVolume("4857tour");*/
  fflush(stdout);
  VipComputeUnbiasedVolume(fullresult,vol);

  VipFreeVolume(fullresult);

  writelib = writelib; /* compilation warning... */
  if(VipWriteVolume(vol,output)==PB) return(VIP_CL_ERROR);

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipBiasCorrection\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"nobias\")}]\n");
  (void)fprintf(stderr,"        [-f[ield] {bias field name (default:\"field\")}]\n");
  (void)fprintf(stderr,"        [-D[imfield] {type of field (2/3) (default:3)}]\n");
  (void)fprintf(stderr,"        [-F[write] {write field: y/n (default:y)}]\n");
  (void)fprintf(stderr,"        [-d[umb] {y/n (default:y)}]\n");
  (void)fprintf(stderr,"        [-Ke[ntropy] {float  (default:1.)}]\n");
  (void)fprintf(stderr,"        [-Kr[egul] {float  (default:20.)}]\n");
  (void)fprintf(stderr,"        [-Ko[ffset] {float  (default:0.5)}]\n");
  (void)fprintf(stderr,"        [-c[ompression] {int [0,14]  (default:auto)}]\n");
  (void)fprintf(stderr,"        [-tl[ow] {int (default:3*2^compression)}]\n");
  (void)fprintf(stderr,"        [-th[igh] {int (default:not used)}]\n");
  (void)fprintf(stderr,"        [-e[dges] {char (default:not used, 2/3)}]\n");
  (void)fprintf(stderr,"        [-vt[ariance] {int (default:not used, else int threshold)}]\n");
  (void)fprintf(stderr,"        [-vp[ourcentage] {int (default:not used, else int threshold)}]\n");
  (void)fprintf(stderr,"        [-T[emperature] {float (default:10.)}]\n");
  (void)fprintf(stderr,"        [-g[eometric] {float (default:0.97)}]\n");
  (void)fprintf(stderr,"        [-n[Increment] {int (default:2)}]\n");
  (void)fprintf(stderr,"        [-G[rid] {int (default:2)}]\n");
  (void)fprintf(stderr,"        [-I[ncrement] {float (default:1.03)}]\n");
  (void)fprintf(stderr,"        [-s[ampling] {float  (default:16mm)}]\n");
  (void)fprintf(stderr,"        [-Z[regulTuning] {float  (default:1.)}]\n");
  (void)fprintf(stderr,"        [-a[mplitude] {float ]1,10]  (default:1.1)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: a, v, s or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help() 
{
 
  VipPrintfInfo("Computes a smooth multiplicative field which corrects for non stationarities.\n");
  (void)printf("This field aims at minimizing the volume entropy (= minimizing information...).\n");
  (void)printf("A tradeoff is found between this entropy and the internal energy of a membrane using annealing.\n");
  (void)printf("\n");
  (void)printf("Usage: VipBiasCorrection\n");
  (void)printf("        -i[nput] {image name}\n");
  (void)printf("        [-o[utput] {image name (default:\"nobias\")}]\n");
  (void)printf("        [-f[ield] {bias field name (default:\"field\")}]\n");
  (void)printf("        [-D[imfield] {type of field (2/3) (default:3)}]\n");
  (void)printf("2: the correction field is constant by slice, 3: constant by cubes\n");
  (void)printf("        [-F[write] {write field: y/n (default:y)}]\n");
  (void)printf("        [-d[umb] {y/n (default:y)}]\n");
  (void)printf("        [-Ke[ntropy] {float  (default:1.)}]\n");
  (void)printf("Weight of the volume entropy which will be multiplied by the field size\n");
  (void)printf("        [-Kr[egul] {float  (default:20.)}]\n");
  (void)printf("Weight of the membrane/spline energy (sum of the squared log of the ratio between 6-neighbors)\n");
  (void)printf("        [-Ko[ffset] {float  (default:0.5)}]\n");
  /*  (void)printf("Weight of the sum of squared log (=distance to the uniform field with 1. value)\n");*/
  (void)printf("Weight of the squared difference between old and new mean\n");
  (void)printf("        [-c[ompression] {int [0,14]  (default:auto)}]\n");
  (void)printf("The number of bits which are discarted during the volume entropy computation (= /2^compression))\n");
  (void)printf("        [-tl[ow] {int (default:3*2^compression)}]\n");
  (void)printf("        [-th[igh] {int (default:not used)}]\n");
  (void)printf("        [-e[ges] {int (default:not used, else 2/3)}]\n");
  (void)printf("remove 2D/3D edges for histogram estimation (discard partial volume)\n"); 
  (void)printf("        [-vt[ariance] {int (default:not used, else int threshold)}]\n");
  (void)printf("high threshold on standard deviation in 26-neighborhood for inclusion in histogram\n"); 
  (void)printf("Values beyond this threshold are not taken into account.\n");
  (void)printf("In return, the field correction is applied to all values to get the final result\n");
  (void)printf("        [-vp[ourcentage] {int (default:not used, else int threshold)}]\n");
  (void)printf("Pourcentage of non null points kept with a ranking stemming from local variance\n");
  (void)printf("        [-T[emperature] {float (default:10.)}]\n");
  (void)printf("Initial temperature for annealing\n");
  (void)printf("        [-G[rid] {int (default:2)}]\n");
  (void)printf("Number of grid for minimization (annealing on the highest only)\n");
  (void)printf("        [-g[eometric] {float (default:0.97)}]\n");
  (void)printf("Geometric decreasing for annealing schedule\n");
  (void)printf("        [-s[ampling] {float  (default:16mm)}]\n");
  (void)printf("The sampling of the field: one value for a parallelepipedic volume of points.\n");
  (void)printf("In each direction, the sampling is: sampling/DirVoxelSize.\n");
  (void)printf("The field is a 0-order spline during optimization (piecewise constant)\n");
  (void)printf("               3-order spline during the final bias correction (piecewise linear)\n");
  (void)printf("        [-n[Increment] {int (default:2)}]\n");
  (void)printf("Minimization is performed using a Gibbs sampler or a ICM (deterministic) approach with\n");
  (void)printf("several multiplicative increments are tires for each field point:\n");
  (void)printf("1/I^n,...,1/I,I,...I^n:\n");
  (void)printf("        [-I[ncrement] {float (default:1.03)}]\n");
  (void)printf("elementary multiplicative increment (see above):\n");
  (void)printf("        [-a[mplitude] {float ]1,10]  (default:1.1)}]\n");
  (void)printf("The field random initialization is in the range [1/amplitude, amplitude]\n");
  (void)printf("        [-Z[regulTuning] {float  (default:1)}]\n");
  (void)printf("Multiplicative factor for regularization in z direction (bad coil)\n");
  (void)printf("        [-r[eadformat] {char: a, v, s or t (default:any)}]\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)printf("        [-h[elp]\n");
  printf("More information in:\n");
  printf("Entropy minimization for automatic correction\n");
  printf("of intensity non uniformity, J.-F. Mangin,\n");
  printf("MMBIA (Math. Methods in Biomed. Image Analysis),\n");
  printf("Hilton Head Island, South Carolina, IEEE Press\n");
  printf("162-169, 2000\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
int  VipComputeStatInMaskVolume(Volume *vol, Volume *thresholdedvol, float *mean, float *sigma)
{
  VipOffsetStruct *vos;
  int ix, iy, iz;
  Vip_S16BIT *ptr, *cptr;
  double sum;
  double temp;
  int n;
  double threshold;


     vos = VipGetOffsetStructure(vol);
     ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
     cptr = VipGetDataPtr_S16BIT( thresholdedvol  ) + vos->oFirstPoint;
     sum = 0.;
     n = 0;
     for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
       {
	 for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	   {
	     for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	       {
		 if(*cptr)
		   {
		     n++;
		     sum += *ptr;
		   }
		 ptr++;
                 cptr++;
	       }
	     ptr += vos->oPointBetweenLine;  /*skip border points*/
	     cptr += vos->oPointBetweenLine;  /*skip border points*/
	   }
	 ptr += vos->oLineBetweenSlice; /*skip border lines*/
	 cptr += vos->oLineBetweenSlice; /*skip border lines*/
       }
     if(n==0 || n==1)
       {
         VipPrintfWarning ("empty volume in VipComputeStatInMaskVolume");
         return(PB);
       }
     *mean = (float)(sum/n);

     ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
     cptr = VipGetDataPtr_S16BIT( thresholdedvol ) + vos->oFirstPoint;
     sum = 0.;
     for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
       {
	 for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	   {
	     for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	       {
		 if(*cptr)
		   {
                     temp = *ptr-*mean;
		     sum += temp*temp;
		   }
		 cptr++;
		 ptr++;
	       }
	     ptr += vos->oPointBetweenLine;  /*skip border points*/
	     cptr += vos->oPointBetweenLine;  /*skip border points*/
	   }
	 ptr += vos->oLineBetweenSlice; /*skip border lines*/
	 cptr += vos->oLineBetweenSlice; /*skip border lines*/
       }
     *sigma = (float)sqrt((double)(sum/(n-1)));


     ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
     cptr = VipGetDataPtr_S16BIT( thresholdedvol ) + vos->oFirstPoint;
     threshold = *mean + 3 * *sigma;
     for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
       {
	 for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	   {
	     for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	       {
		 if(*cptr)
		   {
                     if (*ptr>threshold)
                       {
                         temp = *ptr-*mean;
                         sum -= temp*temp;
                         n--;
                       }
		   }
		 cptr++;
		 ptr++;
	       }
	     ptr += vos->oPointBetweenLine;  /*skip border points*/
	     cptr += vos->oPointBetweenLine;  /*skip border points*/
	   }
	 ptr += vos->oLineBetweenSlice; /*skip border lines*/
	 cptr += vos->oLineBetweenSlice; /*skip border lines*/
       }
     *sigma = (float)sqrt((double)(sum/(n-1)));

     return(OK);
   
}
