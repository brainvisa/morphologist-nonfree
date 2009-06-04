/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipSubVolume.c       * TYPE     : Command line
 * AUTHOR      : POUPON F.            * CREATION : 16/08/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Extraction d'un sous-volume
 *
 ****************************************************************************
 *
 * USED MODULES : string.h - vip/subvolume.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *            04/03/97| POUPON F.    | Modifications pour permettre la
 *                    |              | specification du format d'image
 *                    |              | (VIDA ou TIVOLI)
 *--------------------|--------------|---------------------------------------
 *            18/04/97| SECHER B.    | Extension au 4D
 *                    | ROUGETET R.  |
 *                    | POUPON F.    |
 *--------------------|--------------|---------------------------------------
 *              /  /  |              |
 ****************************************************************************/

#include <vip/volume.h>

#include <string.h>
#include <stdlib.h>

static void Usage()
{ (void)fprintf(stderr,"Usage: VipSubVolume\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {image name (default:\"subvolume\")}]\n");
  (void)fprintf(stderr,"\t\t[-x {first X coordinate}] (default:0)\n");
  (void)fprintf(stderr,"\t\t[-y {first Y coordinate}] (default:0)\n");
  (void)fprintf(stderr,"\t\t[-z {first Z coordinate}] (default:0)\n");
  (void)fprintf(stderr,"\t\t[-t {first T coordinate}] (default:0)\n");
  (void)fprintf(stderr,"\t\t[-X {last X coordinate or sub-volume X-dimension}] (default:SizeX)\n");
  (void)fprintf(stderr,"\t\t[-Y {last Y coordinate or sub-volume Y-dimension}] (default:SizeY)\n");
  (void)fprintf(stderr,"\t\t[-Z {last Z coordinate or sub-volume Z-dimension}] (default:SizeZ)\n");
  (void)fprintf(stderr,"\t\t[-T {last T coordinate or sub-volume T-dimension}] (default:SizeT)\n");
  (void)fprintf(stderr,"\t\t[-l] {take X,Y,Z and T as dimensions instead of coordinates}\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

static void Help()
{ VipPrintfInfo("Extraction of a sub-volume");
  (void)printf("\n");
  (void)fprintf(stderr,"Usage: VipSubVolume\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t-f[loat] y/n {default:no}\n");
  (void)fprintf(stderr,"Trigers perfect use of vida scale factors\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {image name (default:\"subvolume\")}]\n");
  (void)fprintf(stderr,"\t\t[-x {first X coordinate}] (default:0)\n");
  (void)fprintf(stderr,"\t\t[-y {first Y coordinate}] (default:0)\n");
  (void)fprintf(stderr,"\t\t[-z {first Z coordinate}] (default:0)\n");
  (void)fprintf(stderr,"\t\t[-t {first T coordinate}] (default:0)\n");
  (void)fprintf(stderr,"\t\t[-X {last X coordinate or sub-volume X-dimension}] (default:SizeX)\n");
  (void)fprintf(stderr,"\t\t[-Y {last Y coordinate or sub-volume Y-dimension}] (default:SizeY)\n");
  (void)fprintf(stderr,"\t\t[-Z {last Z coordinate or sub-volume Z-dimension}] (default:SizeZ)\n");
  (void)fprintf(stderr,"\t\t[-T {last T coordinate or sub-volume T-dimension}] (default:SizeT)\n");
  (void)fprintf(stderr,"\t\t[-l] {take X,Y,Z and T as dimensions instead of coordinates}\n");
  (void)fprintf(stderr,"\t\t[-r[eadformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t[-w[riteformat] {char: v or t (default:v)}]\n");
  (void)fprintf(stderr,"\t\t\tv=VIDA, t=TIVOLI\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

int main(int argc, char *argv[])
{ VIP_DEC_VOLUME(vol);
  VIP_DEC_VOLUME(volout);
  char output[VIP_NAME_MAXLEN]="subvolume";
  char *input=NULL;
  int i, xd=0, yd=0, zd=0, td=0, dx=-1, dy=-1, dz=-1, dt=-1, isDim=0;  

  /***** gestion des arguments *****/
  for (i=1; i<argc; i++)
  { if (!strncmp(argv[i], "-input", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      input = argv[i];
    }
    else if (!strncmp(argv[i], "-output", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      strcpy(output, argv[i]);
    }
    else if (!strncmp(argv[i], "-x", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      xd = atoi(argv[i]);
      if (xd < 0)  xd = 0;
    }
    else if (!strncmp(argv[i], "-y", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      yd = atoi(argv[i]);
      if (yd < 0)  yd = 0;
    }
    else if (!strncmp(argv[i], "-z", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      zd = atoi(argv[i]);
      if (zd < 0)  zd = 0;
    }
    else if (!strncmp(argv[i], "-t", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      td = atoi(argv[i]);
      if (td < 0)  td = 0;
    }
    else if (!strncmp(argv[i], "-X", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      dx = atoi(argv[i]);
    }
    else if (!strncmp(argv[i], "-Y", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      dy = atoi(argv[i]);
    }
    else if (!strncmp(argv[i], "-Z", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      dz = atoi(argv[i]);
    }
    else if (!strncmp(argv[i], "-T", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      dt = atoi(argv[i]);
    }
    else if (!strncmp(argv[i], "-l", 2))  isDim = 1;
    else if (!strncmp(argv[i], "-help", 2))  Help();
    else Usage();
  }

  if (input == NULL)
  { VipPrintfError("input arg is required by VipSubVolume");
    Usage();
  }

  if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      exit(EXIT_FAILURE);
   }

  printf("\nReading initial volume : %s ...\n", input);
  if ( ( vol = VipReadVolume(input) ) == (Volume *)PB)
    {
      VipPrintfError("Error while reading volume.");
      VipPrintfExit("(commandline)VipSubVolume.");
      exit(EXIT_FAILURE);
    }

  if (dx == -1)  dx = mVipVolSizeX(vol)-1*(!isDim);
  if (dy == -1)  dy = mVipVolSizeY(vol)-1*(!isDim);
  if (dz == -1)  dz = mVipVolSizeZ(vol)-1*(!isDim);
  if (dt == -1)  dt = mVipVolSizeT(vol)-1*(!isDim);

  printf("Getting sub-volume...\n");
  volout = VipGetSubVolume(vol, xd, yd, zd, td, dx, dy, dz, dt, isDim);
  if (volout == NULL)
  {
    VipPrintfError("Unable to get sub-volume.");
    VipPrintfExit("(commandline)VipSubVolume.");
    VipFreeVolume(vol);
    exit(EXIT_FAILURE);
  }

  printf("Saving sub-volume : %s ...\n", output);

  VipWriteVolume(volout, output);

  VipFreeVolume(vol);
  VipFreeVolume(volout);
  
  return(0);
}
