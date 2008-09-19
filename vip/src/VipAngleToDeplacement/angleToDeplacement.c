/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipAngleToDeplacement.c      * TYPE     : Command line
 * AUTHOR      : POUPON F.                    * CREATION : 12/04/1999
 * VERSION     : 0.1                          * REVISION :
 * LANGUAGE    : C                            * EXAMPLE  :
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vip/volume.h>
#include <vip/depla.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{     
  /*declarations and initializations*/

  char *output=NULL;
  float tx=0.0, ty=0.0, tz=0.0, rx=0.0, ry=0.0, rz=0.0;
  float gx=0.0, gy=0.0, gz=0.0;
  int i;
  VipDeplacement depla;
  Vip3DPoint_VFLOAT t, tc;
  VipRotation r;

  /*loop on command line arguments*/
  for (i=1; i<argc; i++)
    {	
      if (!strncmp (argv[i], "-output", 2)) 
	{
	  if (++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  output = argv[i];
	}
      else if (!strncmp (argv[i], "-xtrans", 2)) 
	{
	  if (++i >= argc) return(Usage());
	  tx = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-ytrans", 2)) 
	{
	  if (++i >= argc) return(Usage());
	  ty = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-ztrans", 2)) 
	{
	  if (++i >= argc) return(Usage());
	  tz = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-alpha", 2)) 
	{
	  if (++i >= argc) return(Usage());
	  rx = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-beta", 2)) 
	{
	  if (++i >= argc) return(Usage());
	  ry = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-gamma", 2)) 
	{
	  if (++i >= argc) return(Usage());
	  rz = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-Xcenter", 2)) 
	{
	  if (++i >= argc) return(Usage());
	  gx = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-Ycenter", 2)) 
	{
	  if (++i >= argc) return(Usage());
	  gy = atof(argv[i]);
	}
      else if (!strncmp (argv[i], "-Zcenter", 2)) 
	{
	  if (++i >= argc) return(Usage());
	  gz = atof(argv[i]);
	}
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /* check that all required arguments have been given */
  if (output == NULL)
    {
      VipPrintfError("output arg is required by VipAngleToDeplacement");
      return(Usage());
    }

  /* calcul du deplacement */
  t.x = gx;
  t.y = gy;
  t.z = gz;

  VipFillRotation(rx, ry, rz, &r);
  VipApplyRotation(&r, &t, &tc);

  t.x = tx-tc.x+gx;
  t.y = ty-tc.y+gy;
  t.z = tz-tc.z+gz;

  VipFillDeplacement(&t, &r, &depla);

  /* Sauvegarde */
  printf("Writing %s...\n", output);
  VipWriteDeplacement(&depla, output);

  return(EXIT_SUCCESS);
}

/*--------------------------------------------------------------------------*/
static int Usage()
{
  (void)fprintf(stderr,"Usage: VipAngleToDeplacement\n");
  (void)fprintf(stderr,"\t\t-o[utput] {file name}\n");
  (void)fprintf(stderr,"\t\t[-x[trans] {float (default:0.0)}]\n");
  (void)fprintf(stderr,"\t\t[-y[trans] {float (default:0.0)}]\n");
  (void)fprintf(stderr,"\t\t[-z[trans] {float (default:0.0)}]\n");
  (void)fprintf(stderr,"\t\t[-a[lpha] {float (default:0.0)}]\n");
  (void)fprintf(stderr,"\t\t[-b[eta] {float (default:0.0)}]\n");
  (void)fprintf(stderr,"\t\t[-g[amma] {float (default:0.0)}]\n");
  (void)fprintf(stderr,"\t\t[-X[center] {float (default:0.0)}]\n");
  (void)fprintf(stderr,"\t\t[-Y[center] {float (default:0.0)}]\n");
  (void)fprintf(stderr,"\t\t[-Z[center] {float (default:0.0)}]\n");
  (void)fprintf(stderr,"\t\t[-h[elp]\n");
  return(PB);
}

/*****************************************************/
static int Help()
{
  VipPrintfInfo("Creates a deplacement files from translation and rotation angles");
  (void)printf("\n");
  (void)printf("Usage: VipAngleToDeplacement\n");
  (void)printf("\t\t-o[utput] {file name}\n");
  (void)printf("\t\t[-x[trans] X translation in mm {float (default:0.0)}]\n");
  (void)printf("\t\t[-y[trans] Y translation in mm {float (default:0.0)}]\n");
  (void)printf("\t\t[-z[trans] Z translation in mm {float (default:0.0)}]\n");
  (void)printf("\t\t[-a[lpha] rotation around X in degree {float (default:0.0)}]\n");
  (void)printf("\t\t[-b[eta] rotation around Y in degree {float (default:0.0)}]\n");
  (void)printf("\t\t[-g[amma] rotation around Z in degree {float (default:0.0)}]\n");
  (void)printf("\t\t[-X[center] X rotation center position in mm {float (default:0.0)}]\n");
  (void)printf("\t\t[-Y[center] Y rotation center position in mm {float (default:0.0)}]\n");
  (void)printf("\t\t[-Z[center] Z rotation center position in mm {float (default:0.0)}]\n");
  (void)printf("\t\t[-h[elp]\n");
  return(PB);
}
