/****************************************************************************
 * PROJECT     : V.I.P. Library
 ****************************************************************************
 *
 * DESCRIPTION :  -
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



/*#############################################################################

	I N C L U S I O N S

#############################################################################*/
#include <stdio.h>
#include <string.h>

#include <vip/volume.h>
#include <vip/talairach.h>
#include <vip/util.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  char *input = NULL;
  char *maskname = NULL;
  char *output = NULL;
  int xAC=-1, yAC=-1, zAC=-1, xPC=-1, yPC=-1, zPC=-1, xIH=-1, yIH=-1, zIH=-1;
  VIP_DEC_VOLUME( mask );
  VipTalairach tal;
  float	coord[3], mat[3][3], trans[3];
  FILE *out;
  int voronoi = 0;

  int i;

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
	  output = argv[i];
	}
      else if (!strncmp (argv[i], "-mask", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  maskname  = argv[i];
	}
      else if (!strncmp (argv[i], "-voronoi", 2)) 
	{
	  voronoi = 1;
	}
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if( input == NULL || output == NULL || maskname == NULL )
    {
      VipPrintfError("input and arg arguments are required by " 
                     "VipTalairachTransform");
      return(Usage());
    }

  
  if (VipTestImageFileExist(maskname)==PB)
    {
      fprintf( stderr, "Can not open this image: %s\n", maskname );
      return(VIP_CL_ERROR);
    }

  printf( "Reading mask %s...\n", maskname );
  mask = VipReadVolumeWithBorder( maskname, 1 );
  if( mask == NULL ) return(VIP_CL_ERROR);
  if( voronoi )
    VipSingleThreshold( mask, LOWER_THAN, 3, GREYLEVEL_RESULT );
  printf( "----------------------------\n" );

  if(GetCommissureCoordinates( mask, input, &tal,
                               xAC, yAC, zAC, 
                               xPC, yPC, zPC, 
                               xIH, yIH, zIH, VFALSE ) == PB )
    {
      return(VIP_CL_ERROR);
    }
  printf( "Processing Talairach transformation...\n" );
  VipComputeTalairachTransformation( mask, &tal );
  VipFreeVolume( mask );

  out = fopen( output, "w" );
  if( out == NULL ) return( VIP_CL_ERROR );

  coord[0] = 0.;
  coord[1] = 0.;
  coord[2] = 0.;
  VipTalairachTransformation( coord, &tal, trans );
  trans[0] *= tal.Scale.x;
  trans[1] *= tal.Scale.y;
  trans[2] *= tal.Scale.z;
  fprintf( out, "%f %f %f\n", trans[0], trans[1], trans[2] );
  coord[0] = 1.;
  coord[1] = 0.;
  coord[2] = 0.;
  VipTalairachTransformation( coord, &tal, mat[0] );
  mat[0][0] *= tal.Scale.x;
  mat[0][1] *= tal.Scale.y;
  mat[0][2] *= tal.Scale.z;
  coord[0] = 0.;
  coord[1] = 1.;
  coord[2] = 0.;
  VipTalairachTransformation( coord, &tal, mat[1] );
  mat[1][0] *= tal.Scale.x;
  mat[1][1] *= tal.Scale.y;
  mat[1][2] *= tal.Scale.z;
  coord[0] = 0.;
  coord[1] = 0.;
  coord[2] = 1.;
  VipTalairachTransformation( coord, &tal, mat[2] );
  mat[2][0] *= tal.Scale.x;
  mat[2][1] *= tal.Scale.y;
  mat[2][2] *= tal.Scale.z;
  fprintf( out, "%f %f %f\n", mat[0][0] - trans[0], mat[1][0] - trans[0], 
           mat[2][0] - trans[0] );
  fprintf( out, "%f %f %f\n", mat[0][1] - trans[1], mat[1][1] - trans[1], 
           mat[2][1] - trans[1] );
  fprintf( out, "%f %f %f\n", mat[0][2] - trans[2], mat[1][2] - trans[2], 
           mat[2][2] - trans[2] );

  fclose( out );

  return EXIT_SUCCESS;
}


/*---------------------------------------------------------------------------*/

static int Usage()
{
  fprintf(stderr,"Usage: VipTalairachTransform\n");
  fprintf(stderr,"        -i[nput] {commissures file name (.APC)}\n");
  fprintf(stderr,"        -o[utput] {transformation file name (.trm)}\n");
  fprintf(stderr,"        -m[ask] {brain mask file name, must includes hemispheres only (no cerebellum) - if a voronoi (with cereb) is provided, use the -v option too}\n");
  fprintf(stderr,"        [-v[oronoi]]\n");
  fprintf(stderr,"        [-h[elp]\n");
  fprintf(stderr,"        in voronoi mode, the mask in thresholded to exclude label 3 (cerebellum) from a standard Vip voronoi image\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Computes the Talairach referential and writes a " 
                "transformation matrix file from a .APC commissures file");
  (void)printf("\n");
  Usage();
  return(VIP_CL_ERROR);

}

/******************************************************/
