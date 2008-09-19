/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipDistanceMap.c     * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 13/10/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
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

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol); 
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "distmap";
  int xmasksize = 3;
  int ymasksize = 3;
  int zmasksize = 3;
  float multfactor = 50. ;
  char stringconnectivity[256] = "6";
  int connectivity=0;
  int connectivity_activ = VFALSE;
  int outside = -1;
  int domain = 0;
  int geodesic_activ = VFALSE;
  int niter = 5;
  char geodesic_mode = 'u';
  char lmodechar = 'n';
  int lmode = VIP_NO_LIMIT_IN_PROPAGATION;
  int limit = 0;
  int limit_activ = VFALSE;
  /* temporary stuff */
  int readlib, writelib;
 
  int i;

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
      else if (!strncmp (argv[i], "-connectivity", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  {
	    strcpy(stringconnectivity,argv[i]);
	    connectivity_activ = VTRUE;
	  }
	} 
      else if (!strncmp (argv[i], "-geodesic", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  {
	    geodesic_mode = argv[i][0];
	    geodesic_activ = VTRUE;
	  }
	} 
      else if (!strncmp (argv[i], "-domain", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  domain = atoi(argv[i]);
	  geodesic_activ = VTRUE;

	}
      else if (!strncmp (argv[i], "-forbiden", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  outside = atoi(argv[i]);
	  geodesic_activ = VTRUE;

	}
      else if (!strncmp (argv[i], "-lmode", 3)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  lmodechar = argv[i][0];
	}
      else if (!strncmp (argv[i], "-limit", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  limit = atoi(argv[i]);
	  limit_activ = VTRUE;
	}
      else if (!strncmp (argv[i], "-niter", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  niter = atoi(argv[i]);
	  geodesic_activ = VTRUE;

	}
      else if (!strncmp (argv[i], "-xmasksize", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  xmasksize = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-ymasksize", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  ymasksize = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-zmasksize", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  zmasksize = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-multfactor", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  multfactor = atof(argv[i]);
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
	      VipPrintfExit("(commandline)VipDistanceMap");
	      return(VIP_CL_ERROR);
	    }
	}
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') writelib = TIVOLI;
	  else if(argv[i][0]=='v') writelib = VIDA;
	  else if(argv[i][0]=='s') writelib = SPM;
	  else
	    {
	      VipPrintfError("This format is not implemented for writing");
	      VipPrintfExit("(commandline)VipDistanceMap");
	      return(VIP_CL_ERROR);
	    }
	}    
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      VipPrintfError("input arg is required by VipDistanceMap");
      return(Usage());
    }
  if (VipTestImageFileExist(input)==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
    }
  
  if(connectivity_activ==VTRUE)
    {
      connectivity = VipConvertStringToConnectivity(stringconnectivity);
      if(connectivity==PB)
	{
	  VipPrintfError("This connectivity is unknown...");
	  return(Usage());
	}
    }
  
  if((limit_activ==VTRUE)&&(geodesic_activ==VTRUE)&&(geodesic_mode=='i'))
     {
       VipPrintfWarning("I trigger front propagation to speed up limited distance transform");       
       geodesic_activ = VTRUE;
       geodesic_mode = 'f';
     }
     
  if(limit_activ==VTRUE)
    {
      if((lmodechar!='l')&&(lmodechar!='f')&&(lmodechar!='i')&&(lmodechar!='n'))
	{
	  VipPrintfError("Unknown lmode");
	  return(Usage());
	}
      if(lmodechar=='l') lmode=VIP_PUT_LIMIT_TO_LIMIT;
      else if (lmodechar=='f') lmode = VIP_PUT_LIMIT_TO_OUTSIDE;
      else if (lmodechar=='i') lmode = VIP_PUT_LIMIT_TO_INFINITE;
      else
	{
	  VipPrintfWarning("You have to choose l(imit)mode!\n");
	  return(Usage());
	}
    }
  else lmode = VIP_NO_LIMIT_IN_PROPAGATION;

 
  if(geodesic_activ==VTRUE)
    {
      if(geodesic_mode=='u')
	{
	  VipPrintfError("Sorry, you have to trigger geodesic mode to use these options (domain, forbidden, nitermax)...");
	  return(Usage());
	}
      else if(geodesic_mode=='f')
	{
	  }
      else if (geodesic_mode=='i')
	{	 
	}
      else
	{
	  VipPrintfError("Unknown geodesic mode...");
	  return(Usage());
	}
    }

  if (readlib == TIVOLI)
    vol = VipReadTivoliVolumeWithBorder(input,1);
  else if (readlib == SPM)
    vol = VipReadSPMVolumeWithBorder(input,1);
  else
    vol = VipReadVolumeWithBorder(input,1);

  if(vol==NULL) return(VIP_CL_ERROR);

  if(connectivity_activ==VFALSE)
    {
      if(geodesic_activ==VFALSE)
	{
	  if(limit_activ==VFALSE)
	  {
	    if (VipComputeCustomizedChamferDistanceMap (vol, xmasksize, ymasksize, zmasksize, multfactor)==PB) return(VIP_CL_ERROR);
	  }
	  else
	    {
	      if (VipComputeCustomizedFrontPropagationChamferDistanceMap (vol, domain, outside,
									  lmode, limit, xmasksize,									ymasksize, zmasksize, multfactor)==PB) return(VIP_CL_ERROR);
	    }
	}
      else
	{
	  if(geodesic_mode=='i')
	    {
	      if (VipComputeCustomizedGeodesicChamferDistanceMap (vol, domain, outside,
								  niter, xmasksize, ymasksize,
								  zmasksize, multfactor)==PB) return(VIP_CL_ERROR);
	    }
	  else
	    {
	      if (VipComputeCustomizedFrontPropagationChamferDistanceMap (vol, domain, outside,
									lmode, limit, xmasksize,
									ymasksize, zmasksize, multfactor)==PB) return(VIP_CL_ERROR);
	    }
	}
  
    }
  else
    {
      if(geodesic_activ==VFALSE)
	{
	  if(limit_activ==VFALSE)
	    {
	      if (VipComputeConnectivityChamferDistanceMap (vol,connectivity)==PB) return(VIP_CL_ERROR);
	    }
	  else
	    {
	      if (VipComputeFrontPropagationConnectivityDistanceMap (vol, domain, outside,
									lmode, limit, connectivity)==PB) return(VIP_CL_ERROR);
	    }
	}
      else
	{
	  if(geodesic_mode=='i')
	    {
	      if (VipComputeConnectivityGeodesicChamferDistanceMap (vol, domain, outside, niter, connectivity)==PB) return(VIP_CL_ERROR);
	    }
	  else
	    {
	      if (VipComputeFrontPropagationConnectivityDistanceMap (vol, domain, outside,
									lmode, limit, connectivity)==PB) return(VIP_CL_ERROR);
	    }
	}
    }
  if (writelib == TIVOLI)
    {
      if(VipWriteTivoliVolume(vol,output)==PB) return(VIP_CL_ERROR);
    }
  else if (writelib == SPM)
    {
      if(VipWriteSPMVolume(vol,output)==PB) return(VIP_CL_ERROR);
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
  (void)fprintf(stderr,"Usage: VipDistanceMap\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"distmap\")}]\n");
  (void)fprintf(stderr,"        [-c[onnectivity] {string:6/18/26/4/8/4s/8s/4c/8c (default:not used)}]\n");
  (void)fprintf(stderr,"        [-g[eodesic] {string:'i' or 'f' (default:not used)}]\n");
  (void)fprintf(stderr,"        [-d[omain] {int (default:0)}]\n");
  (void)fprintf(stderr,"        [-f[orbiden] {int (default:-1)}]\n");
  (void)fprintf(stderr,"        [-l[imit] {int (default:not used)}]\n");
  (void)fprintf(stderr,"        [-lm[ode] {int ('n,'l','f','i') (default:n)}]\n");
  (void)fprintf(stderr,"        [-n[itermax] {int (default:5)}]\n");
  (void)fprintf(stderr,"        [-x[masksize] {int (default:3)}]\n");
  (void)fprintf(stderr,"        [-y[masksize] {int (default:3)}]\n");
  (void)fprintf(stderr,"        [-z[masksize] {int (default:3)}]\n");
  (void)fprintf(stderr,"        [-m[ultfactor] {float (default:50)}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: a, v, s or t (default:any)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Computes a chamfer distance to the object defined as all non zero points");
  (void)printf("Distances either correspond to Euclidean distance multiplied by multfactor.\n");
 (void)printf("or correspond to the distance related to the specified connectivity.\n");
  (void)printf("\n");
  (void)printf("Usage: VipDistanceMap\n");
  (void)printf("        -i[nput] {image name} : object definition\n");
  (void)printf("        [-o[utput] {image name (default:\"distmap\")}]\n");
  (void)printf("        [-c[onnectivity] {string:6/18/26/4/8/4s/8s/4c/8c (default:not used)}]\n");
  (void)printf("        [-g[eodesic] {string:'i' or 'f' (default:not used)}]\n");
  (void)printf("geodesic distance map: 'i':iterated chamfer transform\n");
  (void)printf("                       'f':thick front propagation\n");
  (void)printf("        [-d[omain] {int (default:0)}]\n");
  (void)printf("this value defines propagation domain for the geodesic distance\n");
  (void)printf("        [-f[orbiden] {int (default:-1)}]\n");
  (void)printf("this value defines outside of domain for the geodesic distance\n");
  (void)printf("        [-l[imit] {int (default:not used)}]\n");
  (void)printf("For front propagation, the propagation is stoped beyond limit (mm or bonds)\n");
  (void)printf("        [-lm[ode] {int ('n,'l','f','i') (default:n)}]\n");
  (void)printf("according to this mode, the value given to the domain beyond limit is\n");
  (void)printf("l:limit*MULT_FACTOR, f: forbidden value, i: infinite (large) value\n");
  (void)printf("        [-n[itermax] {int (default:5)}]\n");
  (void)printf("maximum number of iterations for iterated chamfer transform\n");
  (void)printf("Suffixes \"s\" and \"c\" means sagittal and coronal orientations\n");
  (void)printf("        [-x[masksize] {int (default:3)}: chamfer mask xsize]\n");
  (void)printf("        [-y[masksize] {int (default:3)}: chamfer mask ysize]\n");
  (void)printf("        [-z[masksize] {int (default:3)}: chamfer mask zsize]\n");
  (void)printf("        [-m[ultfactor] {float (default:50)}]\n");
  (void)printf("        [-r[eadformat] {char: a, v, s or t (default:any)}]\n");
  (void)printf("Forces the reading of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default:t)}]\n");
  (void)printf("Forces the writing of VIDA, SPM, or TIVOLI image file format\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
