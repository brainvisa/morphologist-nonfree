/***************************************************************************
 * PROJECT     : 
 * MODULE NAME : VipSetPointValue.c   * TYPE     : Function
 * AUTHOR      : POUPON F.            * CREATION : 06/03/2000
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
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
#include <vip/util.h>
#include <vip/alloc.h>

#include <stdlib.h>
#include <string.h>

static void Usage()
{ 
  (void)fprintf(stderr,"Usage: VipSetPointValue\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t[-x[coord] {X coordinate of point (default:0)}]\n");
  (void)fprintf(stderr,"\t\t[-y[coord] {Y coordinate of point (default:0)}]\n");
  (void)fprintf(stderr,"\t\t[-z[coord] {Z coordinate of point (default:0)}]\n");
  (void)fprintf(stderr,"\t\t[-v[alue] {int: filling value (default:255)}]\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {image name (default:\"filled\")}]\n");
  (void)fprintf(stderr,"\t\t[-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

static void Help()
{ 
  VipPrintfInfo("Set a particular point to a specified value");
  (void)printf("\n");
  (void)printf("Usage: VipSetPointValue\n");
  (void)printf("\t\t-i[nput] {image name}\n");
  (void)printf("\t\t[-x[coord] {X coordinate of point (default:0)}]\n");
  (void)printf("\t\t[-y[coord] {Y coordinate of point (default:0)}]\n");
  (void)printf("\t\t[-z[coord] {Z coordinate of point (default:0)}]\n");
  (void)printf("\t\t[-v[alue] {int: filling value (default:255)}]\n");
  (void)printf("\t\t[-o[utput] {image name (default:\"filled\")}]\n");
  (void)printf("\t\t[-r[eadformat] {char: v, s or t (default:v)}]\n");
  (void)printf("\t\t[-w[riteformat] {char: v, s or t (default:v)}]\n");
  (void)printf("\t\t\tv=VIDA, s=SPM, t=TIVOLI\n");
  (void)printf("\t\t[-h[elp]]\n");
  exit(-1);
}

int main(int argc, char *argv[])
{
  VIP_DEC_VOLUME(vol); 
  char output[VIP_NAME_MAXLEN]="filled\0";
  char *input=NULL;
  int i, readlib=VIDA, writelib=VIDA; 
  int lab=255;
  int x=0, y=0, z=0;
  long ofp, oline, oslice;
  char *imptr;

  /***** gestion des arguments *****/
  for (i=1; i<argc; i++)
    { 
      if (!strncmp(argv[i], "-input", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          input = argv[i];
        }
      else if (!strncmp(argv[i], "-xcoord", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          x = atoi( argv[i] );
        }
      else if (!strncmp(argv[i], "-ycoord", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          y = atoi( argv[i] );
        }
      else if (!strncmp(argv[i], "-zcoord", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          z = atoi( argv[i] );
        }
      else if (!strncmp(argv[i], "-value", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          lab = atoi( argv[i] );
        }
      else if (!strncmp(argv[i], "-output", 2))
        { 
          if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
          strcpy( output, argv[i] );
        }
      else if (!strncmp(argv[i], "-readformat", 2)) 
        { 
          if ((++i >= argc) || !strncmp(argv[i],"-",1))  Usage();
          if (argv[i][0] == 't')  readlib = TIVOLI;
          else if (argv[i][0] == 's')  readlib = SPM;
          else if (argv[i][0] == 'v')  readlib = VIDA;
          else
            { 
              VipPrintfError("This format is not implemented for reading");
              VipPrintfExit("(commandline)VipSetPointValue");
              Usage();
            }
        }
      else if (!strncmp(argv[i], "-writeformat", 2)) 
        { 
          if ((++i >= argc) || !strncmp(argv[i],"-",1))  Usage();
          if (argv[i][0] == 't')  writelib = TIVOLI;
          else if (argv[i][0] == 's')  writelib = SPM;
          else if (argv[i][0] == 'v')  writelib = VIDA;
          else
            { 
              VipPrintfError("This format is not implemented for writing");
              VipPrintfExit("(commandline)VipSetPointValue");
              Usage();
            }
        }
      else if (!strncmp(argv[i], "-help", 2))  Help();
      else Usage();
    }

  if (input == NULL)
    { 
      VipPrintfError("input arg is required by VipSetPointValue");
      Usage();
    }

  /***** Lecture des images *****/
  printf("Reading file : %s ...\n", input);
  switch(readlib)
    {
    case VIDA:
      {
        vol = VipReadVolume(input);
        break;
      }
    case TIVOLI:
      {
        vol = VipReadTivoliVolume(input);
        break;
      }
    case SPM:
      {
        vol = VipReadSPMVolume(input);
        break;
      }
    }

  ofp = VipOffsetFirstPoint( vol );
  oline = VipOffsetLine( vol );
  oslice = VipOffsetSlice( vol );

  printf( "Filling point (%d,%d,%d) to %d...\n", x, y, z, lab );
  switch( mVipVolType( vol ) )
    {
    case U8BIT:
      {
        imptr = (char *) ( VipGetDataPtr_U8BIT( vol ) + ofp );
        *((Vip_U8BIT *)imptr + x + y*oline + z*oslice) = (Vip_U8BIT)lab;
        break;
      }
    case S8BIT:
      {
        imptr = (char *) ( VipGetDataPtr_S8BIT( vol ) + ofp );
        *((Vip_S8BIT *)imptr + x + y*oline + z*oslice) = (Vip_S8BIT)lab;
        break;
      }
    case U16BIT:
      {
        imptr = (char *) ( VipGetDataPtr_U16BIT( vol ) + ofp );
        *((Vip_U16BIT *)imptr + x + y*oline + z*oslice) = (Vip_U16BIT)lab;
        break;
      }
    case S16BIT:
      {
        imptr = (char *) ( VipGetDataPtr_S16BIT( vol ) + ofp );
        *((Vip_S16BIT *)imptr + x + y*oline + z*oslice) = (Vip_S16BIT)lab;
        break;
      }
    case S32BIT:
      {
        imptr = (char *) ( VipGetDataPtr_S32BIT( vol ) + ofp );
        *((Vip_S32BIT *)imptr + x + y*oline + z*oslice) = (Vip_S32BIT)lab;
        break;
      }
    case VFLOAT:
      {
        imptr = (char *) ( VipGetDataPtr_VFLOAT( vol ) + ofp );
        *((Vip_FLOAT *)imptr + x + y*oline + z*oslice) = (Vip_FLOAT)lab;
        break;
      }
    case VDOUBLE:
      {
        imptr = (char *) ( VipGetDataPtr_VDOUBLE( vol ) + ofp );
        *((Vip_DOUBLE *)imptr + x + y*oline + z*oslice) = (Vip_DOUBLE)lab;
        break;
      }
    }

  /***** sauvegarde du resultat *****/
  printf("Writing volume : %s...\n", output);
  switch(writelib)
    {
    case VIDA:
      {
        VipWriteVolume(vol, output);
        break;
      }
    case TIVOLI:
      {
        VipWriteTivoliVolume(vol, output);
        break;
      }
    case SPM:
      {
        VipWriteSPMVolume(vol, output);
        break;
      }
    }

  VipFreeVolume(vol);
  VipFree(vol);

  return(EXIT_SUCCESS);
}
