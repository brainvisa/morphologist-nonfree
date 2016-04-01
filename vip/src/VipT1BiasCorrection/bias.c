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
#include <vip/connex.h>
#include <vip/distmap.h>
#include <vip/deriche.h>
#include <vip/geometry.h>
#include <vip/gaussian.h>
#include <vip/histo.h>
#include <vip/morpho.h>	
#include <vip/brain.h>	


#define F2D_REGULARIZED_FIELD 22
#define F3D_REGULARIZED_FIELD 33

#define MEAN_QUADRATIC_VARIATION 321
#define STANDARD_DEVIATION 432
#define GEOMETRY 543

#define VIP_BIAS_HUMAN_KEEP 75.0
#define VIP_BIAS_MACACA_KEEP 35.0
#define VIP_BIAS_T1_DERICHEPCT 0.05
#define VIP_BIAS_T1_MCTH -0.4
#define VIP_BIAS_T1_MCMETH LOWER_OR_EQUAL_TO
#define VIP_BIAS_T2_DERICHEPCT 0.005
#define VIP_BIAS_T2_MCTH 0.4
#define VIP_BIAS_T2_MCMETH GREATER_OR_EQUAL_TO

/*---------------------------------------------------------------------------*/
static int Usage();
static int Help();
/*---------------------------------------------------------------------------*/
static Volume *VipComputeSmoothCrest(Volume *crest, Volume *vol);
static Volume *VipComputeCrestGradExtrema(Volume *grad, Volume *vol);
// extern int  VipComputeRobustStatInMaskVolume(Volume *vol, Volume *thresholdedvol, float *mean, float *sigma, int robust);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
extern Volume *VipComputeT1BiasFieldMultiGrid(
int mode,
int dumb,
Volume *vol,
Volume *crest,
float undersampling,
float Kentropy,
float Kregularization,
float Kcrest,
float Koffset,
float amplitude,
float Tinit,
float geom,
int fieldtype,
int nb_sample_proba,
float increment,
int ngrid,
float RegulZTuning);
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

/*---------------------------------------------------------------------------*/
extern Volume *VipComputeCrestGrad(Volume *crest, Volume *vol);
/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  /*declarations and initializations*/
  VIP_DEC_VOLUME(target);
  VIP_DEC_VOLUME(arrow);
  VIP_DEC_VOLUME(variance_brute);
  VIP_DEC_VOLUME(mask);
  VIP_DEC_VOLUME(vol);
  VIP_DEC_VOLUME(deriche);
  VIP_DEC_VOLUME(deriche_norm);
  VIP_DEC_VOLUME(thresholdedvol);
  VIP_DEC_VOLUME(masked);
  VIP_DEC_VOLUME(converter);
  VIP_DEC_VOLUME(compressed);
  VIP_DEC_VOLUME(result);
  VIP_DEC_VOLUME(fullresult);
  VIP_DEC_VOLUME(mc);
  VIP_DEC_VOLUME(smooth);
  VIP_DEC_VOLUME(white_crest);
  VIP_DEC_VOLUME(white_cresttemp);
  VIP_DEC_VOLUME(gradient);
  VIP_DEC_VOLUME(extrema);
  VIP_DEC_VOLUME(variance);
  VIP_DEC_VOLUME(classif);
  VIP_DEC_VOLUME(copyvol);
  VIP_DEC_VOLUME(copyvol2);
  VIP_DEC_VOLUME(bias);
  int flag8bit = VFALSE;
  char *input = NULL;
  char fieldname[VIP_NAME_MAXLEN] = "biasfield";
  int writefield = VFALSE;
  char wridgesname[VIP_NAME_MAXLEN] = "white_ridges";
  int writeridges = VFALSE;
  char edgesname[VIP_NAME_MAXLEN] = "edges";
  int writeedges = VFALSE;
  char variancename[VIP_NAME_MAXLEN] = "variance";
  int writevariance = VFALSE;
  char meancurvaturename[VIP_NAME_MAXLEN] = "mean_curvature";
  int writemeancurvature = VFALSE;
  char hfilteredname[VIP_NAME_MAXLEN] = "hfiltered";
  int writehfiltered = VFALSE;
  int readridges = VFALSE;
  char output[VIP_NAME_MAXLEN] = "nobias";
  float amplitude = 1.1;
  int Last = 0;
  float mcsigma = 1.;
  /*temporary stuff*/
  int readlib, writelib;
  int compression = 0;
  int compressionset = VFALSE;
  int tauto = VTRUE;
  float thbackground = 15;
  int thresholdlow = 15;
  int thresholdlowset = VFALSE;
  int thresholdhigh = 100000;
  int thresholdhighset = VFALSE;
  float mult_factor = 10.;
  int nInc = 2;
  float Inc = 1.03;
  float Kentropy = 1.;
  float Kregul = 50.;
  float Koffset = 0.5;
  float Kcrest = 20.;
  float sampling = 16.;
  float temperature = 10.;
  float geom = 0.97;
  int dumb = VTRUE;
  VipHisto *histo, *comphisto;
  int ratio = 0;
  int fieldtype = F3D_REGULARIZED_FIELD;
  int ngrid = 2;
  float RegulZTuning = 1.;
  int variance_threshold = -1;
  int variance_pourcentage = 75;
  int deriche_edges = -1;
  float lemax, tlow, thigh;
  int connectivity = CONNECTIVITY_26;
  Volume *edges = NULL;
  int mode = GEOMETRY;
  float max_gradient;
  float min_volume;
  float mean = 0;
  float sigma = 0;
  VipHisto *histo_edges; 
  int i;
  int threshold_edges = 2;
  int controlled = VFALSE;
  int xCA = 0, yCA = 0, zCA = 0; 
  int xCP = 0, yCP = 0, zCP = 0;
  int xP = 0, yP = 0, zP = 0;
  char *point_filename = NULL;
  int talset = VFALSE;
  VipTalairach tal, *coord = NULL;
  int docorrection = VTRUE;
  VipHisto *shorthisto, *historesamp = NULL;
  Vip1DScaleSpaceStruct *volstruct;
  SSSingularity *slist = NULL;
  SSCascade *clist = NULL, *chigh;
  VipT1HistoAnalysis *ana = 0;
  int undersampling_factor = 0;
  int factor;
  int n, u;
  int undersampling_factor_possible[5][5] = {{0},{0},{0},{0},{0}};
  int j = 0, k = 0, l = 0;
  float contrast = 0, ratio_GW = 0;
  float little_opening_size;
  int random_seed = time(NULL);
// extend cases T1/T2, human/macaca >
  float keep = VIP_BIAS_HUMAN_KEEP;
  int keepset = VFALSE;
  float derichepct = VIP_BIAS_T1_DERICHEPCT;
  int derichepctset = VFALSE;
  float mcthreshold = VIP_BIAS_T1_MCTH;
  int mcthresholdset = VFALSE;
  int mcmethod = VIP_BIAS_T1_MCMETH;
  int mcmesthodset = VFALSE;
