#define _DEFAULT_SOURCE /* for strncasecmp */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <vip/epidistorsion/param.h>
#define NDEF_PARAM 0
#include <vip/epidistorsion/readwrite.h>
#include <vip/epidistorsion/param_io.h>
#include <vip/util.h>

#define ERROR 1
#ifndef OK
#define OK 0
#endif

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/


int main (int argc, char *argv[]) {
/* --------------------------------------
   this functions edits a parameter-file 
   that can be used as an argument for
   fieldmap and epi_correct
   -------------------------------------- */

  char buf[200], cmdline[200];
  
  FILE *fid;
    
  param par;
  param_acq par_a;
  param_mat par_echo, par_epi;
  int n;
  int i;
  
  /* getting the parameter file names */
  Write  ("parameter file :");
  if (argc > 1) 
    {
      if( !strcmp( argv[1], "-h" ) )
        {
          return Help();
        }
      else
        (void) strcpy(buf, argv[1]); 
    }
  else
    ReadString (buf,stdin);
  Write (buf);
  NewLine();
  
  /* getting the default values : update or standard */
  
  if ( ((fid = fopen (buf,"r")) == NULL) 
       ||((ReadParam(fid, &par, &par_a, &par_echo, &par_epi)) == ERROR) )
    {    
    Write ("edit_param : using default model");
    NewLine();
    DefaultParam (&par, &par_a, &par_echo, &par_epi);
  }
  if ((fid != NULL)&&(fclose(fid) != 0)) {
    Write ("edit_param : couldn't close the edited file after reading.");
    NewLine();
    Write ("Exiting");
    NewLine();
    return ERROR;
  }

  if (!strncmp (argv[2], "-batch", 2)){
/*    printf("nb arg=%d\t option=%s\n",argc,argv[2]);
    if (ReadParam (fid, &par, &par_a, &par_echo, &par_epi) == ERROR) {
	    Write (" Incorrect parameter file ; please use edit_param without -filled flag");
	    NewLine();
	    return ERROR;
    } else {
      Write ("parameter file successfully read ; exiting...");
      NewLine();
      return OK;
    }
*/
  /*loop on command line arguments*/
    
    for(i=3;i<argc;i++) {
      if (!strncmp (argv[i], "-dex", 4))  {
	      if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
        par_echo.s_x = (int) strtol(argv[i], (char**)NULL, 10);
	    }
      else if (!strncmp (argv[i], "-dey", 4))  {
	      if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
        par_echo.s_y = (int) strtol(argv[i], (char**)NULL, 10);
	    }
      else if (!strncmp (argv[i], "-dez", 4))  {
	      if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
        par_echo.s_z = (int) strtol(argv[i], (char**)NULL, 10);
        par_epi.s_z = par_echo.s_z;
	    }
      else if (!strncmp (argv[i], "-epix", 5))  {
	      if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
        par_epi.s_x = (int) strtol(argv[i], (char**)NULL, 10);
	    }
      else if (!strncmp (argv[i], "-epiy", 5))  {
	      if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
        par_epi.s_y = (int) strtol(argv[i], (char**)NULL, 10);
	    }
      else if (!strncmp (argv[i], "-epit", 5))  {
	      if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
        par_epi.s_t = (int) strtol(argv[i], (char**)NULL, 10);
	    }
      else if (!strncmp (argv[i], "-revz", 5))  {
	      if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
        if (!strncasecmp(argv[i],"yes",1))  par_epi.reverse_z = -1;
        else if (!strncasecmp(argv[i],"no",1)) par_epi.reverse_z = 1;
        else
	      {
	        (void)fprintf(stderr,"This value is incompatible for parameter epi - reverse z axis ?\n");
	        Usage();
	      }
	    }
      else if (!strncmp (argv[i], "-iet", 4))  {
	      if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
        par_a.inter_echo_time = atof(argv[i]);
	    }
      else if (!strncmp (argv[i], "-dwt", 4))  {
	      if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
        par_a.dwell_time = atof(argv[i]);
	    }
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }
  }
  else {

      /* Modifying the values */
    n =2;

    GetInt (&par_echo.s_x, "double echo image - size x",argc,argv,n); n++;
    GetInt (&par_echo.s_y, "double echo image - size y",argc,argv,n); n++;
    GetInt (&par_echo.s_z, "double echo image - size z",argc,argv,n); n++;
    GetInt (&par_epi.s_x, "epi image - size x",argc,argv,n); n++;
    GetInt (&par_epi.s_y, "epi image - size y",argc,argv,n); n++;
    /*  GetInt (&par_epi.s_z, "epi image - size z",argc,argv,9);  */
    Write ("epi image - size z (same as grad echo image) : ");
    par_epi.s_z = par_echo.s_z; WriteInt (par_epi.s_z); NewLine();
    GetInt (&par_epi.s_t, "epi image - number of repetitions",argc,argv,n); n++;

    /* reversing the axes */
    GetInt (&par_epi.reverse_z, "epi - reverse z axis ? (1/-1)",argc,argv,n); n++;

    NewLine();
    Write ("Acquisition parameters");NewLine();
    NewLine();
    GetDouble (&par_a.inter_echo_time, "inter_echo_time",argc,argv,n); n++;
    GetDouble (&par_a.dwell_time, "dwell_time",argc,argv,n); n++;

    NewLine();
    Write ("Do you want to edit the algorithm parameters ? (y/n)");
    ReadString (cmdline,stdin);
    NewLine();
    /* algorithm parameters */
    if (cmdline[0] == 'y') {
      GetDouble (&par.noise_threshold, "noise threshold",argc,argv,n); n++;
      GetInt (&par.ref_x, "unwrapping starting point - coord x",argc,argv,n); n++;
      GetInt (&par.ref_y, "unwrapping starting point - coord y",argc,argv,n); n++;
      GetInt (&par.unwrap_base_size, "unwrapping base size (should be a square number)",argc,argv,n); n++;
      GetInt (&par.extrapol_base_size, "extrapolating base size(should be a square number)",argc,argv,n); n++;
      GetInt (&par.extrapol_kernel_size, "extrapolating kernel size",argc,argv,n); n++;
      GetInt (&par.erode_radius, "eroding radius",argc,argv,n); n++;
      GetInt (&par.dilate_radius, "dilating radius",argc,argv,n); n++;
      NewLine();
      GetInt (&par.actual_map, "Use the actual phasemap (1=yes / -1=no)",argc,argv,n); n++;
      GetInt (&par.segmentation, "Perform segmentation (1=yes / -1=mask provided)",argc,argv,n); n++;
      GetString (par.fname_mask,"filename for the mask (skip if segmentation performed)",argc,argv,n); n++;
      GetInt (&par.segmentation, "Provided mask : reverse z axis ? (-1=yes / 1=no)",argc,argv,n); n++;
      GetInt (&par_echo.reverse_x, "gradient echo - reverse x axis ? (1/-1)",argc,argv,n); n++;
      GetInt (&par_echo.reverse_y, "gradient echo - reverse y axis ? (1/-1)",argc,argv,n); n++;
      GetInt (&par_echo.reverse_z, "gradient echo - reverse z axis ? (1/-1)",argc,argv,n); n++;
      GetInt (&par_epi.reverse_x, "epi - reverse x axis ? (1/-1)",argc,argv,n); n++;
      GetInt (&par_epi.reverse_y, "epi - reverse y axis ? (1/-1)",argc,argv,n); n++;
    }
  } 
  /* writing the parameter file */
  if ( ((fid = fopen (buf,"w")) == NULL)
       ||((WriteParam(fid, &par, &par_a, &par_echo, &par_epi)) == ERROR)
       ||(fclose(fid) != 0) ) {
    Write ("edit_param : an error occured while writing the data");
    NewLine();
    Write ("Check the file manually");
    return ERROR;
  } else {
    Write ("data successfully written ; exiting...");
    NewLine();
    return OK;
  }
}

