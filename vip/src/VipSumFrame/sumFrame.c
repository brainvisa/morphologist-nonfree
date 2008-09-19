/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipSumFrame.c        * TYPE     : Command line
 * AUTHOR      : Frouin V.            * CREATION : 04/03/99
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Sommationdes frame d'une serie.
 *
 ****************************************************************************
 *
 * USED MODULES : string.h - vip/subvolume.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *--------------------|--------------|---------------------------------------
 *              /  /  |              |
 ****************************************************************************/

#include <vip/volume.h>

#include <string.h>
#include <stdlib.h>

extern Volume *VipSumFrame(Volume *vol, int from, int to);

static void Usage()
{ (void)fprintf(stderr,"Usage: VipSumFrame\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t-s[cale] y/n {default:no}\n");
  (void)fprintf(stderr,"\t\tTriggers perfect use of vida scale factors\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {image name (default:\"summed\")}]\n");
  (void)fprintf(stderr,"\t\t-f[rom] {first frame (default:0)}\n");
  (void)fprintf(stderr,"\t\t-t[o] {last frame (default:last frame of serie)}\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  exit(-1);
}

static void Help()
{
  (void)fprintf(stderr,"Usage: VipSumFrame\n");
  (void)fprintf(stderr,"\t\t-i[nput] {image name}\n");
  (void)fprintf(stderr,"\t\t-s[cale] y/n {default:no}\n");
  (void)fprintf(stderr,"\t\tTrigers perfect use of vida scale factors\n");
  (void)fprintf(stderr,"\t\t[-o[utput] {image name (default:\"summed\")}]\n");
  (void)fprintf(stderr,"\t\t-f[rom] {first frame (default:0)}\n");
  (void)fprintf(stderr,"\t\t-t[o] {last frame (default:last frame of serie)}\n");
  (void)fprintf(stderr,"\t\t[-h[elp]]\n");
  (void)fprintf(stderr,"-----------------------------\n");
  (void)fprintf(stderr,"VipSumFrame performs for each voxel, the sum \n");
  (void)fprintf(stderr,"            of the activity found in volume serie\n");
  (void)fprintf(stderr,"            starting at 'from' ending at 'to'\n");
  (void)fprintf(stderr,"            \n");           
  (void)fprintf(stderr,"            .No Decay correction is performed\n");
  (void)fprintf(stderr,"            \n");
  (void)fprintf(stderr,"            .Relative dynamic of the different\n");
  (void)fprintf(stderr,"            frames is accounted for whether -scale\n");
  (void)fprintf(stderr,"            switch is yes or no.\n");
  (void)fprintf(stderr,"            .When very different dynamics exist\n");
  (void)fprintf(stderr,"            between frames '-scale y' is preferable\n");
  (void)fprintf(stderr,"            for accuracy at a non neglictable \n");
  (void)fprintf(stderr,"            computing cost.\n");
  (void)fprintf(stderr,"            .Only FLOAT and S16BIT data can be processed.\n");
  (void)fprintf(stderr,"-----------------------------\n");
  exit(-1);
}

int main(int argc, char *argv[])
{ 
  VIP_DEC_VOLUME(           vol);
  VIP_DEC_VOLUME(           volout);
  char                      output[VIP_NAME_MAXLEN]="summed";
  int                       scalefactors = VFALSE;
  char                      *input=NULL;
  int                       i;
  int                       from, to;
  /***** gestion des arguments *****/
  from = to = -1;
  for (i=1; i<argc; i++)
  { if (!strncmp(argv[i], "-input", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      input = argv[i];
    }
    else if (!strncmp(argv[i], "-scale", 2)) 
    { if ((++i >= argc) || !strncmp(argv[i],"-",1))  Usage();
      if (argv[i][0] == 'y')  scalefactors = VTRUE;
      else if (argv[i][0] == 'n')  scalefactors = VFALSE;
      else
      { VipPrintfError("float flag asks for yes or no answer");
        VipPrintfExit("(commandline)VipSumFrame");
        Usage();
      }
    }
    else if (!strncmp(argv[i], "-output", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      strcpy(output, argv[i]);
    }
    else if (!strncmp(argv[i], "-from", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      from = atoi(argv[i]);
      if (from < 0)  from = -1;
    }
    else if (!strncmp(argv[i], "-to", 2))
    { if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
      to = atoi(argv[i]);
      if (to < 0)  to = -1;
    }
    else if (!strncmp(argv[i], "-help", 2))  Help();
    else Usage();
  }

  if (input == NULL)
  { VipPrintfError("input arg is required by VipSumFrame");
    Usage();
  }

  /***** supprime les extensions des noms si elles existent  *****/
  if (strstr(input, ".vimg") != NULL)  *strstr(input, ".vimg") = '\0';
  if (strstr(input, ".ima") != NULL)  *strstr(input, ".ima") = '\0';
  if (strstr(output, ".vimg") != NULL)  *strstr(output, ".vimg") = '\0';
  if (strstr(output, ".ima") != NULL)  *strstr(output, ".ima") = '\0';

  if (VipTestImageFileExist(input)==PB)
   {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      exit(EXIT_FAILURE);
   }

  printf("\nReading initial volume : %s ...\n", input);
  if(scalefactors==VTRUE)
    {
      vol = VipReadVidaFloatVolume(input);
      if(!vol)
	{
	  VipPrintfError("Error while reading volume.");
	  VipPrintfExit("(commandline)VipSubVolume.");
	  exit(EXIT_FAILURE);
	}
    }
  else
    {
      if ( (vol = VipReadVolume(input)) == (Volume *)PB)
	{ 
	  VipPrintfError("Error while reading volume.");
	  VipPrintfExit("(commandline)VipSumFrame.");
	  exit(EXIT_FAILURE);
	}
    }

  if (to == -1)    to = mVipVolSizeT(vol)-1;
  if (to >  mVipVolSizeT(vol)-1) 
    {
      to = mVipVolSizeT(vol)-1;
      fprintf(stderr, 
	      "to value > number of Frame. Set to = %d. Continuing...\n", to);
    }
  if (from == -1)  from = 0;



  printf("Summing frames in output vol...\n");
  volout = VipSumFrame(vol, from, to);
  if (volout == NULL)
    {
      VipPrintfError("Unable to sum frames.");
      VipPrintfExit("(commandline)VipSumFrame.");
      VipFreeVolume(vol);
      exit(EXIT_FAILURE);
    }


  printf("Saving sub-volume : %s ...\n", output);



  VipWriteVidaScaledVolume(volout,output);      

  VipFreeVolume(vol);
  VipFreeVolume(volout);
  
  return(0);
}
