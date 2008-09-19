/***************************************************************************
 * PROJECT     : 
 * MODULE NAME : VipLabelOverlap.c   * TYPE     : Function
 * AUTHOR      : POUPON F.           * CREATION : 10/02/2000
 * VERSION     : 0.1                 * REVISION :
 * LANGUAGE    : C++                 * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ***************************************************************************
 *
 * DESCRIPTION : 
 *
 ***************************************************************************
 *
 * USED MODULES : 
 *
 ***************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|--------------------------------------
 *              /  /  |              |
 ***************************************************************************/

#include <vip/volume.h>
#include <vip/alloc.h>

#include <stdlib.h>
#include <string.h>

static void Usage()
{ 
  (void)fprintf(stderr,"Usage: VipLabelOverlap\n");
  (void)fprintf(stderr,"\t\t-r[eference] {reference image name}\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t[-rl[abel] {reference label (default:use all label found)}]\n");
  (void)fprintf(stderr,"\t\t[-il[abel] {input image label (default:use same label as reference)}]\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

static void Help()
{ 
  VipPrintfInfo("Overlap measurement between structures defined by a label");
  (void)printf("\n");
  (void)printf("Usage: VipLabelOverlap\n");
  (void)printf("\t\t-r[eference] {reference image name}\n");
  (void)printf("\t\t-i[nput] {image name}\n");
  (void)printf("\t\t[-l[abel] {reference label (default:use all label found)}]\n");
  (void)printf("\t\t[-il[abel] {input image label (default:use same label as reference)}]\n");
  (void)printf("\t\t[-h[elp]]\n");
  exit(-1);
}

int main(int argc, char *argv[])
{
  VIP_DEC_VOLUME(vol); 
  VIP_DEC_VOLUME(ref);
  char *input=NULL;
  char *refer=NULL;
  int i, lab=-1, labb=-1, multi=0;
  int x, y, z, dx, dy, dz;
  float sx, sy, sz, ct;
  long ofp, olbs, opbl;
  Vip_S16BIT *imptr, *vptr;
  int lablst[256], nbLab=0, lab2[256], nbLab2=0, isok=0, k;
  float v1[256], v2[256], vi[256], v1t=0.0, v2t=0.0, vit=0.0;
  float rec1, rec2, vv1, vv2, vvi;

  /***** gestion des arguments *****/
  for (i=1; i<argc; i++)
    { 
      if (!strncmp(argv[i], "-input", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          input = argv[i];
        }
      else if (!strncmp(argv[i], "-reference", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          refer = argv[i];
        }
      else if (!strncmp(argv[i], "-rlabel", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          lab = atoi( argv[i] );
        }
      else if (!strncmp(argv[i], "-ilabel", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          labb = atoi( argv[i] );
        }
      else if (!strncmp(argv[i], "-help", 2))  Help();
      else Usage();
    }

  if ( input == NULL )
    { 
      VipPrintfError("input arg is required by VipLabelOverlap");
      Usage();
    }

  if ( refer == NULL )
    { 
      VipPrintfError("reference arg is required by VipLabelOverlap");
      Usage();
    }

  /***** Lecture des images *****/
  printf("Reading reference : %s ...\n", refer);
  ref = VipReadVolume( refer );

  printf("Reading image : %s ...\n", input);
  vol = VipReadVolume( input );

  /***** Verifications *****/
  ofp = VipOffsetFirstPoint( ref );
  olbs = VipOffsetLineBetweenSlice( ref );
  opbl = VipOffsetPointBetweenLine( ref );
  VipGet3DSize( ref, &dx, &dy, &dz );
  VipGet3DVoxelSize( ref, &sx, &sy, &sz );
  ct = sx * sy * sz;

  /***** Recherche des etiquettes de l'image de reference *****/
  printf( "Looking for labels in reference image...\n" );
  imptr = VipGetDataPtr_S16BIT( ref ) + ofp;
  for ( z=dz; z--; imptr+=olbs )
    for ( y=dy; y--; imptr+=opbl )
      for ( x=dx; x--; imptr++ )
        if ( *imptr )
          {
            k = 0;
            while( k<nbLab && lablst[k]!=(int)*imptr )  k++;
            if ( k == nbLab )  lablst[nbLab++] = *imptr;
          }
 
  printf( "Labels found (reference image) :\n\t" );
  for ( k=0; k<nbLab; k++ )
    printf( "%d, ", lablst[k] );
  printf("\n");
  
  /***** Recherche des etiquettes de l'image a traiter *****/
  vptr = VipGetDataPtr_S16BIT( vol ) + ofp;
  printf( "Looking for labels...\n" );
  for ( z=dz; z--; vptr+=olbs )
    for ( y=dy; y--; vptr+=opbl )
      for ( x=dx; x--; vptr++ )
        if ( *vptr )
          {
            k = 0;
            while( k<nbLab2 && lab2[k]!=(int)*vptr )  k++;
            if ( k == nbLab2 )  lab2[nbLab2++] = *vptr;
          }
 
  printf( "Labels found :\n\t" );
  for ( k=0; k<nbLab2; k++ )
    printf( "%d, ", lab2[k] );
  printf("\n");                                                                                      
  /***** Verifie si toutes les etiquettes sont presentes *****/
  if ( lab == -1 && labb == -1 )
    {
      for ( x=0; x<nbLab; x++ )
        {
          k = 0;
          while( k<nbLab2 && lab2[k]!=lablst[x] )  k++;
          if ( k != nbLab2 ) isok++;
        }
 
      if ( isok != nbLab )
        {
          VipPrintfError("Some label are missing in the image to process.");
          exit(EXIT_FAILURE);
        }

      for ( x=0; x<nbLab; x++ )  lab2[x] = lablst[x];
      multi = 1;
    }
  else
    {
      if ( lab == -1 )  lab = labb;
      else if ( labb == -1 )  labb = lab;

      k = 0;
      while( k<nbLab && lablst[k]!=lab )  k++;
      if ( k != nbLab )  isok++;
 
      k = 0;
      while( k<nbLab2 && lab2[k]!=labb )  k++;
      if ( k != nbLab2 )  isok++;
 
      if ( isok != 2 )
        {
          VipPrintfError("Label not found in both images..." );
          exit(EXIT_FAILURE);
        }
 
      nbLab = 1;
      lablst[0] = lab;
      lab2[0] = labb;
      multi = 0;
    }

  printf("Ok, all labels found...\n");

  /***** Calcul du recouvrement *****/
  printf( "Computing overlap...\n" );
  imptr = VipGetDataPtr_S16BIT( ref ) + ofp;
  vptr = VipGetDataPtr_S16BIT( vol ) + ofp;

  for ( k=0; k<256; k++)
    v1[k] = v2[k] = vi[k] = 0.0;

  for ( z=dz; z--; imptr+=olbs, vptr+=olbs )
    for ( y=dy; y--; imptr+=opbl, vptr+=opbl )
      for ( x=dx; x--; imptr++, vptr++ )
	{
	  if ( *imptr != 0 )  v1[(int)*imptr]++;
	  if ( *vptr != 0 )  v2[(int)*vptr]++;
	  if ( ( *imptr != 0 ) && ( ( multi && *imptr == *vptr ) || 
				    ( !multi && *imptr == lab && *vptr == labb ) ) )
	    vi[(int)*imptr]++;
	}

  /***** Affichage des resultats *****/
  for ( k=0; k<nbLab; k++)
    {
      vvi = vi[ lablst[ k ] ];
      vv1 = v1[ lablst[ k ] ];
      vv2 = v2[ lab2[ k ] ];

      rec1 = 100.0 * vvi / vv1;
      rec2 = 100.0 * vvi / vv2;

      v1t += vv1;
      v2t += vv2;
      vit += vvi;

      vvi *= ct;
      vv1 *= ct;
      vv2 *= ct;

      printf( "\nResults for labels %d vs. %d:\n", lablst[ k ], lab2[ k ] );
      printf( "\tVolume of %s : %.2f mm3\n", refer, vv1 );
      printf( "\tVolume of %s : %.2f mm3\n", input, vv2 );
      printf( "\tVolume of overlap : %.2f mm3\n", vvi );
      printf( "\tOverlap %s vs %s : %6.2f%%\n", refer, input, rec1 );
      printf( "\tOverlap %s vs %s : %6.2f%%\n", input, refer, rec2 );
    }

  rec1 = 100.0 * vit / v1t;
  rec2 = 100.0 * vit / v2t;

  printf( "\nGlobal result:\n" );
  printf( "\tTotal overlap %s vs %s : %6.2f%%\n", refer, input, rec1 );
  printf( "\tTotal overlap %s vs %s : %6.2f%%\n", input, refer, rec2 );

  VipFreeVolume(vol);
  VipFree(vol);
  VipFreeVolume(ref);
  VipFree(ref);

  return(EXIT_SUCCESS);
}