/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipEpiDistParam\n");

  fprintf( stderr, "*WARNING* This documentation is outdated and wrong.\n" );

  (void)fprintf(stderr,"        [-b[atch] {command line mode}]\n");
  (void)fprintf(stderr,"          [-dex {double echo image - size x (default 128)}]\n");
  (void)fprintf(stderr,"          [-dey {double echo image - size y (default 128)}]\n");
  (void)fprintf(stderr,"          [-dez {double echo image - size z (default 128)}]\n");
  (void)fprintf(stderr,"          [-epix {epi image - size x  (default 64)}]\n");
  (void)fprintf(stderr,"          [-epiy {double echo image - size z (default 64)}]\n");
  (void)fprintf(stderr,"          [-epit {epi image - number of repetitions (default 22)}]\n");
  (void)fprintf(stderr,"          [-revz {epi - reverse z axis ? (y/n) (default n)}]\n");
  (void)fprintf(stderr,"          [-iet {inter_echo_time in ms: delay between successive echos (default 2.28)}]\n");
  (void)fprintf(stderr,"          [-dwt {dwell_time in ms: delay between successive echos in EPI image (default 0.64)}]\n");
  (void)fprintf(stderr,"        [-help]\n");
  return(PB);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Fill in parameter file required by VipEpiCorrect");
  (void)printf("\n");
  (void)printf("Usage: VipEpiDistParam\n");
  (void)printf("        param filename (interactive mode)\n");
  (void)printf("        -b[atch] {command line mode} with the following options:\n");
  (void)printf("          [-dex {double echo image - size x (default 128)}]\n");
  (void)printf("          [-dey {double echo image - size y (default 128)}]\n");
  (void)printf("          [-dez {double echo image - size z (default 128)}]\n");
  (void)printf("          [-epix {epi image - size x  (default 64)}]\n");
  (void)printf("          [-epiy {double echo image - size z (default 64)}]\n");
  (void)printf("          [-epit {epi image - number of repetitions (default 22)}]\n");
  (void)printf("          [-revz {epi - reverse z axis ? (y/n) (default y)}]\n");
  (void)printf("          [-iet {inter_echo_time in ms: delay between successive echos (default 2.28)}]\n");
  (void)printf("          [-dwt {dwell_time in ms: delay between successive echos in EPI image (default 0.64)}]\n");
  (void)printf("        [-h[elp]\n");
  (void)printf("Example:\n");
  (void)printf("-----------------------------------------------------\n");
  (void)printf("Mode interactif: le répertoire courant contient fidNN.raw au format Brucker\n");
  (void)printf("-->VipEpiDistParam distparam.txt\n");
  (void)printf("-----------------------------------------------------\n");
  (void)printf("Mode batch\n");
  (void)printf("Mode batch: le répertoire courant contient fidNN.raw au format Brucker\n");
  (void)printf("Choix par défaut pour tous les paramètres: \n");
  (void)printf("-->VipEpiDistParam distparam.txt -b \n");
  (void)printf("Autres choix pour les paramètres dex dey dez epix epiy epit revz\n");
  (void)printf("VipEpiDistParam distparam.txt -b -dex 256 -dey 256 -dez 256 -epix 128 -epiy 128 -epit 30 -revz n \n");
  (void)printf("-----------------------------------------------------\n");
  return(PB);

}

/******************************************************/