// < extend cases
  
  readlib = ANY_FORMAT;
  writelib = TIVOLI;
  
  /*loop on command line arguments*/
  for(i=1; i<argc; i++)
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
      else if (!strncmp (argv[i], "-vname", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(variancename,argv[i]);
	}
      else if (!strncmp (argv[i], "-ename", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(edgesname,argv[i]);
	}
     else if (!strncmp (argv[i], "-eWrite", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') writeedges=VTRUE;
	  else if(argv[i][0]=='n') writeedges = VFALSE;
	  else
	    {
	      VipPrintfError("eWrite: y/n choice");
	      VipPrintfExit("(commandline)VipT1BiasCorrection");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-hname", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(hfilteredname,argv[i]);
	}
      else if (!strncmp (argv[i], "-hWrite", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') writehfiltered=VTRUE;
	  else if(argv[i][0]=='n') writehfiltered = VFALSE;
	  else
	    {
	      VipPrintfError("hWrite: y/n choice");
	      VipPrintfExit("(commandline)VipT1BiasCorrection");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-mname", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(meancurvaturename,argv[i]);
	}
      else if (!strncmp (argv[i], "-wridges", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(wridgesname,argv[i]);
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
      else if (!strncmp (argv[i], "-Dimfield", 3)) 
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
      else if (!strncmp (argv[i], "-Kcrest", 3)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          Kcrest = atof(argv[i]);
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
        }
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
          else if(argv[i][0]=='n') deriche_edges = -1;
          else
            {
              VipPrintfError("n/2/3 choice!");
              VipPrintfExit("2D/3D edges: (commandline)VipBiasCorrection");
              return(VIP_CL_ERROR);
            }
        }
      else if (!strncmp(argv[i], "-srand", 6)){
    	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
    	  random_seed = atoi(argv[i]);
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
     else if (!strncmp (argv[i], "-mWrite", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') writemeancurvature=VTRUE;
	  else if(argv[i][0]=='n') writemeancurvature = VFALSE;
	  else
	    {
	      VipPrintfError("vWrite: y/n choice");
	      VipPrintfExit("(commandline)VipT1BiasCorrection");
	      return(VIP_CL_ERROR);
	    }
	}
     else if (!strncmp (argv[i], "-mcsigma", 4)) 
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          mcsigma = atof(argv[i]);
        }
     else if (!strncmp (argv[i], "-Last", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          if(argv[i][0]=='a') Last = 3000;
	  else Last = atoi(argv[i]);
	}
     else if (!strncmp (argv[i], "-Points", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  point_filename = argv[i];
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
	      VipPrintfExit("(commandline)VipT1BiasCorrection");
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
	      VipPrintfExit("(commandline)VipT1BiasCorrection");
	      return(VIP_CL_ERROR);
	    }
	}
     else if (!strncmp (argv[i], "-vWrite", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') writevariance=VTRUE;
	  else if(argv[i][0]=='n') writevariance = VFALSE;
	  else
	    {
	      VipPrintfError("vWrite: y/n choice");
	      VipPrintfExit("(commandline)VipT1BiasCorrection");
	      return(VIP_CL_ERROR);
	    }
	}
     else if (!strncmp (argv[i], "-Wwrite", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') writeridges=VTRUE;
	  else if(argv[i][0]=='n') writeridges = VFALSE;
	  else if(argv[i][0]=='r') 
            {
              writeridges = VFALSE;
              readridges = VTRUE;
            }
	  else
	    {
	      VipPrintfError("Wwrite: y/n/r choice");
	      VipPrintfExit("(commandline)VipT1BiasCorrection");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-Dcorrect", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') docorrection = VTRUE;
	  else if(argv[i][0]=='n') docorrection = VFALSE;
          else
	    {
	      VipPrintfError("Dcorrect: y/n choice");
	      VipPrintfExit("(commandline)VipT1BiasCorrection");
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
	      VipPrintfExit("(commandline)VipT1BiasCorrection");
	      return(VIP_CL_ERROR);
	    }
	}
// extend cases T1/T2 - human/macaque >
      else if( !strncmp(argv[i],"-Dkeep",3) )
      {
        if( ++i >= argc || !strncmp(argv[i],"-",1) ) return(Usage());
        keep = atof(argv[i]);
        keepset = VTRUE;
        Last = 3000;
      }
      else if( !strncmp(argv[i],"-pvariance",3) )
      {
        if( ++i >= argc || !strncmp(argv[i],"-",1) ) return(Usage());
        derichepct = atof(argv[i]);
        derichepctset = VTRUE;
      }
      else if( !strncmp(argv[i],"-Cthreshold",3) )
      {
        if( ++i >= argc ) return(Usage());
        mcthreshold = atof(argv[i]);
        mcthresholdset = VTRUE;
      }
      else if( !strncmp(argv[i],"-Cmethod",3) )
      {
        if( ++i >= argc || !strncmp(argv[i],"-",1) ) return(Usage());
        if( !strncmp(argv[i],"LOWER_OR_EQUAL_TO",17) )
          mcmethod = LOWER_OR_EQUAL_TO;
        else if( !strncmp(argv[i],"LE",2) )
          mcmethod = LOWER_OR_EQUAL_TO;
        else if( !strncmp(argv[i],"GREATER_OR_EQUAL_TO",19) )
          mcmethod = GREATER_OR_EQUAL_TO;
        else if( !strncmp(argv[i],"GE",2) )
          mcmethod = GREATER_OR_EQUAL_TO;
        else
          printf( "-Cmethod: Unsupported thresholding method" );
          return(Usage());
        mcmethod = VTRUE;
      }
      else if( !strncmp(argv[i],"-Specie",2) )
      {
        if( ++i >= argc || !strncmp(argv[i],"-",1) ) return(Usage());
        if( !strncmp(argv[i],"Human",1) )
        {
          if( keepset == VFALSE )
            keep = VIP_BIAS_HUMAN_KEEP;
        }
        else if( !strncmp(argv[i],"Macaca",1) )
        {
          if( keepset == VFALSE )
            keep = VIP_BIAS_MACACA_KEEP;
        }
        Last = 3000;
      }
      else if( !strncmp(argv[i],"-Cweight",3) )
      {
        if( ++i >= argc || !strncmp(argv[i],"-",1) ) return(Usage());
        if( !strncmp(argv[i],"T1",2) )
        {
          if( derichepctset == VFALSE )
            derichepct = VIP_BIAS_T1_DERICHEPCT;
          if( mcthresholdset == VFALSE )
            mcthreshold = VIP_BIAS_T1_MCTH;
          if( mcmesthodset == VFALSE )
            mcmethod = VIP_BIAS_T1_MCMETH;
        }
        else if( !strncmp(argv[i],"T2",2) )
        {
          if( derichepctset == VFALSE )
            derichepct = VIP_BIAS_T2_DERICHEPCT;
          if( mcthresholdset == VFALSE )
            mcthreshold = VIP_BIAS_T2_MCTH;
          if( mcmesthodset == VFALSE )
            mcmethod = VIP_BIAS_T2_MCMETH;
        }
      }
// < extend cases
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
      VipPrintfError("Input arg is required by VipBiasCorrection");
      return(Usage());
  }
  if (VipTestImageFileExist(input)==PB)
  {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
  }
  
  srand(random_seed);
  
  if (fieldtype==F2D_REGULARIZED_FIELD) printf("2D field regularization\n");
  if (fieldtype==F3D_REGULARIZED_FIELD) printf("3D field regularization\n");
  
  /* PREPROCESSING: better estimation of histo
     estimation of white matter crest line */
  
  /*decembre 04, Montreal, develop automatic definition
    of threshold for tissues/background*/
  
  vol = VipReadVolumeWithBorder(input,0);
  min_volume = VipGetVolumeMin(vol);
  printf("min_volume=%f\n", min_volume), fflush(stdout);
  VipFreeVolume(vol);
  
  if(tauto==VTRUE)
  {
      /*je plonge le volume ds un plus grand a cause des images normalisees,
        a la SPM, avec la tete coupee. Il n'y a plus de contour sur les bords
        et on chope trop de tissus avec extedge*/
      vol = VipReadVolumeWithBorder(input,3);

      if(mVipVolType(vol)==U8BIT)
      {
          converter = VipTypeConversionToS16BIT(vol, RAW_TYPE_CONVERSION);
          if(converter==PB) return(VIP_CL_ERROR);
          flag8bit = VTRUE;
          VipFreeVolume(vol);
          vol = converter;
      }

      little_opening_size = 0.1;
      if(mVipVolVoxSizeX(vol)>little_opening_size) little_opening_size=mVipVolVoxSizeX(vol)+0.1;
      if(mVipVolVoxSizeY(vol)>little_opening_size) little_opening_size=mVipVolVoxSizeY(vol)+0.1;
      if(mVipVolVoxSizeZ(vol)>little_opening_size) little_opening_size=mVipVolVoxSizeZ(vol)+0.1;
      printf("little_opening_size=%f\n", little_opening_size), fflush(stdout);
      
      if(Last==3000 && point_filename!=NULL)
      {
          if(GetCommissureCoordinates(vol, point_filename, &tal,
                                      xCA, yCA, zCA, xCP, yCP, zCP,
                                      xP, yP, zP, talset)!=PB)
          {
              coord = &tal;
              xCA = (int)(coord->AC.x); yCA = (int)(coord->AC.y); zCA = (int)(coord->AC.z);
              xCP = (int)(coord->PC.x); yCP = (int)(coord->PC.y); zCP = (int)(coord->PC.z);
              xP = (int)(coord->Hemi.x); yP = (int)(coord->Hemi.y); zP = (int)(coord->Hemi.z);
              
              Last = (int)(mVipVolSizeZ(vol) - ((2*zCP-zCA) + (keep/mVipVolVoxSizeZ(vol))));
              if(Last<0) Last = 0;
          }
          else
          {
              printf("Something went wrong during the reading of the commissure coordinates.\n");
              Last = 0;
          }
      }
      else
      {
          printf("Commissure Coordinates are necessary to delete automatically the last slides\n");
          Last = 0;
      }
      printf("deleting last %d slices\n",Last);
      for(i=0; i<Last; i++)
          VipPutOneSliceTwoZero(vol,mVipVolSizeZ(vol)-i-1);
      
      VipSetBorderLevel(vol, 0);
      VipResizeBorder(vol, 0);
      
      deriche = VipCopyVolume(vol, "deriche");
      if (deriche==PB) return(VIP_CL_ERROR);
      if (VipDeriche3DGradientNorm(deriche, 2., DERICHE_EXTREMA, 0.)==PB) return(VIP_CL_ERROR);
      /*compress to prevent spurious minima related to too many bins*/
      if (compressionset==VFALSE)
      {
          histo = VipComputeVolumeHisto(deriche);
          ratio = 0;
          comphisto = VipGetPropUndersampledHisto(histo, 50, &ratio, &compression, 1, 100);//Test a 80, retrouver pourquoi.
          printf("Gradient image compression: %d\n", ratio);
          VipFreeHisto(comphisto);
          VipFreeHisto(histo);
      }
      if (compression!=0)
      {
          printf("Compressing volume (forgetting %d bits)...\n", compression);
          compressed = VipComputeCompressedVolume(deriche, compression);
          if (compressed==PB) return(VIP_CL_ERROR);
          VipFreeVolume(deriche);
          deriche = compressed;
          compressed = NULL;
      }
      VipResizeBorder(deriche, 3);
      if (writeedges==VTRUE) VipWriteVolume(deriche, edgesname);
      
      VipResizeBorder(vol, 3);
      
      /*-----Determination of the background mean intensity-----*/
      printf("Determination of the background mean intensity...\n");
      deriche_norm = VipCopyVolume(vol, "deriche_norm");
      if (deriche_norm==PB) return(VIP_CL_ERROR);
      if (VipDeriche3DGradientNorm(deriche_norm, 2., DERICHE_NORM, 0.)==PB) return(VIP_CL_ERROR);
//       VipWriteVolume(deriche_norm, "deriche_norm");
      VipResizeBorder(vol, 0);
      
      max_gradient = VipGetVolumeMax(deriche_norm);
      
//       VipMaskVolume(deriche,boundingbox);
//       VipFreeVolume(boundingbox);
      
      histo_edges = VipComputeVolumeHisto(deriche_norm);
      if (histo_edges==PB) return(VIP_CL_ERROR);
      for(i=5; i<=histo_edges->range_max-3; i++) 
      {
          if ((histo_edges->val[i]<=histo_edges->val[i-1])
              && (histo_edges->val[i]<=histo_edges->val[i+1])
              && (histo_edges->val[i]<=histo_edges->val[i+2])
              && (histo_edges->val[i]<=histo_edges->val[i+3])
              /*&& (histo_edges->val[i]<=histo_edges->val[i+4])*/) break;
      }
      printf("edge magnitude minimum: %d\n", i);
      VipFreeHisto(histo_edges);
      
      if (i>(max_gradient*0.1) || i<(max_gradient*0.01))
      {
          threshold_edges = (int)(max_gradient)*derichepct;
      }
      else threshold_edges = (int)(i*1);
      printf("Tissue/background gradient threshold: %d\n", threshold_edges);
      /*strong outline mask*/
      thresholdedvol = VipCreateSingleThresholdedVolume(deriche_norm, GREATER_THAN, threshold_edges, BINARY_RESULT);
      VipResizeBorder(thresholdedvol, 1);
      VipCustomizedChamferOpening(thresholdedvol , 1.4*little_opening_size, 3, 3, 3, VIP_USUAL_DISTMAP_MULTFACT, FRONT_PROPAGATION);
      /*background in the corner mask*/
      if (VipExtRayCorner(thresholdedvol, EXTEDGE3D_ALL_EXCEPT_Z_BOTTOM, SAME_VOLUME)==PB) return(VIP_CL_ERROR);
      VipResizeBorder( thresholdedvol, 0 );
      /*compute a mask of the voxel at zero*/
      masked = VipCopyVolume(vol, "voxel_zero");
      VipResizeBorder(masked, 1);
      if (VipSingleThreshold(masked, LOWER_OR_EQUAL_TO, min_volume, BINARY_RESULT)==PB) return(VIP_CL_ERROR);
      VipConnectivityChamferErosion(masked, mVipMax(mVipVolVoxSizeX(vol), mVipVolVoxSizeY(vol))+0.1, CONNECTIVITY_26, FRONT_PROPAGATION);
      if (VipConnexVolumeFilter(masked, CONNECTIVITY_6, -1, CONNEX_BINARY)==PB) return(PB); //Ne pas garder la plus grande composante connexe mais virer les plus petites cc avec un hystersis?
      VipConnectivityChamferDilation(masked, mVipMax(mVipVolVoxSizeX(vol), mVipVolVoxSizeY(vol))+0.1, CONNECTIVITY_26, FRONT_PROPAGATION);
      VipSetBorderLevel( masked,255);
      VipResizeBorder( masked, 0 );
      /*merge the voxel_zero mask with the corner mask to not take into account
        the zero voxel in the background mean intensity*/
      VipComputeRobustStatInMaskVolume(vol, thresholdedvol, &mean, &sigma, VFALSE);
      printf("Corner background stats: mean: %f; sigma: %f\n", mean, sigma);
      thbackground = (mean+1.*sigma+0.5);
      printf("thbackground=%f\n", thbackground), fflush(stdout);
      if (thbackground>0) 
      {
          printf("merge\n");
          VipMerge( thresholdedvol, masked, VIP_MERGE_ONE_TO_ONE, 255, 0 );
      }
      
      VipResizeBorder( vol, 3 );
      VipResizeBorder( thresholdedvol, 3 );
      VipResizeBorder( masked, 3 );
      
      VipComputeRobustStatInMaskVolume(vol,thresholdedvol, &mean, &sigma, VFALSE);
      
      VipFreeVolume(thresholdedvol);
      VipFreeVolume(deriche_norm);
      
      printf("Corner background stats: mean: %f; sigma: %f\n", mean, sigma);
      thresholdlowset = VTRUE;
      thresholdlow = (int)(mean+1.*sigma+0.5);
      if (thresholdlow==0) thresholdlow = 1;
      printf("threshold for corner background/tissue: %d\n", thresholdlow);
      
      thresholdedvol = VipCopyVolume(vol,"closing");
      if(!thresholdedvol) return(VIP_CL_ERROR);
      VipSingleThreshold(thresholdedvol, LOWER_THAN, thresholdlow, BINARY_RESULT);
      if (thbackground>0) VipMerge(thresholdedvol, masked, VIP_MERGE_ONE_TO_ONE, 255, 0);
      VipFreeVolume(masked);
      VipConnectivityChamferClosing (thresholdedvol, 1, CONNECTIVITY_26, FRONT_PROPAGATION);
      
      masked = VipCopyVolume(vol,"masked");
      if(!masked) return(VIP_CL_ERROR);
      VipMaskVolume(masked,thresholdedvol);
      VipComputeRobustStatInMaskVolume(masked,thresholdedvol, &mean, &sigma, VFALSE);
      VipFreeVolume(thresholdedvol);
      VipFreeVolume(masked);

      printf("Background stats: mean: %f; sigma: %f\n", mean, sigma);
      thresholdlowset=VTRUE;
      thresholdlow = (int)(mean+2.*sigma+0.5);
      printf("Global threshold background/tissue: %d\n", thresholdlow);
      mask = VipCreateSingleThresholdedVolume(vol, GREATER_OR_EQUAL_TO, thresholdlow, GREYLEVEL_RESULT);//GREATER_THAN ou GREATER_OR_EQUAL_TO ?
      //Peut etre rajouter un filtre pour enlever des voxels de bruit se baladant ?
    }
  
  if (variance_threshold!=-1 || variance_pourcentage!=-1)
    {
      if(vol==NULL)
      {
          vol = VipReadVolumeWithBorder(input,1);
          if(mVipVolType(vol)==U8BIT)
          {
              converter = VipTypeConversionToS16BIT(vol, RAW_TYPE_CONVERSION);
              if(converter==PB) return(VIP_CL_ERROR);
              flag8bit = VTRUE;
              VipFreeVolume(vol);
              vol = converter;
          }
          mask = VipCreateSingleThresholdedVolume(vol, GREATER_OR_EQUAL_TO, thresholdlow, GREYLEVEL_RESULT);
      }
      variance_brute = VipComputeVarianceVolume(vol);
      if (variance_brute==PB) return(VIP_CL_ERROR);
      
      if (writevariance==VTRUE) VipWriteVolume(variance_brute, variancename);
      /*
      VipComputeRobustStatInMaskVolume(variance,variance, &mean, &sigma, VTRUE);
      printf("variance in tissues: mean: %f; sigma: %f\n", mean, sigma);
      variance_threshold = (int)(mean-0*sigma+0.5);
      */
      VipMaskVolume(variance_brute, mask);
      VipFreeVolume(vol);
  }
  
  vol = VipReadVolumeWithBorder(input,0);
  if(vol==NULL) return(VIP_CL_ERROR);
  
  if(mVipVolType(vol)==U8BIT)
  {
      converter = VipTypeConversionToS16BIT(vol, RAW_TYPE_CONVERSION);
      if(converter==PB) return(VIP_CL_ERROR);
      flag8bit = VTRUE;
      VipFreeVolume(vol);
      vol = converter;
  }
  
  if(tauto==VFALSE)
  {
      if(Last==3000 && point_filename!=NULL)
      {
          if(GetCommissureCoordinates(vol, point_filename, &tal,
                                      xCA, yCA, zCA, xCP, yCP, zCP,
                                      xP, yP, zP, talset)!=PB)
          {
              coord = &tal;
              xCA = (int)(coord->AC.x); yCA = (int)(coord->AC.y); zCA = (int)(coord->AC.z);
              xCP = (int)(coord->PC.x); yCP = (int)(coord->PC.y); zCP = (int)(coord->PC.z);
              xP = (int)(coord->Hemi.x); yP = (int)(coord->Hemi.y); zP = (int)(coord->Hemi.z);

              Last = (int)(mVipVolSizeZ(vol) - ((2*zCP-zCA) + (75/mVipVolVoxSizeZ(vol))));
              if(Last<0) Last = 0;
          }
          else
          {
              printf("Something went wrong during the reading of the commissure coordinates.\n");
              Last = 0;
          }
      }
      else
      {
          printf("Commissure Coordinates are necessary to delete automatically the last slides\n");
          Last = 0;
      }
  }
  printf("Deleting last %d slices\n", Last);
  for(i=0; i<Last; i++) VipPutOneSliceTwoZero(vol,mVipVolSizeZ(vol)-i-1);
  
  if (deriche_edges!=-1)
    {
      if (deriche_edges==2)
        {
          edges = VipDeriche2DGradientNorm(vol, 2.5, NEW_VOLUME, DERICHE_EXTREMA, 0.);
          connectivity = CONNECTIVITY_8;
          lemax = VipGetVolumeMax(edges);
          tlow = 0.04*lemax;
        }
      else
        {
          if (deriche)
            {
              edges = deriche;
              tlow = threshold_edges;
            }
          else
            {
              edges = VipCopyVolume(vol,"edges");
              if (!edges) return(VIP_CL_ERROR);
              if(VipDeriche3DGradientNorm(edges, 2., DERICHE_EXTREMA, 0.)==PB) return(VIP_CL_ERROR);
              lemax = VipGetVolumeMax(edges);
              tlow = 0.04*lemax;
            }
        }
      thigh = 2*tlow;
      if(VipHysteresisThresholding(edges,connectivity,SAME_VOLUME,CONNEX_BINARY,tlow,thigh,HYSTE_NUMBER,1)==PB) return(VIP_CL_ERROR);
      VipInvertBinaryVolume(edges);
    }
  
  /*discard background before computing actual compression*/
  if (compressionset==VFALSE)
  {
      histo = VipComputeVolumeHisto(vol);
      ratio = 0;
      /* discard potential modes higher than white matter*/
      comphisto = VipGetPropUndersampledHisto( histo, 95, &ratio, &compression, thresholdlow, 100 );
      printf("%d\n", ratio);
      VipFreeHisto(comphisto);
      VipFreeHisto(histo);
  }
  if (compression!=0)
  {
      printf("Compressing volume (forgetting %d bits)...", compression);
      fflush(stdout);
  }
  /* compress anyway because of the following threshold*/
  compressed = VipComputeCompressedVolume(vol, compression);
  if (compressed==PB) return(VIP_CL_ERROR);
  if (compression!=0)
    {
      printf("\n");
    }

  /*
  if(VipWriteTivoliVolume(compressed,"compressed")==PB) return(VIP_CL_ERROR);
  */
  /*VipWriteVolume(compressed, "compressed");*/
  if(thresholdlowset==VTRUE)
      thresholdlow = thresholdlow >> compression ;
  if(thresholdhighset==VTRUE)
      thresholdhigh = thresholdhigh >> compression ;
  printf("Low threshold: %d, High threshold: %d (after compression)\n", thresholdlow, thresholdhigh);
  VipDoubleThreshold(compressed,VIP_BETWEEN,thresholdlow,thresholdhigh,GREYLEVEL_RESULT);
  
  if(readridges==VFALSE)
    {
      smooth = VipDeriche3DGaussian(vol, mcsigma, NEW_FLOAT_VOLUME);
      mc = Vip3DGeometry(smooth, MEAN_CURVATURE);
      if (writemeancurvature==VTRUE) VipWriteVolume(mc, meancurvaturename);

      VipSingleFloatThreshold(mc, mcmethod, mcthreshold, BINARY_RESULT);
      white_crest = VipTypeConversionToS16BIT(mc , RAW_TYPE_CONVERSION);
      VipFreeVolume(mc);
      
      /*define target crest line: may miss some small part, but include no spurious ones
        the goal is to do a kind of hysteresis*/
      
      variance_threshold = VipPourcentageLowerThanThreshold(variance_brute, 1, 80);
      
      printf("strongest high threshold on variance: %d\n", variance_threshold);
      variance = VipCreateDoubleThresholdedVolume(variance_brute,VIP_BETWEEN_OR_EQUAL_TO,
                                                  0,variance_threshold,BINARY_RESULT);
      VipMaskVolume(variance, mask);
      VipConnectivityChamferErosion( variance, 1, CONNECTIVITY_26, FRONT_PROPAGATION );
      
      target = VipCreateSingleThresholdedVolume( white_crest, GREATER_THAN, 1 , BINARY_RESULT);

      if(!target) return(VIP_CL_ERROR);
      VipMaskVolume(target, variance);
      gradient = VipComputeCrestGrad(target, compressed);
      VipFreeVolume(target);
      extrema = VipComputeCrestGradExtrema(gradient, compressed);
      
      VipComputeRobustStatInMaskVolume(gradient,gradient, &mean, &sigma, VTRUE);
      printf("crest gradient: mean: %f; sigma: %f\n", mean, sigma);
      thresholdhigh = (int)(mean+2*sigma+0.5);
      printf("threshold gradient: %d\n", thresholdhigh);
      white_cresttemp = VipCreateDoubleThresholdedVolume(gradient,VIP_BETWEEN_OR_EQUAL_TO,1,thresholdhigh,BINARY_RESULT);
      VipSingleThreshold( extrema, GREATER_OR_EQUAL_TO, (int)(mean+1.*sigma+0.5), BINARY_RESULT);
      VipMerge(white_cresttemp,extrema,VIP_MERGE_ONE_TO_ONE,255,0);
      if (VipConnexVolumeFilter (white_cresttemp, CONNECTIVITY_26, -1, CONNEX_BINARY)==PB) return(VIP_CL_ERROR);
      VipComputeRobustStatInMaskVolume(gradient,white_cresttemp, &mean, &sigma, VFALSE);
      
      VipFreeVolume(white_cresttemp);
      printf("ridge gradient: mean: %f; sigma: %f\n", mean, sigma);
      thresholdhigh = (int)(mean+2*sigma+0.5);
      printf("threshold gradient: %d\n", thresholdhigh);
      
      VipDoubleThreshold(gradient,VIP_BETWEEN_OR_EQUAL_TO,1,thresholdhigh,BINARY_RESULT);
      VipMerge(gradient,extrema,VIP_MERGE_ONE_TO_ONE,255,0);
      if (extrema) VipFreeVolume(extrema);
      
      if (VipConnexVolumeFilter (gradient, CONNECTIVITY_26, -1, CONNEX_BINARY)==PB) return(VIP_CL_ERROR);
      //       if (VipConnexVolumeFilter (gradient, CONNECTIVITY_26, 10000, CONNEX_BINARY)==PB) return(VIP_CL_ERROR);
      target = VipCopyVolume(gradient,"target");
      while(controlled==VFALSE)
        {
          VipFreeVolume(variance);

          variance_threshold = VipPourcentageLowerThanThreshold(variance_brute, 1, variance_pourcentage);
          
          printf("high threshold on variance: %d (pourcentage: %d)\n", variance_threshold, variance_pourcentage);
          variance = VipCreateDoubleThresholdedVolume(variance_brute,VIP_BETWEEN_OR_EQUAL_TO,
                                                      0,variance_threshold,BINARY_RESULT);
          VipMaskVolume(variance, mask);
          VipConnectivityChamferErosion( variance, 1, CONNECTIVITY_26, FRONT_PROPAGATION );
          
          arrow = VipCreateSingleThresholdedVolume( white_crest, GREATER_THAN, 1 , BINARY_RESULT);
          if(!arrow) return(VIP_CL_ERROR);
          VipMaskVolume(arrow, variance);
          
          gradient = VipComputeCrestGrad(arrow, compressed);
          VipFreeVolume(arrow);
          extrema = VipComputeCrestGradExtrema(gradient, compressed);
          
          /*
          VipComputeRobustStatInMaskVolume(gradient,gradient, &mean, &sigma, VTRUE);
          printf("crest gradient: mean: %f; sigma: %f\n", mean, sigma);
          thresholdhigh = (int)(mean+2*sigma+0.5);
          printf("threshold gradient: %d\n", thresholdhigh);
          white_cresttemp = VipCreateDoubleThresholdedVolume(gradient,VIP_BETWEEN_OR_EQUAL_TO,1,thresholdhigh,BINARY_RESULT);
          VipSingleThreshold( extrema, GREATER_OR_EQUAL_TO, (int)(mean+1.*sigma+0.5), BINARY_RESULT);
          VipMerge(white_cresttemp,extrema,VIP_MERGE_ONE_TO_ONE,255,0);
          if (VipConnexVolumeFilter (white_cresttemp, CONNECTIVITY_26, -1, CONNEX_BINARY)==PB) return(VIP_CL_ERROR);
          VipComputeRobustStatInMaskVolume(gradient,white_cresttemp, &mean, &sigma, VFALSE);


          VipFreeVolume(white_cresttemp);
          printf("ridge gradient: mean: %f; sigma: %f\n", mean, sigma);
          thresholdhigh = (int)(mean+2*sigma+0.5);
          printf("threshold gradient: %d\n", thresholdhigh);
          */

          VipDoubleThreshold(gradient, VIP_BETWEEN_OR_EQUAL_TO, 1, thresholdhigh, BINARY_RESULT);
          VipSingleThreshold(extrema, GREATER_OR_EQUAL_TO, (int)(mean+1.*sigma+0.5), BINARY_RESULT);
          
          VipMerge(gradient, extrema, VIP_MERGE_ONE_TO_ONE, 255, 0);
          VipFreeVolume(extrema);
          
          if (VipConnexVolumeFilter (gradient, CONNECTIVITY_26, -1, CONNEX_BINARY)==PB) return(VIP_CL_ERROR);
          
          arrow = VipDuplicateVolumeStructure ( gradient, "arrow" );
          arrow->borderWidth = 1;
          VipTransferVolumeData ( gradient, arrow );
          VipMerge( arrow, target, VIP_MERGE_ONE_TO_ONE, 255, 512 );
          
          VipChangeIntLabel(arrow, 0, -2222);
          VipComputeFrontPropagationConnectivityDistanceMap(arrow, 255, -2222, VIP_NO_LIMIT_IN_PROPAGATION, 0, CONNECTIVITY_26);
          VipChangeIntLabel(arrow, -2222, 0);
          
          lemax = VipGetVolumeMax(arrow);
          printf("Max distance: %f mm\n",lemax);
          if (lemax<30)
          {
              printf("OK\n");
              controlled=VTRUE;
          }
          else variance_pourcentage -= 5;
        }

      if(arrow) VipFreeVolume(arrow);
      if(target) VipFreeVolume(target);

      if (writeridges==VTRUE) VipWriteVolume(gradient,wridgesname);
      if (edges)
        {
          VipMaskVolume(gradient,edges);
        }
    }
  else
    {
      gradient = VipReadVolumeWithBorder(wridgesname,0);
      if (!gradient)  return(VIP_CL_ERROR);
      
      variance_threshold = VipPourcentageLowerThanThreshold(variance_brute, 1, variance_pourcentage);
      
      printf("high threshold on variance: %d\n", variance_threshold);
      variance = VipCreateDoubleThresholdedVolume(variance_brute, VIP_BETWEEN_OR_EQUAL_TO,
                                                  0, variance_threshold, BINARY_RESULT);
      VipMaskVolume(variance,mask);
      VipConnectivityChamferErosion( variance, 1, CONNECTIVITY_26, FRONT_PROPAGATION );
    }
  
//   copyvol =VipCopyVolume(compressed, "copy_compressed");
  
  if (variance)
    {
      VipConnectivityChamferDilation( variance, 1, CONNECTIVITY_26, FRONT_PROPAGATION );
//       if (VipConnexVolumeFilter (variance, CONNECTIVITY_26, -1, CONNEX_GREYLEVEL)==PB) return(VIP_CL_ERROR);
//       VipConnectivityChamferDilation( variance, 1, CONNECTIVITY_26, FRONT_PROPAGATION );
      VipMaskVolume(compressed, variance);
      VipFreeVolume(variance);
    }
     
  if (edges)
    {
//       VipMaskVolume(compressed,edges); //test
      VipFreeVolume(edges);
    }
  
  if (VipConnexVolumeFilter (compressed, CONNECTIVITY_26, -1, CONNEX_GREYLEVEL)==PB) return(VIP_CL_ERROR);

//   printf("--------------------------------------------------------------\n");
//   printf("First bias correction whith the none corrected white_ridges...\n");
//   printf("--------------------------------------------------------------\n");
  
  printf("Computing Bias with Kentropy = %f, Kregularization = %f, Kcrest = %f Koffset = %f\n",
         Kentropy,Kregul,Kcrest,Koffset);

  result = VipComputeT1BiasFieldMultiGrid(mode, dumb, compressed, gradient,
                                          sampling, Kentropy, Kregul, Kcrest,
                                          Koffset, amplitude, temperature, geom,
                                          fieldtype, nInc, Inc, ngrid, RegulZTuning);
  if(result==PB) return(VIP_CL_ERROR);
  
  if (writehfiltered==VTRUE)
  {
      masked = VipCreateSingleThresholdedVolume( compressed, GREATER_OR_EQUAL_TO, 1, BINARY_RESULT );
//       VipResizeBorder( masked, 1 );
//       VipDilation( masked, CHAMFER_BALL_3D, 2.*little_opening_size );
//       VipSetBorderLevel( masked, 0 );
//       VipResizeBorder( masked, 0 );
      VipWriteVolume(masked,hfilteredname);
  }
  VipFreeVolume(compressed);
  /*debug  vol = compressed;*/
//   copyvol = VipCopyVolume(vol, "copyvol");
  fullresult = VipResampleField(result, vol);
  VipFreeVolume(result);
  if(fullresult==PB) return(VIP_CL_ERROR);
/*  
  VipComputeUnbiasedVolume(fullresult,copyvol);
  VipFreeVolume(fullresult);
  
  printf("------------------------------------------------------------------------\n");
  printf("Histogram analysis of the nobias volume to correct the white_ridges...\n");
  printf("------------------------------------------------------------------------\n");
  
  copyvol2 = VipCopyVolume(copyvol, "copyvol2");
  printf("Masking volume with %s...\n",hfilteredname);
  VipMaskVolume(copyvol,masked);
  VipFreeVolume(masked);
  
  VipComputeStatInRidgeVolume(copyvol,gradient, &mean, &sigma, VTRUE);
//   VipComputeStatInRidgeVolume(compressed,gradient, &mean, &sigma, VTRUE);
  printf("ridge stats: mean: %f; sigma: %f\n", mean, sigma);
  
  printf("Computing histogram\n");
  shorthisto = VipComputeVolumeHisto(copyvol);
//   shorthisto = VipComputeVolumeHisto(compressed);
  VipSetHistoVal(shorthisto,0,0);
  historesamp = VipGetPropUndersampledHisto(shorthisto, 95, &undersampling_factor, &factor, 0, 100);
  
  if(factor==0 && undersampling_factor==1) u = 1;
  else if (factor==1 && undersampling_factor==2) u = undersampling_factor/2;
  else
  {
      undersampling_factor /= 2;
      u = undersampling_factor/2;
  }
  while(u<=undersampling_factor*2)
  {
      volstruct = VipCompute1DScaleSpaceStructUntilLastCascade(shorthisto,0.5,0,2,u);
      if(volstruct==PB) printf("Error in VipCompute1DScaleSpaceStructUntilLastCascade\n");
      slist = VipComputeSSSingularityList(volstruct,5,VFALSE,VTRUE,VTRUE,VFALSE,VFALSE);
      if(slist==PB) printf("Error in VipComputeSSSingularityList\n");
      
      printf("Detecting D1/D2 singularity matings and cascades...\n"), fflush(stdout);
      if(VipFindSSSingularityMates(slist)==PB) printf("Error in VipFindSSSingularityMates\n");
      else
      {
          chigh = NULL;
          VipCountSingularitiesStillAlive(slist,&n,volstruct->itermax);
          if((n<=5)) chigh = VipCreateHighestCascade(slist,volstruct->itermax,n);
          clist = VipComputeOrderedCascadeList( slist, 1000, volstruct->hcumul);
          
          if(chigh!=NULL)
          {
              chigh->next = clist;
              clist = chigh;
              printf("Analysing histogram knowing white ridge statistics...\n");
              ana = VipAnalyseCascadesRidge( clist, volstruct, mean);
              if(ana==PB) printf("Error in VipAnalyseCascadesRidge\n");
              else 
              {
                  VipMarkAnalysedObjects( ana, volstruct );
                  
                  contrast = ((float)ana->white->mean - (float)ana->gray->mean)/((float)ana->white->mean);
                  ratio_GW = (float)(shorthisto->val[ana->gray->mean])/(float)(shorthisto->val[ana->white->mean]);
                  
                  printf("\ncontrast = %.3f\n", contrast), fflush(stdout);
                  printf("ratio_GW = %.3f, val_histo_gray = %d, val_histo_white = %d\n", ratio_GW, shorthisto->val[ana->gray->mean], shorthisto->val[ana->white->mean]), fflush(stdout);
                  
                  if((0.09<contrast && contrast<0.55) && (0.30<ratio_GW && ratio_GW<2.5))
                  {
                      undersampling_factor_possible[j][0] = u;
                      undersampling_factor_possible[j][1] = ana->gray->mean;
                      undersampling_factor_possible[j][2] = ana->gray->sigma;
                      undersampling_factor_possible[j][3] = ana->white->mean;
                      undersampling_factor_possible[j][4] = ana->white->sigma;
                      j++;
                  }
              }
          }
          else VipPrintfError("Sorry, the histogram analysis can not proceed further");
      }
      if(u<undersampling_factor && u!=1) u+=undersampling_factor/4;
      else u+=(undersampling_factor+1)/2;
  }
  u = VipIterateToGetPropUndersampledRatio(shorthisto, &undersampling_factor, undersampling_factor_possible, j);
  
  volstruct = VipCompute1DScaleSpaceStructUntilLastCascade(shorthisto,0.5,0,2,u);
  slist = VipComputeSSSingularityList(volstruct,5,VFALSE,VTRUE,VTRUE,VFALSE,VFALSE);
  VipFindSSSingularityMates(slist);
  chigh = NULL;
  VipCountSingularitiesStillAlive(slist,&n,volstruct->itermax);
  if((n<=5)) chigh = VipCreateHighestCascade(slist,volstruct->itermax,n);
  clist = VipComputeOrderedCascadeList(slist, 1000, volstruct->hcumul);
  if(chigh!=NULL && clist!=PB)
  {
      chigh->next = clist;
      clist = chigh;
      ana = VipAnalyseCascadesRidge( clist, volstruct, mean );
      VipMarkAnalysedObjects( ana, volstruct );
  }
  else ana=PB;
  
  if(ana->gray==NULL || ana->white==NULL)
  {
      printf("Error in the histogram analysis\n");
  }
  else
  {
      printf("---------------------------------------------------------------------\n");
      printf("Gray/White classification on the whole brain to clean white_ridges...\n");
      printf("---------------------------------------------------------------------\n");
      
//       masked = VipCopyVolume(copyvol, "copyvol");
      masked = VipCopyVolume(copyvol2, "copyvol");

      printf("Classification to get the whole brain...\n");
      classif = VipCSFGrayWhiteFatClassificationRegularisationForRobustApproach( copyvol2, ana, NO, 1, ana->gray->mean - 2.5*ana->gray->sigma, ana->gray->mean - 1.7*ana->gray->sigma, ana->white->mean + 4*ana->white->sigma, ana->white->mean + 6*ana->white->sigma, ana->gray->mean + (ana->white->mean - ana->gray->mean)/2);
      VipSingleThreshold( classif, EQUAL_TO, BRAIN_LABEL, BINARY_RESULT );
      VipConnexVolumeFilter( classif, CONNECTIVITY_6, -1, CONNEX_BINARY );

      VipMaskVolume(masked, classif);
      VipFreeVolume(classif);
      
      printf("Gray/White classification to get the white matter...\n");
      classif = VipGrayWhiteClassificationRegularisationForVoxelBasedAna(masked, ana, VFALSE, 5, 20, CONNECTIVITY_26);
      VipFreeVolume(masked);
      VipChangeIntLabel(classif,VOID_LABEL,0);
      masked = VipExtedge(classif,EXTEDGE3D_ALL,NEW_VOLUME);
      VipMerge(classif,masked,VIP_MERGE_ALL_TO_ONE,255,GRAY_LABEL);
      VipFreeVolume(masked);
      
      VipSingleThreshold(classif, EQUAL_TO, WHITE_LABEL, BINARY_RESULT );
      VipConnexVolumeFilter( classif, CONNECTIVITY_6, -1, CONNEX_BINARY );

      printf("Cleaning white_ridges...\n");
      VipMaskVolume(gradient, classif);
      if (writeridges==VTRUE) VipWriteVolume(gradient,wridgesname);
      
      printf("--------------------------------------------------------------\n");
      printf("Second bias correction whith the corrected white_ridges...\n");
      printf("--------------------------------------------------------------\n");
      
      printf("Computing Bias with Kentropy = %f, Kregularization = %f, Kcrest = %f Koffset = %f\n",
             Kentropy,Kregul,Kcrest,Koffset);

      result = VipComputeT1BiasFieldMultiGrid(mode,dumb,compressed,gradient,
                                              sampling,Kentropy,Kregul,Kcrest,Koffset,
                                              amplitude,temperature,geom,fieldtype,nInc,Inc,ngrid,RegulZTuning);
      if(result==PB) return(VIP_CL_ERROR);
      
      fullresult = VipResampleField(result, vol);
      VipFreeVolume(result);
      if(fullresult==PB) return(VIP_CL_ERROR);
  }*/

  if(writefield==VTRUE)
      {
	  if(VipWriteVolume(fullresult,fieldname)==PB) return(VIP_CL_ERROR);
      }

  if(docorrection==VTRUE)
  {
      printf("Correcting volume...\n");
      VipComputeUnbiasedVolume(fullresult,vol);
      
      VipFreeVolume(fullresult);
      
      if(VipWriteVolume(vol,output)==PB) return(VIP_CL_ERROR);
  }
  /*
  compressed = VipComputeCompressedVolume( vol, compression);
  gradient = VipComputeCrestGrad(white_crest, compressed);
 
  VipComputeRobustStatInMaskVolume(gradient,gradient, &mean, &sigma, VTRUE);
  printf("crest gradient: mean: %f; sigma: %f\n", mean, sigma);
  thresholdhigh = (int)(mean+2*sigma+0.5);
  printf("threshold gradient: %d\n", thresholdhigh);
  white_cresttemp = VipCreateDoubleThresholdedVolume(gradient,VIP_BETWEEN_OR_EQUAL_TO,1,thresholdhigh,BINARY_RESULT);
  if (VipConnexVolumeFilter (white_cresttemp, CONNECTIVITY_26, -1, CONNEX_BINARY)==PB) return(VIP_CL_ERROR);
  VipComputeRobustStatInMaskVolume(gradient,white_cresttemp, &mean, &sigma, VFALSE);
  VipFreeVolume(white_cresttemp);
  printf("ridge gradient: mean: %f; sigma: %f\n", mean, sigma);
  thresholdhigh = (int)(mean+2*sigma+0.5);
  printf("threshold gradient: %d\n", thresholdhigh);
  VipDoubleThreshold(gradient,VIP_BETWEEN_OR_EQUAL_TO,1,thresholdhigh,BINARY_RESULT);
  if (VipConnexVolumeFilter (gradient, CONNECTIVITY_26, -1, CONNEX_BINARY)==PB) return(VIP_CL_ERROR);
  if (writeridges==VTRUE) VipWriteTivoliVolume(gradient,"wridgesname2");
  */
  return(0);

}

/*---------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipT1BiasCorrection\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"nobias\")}]\n");
  (void)fprintf(stderr,"        [-m[ode] {char: s, g (default:g)}]\n");
  (void)fprintf(stderr,"        [-w[ridges] {white matter ridges image (default:\"white_ridges\")}]\n");
  (void)fprintf(stderr,"        [-W[write] {write ridges: y/n/r (default:n)}]\n");
  (void)fprintf(stderr,"        [-hW[rite] {write volume for histo estimation: y/n (default:n)}]\n");
  (void)fprintf(stderr,"        [-hn[ame] {volume for histo estimation image: (default:hfiltered)}]\n");
  (void)fprintf(stderr,"        [-vW[rite] {write variance: y/n (default:n)}]\n");
  (void)fprintf(stderr,"        [-vn[ame] {variance image: (default:variance)}]\n");
  (void)fprintf(stderr,"        [-eW[rite] {write edges: y/n (default:n)}]\n");
  (void)fprintf(stderr,"        [-en[ame] {edge image: (default:edges)}]\n");
  (void)fprintf(stderr,"        [-mW[rite] {write mean curvature: y/n (default:n)}]\n");
  (void)fprintf(stderr,"        [-mn[ame] {mean curvature image: (default:mean_curvature)}]\n");
  (void)fprintf(stderr,"        [-mcs[igma] {float (mm) (default:1mm)}]\n");
  (void)fprintf(stderr,"        [-f[ield] {bias field name (default:\"biasfield\")}]\n");
  (void)fprintf(stderr,"        [-Di[mfield] {type of field (2/3) (default:3)}]\n");
  (void)fprintf(stderr,"        [-F[write] {write field: y/n (default:n)}]\n");
  (void)fprintf(stderr,"        [-d[umb] {y/n (default:y)}]\n");
  (void)fprintf(stderr,"        [-Ke[ntropy] {float  (default:1.)}]\n");
  (void)fprintf(stderr,"        [-Kr[egul] {float  (default:50.)}]\n");
  (void)fprintf(stderr,"        [-Ko[ffset] {float  (default:0.5)}]\n");
  (void)fprintf(stderr,"        [-Kc[rest] {float  (default:20.)}]\n");
  (void)fprintf(stderr,"        [-c[ompression] {int [0,14]  (default:auto)}]\n");
  (void)fprintf(stderr,"        [-t[auto] {char: y/n default:y}]\n");
  (void)fprintf(stderr,"        [-tl[ow] {int (default:3*2^compression)}]\n");
  (void)fprintf(stderr,"        [-th[igh] {int (default:not used)}]\n");
  (void)fprintf(stderr,"        [-e[dges] {char (default:not used, n/2/3)}]\n");
  (void)fprintf(stderr,"        [-vt[ariance] {int (default:not used, else int threshold)}]\n");
  (void)fprintf(stderr,"        [-vp[ourcentage] {int (default:75, else int threshold)}]\n");
  (void)fprintf(stderr,"        [-T[emperature] {float (default:10.)}]\n");
  (void)fprintf(stderr,"        [-g[eometric] {float (default:0.97)}]\n");
  (void)fprintf(stderr,"        [-n[Increment] {int (default:2)}]\n");
  (void)fprintf(stderr,"        [-G[rid] {int (default:2)}]\n");
  (void)fprintf(stderr,"        [-I[ncrement] {float (default:1.03)}]\n");
  (void)fprintf(stderr,"        [-s[ampling] {float  (default:16mm)}]\n");
  (void)fprintf(stderr,"        [-Z[regulTuning] {float  (default:1.)}]\n");
  (void)fprintf(stderr,"        [-a[mplitude] {float ]1,10]  (default:1.1)}]\n");
  (void)fprintf(stderr,"        [-L[ast] {int (default:0)}]\n");
  (void)fprintf(stderr,"        [-P[oints] {AC,PC,IH coord filename (*.tal) (default:not used)}]\n");
  (void)fprintf(stderr,"        [-Dk[eep] {float (default:75.0)}]\n");
  (void)fprintf(stderr,"        [-Dc[orrect] {Do the biais correction: y/n (default:y)}]\n");
  (void)fprintf(stderr,"        [-pv[ariance] {float (default:0.05)}]\n");
  (void)fprintf(stderr,"        [-Ct[hreshold] {float (default:-0.4)}]\n");
  (void)fprintf(stderr,"        [-Cm[ethod] {string: HE, LE (default:LE)}]\n");
  (void)fprintf(stderr,"        [-S[pecie] {string: H[uman], M[acaca] (default:H)}]\n");
  (void)fprintf(stderr,"        [-Cw[eight] {string: T1, T2 (default:T1)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: a, v, s or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-srand {int (default: time}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);
}

/*---------------------------------------------------------------------------*/

static int Help() 
{
  VipPrintfInfo("Computes a smooth multiplicative field which corrects for non stationarities.\n");
  (void)printf("This field aims at minimizing the volume entropy (= minimizing information...).\n");
  (void)printf("A tradeoff is found between this entropy and the internal energy of a membrane using annealing.\n");
  (void)printf("\n");
  (void)printf("Usage: VipT1BiasCorrection\n");
  (void)printf("        -i[nput] {image name}\n");
  (void)printf("        [-o[utput] {image name (default:\"nobias\")}]\n");
  (void)printf("        [-m[ode] {char: s,g (default:g)}]\n");
  (void)printf("Switch between various terms to prevent compression of information\n");
  (void)printf("s: without g addition\n");
  (void)printf("g: minimize variance of white matter ridges\n");
  (void)printf("        [-f[ield] {bias field name (default:\"biasfield\")}]\n");
  (void)printf("        [-hW[rite] {write volume for histo estimation: y/n (default:n)}]\n");
  (void)printf("        [-hn[ame] {volume for histo estimation image: (default:variance)}]\n");
  (void)printf("        [-w[ridges] {white matter ridges image (default:\"white_ridges\")}]\n");
  (void)printf("        [-W[write] {write ridges: y/n/r (default:n)}]\n");
  (void)printf("        [-vW[rite] {write variance: y/n (default:n)}]\n");
  (void)printf("        [-vn[ame] {variance image: (default:variance)}]\n");
  (void)printf("        [-eW[rite] {write edges: y/n (default:n)}]\n");
  (void)printf("        [-en[ame] {edge image: (default:edges)}]\n");
  (void)printf("        [-mW[rite] {write mean curvature: y/n (default:n)}]\n");
  (void)printf("        [-mn[ame] {mean curvature image: (default:mean_curvature)}]\n");
  (void)printf("        [-mcs[igma] {float (mm) (default:1mm)}]\n");
  (void)printf("the sigma of the Gaussian smoothing before mean curvature computation\n");
  (void)printf("        [-Di[mfield] {type of field (2/3) (default:3)}]\n");
  (void)printf("2: the correction field is constant by slice, 3: constant by cubes\n");
  (void)printf("        [-F[write] {write field: y/n (default:n)}]\n");
  (void)printf("        [-d[umb] {y/n (default:y)}]\n");
  (void)printf("        [-Ke[ntropy] {float  (default:1.)}]\n");
  (void)printf("Weight of the volume entropy which will be multiplied by the field size\n");
  (void)printf("        [-Kr[egul] {float  (default:50.)}]\n");
  (void)printf("Weight of the membrane/spline energy (sum of the squared log of the ratio between 6-neighbors)\n");
  (void)printf("        [-Ko[ffset] {float  (default:0.5)}]\n");
  /*  (void)printf("Weight of the sum of squared log (=distance to the uniform field with 1. value)\n");*/
  (void)printf("Weight of the squared difference between old and new mean\n");
  (void)printf("        [-Kc[rest] {float  (default:20.)}]\n");
  (void)printf("Weight of the voxel in the white ridge volume\n");
  (void)printf("        [-c[ompression] {int [0,14]  (default:auto)}]\n");
  (void)printf("The number of bits which are discarted during the volume entropy computation (= /2^compression))\n");
  (void)printf("        [-t[auto] {char: y/n default:n}]\n");
  (void)printf("        [-tl[ow] {int (default:3*2^compression)}]\n");
  (void)printf("        [-th[igh] {int (default:not used)}]\n");
  (void)printf("        [-e[dges] {int (default:not used, else n/2/3)}]\n");
  (void)printf("remove 2D/3D edges for histogram estimation (discard partial volume)\n"); 
  (void)printf("        [-vt[ariance] {int (default:not used, else int threshold)}]\n");
  (void)printf("high threshold on standard deviation in 26-neighborhood for inclusion in histogram\n"); 
  (void)printf("Values beyond this threshold are not taken into account.\n");
  (void)printf("In return, the field correction is applied to all values to get the final result\n");
  (void)printf("        [-vp[ourcentage] {int (default:75, else int threshold)}]\n");
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
  (void)printf("        [-L[ast] {int (default:0)}]\n");
  (void)printf("Mask the Last last slices in all estimation (for field of view going to the shoulder)\n");
  (void)printf("        [-P[oints] {AC,PC,IH coord filename (*.tal) (default:not used)}]\n");
  (void)printf("Correct format for the commissure coordinate file toto.APC:\n");
  (void)printf("AC: 91 88 113\nPC: 91 115 109\nIH: 90 109 53\n");
  (void)printf("        [-Dk[eep] {float (default:75.0)}]\n");
  (void)printf("Mask slices more than Dkeep mm below commissure points\n");
  (void)printf("        [-Dc[orrect] {Do the biais correction: y/n (default:y)}]\n");
  (void)printf("Write the images as hfiltered or whiteridge whithout doing the bias correction if it has been done by another software\n");
  (void)printf("        [-pv[ariance] {float (default:0.05)}]\n");
  (void)printf("Preliminary brain mask computation: this percentage of maximal variance might be used as threshold.\n");
  (void)printf("Value may need being changed depending on image contrast.\n");
  (void)printf("        [-Ct[hreshold] {float (default:-0.4)}]\n");
  (void)printf("Threshold on mean curvature. Used to compute white ridges.\n");
  (void)printf("        [-Cm[ethod] {string: HE, LE (default:LE)}]\n");
  (void)printf("Method for thresholding mean curvature (higher or equal, lower or equal)\n");
  (void)printf("        [-S[pecie] {string: H[uman], M[acaca] (default:H)}]\n");
  (void)printf("        [-Cw[eight] {string: T1, T2 (default:T1)}]\n");
  (void)printf("        [-r[eadformat] {char: a, v, s or t (default:any)}]\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)printf("        [-srand {int (default: time}]\n");
  (void)printf("Initialization of the random seed, useful to get reproducible results\n");
  (void)printf("        [-h[elp]\n");
  printf("More information in:\n");
  printf("Entropy minimization for automatic correction\n");
  printf("of intensity non uniformity, J.-F. Mangin,\n");
  printf("MMBIA (Math. Methods in Biomed. Image Analysis),\n");
  printf("Hilton Head Island, South Carolina, IEEE Press\n");
  printf("162-169, 2000\n");
  return(VIP_CL_ERROR);
}

/*---------------------------------------------------------------------------*/

// int VipComputeRobustStatInMaskVolume(Volume *vol, Volume *thresholdedvol, float *mean, float *sigma, int robust)
// {
//   VipOffsetStruct *vos;
//   int ix, iy, iz;
//   Vip_S16BIT *ptr, *cptr;
//   double sum, sum2;
//   double temp;
//   int n;
//   double threshold;
// 
// 
//      vos = VipGetOffsetStructure(vol);
//      ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
//      cptr = VipGetDataPtr_S16BIT( thresholdedvol  ) + vos->oFirstPoint;
//      sum = 0.;
//      n = 0;
//      for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
//        {
// 	 for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
// 	   {
// 	     for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
// 	       {
// 		 if(*cptr)
// 		   {
// 		     n++;
// 		     sum += *ptr;
// 		   }
// 		 ptr++;
//                  cptr++;
// 	       }
// 	     ptr += vos->oPointBetweenLine;  /*skip border points*/
// 	     cptr += vos->oPointBetweenLine;  /*skip border points*/
// 	   }
// 	 ptr += vos->oLineBetweenSlice; /*skip border lines*/
// 	 cptr += vos->oLineBetweenSlice; /*skip border lines*/
//        }
//      if(n==0 || n==1)
//        {
//          VipPrintfWarning ("empty volume in VipComputeRobustStatInMaskVolume");
//          return(PB);
//        }
//      *mean = (float)(sum/n);
//      ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
//      cptr = VipGetDataPtr_S16BIT( thresholdedvol ) + vos->oFirstPoint;
//      sum2 = 0.;
//      for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
//        {
// 	 for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
// 	   {
// 	     for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
// 	       {
// 		 if(*cptr)
// 		   {
//                      temp = *ptr-*mean;
// 		     sum2 += temp*temp;
// 		   }
// 		 cptr++;
// 		 ptr++;
// 	       }
// 	     ptr += vos->oPointBetweenLine;  /*skip border points*/
// 	     cptr += vos->oPointBetweenLine;  /*skip border points*/
// 	   }
// 	 ptr += vos->oLineBetweenSlice; /*skip border lines*/
// 	 cptr += vos->oLineBetweenSlice; /*skip border lines*/
//        }
//      *sigma = (float)sqrt((double)(sum2/(n-1)));
// 
//      if (robust==VTRUE)
//        {
// 
//          threshold = *mean + 3 * *sigma;
// 
// 
//          ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
//          cptr = VipGetDataPtr_S16BIT( thresholdedvol  ) + vos->oFirstPoint;
//          for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
//            {
//              for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
//                {
//                  for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
//                    {
//                      if(*cptr)
//                        {
//                          if (*ptr>threshold)
//                            {
//                              n--;
//                              sum -= *ptr;
//                            }
//                        }
//                      ptr++;
//                      cptr++;
//                    }
//                  ptr += vos->oPointBetweenLine;  /*skip border points*/
//                  cptr += vos->oPointBetweenLine;  /*skip border points*/
//                }
//              ptr += vos->oLineBetweenSlice; /*skip border lines*/
//              cptr += vos->oLineBetweenSlice; /*skip border lines*/
//            }
//          
//          *mean = (float)(sum/n);
//          
//          sum2=0.;
//          ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
//          cptr = VipGetDataPtr_S16BIT( thresholdedvol ) + vos->oFirstPoint;
//          for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
//            {
//              for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
//                {
//                  for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
//                    {
//                      if(*cptr)
//                        {
//                          if (*ptr>threshold)
//                            {
//                              temp = *ptr-*mean;
//                              sum2 += temp*temp;
//                            }
//                        }
//                      cptr++;
//                      ptr++;
//                    }
//                  ptr += vos->oPointBetweenLine;  /*skip border points*/
//                  cptr += vos->oPointBetweenLine;  /*skip border points*/
//                }
//              ptr += vos->oLineBetweenSlice; /*skip border lines*/
//              cptr += vos->oLineBetweenSlice; /*skip border lines*/
//            }
//          *sigma = (float)sqrt((double)(sum2/(n-1)));
//        }
//      return(OK);
// }

/*---------------------------------------------------------------------------*/

static Volume *VipComputeSmoothCrest(Volume *crest, Volume *vol)
{
  VipConnectivityStruct *vcs;
  Vip_S16BIT *smoothptr;
  Vip_S16BIT *volptr, *volvoisin;
  Vip_S16BIT *crestptr, *crestvoisin;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  int icon;
  Volume *smooth = NULL;
  float lemin, lemax;
  int sum, n;

  smooth = VipDuplicateVolumeStructure (crest, "smooth");
  if(!smooth) return(PB);
  VipAllocateVolumeData(smooth);
  VipSetVolumeLevel(smooth, 0);

  vcs = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );
  if(vcs==PB) return(PB);

  vos = VipGetOffsetStructure(vol);

  /*smooth with mean (median would be better)*/

  crestptr = VipGetDataPtr_S16BIT( crest ) + vos->oFirstPoint;
  smoothptr = VipGetDataPtr_S16BIT( smooth ) + vos->oFirstPoint;
  volptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;

  for ( iz = 0; iz < mVipVolSizeZ(vol); iz++ )
    {
      for ( iy = mVipVolSizeY(vol); iy-- ; )
        {
          for ( ix = mVipVolSizeX(vol); ix--; )
            {
              if(*crestptr)
                {
                  sum = *volptr;
                  n = 1;
                  lemin = *volptr;
                  lemax = *volptr;
                  for ( icon=0; icon<vcs->nb_neighbors; icon++ )
                    {
                      crestvoisin = crestptr + vcs->offset[icon];
                      if (*crestvoisin)
                        {
                          volvoisin = volptr + vcs->offset[icon];
                          if(*volvoisin<lemin) lemin = *volvoisin;
                          if(*volvoisin>lemax) lemax = *volvoisin;
                          sum += *volvoisin;
                          n++;
                        }
                    }
                  if(n>2)
                    {
                      sum -= lemin;
                      sum -= lemax;
                      n -= 2;
                    }
                  *smoothptr = (int)(sum/n);
                }
              crestptr++;
              smoothptr++;
              volptr++;
            }
          crestptr += vos->oPointBetweenLine;  /*skip border points*/
          volptr += vos->oPointBetweenLine;  /*skip border points*/
          smoothptr += vos->oPointBetweenLine;  /*skip border points*/
        }
      crestptr += vos->oLineBetweenSlice; /*skip border lines*/
      volptr += vos->oLineBetweenSlice; /*skip border lines*/
      smoothptr += vos->oLineBetweenSlice; /*skip border lines*/
    }

  VipFreeConnectivityStruct(vcs);

  return(smooth);
}

/*---------------------------------------------------------------------------*/

Volume *VipComputeCrestGrad(Volume *crest, Volume *vol)
{
  VipConnectivityStruct *vcs;
  Vip_S16BIT *gradptr;
  Vip_S16BIT *volptr, *volvoisin;
  Vip_S16BIT *crestptr, *crestvoisin;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  int icon;
  Volume *grad = NULL, *smooth = NULL;
  float lemin, lemax;

  vcs = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );
  if(vcs==PB) return(PB);

  vos = VipGetOffsetStructure(vol);

  /*smooth with mean (median would be better)*/

  smooth = VipComputeSmoothCrest(crest, vol);
  
  grad = VipDuplicateVolumeStructure (crest, "gradient");
  if(!grad) return(PB);
  VipAllocateVolumeData(grad);
  VipSetVolumeLevel(grad, 0);

  crestptr = VipGetDataPtr_S16BIT( crest ) + vos->oFirstPoint;
  gradptr = VipGetDataPtr_S16BIT( grad ) + vos->oFirstPoint;
  volptr = VipGetDataPtr_S16BIT( smooth ) + vos->oFirstPoint;

  for ( iz = 0; iz < mVipVolSizeZ(vol); iz++ )
    {
      for ( iy = mVipVolSizeY(vol); iy-- ; )
        {
          for ( ix = mVipVolSizeX(vol); ix--; )
            {
              if(*crestptr)
                {
                  lemin = *volptr;
                  lemax = *volptr;
                  for ( icon=0; icon<vcs->nb_neighbors; icon++ )
                    {
                      crestvoisin = crestptr + vcs->offset[icon];
                      if (*crestvoisin)
                        {
                          volvoisin = volptr + vcs->offset[icon];
                          if(*volvoisin<lemin) lemin = *volvoisin;
                          if(*volvoisin>lemax) lemax = *volvoisin;
                        }
                    }
                  /* *gradptr = (int)mVipMax(lemax-*volptr,*volptr - lemin);*/
                  if(lemax==lemin) *gradptr = 1;
                  else *gradptr = (int)(100*(lemax-lemin)/mVipMax(1, *volptr));
                  /* *gradptr = lemax-lemin;*/
                }
              crestptr++;
              gradptr++;
              volptr++;
            }
          crestptr += vos->oPointBetweenLine;  /*skip border points*/
          volptr += vos->oPointBetweenLine;  /*skip border points*/
          gradptr += vos->oPointBetweenLine;  /*skip border points*/
        }
      crestptr += vos->oLineBetweenSlice; /*skip border lines*/
      volptr += vos->oLineBetweenSlice; /*skip border lines*/
      gradptr += vos->oLineBetweenSlice; /*skip border lines*/
    }
  VipFreeConnectivityStruct(vcs);
  VipFreeVolume(smooth);
  return(grad);
}

/*---------------------------------------------------------------------------*/

Volume *VipComputeCrestGradExtrema(Volume *grad, Volume *vol)
{
  VipConnectivityStruct *vcs;
  Vip_S16BIT *extremaptr;
  Vip_S16BIT *volptr, *volvoisin;
  Vip_S16BIT *crestptr, *crestvoisin;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  int icon, iconmin, iconmax;
  Volume *extrema = NULL, *smooth = NULL;
  float lemin, lemax;
  int gradmin, gradmax;

  vcs = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );
  if(vcs==PB) return(PB);

  vos = VipGetOffsetStructure(vol);

  /*smooth with mean (median would be better)*/

  smooth = VipComputeSmoothCrest(grad, vol);

  extrema = VipDuplicateVolumeStructure (grad, "gradient");
  if(!extrema) return(PB);
  VipAllocateVolumeData(extrema);
  VipSetVolumeLevel(extrema, 0);

  crestptr = VipGetDataPtr_S16BIT( grad ) + vos->oFirstPoint;
  extremaptr = VipGetDataPtr_S16BIT( extrema ) + vos->oFirstPoint;
  volptr = VipGetDataPtr_S16BIT( smooth ) + vos->oFirstPoint;

  for ( iz = 0; iz < mVipVolSizeZ(vol); iz++ )
    {
      for ( iy = mVipVolSizeY(vol); iy-- ; )
        {
          for ( ix = mVipVolSizeX(vol); ix--; )
            {
              if(*crestptr)
                {
                  lemin = *volptr;
                  lemax = *volptr;
                  iconmin = 0;
                  iconmax = 0;
                  for ( icon=0; icon<vcs->nb_neighbors; icon++ )
                    {
                      crestvoisin = crestptr + vcs->offset[icon];
                      if (*crestvoisin)
                        {
                          volvoisin = volptr + vcs->offset[icon];
                          if(*volvoisin<lemin)
                            {
                              lemin = *volvoisin;
                              iconmin = icon;
                            }
                          if(*volvoisin>lemax)
                            {
                              lemax = *volvoisin;
                              iconmax = icon;
                            }
                        }
                    }
                  gradmin = *(crestptr+vcs->offset[iconmin]);
                  gradmax = *(crestptr+vcs->offset[iconmax]);
                  if (*crestptr<=gradmin || *crestptr<=gradmax) *extremaptr = 0;
                  else *extremaptr = *crestptr;
                }
              crestptr++;
              extremaptr++;
              volptr++;
            }
          crestptr += vos->oPointBetweenLine;  /*skip border points*/
          volptr += vos->oPointBetweenLine;  /*skip border points*/
          extremaptr += vos->oPointBetweenLine;  /*skip border points*/
        }
      crestptr += vos->oLineBetweenSlice; /*skip border lines*/
      volptr += vos->oLineBetweenSlice; /*skip border lines*/
      extremaptr += vos->oLineBetweenSlice; /*skip border lines*/
    }
  VipFreeConnectivityStruct(vcs);
  VipFreeVolume(smooth);
  /*VipWriteTivoliVolume(extrema,"extrema");*/
  return(extrema);
}
