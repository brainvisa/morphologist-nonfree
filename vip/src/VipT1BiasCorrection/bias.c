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



/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/
static Volume *VipComputeSmoothCrest(Volume *crest, Volume *vol);
static Volume *VipComputeCrestGradExtrema(Volume *grad, Volume *vol);
extern int  VipComputeStatInMaskVolume(Volume *vol, Volume *thresholdedvol, float *mean, float *sigma, int robust);

/*---------------------------------------------------------------------------*/
extern Volume *VipComputeT1BiasFieldMultiGrid(int mode, int dumb, Volume *vol, Volume *crest, float undersampling, 
				     float Kentropy,
                                              float Kregularization, float Kcrest, float Koffset,
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
  VIP_DEC_VOLUME(boundingbox);
  VIP_DEC_VOLUME(deriche);
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
  /* temporary stuff */
  int readlib, writelib;
  int compression = 0;
  int compressionset = VFALSE;
  int tauto = VTRUE;
  int thresholdlow = 15;
  int thresholdlowset = VFALSE;
  int thresholdhigh = 100000;
  int thresholdhighset = VFALSE;
  float mult_factor = 10.;
  int nInc =2;
  float Inc = 1.03;
  float Kentropy=1.;
  float Kregul=50.;
  float Koffset=0.5;
  float Kcrest=20.;
  float sampling = 16.;
  float temperature = 10.;
  float geom = 0.97;
  int dumb = VTRUE;
  VipHisto *histo, *comphisto;
  int ratio=0;
  int fieldtype = F3D_REGULARIZED_FIELD;
  int ngrid = 2;
  float RegulZTuning=1.;
  int variance_threshold = -1;
  int variance_pourcentage = -1;
  int deriche_edges = -1;
  float lemax, tlow, thigh;
  int connectivity=CONNECTIVITY_26;
  Volume *edges=NULL;
  int mode = GEOMETRY;
  float max_gradient;
  float mean=0;
  float sigma = 0;
  VipHisto *histo_edges; 
  int i;
  int threshold_edges = 2;
  int controlled = VFALSE;
  int xCA=0, yCA=0, zCA=0; 
  int xCP=0, yCP=0, zCP=0;
  int xP=0, yP=0, zP=0;
  char point_filename[VIP_NAME_MAXLEN]="";
  int talset = VFALSE;
  VipTalairach tal, *coord=NULL;
  int docorrection = VTRUE;

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
     else if (!strncmp (argv[i], "-Last", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          if(argv[i][0]=='a') Last = 3000;
	  else Last = atoi(argv[i]);
	}
     else if (!strncmp (argv[i], "-Points", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(point_filename,argv[i]);
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

  if (fieldtype == F2D_REGULARIZED_FIELD ) printf("2D field regularization\n");
  if (fieldtype == F3D_REGULARIZED_FIELD ) printf("3D field regularization\n");

  /* PREPROCESSING: better estimation of histo
     estimation of white matter crest line */

  /*decembre 04, Montreal, develop automatic definition
    of threshold for tissues/background*/

  if(tauto==VTRUE)
    {
      /*je plonge le volume ds un plus grand a cause des images normalisees,
        a la SPM, avec la tete coupee. Il n'y a plus de contour sur les bords
        et on chope trop de tissus avec extedge*/   
      vol = VipReadVolumeWithBorder(input,3);
      if(mVipVolType(vol)==U8BIT)
      {
	  converter = VipTypeConversionToS16BIT(vol,RAW_TYPE_CONVERSION);
	  if(converter==PB) return(VIP_CL_ERROR);
	  flag8bit = VTRUE;
	  VipFreeVolume(vol);
	  vol = converter;
      }

      if(Last==3000)
      {
          if(GetCommissureCoordinates(vol, point_filename, &tal,
			xCA, yCA, zCA,
			xCP, yCP, zCP,
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
              printf("Commissure Coordinates are necessary to delete automatically the last slides\n");
              Last = 0;
          }
      }
      printf("deleting last %d slices\n",Last);
      for(i=0;i<Last;i++)
        VipPutOneSliceTwoZero(vol,mVipVolSizeZ(vol)-i-1);
  
      boundingbox = VipCreateSingleThresholdedVolume( vol, EQUAL_TO, 0 , BINARY_RESULT);
      VipSetBorderLevel( boundingbox,255);
      VipResizeBorder( boundingbox, mVipVolBorderWidth(boundingbox)-1 );
      if( VipConnexVolumeFilter(boundingbox , CONNECTIVITY_26, -1, 
        CONNEX_BINARY) == PB) return(PB);
      VipConnectivityChamferDilation (boundingbox,1,CONNECTIVITY_26, 
        FRONT_PROPAGATION);
      VipSetBorderLevel( boundingbox,255);
      VipResizeBorder( boundingbox, 0 );
      VipInvertBinaryVolume(boundingbox);
      /*VipWriteTivoliVolume( boundingbox, "boundingbox");*/
      VipSetBorderLevel( vol,0);
      VipResizeBorder( vol, 0 );
      /*
      VipSet3DSize(vol,mVipVolSizeX(vol)+4,mVipVolSizeY(vol)+4,mVipVolSizeZ(vol)+4);
      VipSetBorderWidth(vol,0); j'attend de voir la fin du plug aimsIO*/
      deriche = VipCopyVolume(vol,"deriche");
      if (deriche==PB) return(VIP_CL_ERROR);
      if(VipDeriche3DGradientNorm(deriche, 2., DERICHE_EXTREMA, 0.)==PB) return(VIP_CL_ERROR);

      /*compress to prevent spurious minima related to too many bins*/
       
      if(compressionset==VFALSE)
        {
          histo = VipComputeVolumeHisto(deriche);
          ratio = 0;
          comphisto = VipGetPropUndersampledHisto( histo, 50, &ratio, &compression, 1, 100 );
          printf("gradient image compression: %d\n",ratio);
          VipFreeHisto(comphisto);
          VipFreeHisto(histo);	  
        }
      if (compression !=0)
        {    
          printf("Compressing volume (forgetting %d bits)...",compression);
          compressed = VipComputeCompressedVolume( deriche, compression);
          if(compressed == PB) return(VIP_CL_ERROR);
          printf("\n");
          VipFreeVolume(deriche);
          deriche=compressed;
          compressed=NULL;
        }

      max_gradient = VipGetVolumeMax(deriche);

      VipMaskVolume(deriche,boundingbox);
      VipFreeVolume(boundingbox);

      histo_edges = VipComputeVolumeHisto(deriche);

      if (histo_edges==PB) return(VIP_CL_ERROR);
      for(i=1;i<=histo_edges->range_max-3;i++) 
        {
           if ((histo_edges->val[i]<=histo_edges->val[i-1])
               && (histo_edges->val[i]<=histo_edges->val[i+1])
               && (histo_edges->val[i]<=histo_edges->val[i+2])) break;
        }
      printf("edge magnitude minimum: %d\n", i);
      VipFreeHisto(histo_edges);	  

      if (i>(max_gradient*0.1) || i<(max_gradient*0.01))
        {
          threshold_edges = (int)(max_gradient)*0.05;
        }
      else threshold_edges = (int)(i*1);
    
      printf("Tissue/background gradient threshold: %d\n", threshold_edges);


      thresholdedvol = VipCreateSingleThresholdedVolume( deriche, GREATER_THAN, threshold_edges , BINARY_RESULT); 
      
      if (VipExtRayCorner(thresholdedvol, EXTEDGE3D_ALL_EXCEPT_Z_BOTTOM, SAME_VOLUME)==PB) return(VIP_CL_ERROR);

      VipResizeBorder( vol, 3 );
      VipResizeBorder( thresholdedvol, 3 );
      /*VipWriteTivoliVolume( thresholdedvol, "outside");*/

      VipComputeStatInMaskVolume(vol,thresholdedvol, &mean, &sigma, VTRUE);
      /*debug
      VipWriteTivoliVolume( thresholdedvol, "outside");
      */
      VipFreeVolume(thresholdedvol);

      VipResizeBorder( deriche, 3 );
      if (writeedges==VTRUE) VipWriteVolume( deriche, edgesname);

      printf("Corner background stats: mean: %f; sigma: %f\n", mean, sigma);
      thresholdlowset=VTRUE;
      thresholdlow = (int)(mean+2*sigma+0.5);
      printf("threshold for corner background/tissue: %d\n", thresholdlow);

/**/
      masked = VipCopyVolume(vol,"extray");
      if (VipExtRay(masked, EXTEDGE3D_ALL_EXCEPT_Z_BOTTOM, SAME_VOLUME)==PB) return(VIP_CL_ERROR);
//       VipComputeCustomizedFrontPropagationChamferDistanceMap( masked, 255, -1, 
//               VIP_NO_LIMIT_IN_PROPAGATION, 0, 3, 3, 3, 50 );
      if( mean < 0.3 )
      {
          printf("The volume has been already cutout, redefinition of the threshold...\n");
          thresholdedvol = VipCopyVolume(vol,"closing");
          if(!thresholdedvol) return(VIP_CL_ERROR);
          VipFreeVolume(masked);

          VipSingleThreshold( thresholdedvol, GREATER_OR_EQUAL_TO, thresholdlow, BINARY_RESULT );
          masked = VipCopyVolume(thresholdedvol,"opening");
          VipConnectivityChamferOpening (masked,2.5,CONNECTIVITY_26,FRONT_PROPAGATION);
//           VipConnectivityChamferErosion (masked, mVipMax(mVipVolVoxSizeX(vol),
//                                          mVipVolVoxSizeY(vol))+0.1, CONNECTIVITY_26, FRONT_PROPAGATION);

          VipConnectivityChamferClosing (thresholdedvol,2.5,CONNECTIVITY_26,FRONT_PROPAGATION);
          VipConnectivityChamferDilation (thresholdedvol,mVipMax(mVipVolVoxSizeX(vol), 
                                          mVipVolVoxSizeY(vol))+0.1,CONNECTIVITY_26,FRONT_PROPAGATION);

          VipMerge( thresholdedvol, masked, VIP_MERGE_ONE_TO_ONE, 255, 0 );
          VipFreeVolume(masked);
      }
      else
      {
          thresholdedvol = VipCopyVolume(vol,"closing");
          if(!thresholdedvol) return(VIP_CL_ERROR);
          VipSingleThreshold( thresholdedvol, LOWER_THAN, thresholdlow, BINARY_RESULT );
          VipMerge( thresholdedvol, masked, VIP_MERGE_ONE_TO_ONE, 255, 0 );
          VipFreeVolume(masked);
//           VipWriteTivoliVolume( thresholdedvol, "closing");
          VipConnectivityChamferClosing (thresholdedvol,1,CONNECTIVITY_26,FRONT_PROPAGATION);
      }
/**/

      masked = VipCopyVolume(vol,"masked");
      if(!masked) return(VIP_CL_ERROR);
      VipMaskVolume(masked,thresholdedvol);
      VipComputeStatInMaskVolume(masked,thresholdedvol, &mean, &sigma, VFALSE);
      VipFreeVolume(thresholdedvol);
      VipFreeVolume(masked);

      printf("Background stats: mean: %f; sigma: %f\n", mean, sigma);
      thresholdlowset=VTRUE;
      thresholdlow = (int)(mean+2*sigma+0.5);
      printf("global threshold background/tissue: %d\n", thresholdlow);
    }
  /*1
  thresholdlow=0;
  */
  if (variance_threshold!=-1 || variance_pourcentage!=-1)
    {           
      if(vol==NULL)
        {
          vol = VipReadVolumeWithBorder(input,1);
          if(mVipVolType(vol)==U8BIT)
            {
              converter = VipTypeConversionToS16BIT(vol,RAW_TYPE_CONVERSION);
              if(converter==PB) return(VIP_CL_ERROR);
              flag8bit = VTRUE;
              VipFreeVolume(vol);
              vol = converter;
            }
        }
      /*O*/
      /**/
      variance_brute = VipComputeVarianceVolume(vol);
      if (variance_brute==PB) return(VIP_CL_ERROR);

      if (writevariance==VTRUE) VipWriteVolume( variance_brute, variancename);
      /*
      VipComputeStatInMaskVolume(variance,variance, &mean, &sigma, VTRUE);
      printf("variance in tissues: mean: %f; sigma: %f\n", mean, sigma);
      variance_threshold = (int)(mean-0*sigma+0.5);
      */

      VipSingleThreshold( vol, GREATER_OR_EQUAL_TO, thresholdlow, GREYLEVEL_RESULT );
      mask = VipCopyVolume(vol,"mask");
      VipFreeVolume(vol);

      VipMaskVolume(variance_brute,mask);      
    }          

  vol = VipReadVolumeWithBorder(input,0);

  if(vol==NULL) return(VIP_CL_ERROR);

  if(mVipVolType(vol)==U8BIT)
      {
	  converter = VipTypeConversionToS16BIT(vol,RAW_TYPE_CONVERSION);
	  if(converter==PB) return(VIP_CL_ERROR);
	  flag8bit = VTRUE;
	  VipFreeVolume(vol);
	  vol = converter;
      }

  if(Last==3000)
  {
      if(GetCommissureCoordinates(vol, point_filename, &tal,
         xCA, yCA, zCA,
         xCP, yCP, zCP,
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
          printf("Commissure Coordinates are necessary to delete automatically the last slides\n");
          Last = 0;
      }
  }

  printf("deleting last %d slices\n", Last);
  for(i=0;i<Last;i++)
    VipPutOneSliceTwoZero(vol,mVipVolSizeZ(vol)-i-1);
  
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
  if(compressionset==VFALSE)
      {
	histo = VipComputeVolumeHisto(vol);
	ratio = 0;
	/* discard potential modes higher than white matter*/
	comphisto = VipGetPropUndersampledHisto( histo, 95, &ratio, &compression, thresholdlow, 100 );
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
  if(thresholdlowset==VTRUE)
      thresholdlow = thresholdlow >> compression ;
  if(thresholdhighset==VTRUE)
      thresholdhigh = thresholdhigh >> compression ;
  printf("Low threshold: %d, High threshold: %d (after compression)\n", thresholdlow, thresholdhigh);
  VipDoubleThreshold(compressed,VIP_BETWEEN,thresholdlow,thresholdhigh,GREYLEVEL_RESULT);

 
  if(readridges==VFALSE)
    {

      smooth = VipDeriche3DGaussian( vol, 1., NEW_FLOAT_VOLUME);
      mc = Vip3DGeometry(smooth,MEAN_CURVATURE);
      if (writemeancurvature==VTRUE) VipWriteVolume( mc, meancurvaturename);

      VipSingleFloatThreshold(mc,LOWER_OR_EQUAL_TO,-0.4,BINARY_RESULT);    
      white_crest = VipTypeConversionToS16BIT( mc , RAW_TYPE_CONVERSION); 
      VipFreeVolume(mc);

      /*define target crest line: may miss some small part, but include no spurious ones
        the goal is to do a kind of hysteresis*/

      variance_threshold = VipPourcentageLowerThanThreshold(variance_brute, 1, 80);
      
      printf("strongest high threshold on variance: %d\n", variance_threshold);
      variance = VipCreateDoubleThresholdedVolume(variance_brute,VIP_BETWEEN_OR_EQUAL_TO,
                                                  0,variance_threshold,BINARY_RESULT);

      VipMaskVolume(variance,mask);      
      VipConnectivityChamferErosion( variance, 1, CONNECTIVITY_26, FRONT_PROPAGATION );

      target = VipCreateSingleThresholdedVolume( white_crest, GREATER_THAN, 1 , BINARY_RESULT); 

      if(!target) return(VIP_CL_ERROR);
      VipMaskVolume(target,variance);

      gradient = VipComputeCrestGrad(target, compressed);
      VipFreeVolume(target);
      extrema = VipComputeCrestGradExtrema(gradient, compressed);

      VipComputeStatInMaskVolume(gradient,gradient, &mean, &sigma, VTRUE);
      printf("crest gradient: mean: %f; sigma: %f\n", mean, sigma);
      thresholdhigh = (int)(mean+2*sigma+0.5);
      printf("threshold gradient: %d\n", thresholdhigh);
      white_cresttemp = VipCreateDoubleThresholdedVolume(gradient,VIP_BETWEEN_OR_EQUAL_TO,1,thresholdhigh,BINARY_RESULT);
      VipSingleThreshold( extrema, GREATER_OR_EQUAL_TO, (int)(mean+1.*sigma+0.5), BINARY_RESULT);
      VipMerge(white_cresttemp,extrema,VIP_MERGE_ONE_TO_ONE,255,0);
      if (VipConnexVolumeFilter (white_cresttemp, CONNECTIVITY_26, -1, CONNEX_BINARY)==PB) return(VIP_CL_ERROR);
      VipComputeStatInMaskVolume(gradient,white_cresttemp, &mean, &sigma, VFALSE);


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

          VipMaskVolume(variance,mask);   
          
          VipConnectivityChamferErosion( variance, 1, CONNECTIVITY_26, FRONT_PROPAGATION );

          arrow = VipCreateSingleThresholdedVolume( white_crest, GREATER_THAN, 1 , BINARY_RESULT); 

          if(!arrow) return(VIP_CL_ERROR);
          VipMaskVolume(arrow,variance);

          gradient = VipComputeCrestGrad(arrow, compressed);
          VipFreeVolume(arrow);
          extrema = VipComputeCrestGradExtrema(gradient, compressed);
          /*
          VipComputeStatInMaskVolume(gradient,gradient, &mean, &sigma, VTRUE);
          printf("crest gradient: mean: %f; sigma: %f\n", mean, sigma);
          thresholdhigh = (int)(mean+2*sigma+0.5);
          printf("threshold gradient: %d\n", thresholdhigh);
          white_cresttemp = VipCreateDoubleThresholdedVolume(gradient,VIP_BETWEEN_OR_EQUAL_TO,1,thresholdhigh,BINARY_RESULT);
          VipSingleThreshold( extrema, GREATER_OR_EQUAL_TO, (int)(mean+1.*sigma+0.5), BINARY_RESULT);
          VipMerge(white_cresttemp,extrema,VIP_MERGE_ONE_TO_ONE,255,0);
          if (VipConnexVolumeFilter (white_cresttemp, CONNECTIVITY_26, -1, CONNEX_BINARY)==PB) return(VIP_CL_ERROR);
          VipComputeStatInMaskVolume(gradient,white_cresttemp, &mean, &sigma, VFALSE);


          VipFreeVolume(white_cresttemp);
          printf("ridge gradient: mean: %f; sigma: %f\n", mean, sigma);
          thresholdhigh = (int)(mean+2*sigma+0.5);
          printf("threshold gradient: %d\n", thresholdhigh);
          */

          VipDoubleThreshold(gradient,VIP_BETWEEN_OR_EQUAL_TO,1,thresholdhigh,BINARY_RESULT);
          VipSingleThreshold( extrema, GREATER_OR_EQUAL_TO, (int)(mean+1.*sigma+0.5), BINARY_RESULT);

          VipMerge(gradient,extrema,VIP_MERGE_ONE_TO_ONE,255,0);
          VipFreeVolume(extrema);

          if (VipConnexVolumeFilter (gradient, CONNECTIVITY_26, -1, CONNEX_BINARY)==PB) return(VIP_CL_ERROR);
          arrow = VipDuplicateVolumeStructure ( gradient, "arrow" );
          arrow->borderWidth = 1;
          VipTransferVolumeData ( gradient, arrow );
          VipMerge( arrow, target, VIP_MERGE_ONE_TO_ONE, 255, 512 );
          /*VipWriteTivoliVolume(arrow,"arrow");*/

          VipChangeIntLabel(arrow,0,-2222);
          VipComputeFrontPropagationConnectivityDistanceMap(arrow,255,-2222,VIP_NO_LIMIT_IN_PROPAGATION,0,CONNECTIVITY_26);
          /*il y a une grosse merde la dedans, faut vraiment y faire un tour un jour
          VipComputeCustomizedFrontPropagationChamferDistanceMap(arrow,255,-2222,VIP_NO_LIMIT_IN_PROPAGATION,0,
                                                                 3, 3, 3, mult_factor);
          */
          VipChangeIntLabel(arrow,-2222,0);

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
      variance = VipCreateDoubleThresholdedVolume(variance_brute,VIP_BETWEEN_OR_EQUAL_TO,
                                                  0,variance_threshold,BINARY_RESULT);

      VipMaskVolume(variance,mask);      
      VipConnectivityChamferErosion( variance, 1, CONNECTIVITY_26, FRONT_PROPAGATION );
    }
  if (variance)
    {
      VipConnectivityChamferDilation( variance, 1, CONNECTIVITY_26, FRONT_PROPAGATION );
      VipMaskVolume(compressed,variance);
      VipFreeVolume(variance);
    }
     
  if (edges)
    {
      VipMaskVolume(compressed,edges);
      VipFreeVolume(edges);
    }
  if (VipConnexVolumeFilter (compressed, CONNECTIVITY_26, -1, CONNEX_GREYLEVEL)==PB) return(VIP_CL_ERROR);

  printf("Computing Bias with Kentropy = %f, Kregularization = %f, Kcrest = %f Koffset = %f\n",
	 Kentropy,Kregul,Kcrest,Koffset);

  
  result = VipComputeT1BiasFieldMultiGrid(mode, dumb,compressed,gradient,
                                          sampling,Kentropy,Kregul,Kcrest,Koffset,
                                          amplitude,temperature,geom,fieldtype,nInc,Inc,ngrid,RegulZTuning);

  if(result==PB) return(VIP_CL_ERROR);
  

  if (writehfiltered==VTRUE)
    {
      VipSingleThreshold( compressed, GREATER_OR_EQUAL_TO, 1, BINARY_RESULT );
      VipWriteVolume(compressed,hfilteredname);

    }
  VipFreeVolume(compressed);
  /* debug  vol = compressed;*/
  fullresult = VipResampleField(result, vol);
  VipFreeVolume(result);
  
  if(fullresult==PB) return(VIP_CL_ERROR);
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
 
  VipComputeStatInMaskVolume(gradient,gradient, &mean, &sigma, VTRUE);
  printf("crest gradient: mean: %f; sigma: %f\n", mean, sigma);
  thresholdhigh = (int)(mean+2*sigma+0.5);
  printf("threshold gradient: %d\n", thresholdhigh);
  white_cresttemp = VipCreateDoubleThresholdedVolume(gradient,VIP_BETWEEN_OR_EQUAL_TO,1,thresholdhigh,BINARY_RESULT);
  if (VipConnexVolumeFilter (white_cresttemp, CONNECTIVITY_26, -1, CONNEX_BINARY)==PB) return(VIP_CL_ERROR);
  VipComputeStatInMaskVolume(gradient,white_cresttemp, &mean, &sigma, VFALSE);
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
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipBiasCorrection\n");
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
  (void)fprintf(stderr,"        [-f[ield] {bias field name (default:\"biasfield\")}]\n");
  (void)fprintf(stderr,"        [-Di[mfield] {type of field (2/3) (default:3)}]\n");
  (void)fprintf(stderr,"        [-F[write] {write field: y/n (default:n)}]\n");
  (void)fprintf(stderr,"        [-d[umb] {y/n (default:y)}]\n");
  (void)fprintf(stderr,"        [-Ke[ntropy] {float  (default:1.)}]\n");
  (void)fprintf(stderr,"        [-Kr[egul] {float  (default:50.)}]\n");
  (void)fprintf(stderr,"        [-Ko[ffset] {float  (default:0.5)}]\n");
  (void)fprintf(stderr,"        [-c[ompression] {int [0,14]  (default:auto)}]\n");
  (void)fprintf(stderr,"        [-t[auto] {char: y/n default:y}]\n");
  (void)fprintf(stderr,"        [-tl[ow] {int (default:3*2^compression)}]\n");
  (void)fprintf(stderr,"        [-th[igh] {int (default:not used)}]\n");
  (void)fprintf(stderr,"        [-e[dges] {char (default:not used, n/2/3)}]\n");
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
  (void)fprintf(stderr,"        [-L[ast] {int (default:0)}]\n");
  (void)fprintf(stderr,"        [-P[oints] {AC,PC,IH coord filename (*.tal) (default:not used)}]\n");
  (void)fprintf(stderr,"        [-Dc[orrect] {Do the biais correction: y/n (default:y)}]\n");
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
  (void)printf("        [-L[ast] {int (default:0)}]\n");
  (void)printf("Mask the Last last slices in all estimation (for field of view going to the shoulder)\n");
  (void)printf("        [-P[oints] {AC,PC,IH coord filename (*.tal) (default:not used)}]\n");
  (void)printf("Correct format for the commissure coordinate file toto.APC:\n");
  (void)printf("AC: 91 88 113\nPC: 91 115 109\nIH: 90 109 53\n");
  (void)printf("        [-Dc[orrect] {Do the biais correction: y/n (default:y)}]\n");
  (void)printf("Write the images as hfiltered or whiteridge whithout doing the bias correction if it has been done by another software\n");
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
int VipComputeStatInMaskVolume(Volume *vol, Volume *thresholdedvol, float *mean, float *sigma, int robust)
{
  VipOffsetStruct *vos;
  int ix, iy, iz;
  Vip_S16BIT *ptr, *cptr;
  double sum, sum2;
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
     sum2 = 0.;
     for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
       {
	 for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
	   {
	     for ( ix = mVipVolSizeX(vol); ix-- ; )/* loop on points */
	       {
		 if(*cptr)
		   {
                     temp = *ptr-*mean;
		     sum2 += temp*temp;
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
     *sigma = (float)sqrt((double)(sum2/(n-1)));

     if (robust==VTRUE)
       {

         threshold = *mean + 3 * *sigma;


         ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
         cptr = VipGetDataPtr_S16BIT( thresholdedvol  ) + vos->oFirstPoint;
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
                             n--;
                             sum -= *ptr;
                           }
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
         
         *mean = (float)(sum/n);
         
         sum2=0.;
         ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
         cptr = VipGetDataPtr_S16BIT( thresholdedvol ) + vos->oFirstPoint;
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
                             sum2 += temp*temp;
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
         *sigma = (float)sqrt((double)(sum2/(n-1)));
       }
     return(OK);
   
}
/******************************************************/
static Volume *VipComputeSmoothCrest(Volume *crest, Volume *vol)
{
  VipConnectivityStruct *vcs;
  Vip_S16BIT *smoothptr;
  Vip_S16BIT *volptr, *volvoisin;
  Vip_S16BIT *crestptr, *crestvoisin;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  int icon;
  Volume *smooth=NULL;
  float lemin, lemax;
  int sum,n;

  smooth = VipDuplicateVolumeStructure (crest,"smooth");
  if(!smooth)  return(PB);
  VipAllocateVolumeData(smooth);
  VipSetVolumeLevel(smooth,0);

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
                  sum=*volptr;
                  n=1;
                  lemin = *volptr;
                  lemax = *volptr;
                  for ( icon=0; icon<vcs->nb_neighbors;icon++)
                    {
                      crestvoisin =  crestptr + vcs->offset[icon];
                      if (*crestvoisin)
                        {
                          volvoisin =  volptr + vcs->offset[icon];
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
                      n -=2;                      
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

  return(smooth);


}

/*-------------------------------------------------*/

Volume *VipComputeCrestGrad(Volume *crest, Volume *vol)
{
  VipConnectivityStruct *vcs;
  Vip_S16BIT *gradptr;
  Vip_S16BIT *volptr, *volvoisin;
  Vip_S16BIT *crestptr, *crestvoisin;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  int icon;
  Volume *grad = NULL, *smooth=NULL;
  float lemin, lemax;

  vcs = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );
  if(vcs==PB) return(PB);

  vos = VipGetOffsetStructure(vol);

  /*smooth with mean (median would be better)*/

  smooth = VipComputeSmoothCrest(crest, vol);
  
  grad = VipDuplicateVolumeStructure (crest,"gradient");
  if(!grad)  return(PB);
  VipAllocateVolumeData(grad);
  VipSetVolumeLevel(grad,0);
 

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
                  for ( icon=0; icon<vcs->nb_neighbors;icon++)
                    {
                      crestvoisin =  crestptr + vcs->offset[icon];
                      if (*crestvoisin)
                        {
                          volvoisin =  volptr + vcs->offset[icon];
                          if(*volvoisin<lemin) lemin = *volvoisin;
                          if(*volvoisin>lemax) lemax = *volvoisin;
                        }
                    }
                  /*
                   *gradptr = (int)mVipMax(lemax-*volptr,*volptr - lemin);*/
                   *gradptr =(int)(100*(lemax-lemin)/mVipMax(1,*volptr)); 
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

  VipFreeVolume(smooth);
  return(grad);

}

/*-----------------------------------------------------------*/

Volume *VipComputeCrestGradExtrema(Volume *grad, Volume *vol)
{
  VipConnectivityStruct *vcs;
  Vip_S16BIT *extremaptr;
  Vip_S16BIT *volptr, *volvoisin;
  Vip_S16BIT *crestptr, *crestvoisin;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  int icon, iconmin, iconmax;
  Volume *extrema = NULL, *smooth=NULL;
  float lemin, lemax;
  int gradmin, gradmax;

  vcs = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );
  if(vcs==PB) return(PB);

  vos = VipGetOffsetStructure(vol);

  /*smooth with mean (median would be better)*/

  smooth = VipComputeSmoothCrest(grad, vol);

  extrema = VipDuplicateVolumeStructure (grad,"gradient");
  if(!extrema)  return(PB);
  VipAllocateVolumeData(extrema);
  VipSetVolumeLevel(extrema,0);

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
                  iconmin=0;
                  iconmax=0;
                  for ( icon=0; icon<vcs->nb_neighbors;icon++)
                    {
                      crestvoisin =  crestptr + vcs->offset[icon];
                      if (*crestvoisin)
                        {
                          volvoisin =  volptr + vcs->offset[icon];
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
                  if (*crestptr<=gradmin || *crestptr<=gradmax) *extremaptr=0;
                  else *extremaptr=*crestptr;
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

  VipFreeVolume(smooth);
  /*
  VipWriteTivoliVolume(extrema,"extrema");*/
  return(extrema);

}
