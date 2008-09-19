/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipHistoAnalysis.c   * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 20/08/97
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : sparc5
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


#include <vip/histo.h>
#include <vip/util/file.h>
#include <vip/connex.h>
#include <vip/deriche.h>

static int  VipComputeStatInRidgeVolume(Volume *vol, Volume *thresholdedvol, float *mean, float *sigma, int robust);
Volume *VipComputeCrestAverageIntensity(Volume *crest, Volume *vol);

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{     
    char *input = NULL;
    char *ridgename = NULL;
    char *maskname = NULL;
    Volume *ridge=NULL;
    Volume *mask = NULL;
    char stripped_input[1024];
    char output[256]="";
    char tmphisto[512];
    char systemcommand[256];
    int readlib, writelib;
    int offset=0;
    int scalemax = 1000;
    int nbiter;
    float dscale = 0.5;
    Vip1DScaleSpaceStruct *volstruct;
    int i;
    VipHisto *shorthisto, *histo_surface;
    Volume *vol=NULL;
    SSSingularity *slist=NULL;
    SSCascade *clist=NULL, *chigh;
    char mode = 'v';
    int track = 5;
    int n;
    char surface_name[1024];
    int ncascade = 10;
    VipT1HistoAnalysis *ana = 0;
    char gnuplot = 'n';
    int gnuplotpsfile = VFALSE;
    FILE *gpfile;
    int nderivative = 2;
    int D0WRITE = VFALSE;
    int D1WRITE = VTRUE;
    int D2WRITE = VTRUE;
    int D3WRITE = VFALSE;
    int D4WRITE = VFALSE;
    int gnuplot_title = VTRUE;
    float mean=0;
    float sigma = 0;
    char SAVE = 'n';
    int xmax=-1;
    int hmax=50000;
    float entropy;
    char *extension;
    char *root1, *root2;
    int sequence = VFALSE;
    int undersampling_factor = 0;
    int variance_threshold = -1;
    int variance_pourcentage = -1;
    int deriche_edges = -1;
    float lemax, tlow, thigh;
    int connectivity=CONNECTIVITY_26;
    Volume *edges=NULL;
    Volume *discard=NULL;
    /*    char mask_name[256], temp_string[256];*/

    readlib = ANY_FORMAT;
    writelib = VIDA;
 
 

    /*loop on command line arguments*/

    for(i=1;i<argc;i++)
	{	
	    if (!strncmp (argv[i], "-input", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    input = argv[i];
		}
	    else if (!strncmp (argv[i], "-Ridge", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    ridgename = argv[i];
		}
	    else if (!strncmp (argv[i], "-Mask", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    maskname = argv[i];
		}
	    else if (!strncmp (argv[i], "-output", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    strcpy(output,argv[i]);
		}
	    else if (!strncmp (argv[i], "-mode", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    if(argv[i][0]=='f') mode = 'f';
		    else if(argv[i][0]=='a') mode = 'a';
		    else if(argv[i][0]=='e') mode = 'e';
		    else if(argv[i][0]=='v') mode = 'v';
		    else if(argv[i][0]=='c') mode = 'c';
		    else if(argv[i][0]=='C') mode = 'C';
		    else if(argv[i][0]=='s') mode = 's';		   
		    else
			{
			    VipPrintfError("This mode option is unknown");
			    VipPrintfExit("(commandline)VipHistoAnalysis");
			    return(VIP_CL_ERROR);
			}
		}    
	    else if (!strncmp (argv[i], "-0extrema", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    if(argv[i][0]=='y') D0WRITE = VTRUE;
		    else if(argv[i][0]=='n') D0WRITE = VFALSE;
		    else
			{
			    VipPrintfError("y/n choice!");
			    VipPrintfExit("(commandline)VipHistoAnalysis");
			    return(VIP_CL_ERROR);
			}
		}    
	    else if (!strncmp (argv[i], "-Contrast", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    if(argv[i][0]=='h') sequence = MRI_T1_IR;
		    else if(argv[i][0]=='l') sequence = MRI_T1_SPGR;
		    else if(argv[i][0]=='s') sequence = MRI_T1_SEGMENTED_BRAIN;
		    else if(argv[i][0]=='2') sequence = MODES_2;
		    else if(argv[i][0]=='3') sequence = MODES_3;
		    else if(argv[i][0]=='4') sequence = MODES_4;
		    else if(argv[i][0]=='H') sequence = MODE_HISTO;
		    else
			{
			    VipPrintfError("h/l/s/2/3/4/H choice!");
			    VipPrintfExit("(commandline)VipHistoAnalysis");
			    return(VIP_CL_ERROR);
			}
		}    
	    else if (!strncmp (argv[i], "-SAVE", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    if(argv[i][0]=='y') SAVE = 'y';
		    else if(argv[i][0]=='n') SAVE = 'n';
		    else
			{
			    VipPrintfError("y/n choice!");
			    VipPrintfExit("(commandline)VipHistoAnalysis");
			    return(VIP_CL_ERROR);
			}
		}    
	    else if (!strncmp (argv[i], "-1extrema", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    if(argv[i][0]=='y') D1WRITE = VTRUE;
		    else if(argv[i][0]=='n') D1WRITE = VFALSE;
		    else
			{
			    VipPrintfError("y/n choice!");
			    VipPrintfExit("(commandline)VipHistoAnalysis");
			    return(VIP_CL_ERROR);
			}
		}    
	    else if (!strncmp (argv[i], "-2extrema", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    if(argv[i][0]=='y') D2WRITE = VTRUE;
		    else if(argv[i][0]=='n') D2WRITE = VFALSE;
		    else
			{
			    VipPrintfError("y/n choice!");
			    VipPrintfExit("(commandline)VipHistoAnalysis");
			    return(VIP_CL_ERROR);
			}
		}    
	    else if (!strncmp (argv[i], "-3extrema", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    if(argv[i][0]=='y') D3WRITE = VTRUE;
		    else if(argv[i][0]=='n') D3WRITE = VFALSE;
		    else
			{
			    VipPrintfError("y/n choice!");
			    VipPrintfExit("(commandline)VipHistoAnalysis");
			    return(VIP_CL_ERROR);
			}
		}    
	    else if (!strncmp (argv[i], "-4extrema", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    if(argv[i][0]=='y') D4WRITE = VTRUE;
		    else if(argv[i][0]=='n') D4WRITE = VFALSE;
		    else
			{
			    VipPrintfError("y/n choice!");
			    VipPrintfExit("(commandline)VipHistoAnalysis");
			    return(VIP_CL_ERROR);
			}
		}    
	    else if (!strncmp (argv[i], "-Title", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    if(argv[i][0]=='y') gnuplot_title = VTRUE;
		    else if(argv[i][0]=='n') gnuplot_title = VFALSE;
		    else
			{
			    VipPrintfError("y/n choice!");
			    VipPrintfExit("(commandline)VipHistoAnalysis");
			    return(VIP_CL_ERROR);
			}
		}    
	    else if (!strncmp (argv[i], "-Offset", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    offset = atoi(argv[i]);
		}
	    else if (!strncmp (argv[i], "-xmax", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
                    xmax = atoi(argv[i]);
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
                    else
			{
                          printf("(you gave:%c)",argv[i][0]);
                          VipPrintfError("2/3 choice!");
                          VipPrintfExit("2D/3D edges: (commandline)VipHistoAnalysis");
                          return(VIP_CL_ERROR);
			}
		}
	    else if (!strncmp (argv[i], "-undersampling_factor", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    undersampling_factor = atoi(argv[i]);
		}	 
	    else if (!strncmp (argv[i], "-Hmax", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    hmax = atoi(argv[i]);
		}
	    else if (!strncmp (argv[i], "-scalemax", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    scalemax = atoi(argv[i]);
		}
	    else if (!strncmp (argv[i], "-track", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    track = atoi(argv[i]);
		}
	    else if (!strncmp (argv[i], "-cascade", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    ncascade = atoi(argv[i]);
		}
	    else if (!strncmp (argv[i], "-dscale", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    dscale = atof(argv[i]);
		}  
	    else if (!strncmp (argv[i], "-gnuplot", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    if(argv[i][0]=='n') gnuplot = 'n';
		    else if(argv[i][0]=='s') gnuplot = 's';
		    else if(argv[i][0]=='f') gnuplot = 'f';
		    else if(argv[i][0]=='p') gnuplot = 'p';
		    else
			{
			    VipPrintfError("This gnuplot option is unknown");
			    VipPrintfExit("(commandline)VipHistoAnalysis");
			    return(VIP_CL_ERROR);
			}
		}    
	    else if (!strncmp (argv[i], "-readformat", 2)) 
		{
		    if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
		    if(argv[i][0]=='t') readlib = TIVOLI;
		    else if(argv[i][0]=='a') readlib = ANY_FORMAT;
		    else if(argv[i][0]=='v') readlib = VIDA;
		    else if(argv[i][0]=='s') readlib = SPM;
		    else
			{
			    VipPrintfError("This format is not implemented for reading");
			    VipPrintfExit("(commandline)VipHistoAnalysis");
			    return(VIP_CL_ERROR);
			}
		}
	    else if (!strncmp(argv[i], "-help",2)) return(Help());
	    else return(Usage());
	}

    /*check that all required arguments have been given*/
    /*
    if( sequence == MRI_T1_SEGMENTED_BRAIN)
      {
        VipPrintfError("The segmented mode is under development, sorry");
        return(Usage());
      }
    */
    if (input==NULL)
	{
	    VipPrintfError("input arg is required by VipHistoAnalysis");
	    return(Usage());
	}

    strcpy(tmphisto,VipTmpDirectory());
    strcat(tmphisto,"/");
    root2 = input;
    root1 = input;
    while(root2!=NULL)
      {
	root2 = strstr(root1,"/");
	if(root2!=NULL) root1 = root2+1;
      }
    strcpy(stripped_input,root1);

    if(!strcmp(output,"")) strcpy(output,input);
    if(dscale>0.5)
	{
	    VipPrintfError("The maximum dscale allowed for stability is 0.5");
	    return(Usage());
	}

    if(!strcmp(input,"gaussian"))
	{
	    shorthisto = VipCreateThreeGaussianSumHisto(20.,5.,0.,150.,15.,5000.,250.,9.,4000.);
	}
    else
	{
	    extension=strstr(input,".his");

	    if(extension)
		{

		    *extension='\0';
		    if (VipTestHistoExists(input)!=PB)
			{
			    printf("Warning! Reading precomputed histogram...\n");
			    shorthisto = VipReadHisto(input);
			    if(shorthisto==NULL) return(VIP_CL_ERROR);
			}
		    else
			{
			    VipPrintfError("The input histogram does not exist");
			    return(VIP_CL_ERROR);
			}
                    extension=strstr(stripped_input,".his");
                    if(extension)
                      {
                        *extension='\0';			
                      }
                }
	    else  if (VipTestImageFileExist(input)==PB)
		{
		    (void)fprintf(stderr,"Can not open this image: %s\n",input);
		    return(VIP_CL_ERROR);
		}
	    else
		{
		    printf("Reading volume...\n");
                    if(variance_threshold==-1 && variance_pourcentage==-1)
                      {
                        if (readlib == TIVOLI)
                          vol = VipReadTivoliVolumeWithBorder(input,0);
                        else if (readlib == SPM)
                          vol = VipReadSPMVolumeWithBorder(input,0);
                        else if (readlib == VIDA)
                          vol = VipReadVidaVolumeWithBorder(input,0);
                        else
                          vol = VipReadVolumeWithBorder(input,0);
                      }
                    else
                      vol = VipReadVolumeWithBorder(input,1);

		    if(vol==PB) return(VIP_CL_ERROR);

                    if(maskname)
                      {
                        printf("Masking initial volume with %s...\n",maskname);
                        mask = VipReadVolume(maskname);
                        if(mask==PB) return(VIP_CL_ERROR);
                        VipMaskVolume(vol,mask);
                        VipFreeVolume(mask);
                      }
                    if(ridgename)
                      {
                        printf("Reading ridge volume %s...\n",ridgename);
                        ridge = VipReadVolume(ridgename);
                        VipComputeStatInRidgeVolume(vol,ridge, &mean, &sigma, VTRUE);
                        printf("ridge stats: mean: %f; sigma: %f\n", mean, sigma);
                        VipFreeVolume(ridge);
                      }
		    printf("Computing histogram\n");
                    
                    if(variance_threshold==-1 && variance_pourcentage==-1 && deriche_edges==-1)
                      {
                        shorthisto = VipComputeVolumeHisto(vol);
                      }
                    else
                      {
                        if (deriche_edges!=-1)
                          {
                            if (deriche_edges==2)
                              {
                                edges = VipDeriche2DGradientNorm(vol, 1., NEW_VOLUME, DERICHE_EXTREMA, 0.);
                                connectivity = CONNECTIVITY_8;
                              }
                            else
                              {
                                edges = VipCopyVolume(vol,"edges");
                                if (!edges) return(VIP_CL_ERROR);
                                if(VipDeriche3DGradientNorm(edges, 2, DERICHE_EXTREMA, 0.)==PB) return(VIP_CL_ERROR);
                              }
                            lemax = VipGetVolumeMax(edges); /*NB choix un peu arbitraire mais pas tres important*/
                            tlow = 0.04*lemax;
                            thigh = 0.08*lemax;
                            if(VipHysteresisThresholding(edges,connectivity,SAME_VOLUME,CONNEX_BINARY,tlow,thigh,HYSTE_NUMBER,1)==PB) return(VIP_CL_ERROR);
                            /*VipWriteTivoliVolume(edges,"Thresh_edges");*/
                          }
                        if (variance_threshold!=-1 || variance_pourcentage!=-1)
                          {                            
                            discard = VipComputeVarianceVolume(vol);
                            /*VipWriteTivoliVolume(discard,"variance");*/
                            if (discard==PB) return(VIP_CL_ERROR);
                            if (variance_pourcentage!=-1)
                              {
                                variance_threshold = VipPourcentageLowerThanThreshold(discard, 10, variance_pourcentage);
                              }
                            printf("Threshold on local deviations: %d\n", variance_threshold);
                            VipSingleThreshold( discard, GREATER_OR_EQUAL_TO, variance_threshold, BINARY_RESULT );
                            /*VipWriteTivoliVolume(discard,"Thresh_variance");*/

                            if (edges!=NULL)
                              {
                                VipMerge(discard,edges,VIP_MERGE_ALL_TO_ONE,0,255);
                                VipFreeVolume(edges);
                              }
                          }
                        else discard = edges;
                        shorthisto = VipComputeCleanedUpVolumeHisto(vol,discard);
                        VipInvertBinaryVolume(discard);
                        VipMaskVolume(vol,discard);
                        VipFreeVolume(discard);
                        /*
                        strcpy(mask_name,input);
                        sprintf(temp_string,"_%d_%d_",deriche_edges,variance_pourcentage);                        
                        strcat(mask_name,temp_string);
                        strcat(mask_name,"masked");
                        VipWriteTivoliVolume(vol,mask_name);
                        */
                      }
		    VipFreeVolume(vol);
		    if(mode!='e')
			{
			    printf("Writing histogram\n");
			    if(VipWriteHisto(shorthisto,input,WRITE_HISTO_ASCII)==PB)
				VipPrintfWarning("I can not write the histogram but I am going further");
			}
		}
	}
    if(mode=='e')
	{
	    /*
	    VipSetHistoVal(shorthisto,mVipHistoRangeMax(shorthisto)-1,1); bug brucker 3T
	    VipCutRightFlatPartOfHisto(shorthisto,10);
	    
	      if((mVipHistoRangeMax(shorthisto)-mVipHistoRangeMin(shorthisto))>600)
	      {
	      resampflag = VTRUE;
	      historesamp = VipGetUndersampledHisto(shorthisto,&(undersamp_ratio));
	      printf("Undersampling histogram... (factor %d)\n",undersamp_ratio);
	      if(historesamp==PB) return(VIP_CL_ERROR);
	      shorthisto = historesamp;
	      }
	    */
          if(VipGetHistoEntropy(shorthisto,&entropy)==PB) return(VIP_CL_ERROR);

          printf("entropy: %f\n",entropy);
	    return(0);
	}
    if(mode=='v')
	{
          strcpy(tmphisto,VipTmpDirectory());
          strcat(tmphisto,"/");
          /*            strcpy(tmphisto,"/tmp/");*/
	    strcat(tmphisto,stripped_input);
	    if(VipWriteHisto(shorthisto,tmphisto,WRITE_HISTO_ASCII)==PB)
		{
		    VipPrintfWarning("I can not write the histogram in /tmp...ciao");
		    return(VIP_CL_ERROR);
		}
          strcpy(tmphisto,VipTmpDirectory());
          strcat(tmphisto,"/");
          /*strcpy(tmphisto,"/tmp/");*/
	    strcat(tmphisto,stripped_input);
	    strcat(tmphisto,".gp");
	    gpfile = fopen(tmphisto,"w");
	    if(gpfile==NULL)
		{
		    VipPrintfWarning("I can not write the command for gp in: ...ciao");
                    printf("%s\n",VipTmpDirectory());
		    return(VIP_CL_ERROR);
		}

	    if(xmax<0)
		fprintf(gpfile,"plot [:] [0:%d] \"%s%c%s.his\" w l lw 5\n",
                        hmax, VipTmpDirectory(), VipFileSeparator(), input);
	    else if(gnuplot_title==VTRUE)
		fprintf(gpfile,"plot [:%d] [0:%d] \"%s%c%s.his\" w l lw 5\n",
                        xmax,hmax, VipTmpDirectory(), VipFileSeparator(), 
                        input);
	    else if(gnuplot_title==VFALSE)
		fprintf(gpfile, 
                        "plot [:%d] [0:%d] \"%s%c%s.his\" notitle w l lw 5\n", 
                        xmax,hmax, VipTmpDirectory(), VipFileSeparator(), 
                        input);
	    fprintf(gpfile,"pause -1\n");
	    fclose(gpfile);
	    strcpy(systemcommand, "gnuplot ");
	    strcat(systemcommand,tmphisto);
	    if(system(systemcommand))
		VipPrintfError("Can not use gnuplot here (or use \"return\" to quit gnuplot), sorry...\n");
	    return(VIP_CL_ERROR);
	}
      if(mode=='s')
      {
        vol = VipReadVolumeWithBorder(input,1);
        histo_surface = VipGetHistoSurface(shorthisto, vol);
        strcpy(surface_name,input);
        strcat(surface_name,"_surf");
        if(VipWriteHisto(histo_surface,surface_name,WRITE_HISTO_ASCII)==PB)           
          return(VIP_CL_ERROR);
        VipFreeVolume(vol);
        return(0);
      }

    printf("Computing scale space singularities up to order three...\n");
    fflush(stdout);
  
    /*VipSetHistoVal(shorthisto,mVipHistoRangeMax(shorthisto)-1,1);*/ /*bug brucker 3T*/
    VipSetHistoVal(shorthisto,0,0); /*outside field of view for square images*/

    if(mode=='a' && sequence==MODE_HISTO)
      {
        if( (mVipHistoRangeMax(shorthisto) - mVipHistoRangeMin(shorthisto) ) == 256 ) /*coupes histo avec un artefact a la fin de l'histo*/
          {
            VipPrintfWarning("256 values, I put to zero the 5 last values of the range (SHFJ scanner artefact for histology)"); 
            VipSetHistoVal(shorthisto,251,0);
            VipSetHistoVal(shorthisto,252,0);
            VipSetHistoVal(shorthisto,253,0);
            VipSetHistoVal(shorthisto,254,0);
            VipSetHistoVal(shorthisto,255,0);
          }
      }


    nbiter = (int)(scalemax/dscale+0.5);

    if(gnuplot=='p') gnuplotpsfile = VTRUE;

    if(mode=='f')
	{
	    if(D4WRITE==VTRUE) nderivative = 4;
	    else if(D3WRITE==VTRUE) nderivative = 3;
	    else if(D2WRITE==VTRUE) nderivative = 2;
	    else nderivative=1;

	    volstruct = VipCompute1DScaleSpaceStructFromHisto(shorthisto,nbiter,dscale,offset,nderivative,undersampling_factor );
	}
    else
	{
	    D0WRITE = VFALSE;
	    D1WRITE = VTRUE;
	    D2WRITE = VTRUE;	
	    D3WRITE = VFALSE;
	    D4WRITE = VFALSE;
	    volstruct = VipCompute1DScaleSpaceStructUntilLastCascade(shorthisto,dscale,offset,2,undersampling_factor);
	}
    if(volstruct==PB) return(VIP_CL_ERROR); 

    track = (int)(track/dscale+0.5);
    slist = VipComputeSSSingularityList(volstruct,track,D0WRITE,D1WRITE,D2WRITE,D3WRITE,D4WRITE);
    if(slist==PB) return(VIP_CL_ERROR);

    if(mode!='f')
	{
	    printf("Detecting D1/D2 singularity matings and cascades...\n");

	    if(VipFindSSSingularityMates(slist)==PB) return(VIP_CL_ERROR);

	    chigh = NULL;
	    VipCountSingularitiesStillAlive(slist,&n,volstruct->itermax);
  
	    if((n<=5))
		chigh = VipCreateHighestCascade(slist,volstruct->itermax,n);
	  
            if (mode=='C' || (mode=='a' && sequence==MODE_HISTO))
              clist = VipComputeScaleOrderedCascadeList( slist, nbiter, volstruct->hcumul);
            else
              clist = VipComputeOrderedCascadeList( slist, nbiter, volstruct->hcumul);
	    if(chigh!=NULL)
		{
                      chigh->next = clist;
                      clist = chigh;
                 
		}
	    else
		{
		    VipPrintfError("Sorry, the analysis can not proceed further");
		    return(VIP_CL_ERROR);
		}
	}

    if(mode=='a')
	{
          if(ridgename)
            {
              printf("Analysing histogram knowing white ridge statistics...\n");
              ana = VipAnalyseCascadesRidge( clist, volstruct, mean);
              if(ana==PB) return(VIP_CL_ERROR);
            }
          else
            {
              printf("Analysing histogram D1 and D2 singularities...\n");
              ana = VipAnalyseCascades( clist, volstruct, sequence);
              if(ana==PB) return(VIP_CL_ERROR);

              if(ana->sequence == MRI_T1_SPGR)
		printf("This T1-MR image seems to come from a classic SPGR sequence (low gray/white contrast)\n");
              else if(ana->sequence == MRI_T1_IR)
		printf("This T1-MR image seems to come from an Inversion/Recovery sequence (high gray/white contrast)\n");
              else if(ana->sequence == MRI_T1_SEGMENTED_BRAIN)
                printf("Analyse has assumed a segmented brain...\n");
              else if(ana->sequence == MODES_2)
                printf("Analyse will provide 2 main modes...\n");	
              else if(ana->sequence == MODES_3)
                printf("Analyse will provide 3 main modes...\n");	
              else if(ana->sequence == MODES_4)
                printf("Analyse will provide 4 main modes...\n");	
              else if(ana->sequence == MODE_HISTO)
                printf("Analyse of histological slice ...\n");	
              else
		{
                  VipPrintfError("Unknown sequence");
                  return(VIP_CL_ERROR);
		}
            }
            if (ana->sequence==MODE_HISTO)
              VipMarkAnalysedObjectsHisto( ana, volstruct);
	    else VipMarkAnalysedObjects( ana, volstruct);
	    if(SAVE=='y')
		{
                  /*if(ana->sequence == MRI_T1_SEGMENTED_BRAIN)  ana->sequence = MRI_T1_SPGR;*/
                  if(VipWriteT1HistoAnalysis( ana, output)==PB) return(VIP_CL_ERROR);
		}
	       
	}
    else if (mode=='c')
	{	
	    VipMarkNBiggestCascades( clist, ncascade, volstruct);
	}

    else if (mode=='C')
	{	
	    VipMarkNHighestCascades( clist, ncascade, volstruct);
	
	}
  
    if(mode!='f')
	{
	    if(gnuplot!='n')
		{
		    if(VipCreateGnuplotFileFromExtrema(volstruct,stripped_input,SS_CASCADE_EXTREMUM,gnuplotpsfile,
						       gnuplot_title,
						       D0WRITE,D1WRITE,D2WRITE,D3WRITE,D4WRITE)==PB) return(VIP_CL_ERROR);
		}
	}
    else
	{
	    if(gnuplot!='n')
		{
		    if(VipCreateGnuplotFileFromExtrema(volstruct,stripped_input,SS_TRACKED_EXTREMUM,gnuplotpsfile,
						       gnuplot_title,
						       D0WRITE,D1WRITE,D2WRITE,D3WRITE,D4WRITE)==PB) return(VIP_CL_ERROR);
		}
	}

    VipFree1DScaleSpaceStruct(volstruct);

    /*   if(slist!=NULL) VipFreeSSSingularityList(slist); 
         Probleme with histo plantage.his???*/
    if(clist!=NULL) VipFreeCascadeList(clist);
  
    fflush(stdout);
    if(gnuplot=='s')
	{
	    sprintf(systemcommand, "gnuplot %s%c%s.gp", VipTmpDirectory(), 
                    VipFileSeparator(), stripped_input );
	    if(system(systemcommand))
		VipPrintfError("Can not use gnuplot here (or use \"return\" to quit gnuplot), sorry...\n");
	}
#if 0
    /* denis 04/11/2003, the actual system() was already commented... */
    if(gnuplot!='n')
      {
        sprintf( systemcommand, "%s%cgpdir_%s/*", VipTmpDirectory(), 
                 VipFileSeparator(), input );
        VipRm( systemcommand, VipRecursive );
      }
#endif
    
    if(ridgename && ana)
      {
        vol = VipReadVolume(input);
        ridge = VipReadVolume(ridgename);
        vol = VipComputeCrestAverageIntensity(ridge, vol);
        printf("Refining ridges [%d,%d]\n",mVipMin(ana->gray->mean +1*ana->gray->sigma,ana->white->mean -1*ana->white->sigma), ana->white->mean +3*ana->white->sigma);
        VipDoubleThreshold(vol,VIP_BETWEEN_OR_EQUAL_TO,mVipMin(ana->gray->mean +1*ana->gray->sigma,ana->white->mean -1*ana->white->sigma) ,ana->white->mean +2*ana->white->sigma,GREYLEVEL_RESULT);
        if (VipConnexVolumeFilter (vol, CONNECTIVITY_26, -1, CONNEX_BINARY)==PB) return(VIP_CL_ERROR);
        VipWriteTivoliVolume(vol,ridgename);
        
      }
    
    return(0);

  
}


/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipHistoAnalysis\n");
  (void)fprintf(stderr,"        -i[nput] {image name/histo name with extension (image.his)}\n");
  (void)fprintf(stderr,"        [-o[utput] {histo analysis name (default: input))}]\n");
  (void)fprintf(stderr,"        [-S[AVE] {y/n (default:n)}]\n");   
  (void)fprintf(stderr,"        [-R[idge] {White ridge image name (default: not used)}]\n");
  (void)fprintf(stderr,"        [-M[ask] {mask to compute histogram (default: not used)}]\n");
  (void)fprintf(stderr,"        [-m[ode] {char: [entropy], v[isu], f[ree], c[ascade], C[ascade], a[nalyse], m[axima], s[surface], h[surface], default:v}]\n");
  (void)fprintf(stderr,"        [-C[ontrast] {int (default:auto, else h, l, H, s, 2, 3, 4)}]\n");
  (void)fprintf(stderr,"        [-e[dges] {char (default:not used, 2/3)}]\n");
  (void)fprintf(stderr,"        [-vt[ariance] {int (default:not used, else int threshold)}]\n");
  (void)fprintf(stderr,"        [-vp[ourcentage] {int (default:not used, else int threshold)}]\n");
  (void)fprintf(stderr,"        [-u[ndersampling_factor] {int (default:auto)}]\n"); 
  (void)fprintf(stderr,"        [-x[max] {int (default:not used)}]\n"); 
  (void)fprintf(stderr,"        [-H[max] {int (default:50000)}]\n"); 
  (void)fprintf(stderr,"        [-d[scale] {float: (default:0.5)}]\n"); 
  (void)fprintf(stderr,"        [-t[rack] {int (default:5)}]\n");
  (void)fprintf(stderr,"        [-c[ascade] {int (default:10)}]\n");
  (void)fprintf(stderr,"        [-O[ffset] {int (default:0)}]\n"); 
  (void)fprintf(stderr,"        [-s[calemax] {int (default:1000)}]\n");   
  (void)fprintf(stderr,"        [-0[extrema] {y/n (default:n)}]\n");
  (void)fprintf(stderr,"        [-1[extrema] {y/n (default:y)}]\n");  
  (void)fprintf(stderr,"        [-2[extrema] {y/n (default:y)}]\n");  
  (void)fprintf(stderr,"        [-3[extrema] {y/n (default:n)}]\n");  
  (void)fprintf(stderr,"        [-4[extrema] {y/n (default:n)}]\n");    
  (void)fprintf(stderr,"        [-g[nuplot] {char: n[o], s[creen], f[ile], p[ostscript], default:n}]\n"); 
  (void)fprintf(stderr,"        [-T[itle] {char y/n (default:y)}]\n"); 
  (void)fprintf(stderr,"        [-r[eadformat] {char: v, s, t, or a (default:a)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Performs T1-histogram scale space analysis");
  (void)printf("\n");
  (void)printf("Usage: VipHistoAnalysis\n");
  (void)printf("        -i[nput] {image name/histo name with extension (image.his)}\n");
  (void)printf("NB: if the histogram already exists (file name.his or name.his_bin)\n"); 
  (void)printf("it will not be computed again (the volume is not read)"); 
  (void)fprintf(stderr,"        [-o[utput] {histo analysis name (default input))}]\n");
  (void)printf("analysis will get the han extension\n"); 
  (void)printf("        [-S[AVE] {y/n (default:n)}]\n");
  (void)printf("        [-R[idge] {White ridge image name (default: not used)}]\n");
  (void)printf("        [-M[ask] {mask to compute histogram (default: not used)}]\n");
  (void)printf("Save histogram analysis if performed (Extension: output.han)\n");
  (void)printf("        [-m[ode] {char: e[ntropy], v[isu], f[ree], c[ascade], C[ascade], a[nalyse], m[axima], default:v}\n");
  (void)printf("e: computes some kind of entropy...\n");
  (void)printf("m: give n maxima the most significative...\n");
  (void)printf("v: visu mode send histogram to gnuplot\n");
  (void)printf("c: cascade mode detects D1/D2 n largest cascades\n");
  (void)printf("C: cascade mode detects D1/D2 n highest cascades\n");
  (void)printf("a: analyse mode according to contrast\n");
  (void)printf("s: surface\n"); 
  (void)printf("        [-e[ges] {int (default:not used, else 2/3)}]\n");
  (void)printf("remove 2D/3D edges for histogram estimation (discard partial volume)\n"); 
  (void)printf("        [-vt[ariance] {int (default:not used, else int threshold)}]\n");
  (void)printf("high threshold on standard deviation in 26-neighborhood for inclusion in histogram\n"); 
  (void)printf("Values beyond this threshold are not taken into account.\n");
  (void)printf("        [-vp[ourcentage] {int (default:not used, else int threshold)}]\n");
  (void)printf("Pourcentage of non null points kept with a ranking stemming from local variance\n");
  (void)printf("        [-u[ndersampling_factor] {int (default:auto)}]\n"); 
  (void)printf("image values are compressed through a division\n");
  (void)printf("by undersampling_factor before scale space computation\n"); 
  (void)printf("        [-C[ontrast] {int (default:auto, else h, l or s)}]\n"); 
  (void)printf("h = high, l=low, s= segmented brain, H=Histology, trigger different heuristics for analysis\n"); 
  (void)printf("2 = 2 modes, 3 = 3 modes, 4 = 4 modes \n"); 
  (void)printf("        [-x[max] {int (default:not used)}]\n"); 
  (void)printf("        [-H[max] {int (default:50000)}]\n"); 
  (void)printf("maxima of axes in the visu mode\n"); 
  (void)printf("        [-d[scale] {float: (default:0.5)}]\n");
  (void)printf("Scale discretization (0.5 is the maximum step for stability)\n"); 
  (void)printf("        [-t[rack] {int (default:5)}]\n"); 
  (void)printf("Track singularities reaching at least this scale\n");
  (void)printf("        [-O[ffset] {int (default:0)}]\n"); 
  (void)printf("Add an interval of length on the histogram left\n");  
  (void)printf("        [-s[calemax] {int (default:1000)}]\n");   
  (void)printf("        [-c[ascade] {int (default:10)}]\n"); 
  (void)printf("        [-0[extrema] {y/n (default:n)}]\n");
  (void)printf("        [-1[extrema] {y/n (default:y)}]\n");  
  (void)printf("        [-2[extrema] {y/n (default:y)}]\n");  
  (void)printf("        [-3[extrema] {y/n (default:n)}]\n");  
  (void)printf("        [-4[extrema] {y/n (default:n)}]\n");    
  (void)printf("        [-g[nuplot] {char: n[o], s[creen], f[ile], p[ostscript], default:n}]\n");
  (void)printf("allows you to look at scalespace and histogram using gnuplot\n");
  (void)printf("        [-T[itle] {char y/n (default:y)}]\n"); 
  (void)printf("put title in gnuplot drawings\n"); 
  (void)printf("        [-r[eadformat] {char: v, s, t or a (default:a)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, TIVOLI(GIS) or ANY image file format\n");
  printf("More information in:\n");
  printf("Robust brain segmentation using histogram\n");
  printf("scale-space analysis and mathematical morphology\n");
  printf("J.-F. Mangin, O. Coulon, and V. Frouin\n");
  printf("MICCAI, MIT, LNCS-1496, Springer Verlag\n");
  printf("1230-1241, 1998\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
/******************************************************/
static int  VipComputeStatInRidgeVolume(Volume *vol, Volume *thresholdedvol, float *mean, float *sigma, int robust)
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
/******************************************************/

Volume *VipComputeCrestAverageIntensity(Volume *crest, Volume *vol)
{
  VipConnectivityStruct *vcs;
  Vip_S16BIT /* *gradptr, */ *smoothptr;
  Vip_S16BIT *volptr, *volvoisin;
  Vip_S16BIT *crestptr, *crestvoisin;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  int icon;
  Volume /* *grad = NULL, */ *smooth=NULL;
  float lemin, lemax;
  int sum,n;

  smooth = VipDuplicateVolumeStructure (crest,"smooth");
  if(!smooth)  return(PB);
  VipAllocateVolumeData(smooth);
  VipSetVolumeLevel(smooth,0);

  vcs = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );
  if(vcs==PB) return(PB);

  vos = VipGetOffsetStructure(vol);

  /*smooth with kind of mean (actual median would be better)*/

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
