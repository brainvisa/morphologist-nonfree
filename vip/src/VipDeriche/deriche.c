/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipDeriche.c         * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 13/10/1996
 * VERSION     : 0.1                  * REVISION :
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
#include <vip/deriche.h>
#include <vip/bucket.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "deriche";
  float alpha = 1.;
  int threshold = 0;
  int tflag=VFALSE;
  char filter='e';
  int saveg2d = VFALSE;
  char dim='3';
  VipG2DBucket *g2dbuck;
  VipG3DBucket *g3dbuck;
  /* temporary stuff */
  int readlib, writelib;
 
  int i;

  readlib = TIVOLI;
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
      else if (!strncmp (argv[i], "-alpha", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  alpha = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-threshold", 2)) 
	{
	  threshold = atoi(argv[++i]);
	  tflag=VTRUE;
	}
      else if (!strncmp (argv[i], "-filter", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  filter = argv[i][0];
	}
      else if (!strncmp (argv[i], "-dim", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  dim = argv[i][0];
	}
      else if (!strncmp (argv[i], "-save", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') saveg2d = VTRUE;
	  else if(argv[i][0]=='n') saveg2d = VFALSE;
	  else
	    {
	      VipPrintfError("Please answer y or n to flag -s");
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
	      VipPrintfExit("(commandline)VipDeriche");
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
	      VipPrintfExit("(commandline)VipDeriche");
	      return(VIP_CL_ERROR);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipDeriche");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
    }


  if((dim!='2')&&(dim!='3'))
    {
      VipPrintfWarning("Deriche filters are implemented only in 2D and 3D yet...");
      return(Usage());
    }

  if((saveg2d==VTRUE)&&(filter!='e'))
    {
      VipPrintfWarning("Save mode (*.g2D or *.g3D file) is dedicated to extrema filter (e)\n");
      return(Usage());
    }

  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,0);
  else
    vol = VipReadVolumeWithBorder(input,0);

  if(vol==NULL) return(VIP_CL_ERROR);

  if(dim=='2')
    {
      switch(filter)
	{
	case 'e':
	  if (saveg2d==VFALSE)
	    vol = VipDeriche2DGradientNorm(vol, alpha, SAME_VOLUME, DERICHE_EXTREMA, (float)threshold);
	  else
	    {
	      vol = VipDeriche2DGradientNormAndReturnG2DBucket(vol, alpha, SAME_VOLUME, DERICHE_EXTREMA_DIRECTION, (float)threshold, &g2dbuck);
	      if(vol==PB) return(VIP_CL_ERROR);
	      VipSaveG2DBucketList(g2dbuck,output);
	    }
	  break;
	case 'n':
	  vol = VipDeriche2DGradientNorm(vol, alpha, SAME_VOLUME, DERICHE_NORM, (float)threshold);
	  if(vol==PB) return(VIP_CL_ERROR);
	  if(tflag==VTRUE)
	    if(VipSingleThreshold(vol,GREATER_THAN,threshold,GREYLEVEL_RESULT)==PB) return(VIP_CL_ERROR);
	  break;
	case 'x':
	  vol = VipDeriche2DGradientX(vol, alpha, SAME_VOLUME);
	  if(vol==PB) return(VIP_CL_ERROR);
	  if(tflag==VTRUE)
	    if(VipSingleThreshold(vol,GREATER_THAN,threshold,GREYLEVEL_RESULT)==PB) return(VIP_CL_ERROR);
	  break;
	case 'y':
	  vol = VipDeriche2DGradientY(vol, alpha, SAME_VOLUME);
	  if(vol==PB) return(VIP_CL_ERROR);
	  if(tflag==VTRUE)
	    if(VipSingleThreshold(vol,GREATER_THAN,threshold,GREYLEVEL_RESULT)==PB) return(VIP_CL_ERROR);
	  break;
	case 's':
	  vol = VipDeriche2DSmoothing(vol, alpha, SAME_VOLUME);
	  if(vol==PB) return(VIP_CL_ERROR);
	  if(tflag==VTRUE)
	    if(VipSingleThreshold(vol,GREATER_THAN,threshold,GREYLEVEL_RESULT)==PB) return(VIP_CL_ERROR);
	  break;
	case 'z':
          VipPrintfError("Z coordinate in 2D mode?");
          return(Usage());
	default:
	  VipPrintfError("Unknown filter in VipDeriche");
	  return(Usage());
	}
    }
  else
    {
      switch(filter)
	{
	case 'e':
	  if (saveg2d==VTRUE)
	    {
	     if (VipDeriche3DGradientNormAndReturnG3DBucket(vol, alpha, DERICHE_EXTREMA_DIRECTION,
							    (float)threshold, &g3dbuck)
		 ==PB) return(VIP_CL_ERROR);
	     VipSaveG3DBucketList(g3dbuck,output);
	    }
	  else
	    {
	      if(VipDeriche3DGradientNorm(vol, alpha, DERICHE_EXTREMA, (float)threshold)==PB) return(VIP_CL_ERROR);
	    }
	  break;
	case 'n':
	  if(VipDeriche3DGradientNorm(vol, alpha, DERICHE_NORM, (float)threshold)==PB) return(VIP_CL_ERROR);
	  if(tflag==VTRUE)
	    if(VipSingleThreshold(vol,GREATER_THAN,threshold,GREYLEVEL_RESULT)==PB) return(VIP_CL_ERROR);
	  break;
	case 'x':
	  vol = VipDeriche3DGradientX(vol, alpha, SAME_VOLUME);
	  if(vol==PB) return(VIP_CL_ERROR);
	  if(tflag==VTRUE)
	    if(VipSingleThreshold(vol,GREATER_THAN,threshold,GREYLEVEL_RESULT)==PB) return(VIP_CL_ERROR);
	  break;
	case 'y':
	  vol = VipDeriche3DGradientY(vol, alpha, SAME_VOLUME);
	  if(vol==PB) return(VIP_CL_ERROR);
	  if(tflag==VTRUE)
	    if(VipSingleThreshold(vol,GREATER_THAN,threshold,GREYLEVEL_RESULT)==PB) return(VIP_CL_ERROR);
	  break;
	case 'z':
	  vol = VipDeriche3DGradientZ(vol, alpha, SAME_VOLUME);
	  if(vol==PB) return(VIP_CL_ERROR);
	  if(tflag==VTRUE)
	    if(VipSingleThreshold(vol,GREATER_THAN,threshold,GREYLEVEL_RESULT)==PB) return(VIP_CL_ERROR);
	  break;
	case 's':
	  vol = VipDeriche3DSmoothing(vol, alpha, SAME_VOLUME);
	  if(vol==PB) return(VIP_CL_ERROR);
	  if(tflag==VTRUE)
	    if(VipSingleThreshold(vol,GREATER_THAN,threshold,GREYLEVEL_RESULT)==PB) return(VIP_CL_ERROR);
	  break;
	default:
	  VipPrintfError("Unknown filter in VipDeriche");
	  return(Usage());
	}
    }

  if(vol==PB) return(VIP_CL_ERROR);

  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else
    if(VipWriteVolume(vol,output)==PB) return(VIP_CL_ERROR);

  VipFreeVolume(vol);
  VipFree(vol);

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipDeriche\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-a[lpha] {float (default:1.) (range:[0.3,3]}]\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"deriche\")}]\n");
  (void)fprintf(stderr,"        [-f[ilter] {e/n/x/y/z/s (default:e (extrema))}]\n");
  (void)fprintf(stderr,"        [-s[ave] {y/n (default:n)}]\n");
  (void)fprintf(stderr,"        [-t[hreshold] {int (default:0)}]\n");
  (void)fprintf(stderr,"        [-d[im] {2/3 (default:3)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Computes gradient using deriche 2D recursive filters");
  (void)printf("Such filters include optimal smoothing according to Canny criteria.\n");
  (void)printf("\n");
  (void)printf("Usage: VipDeriche\n");
  (void)printf("        -i[nput] {image name}\n");
  (void)printf("        [-a[lpha] {float (default:1.) (range:[0.3,3]}]\n");
  (void)printf("The smoothing importance decreases when alpha increases\n");      
  (void)printf("        [-o[utput] {image name (default:\"deriche\")}]\n");
  (void)printf("        [-s[ave] {y/n (default:n)}]\n");
  (void)printf("save a .g2D file with gradient coordinates for inflation\n");
  (void)printf("during surface matching\n");
  (void)printf("        [-f[ilter] {e/n/x/y/z/s (default:e (extrema))}]\n");
  (void)printf("e: norm extrema in the gradient direction\n");
  (void)printf("n: norm\n");      
  (void)printf("x: x gradient coordinate\n");
  (void)printf("y: y gradient coordinate\n"); 
  (void)printf("z: z gradient coordinate\n");                       
  (void)printf("s: smoothing\n");            
  (void)printf("        [-t[hreshold] {int (default:0)}]\n");
  (void)printf("No extremum detection under threshold value\n");
  (void)printf("Other images are thresholded according to threshold value\n");                        
  (void)printf("        [-d[im] {2/3 (default:3)}]\n");
  (void)printf("        [-r[eadformat] {char: v or t (default:t)}]\n");
  (void)printf("        [-w[riteformat] {char: v or t (default:t)}]\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
