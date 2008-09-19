/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipGeometry.c         * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 7/11/98
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
#include <vip/geometry.h>
#include <vip/gaussian.h>
#include <vip/histo.h>
#include <vip/connex.h>


/*----------------------------------------------------------------------------*/
static Volume			*VipNormalizedLaplacian
/*----------------------------------------------------------------------------*/
(
 Volume    *vol
 );

Volume *VipComputeGrad(Volume *result, char *input, float thigh);

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol);
  VIP_DEC_VOLUME(result);  
  VIP_DEC_VOLUME(normalized); 
  VIP_DEC_VOLUME(grad);   
  char histo_name[256];
  char *input = NULL;
  char *Masked = NULL;
  char output[VIP_NAME_MAXLEN] = "geometry";
  float sigma = -1.;
  int gaussian = VTRUE;
  int gradient = VFALSE;
  /* temporary stuff */
  int readlib;
  char *mode=NULL;
  int flagmode;
  int Histo2D = VFALSE;
  float Ginf = -10.;
  float Gsup = +10.;
  int Threshold = 1;
 
  int i;

  readlib = VIDA;

  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-input", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  input = argv[i];
	}
      else if (!strncmp (argv[i], "-mode", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  mode = argv[i];
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-Masked", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  Masked = argv[i];
	}
      else if (!strncmp (argv[i], "-Ginf", 3)) 
	{
	  if(++i >= argc ) return(Usage());
	  Ginf = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-Threshold", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  Threshold = atof(argv[i]);
	}
     else if (!strncmp (argv[i], "-Gsup", 3)) 
	{
	  if(++i >= argc ) return(Usage());
	  Gsup = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-sigma", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  sigma = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-gaussian", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') gaussian = VTRUE;
	  else if(argv[i][0]=='n') gaussian = VFALSE;
	  else
	    {
	      VipPrintfError("Please answer y or n to flag -g");
	      return(Usage());
	    }
	}
      else if (!strncmp (argv[i], "-Gradient", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') gradient = VTRUE;
	  else if(argv[i][0]=='n') gradient = VFALSE;
	  else
	    {
	      VipPrintfError("Please answer y or n to flag -g");
	      return(Usage());
	    }
	}
      else if (!strncmp (argv[i], "-Histo2D", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') Histo2D = VTRUE;
	  else if(argv[i][0]=='n') Histo2D = VFALSE;
	  else
	    {
	      VipPrintfError("Please answer y or n to flag -H");
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
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if(mode==NULL || !strcmp(mode,"mc")) flagmode  = MEAN_CURVATURE;
  else if(!strcmp(mode,"gc")) flagmode  = GAUSSIAN_CURVATURE;
  else if(!strcmp(mode,"mlvv")) flagmode  = MEAN_LVV;
  else if(!strcmp(mode,"glvv")) flagmode  = GAUSSIAN_LVV;
  else if(!strcmp(mode,"pc1")) flagmode  = PRINCIPAL_CURVATURE_1;
  else if(!strcmp(mode,"pc2")) flagmode  = PRINCIPAL_CURVATURE_2;
  else if(!strcmp(mode,"la")) flagmode  = LAPLACIAN;
  else if(!strcmp(mode,"nla")) flagmode  = LAPLACIAN;
  else 
      {
	  VipPrintfError("Unknown differential invariant, choose a good mode...");
	  return(Usage());
      }


  if (input==NULL)
      {
	  VipPrintfError("input arg is required by VipGeometry");
	  return(Usage());
      }
  if (VipTestImageFileExist(input)==PB)
      {
	  (void)fprintf(stderr,"Can not open this image: %s\n",input);
	  return(VIP_CL_ERROR);
      }



  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,0);
  else
    vol = VipReadVolumeWithBorder(input,0);

  if(vol==NULL) return(VIP_CL_ERROR);

  if(sigma<=0.) sigma = mVipMax(mVipVolVoxSizeX(vol),mVipVolVoxSizeY(vol));

  if(gaussian==VTRUE)
      {
	 result = VipDeriche3DGaussian( vol, sigma, NEW_FLOAT_VOLUME);
	 VipFreeVolume(vol);
	 vol = result;
      }
 
  if(vol==PB) return(VIP_CL_ERROR);

  result = Vip3DGeometry(vol,flagmode);
  if(result==PB) return(VIP_CL_ERROR);

  if(mode!=NULL && !strcmp(mode,"nla"))
    {
      normalized = VipNormalizedLaplacian(result);
      VipFreeVolume(result);
      result = normalized;
    }

  if(VipWriteTivoliVolume(result,output)==PB) return(VIP_CL_ERROR);

  if(gradient==VTRUE)
    {
      grad = VipComputeGrad(result,input, -0.5);
      if(VipWriteTivoliVolume(grad,"gradient-0.5")==PB) return(VIP_CL_ERROR);
    }

  if (Histo2D == VTRUE)
    {
      if (Masked!=NULL) 
        {
          VipFreeVolume(vol);
          vol = VipReadVolume(Masked);
        }
      else Masked = input;
  
      strcpy(histo_name,mode);
      strcat(histo_name,"_");
      strcat(histo_name,input);
      strcat(histo_name,"_");
      strcat(histo_name,Masked);
      strcat(histo_name,"_histo2D");
      Create2DHisto_S16BIT(vol, histo_name, Threshold, result, 0, 255, 256, Ginf, Gsup, 256);
    }

  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipGeometry\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-m[ode] {mc/gc/mlvv/glvv/pc1/pc2/la  (default:mc)}]\n");
  (void)fprintf(stderr,"        [-s[igma] {float (mm)  (default:largest pixel size)}]\n");
  (void)fprintf(stderr,"        [-g[aussian] {y/n (default:y)}]\n");
  (void)fprintf(stderr,"        [-G[radient] {y/n (default:n)}]\n");
  (void)fprintf(stderr,"        [-H[isto2D] {y/n (default:n)}]\n");
  (void)fprintf(stderr,"        [-M[asked] {image name for Histo2D (default:\"input\")}]\n");
  (void)fprintf(stderr,"        [-T[hreshold] {on masked image for Histo2D (default:\"1\")}]\n");
  (void)fprintf(stderr,"        [-G[inf] {float   (min of geometry for Histo2D (default: -10))}]\n");
  (void)fprintf(stderr,"        [-G[sup] {float   (max of geometry for Histo2D (default: 10))}]\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"geometry\")}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Computes a differential invariant");
  (void)printf("For the Gaussian filtering Sigma is adapted to the voxel size in each direction\n");
  (void)printf("The result is a float Tivoli/GIS format\n");
  (void)printf("\n");
  (void)printf("Usage: VipGeometry\n");
  (void)printf("        -i[nput] {image name}\n");
  (void)printf("        [-m[ode] {mc/gc/mlvv/glvv/pc1/pc2  (default:mc)}]\n");
  (void)printf("mc = isophote mean curvature, gc=isophote Gaussian curvature\n");
  (void)printf("mlvv = isophote mean Lvv, gc=isophote Gaussian Lvv\n");
  (void)printf("mc = isophote principal curvature 1, gc=isophote principal curvature 2 (pc1>pc2)\n");
  (void)printf("la = Laplacian, nla = Normalized Laplacian\n");
  (void)printf("        [-s[igma] {float (mm)  (default:largest pixel size)}]\n");
  (void)printf("Apply a smoothing (scale=sigma*sigma) to compute differentials in scale-space\n");
  (void)printf("        [-g[aussian] {y/n (default:y)}]\n");
  (void)printf("        [-H[isto2D] {y/n (default:n)}]\n");
  (void)printf("        [-G[radient] {y/n (default:n)}]\n");
  (void)printf("        [-M[asked] {image name for Histo2D (default:\"input\")}]\n");
  (void)printf("        [-T[hreshold] {on masked image for Histo2D (default:\"1\")}]\n");
  (void)printf("        [-G[inf] {float   (min of geometry for Histo2D (default: -10))}]\n");
  (void)printf("        [-G[sup] {float   (max of geometry for Histo2D (default: 10))}]\n");
  (void)printf("        [-o[utput] {image name (default:\"geometry\")}]\n");
  (void)printf("        [-r[eadformat] {char: v or t (default:v)}]\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
/*----------------------------------------------------------------------------*/
static Volume			*VipNormalizedLaplacian
/*----------------------------------------------------------------------------*/
(
 Volume    *vol
)
{

  VIP_DEC_VOLUME(newv);
  VIP_DEC_VOLUME(norm);
  long i;
  Vip_FLOAT *ptr;
  Vip_FLOAT *nptr;
  Vip_S16BIT *ptrnew;
  char name[VIP_NAME_MAXLEN];
  long volsize;
  int max;
  float invmax;

  if (!VipVerifyAll(vol))
    {
      VipPrintfExit("VipLaplacianToPositiveS16BIT");
      return(PB);
    }

  (void)strcpy(name,mVipVolName(vol));
  (void)strcat(name,"_converted");
  newv = VipDuplicateVolumeStructure (vol,name);
  VipSetType(newv, S16BIT);
  VipAllocateVolumeData(newv);

  volsize = ( mVipVolSizeX(vol) + 2*mVipVolBorderWidth(vol) )
           *( mVipVolSizeY(vol) + 2*mVipVolBorderWidth(vol) )
           *( mVipVolSizeZ(vol) + 2*mVipVolBorderWidth(vol) )
           *( mVipVolSizeT(vol) );

  ptr = VipGetDataPtr_VFLOAT( vol );
  ptrnew = VipGetDataPtr_S16BIT( newv );

  for(i=volsize;i--;) *ptrnew++ = (Vip_S16BIT)(int)(fabs(*ptr++)+0.5);

  max = VipPourcentageLowerThanThreshold(newv, 1, 98);
  VipFreeVolume(newv);
  printf("Robust maximum of the Laplacian: %d normalized to 100\n", max);
      
  
  (void)strcpy(name,mVipVolName(vol));
  (void)strcat(name,"_normalized");
  norm = VipDuplicateVolumeStructure (vol,name);
  VipAllocateVolumeData(norm);
      
  ptr = VipGetDataPtr_VFLOAT( vol );
  nptr = VipGetDataPtr_VFLOAT( norm );

  if (max<0.000001) invmax = 1.;
  else invmax = 100./(float)max;

  for(i=volsize;i--;) *nptr++ = *ptr++ * invmax;

  return(norm);
}

/*----------------------------------------------------------------------------*/

Volume *VipComputeGrad(Volume *result, char *input, float thigh)
{
  VipConnectivityStruct *vcs;
  Vip_S16BIT *gradptr, *volptr, *volvoisin;
  Vip_FLOAT *crestptr, *crestvoisin;
  VipOffsetStruct *vos;
  int ix, iy, iz;
  int icon;
  Volume *grad = NULL;
  Volume *vol=NULL;
  Volume *crest=NULL;
  int lemin, lemax;

  crest = VipCreateSingleFloatThresholdedVolume( result, LOWER_OR_EQUAL_TO,  thigh, BINARY_RESULT);

  vol = VipReadVolumeWithBorder(input,0);

  grad = VipDuplicateVolumeStructure (vol,"gradient");
  VipAllocateVolumeData(grad);
  VipSetVolumeLevel(grad,0);

  vcs = VipGetConnectivityStruct( vol, CONNECTIVITY_26 );
  if(vcs==PB) return(PB);

  vos = VipGetOffsetStructure(vol);

  crestptr = VipGetDataPtr_VFLOAT( crest ) + vos->oFirstPoint;
  gradptr = VipGetDataPtr_S16BIT( grad ) + vos->oFirstPoint;
  volptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;

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
                  *gradptr = lemax - lemin;
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

  VipFreeVolume(crest);

                
  return(grad);

}

/*----------------------------------------------------------------------------*/
