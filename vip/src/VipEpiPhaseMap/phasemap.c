#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <vip/epidistorsion/dyn_alloc.h>
#include <vip/epidistorsion/param.h>
#define NDEF_PARAM 0

#include <vip/epidistorsion/readwrite.h>
#include "models.h"

#include <vip/epidistorsion/param_io.h>
#include <vip/epidistorsion/volume_io.h>
#include <vip/epidistorsion/volume_utils.h>
#include "reconstruct.h"
#include "segmentation.h"
#include "unwrap.h"
#include "unwrap_cst.h"
#include "extrapolation.h"

#include <vip/epidistorsion/cpp_defs.h>

#if DRAW_MAPS
#include <vip/epidistorsion/utils.h>
#include <vip/epidistorsion/debug.h>
#endif


/* global definitions used by models.c */
int sizex,sizey;


static int arguments(int argc, char* argv[], param_mat *g_echo,param_mat *par_epi,  param *par, param_acq *par_a) {
/* ----------------------------------------------------------------------
   definition of parameters from command line
   no security check is performed on the arguments
   -------------------------------------------------------------------- */

  FILE *fid = NULL;
  char buf[200];
  int pfile = -1;

  if (argc == 4) pfile = 0;
  if (argc == 5) pfile = 1;

  if (pfile == -1) {
    Write ("Syntax :");NewLine();
    Write ("phasemap [parameter_file] raw_grad_echo output_phasemap distorted_epi");
    NewLine();
    return ERROR;
  } else {
  
    Write  ("parameter file : "); 
    if (pfile == 1) {
      (void) strcpy(buf, argv[1]); 
      Write (buf);
      NewLine();
      if ((fid = fopen (buf,"r")) == NULL) {
	printf ("couldn't open parameter file %s;",buf);
	return ERROR;
      }
#if TRACE
      else {Write ("OK opening fid"); NewLine();}
#endif
      if (ReadParam (fid,par,par_a,g_echo,par_epi) == ERROR) {
	Write (" Incorrect parameter file ; please use edit_param");
	NewLine();
	return ERROR;
      } 
#if TRACE
      else {Write ("OK ReadParam"); NewLine();}
#endif
  
    } else {
      Write ("using default parameters");
      DefaultParam(par,par_a,g_echo,par_epi);
      NewLine();
    }

   
    (void) strcpy (par->fname_echo,argv[1+pfile]);
    (void) strcpy (par->fname_fieldmap,argv[2+pfile]);
    (void) strcpy (par->fname_epi,argv[3+pfile]);
#if TRACE
     Write ("OK strcopies"); NewLine();
#endif 
    
    if (pfile == 1) {(void) fclose (fid);} /* an error won't compromise the execution of the program */
    
    /* extern parameters used by the model */
    sizex = g_echo->s_x;
    sizey = g_echo->s_y;
    
    return OK;
  }
} /* end of arguments() */


/***************************  MAIN PROCEDURE  ***************************/

int main (int argc, char* argv[]) {
  
  param par;
  param_acq par_a;
  param_mat par_echo;

  param_mat par_epi;
  
  FILE *fin,*fout;
  double ***echo1, ***echo2, ***map, ***modmap;
  double ***mask;
  
#if DRAW_MAPS
  int s_z,s_y,s_x;
  param_mat par_debug;
  unsigned char ***eightbitmap;
  double ***debugmap;
#endif


  /* READING ARGUMENTS */
  if ((arguments (argc,argv,&par_echo,&par_epi,&par, &par_a)) == ERROR) {
    Write ("an error occured reading the parameters; exiting...");
    NewLine();
    return ERROR;
  }

  /* OPENING INPUT AND OUTPUT FILES */
  if ( ((fin = fopen (par.fname_echo,"r")) == NULL)
       ||((fout = fopen (par.fname_fieldmap,"w")) == NULL) ) {
    
    printf ("%s","error opening input and output files");
    return ERROR;
  }

#if DRAW_MAPS
  s_z=par_echo.s_z;
  s_y=par_echo.s_y;
  s_x=par_echo.s_x;
  par_debug.s_z=s_z;
  par_debug.s_y=s_y;
  par_debug.s_x=4*s_x;
#endif


  /* ALLOCATION */
  
  if ( ((Alloc3Dim((void ****)&echo1,sizeof(double),par_echo.s_z,par_echo.s_y,2*par_echo.s_x))==ERROR)
       ||((Alloc3Dim((void ****)&echo2,sizeof(double),par_echo.s_z,par_echo.s_y,2*par_echo.s_x))==ERROR) 
       ||((Alloc3Dim((void ****)&map,sizeof(double),par_echo.s_z,par_echo.s_y,par_echo.s_x))==ERROR)
       ||((Alloc3Dim((void ****)&modmap,sizeof(double),par_echo.s_z,par_echo.s_y,par_echo.s_x))==ERROR)
       ||((Alloc3Dim((void ****)&mask,sizeof(double),par_echo.s_z,par_echo.s_y,par_echo.s_x))==ERROR)  
       
#if DRAW_MAPS
       ||((Alloc3Dim((void ****)&debugmap,sizeof(double),par_debug.s_z,par_debug.s_y,par_debug.s_x))==ERROR)
       /* the output for debugging is an (unsigned char) debugmap */
       ||((Alloc3Dim((void ****)&eightbitmap,sizeof(unsigned char),par_debug.s_z,par_debug.s_y,par_debug.s_x))==ERROR)
#endif
              ) {
    Write ("phasemap : alloc failed\n");
    return ERROR;
  };
  
#if TRACE
  Write ("ReadEcho"); NewLine();
#endif 
  if ((ReadEcho (echo1,echo2,fin,&par_echo)) == ERROR) {
    Write("error reading echo data");
    NewLine();
    return ERROR;
  }
  
  /* RECONSTRUCTION */
  Write ("Reconstruction\n");
  reconstruct(echo1,&par_echo);
  reconstruct(echo2,&par_echo);

 /* 2ND ECHO MODULUS */
  GetMod (modmap,echo2,&par_echo);

#if DRAW_MAPS
  ConvertMod (eightbitmap, modmap,&par_echo);
  if ((WriteDebug (eightbitmap,"anatomy",&par_echo))==ERROR) {
    printf ("%s\n","WriteDebug error");
    return ERROR;
  }; 
#endif    

  /* SEGMENTATION */
  Write ("Segmentation\n");

  if (par.segmentation == 1) {
    
    if (Segmentation(mask,modmap,&par,&par_echo)==ERROR) {
      Write ("Phasemap : an error occured during segmentation. Exiting...\n");
      return ERROR;
      
    }

  } else {   /* WARNING : THIS PART HAS NOT BEEN TESTED !!! */

    if (GetSegmentedMask(mask,&par,&par_echo) == ERROR) {
      Write ("Phasemap : could not read the provided mask. Exiting...");
      return ERROR;
    }
  }   
#if DRAW_MAPS
  ConvertMod (eightbitmap, mask,&par_echo);
  if ((WriteDebug (eightbitmap,"mask",&par_echo))==ERROR) {
    printf ("%s\n","WriteDebug error");
    return ERROR;
  }; 
#endif
  
  /* COMPLEX IMAGE CONTAINING THE PHASE VALUES OF INTEREST */
  Divide (echo1,echo2,echo1,&par_echo);
  
#if DRAW_MAPS
  GetPhase (debugmap,echo1,&par_echo);
#endif   

  /* UNWRAPPING */
  Write ("Unwrapping\n");
  if (Unwrap (map, echo1, mask, &par, &par_echo)== ERROR) {
    Write ("Phasemap : an error occured during Unwrap. Exiting...\n");
    return ERROR;
  }
#if DRAW_MAPS
  copy3D (debugmap,map,s_z,s_y,s_x,0,0,s_x);
#endif
  
  /* CORRECTING A 2*PI ERROR ON THE PHASEMAP */
  Write ("Calculating constant shift\n");
  if(UnwrapCst(map,modmap,mask,&par,&par_a,&par_echo,&par_epi) == ERROR) {
    Write ("Phasemap : an error occured during UnwrapCst. Exiting...\n");
    return ERROR;
  } 
#if DRAW_MAPS
  copy3D (debugmap,map,s_z,s_y,s_x,0,0,2*s_x);
#endif

  /* EXTRAPOLATING THE MAP */
  Write("Extrapolation\n");
  if (Extrapolation(map, mask, &par, &par_echo) == ERROR) {
    Write ("Phasemap : an error occured during Extrapolation. Exiting...\n");
    return ERROR;
  }
#if DRAW_MAPS
  copy3D (debugmap,map,s_z,s_y,s_x,0,0,3*s_x);
#endif

  /* WRITING THE RESULT */
  if ((WriteMap (map,fout,&par_echo))==ERROR) {
    printf ("Phasemap : an error occured while writing the result. Exiting ...\n");
    return ERROR;
  };

#if DRAW_MAPS
  ConvertArg (eightbitmap,debugmap,&par_debug);
  if ((WriteDebug (eightbitmap,"debug",&par_debug))==ERROR) {
    printf ("%s\n","WriteDebug error");
       return ERROR;
  }; 
#endif

/******** DEBUG MAPS *******/

 Free3Dim((void ***)echo1,par_echo.s_z,par_echo.s_y);
 Free3Dim((void ***)echo2,par_echo.s_z,par_echo.s_y);
 Free3Dim((void ***)map,par_echo.s_z,par_echo.s_y);
 Free3Dim((void ***)modmap,par_echo.s_z,par_echo.s_y);
 Free3Dim((void ***)mask,par_echo.s_z,par_echo.s_y);
#if DRAW_MAPS
 Free3Dim((void ***)debugmap,par_debug.s_z,par_debug.s_y);
 Free3Dim((void ***)eightbitmap,par_debug.s_z,par_debug.s_y);
#endif

 FreeModels();
 
 (void) fclose (fin);
 (void) fclose (fout);

 return OK;
  
}


/**************************  ARCHIVES DEBUG **************************/
  /* testing Dilate and Erode */
/*
  GetMod (map,echo2,&par_echo);
  GetMask (mask,map,par.noise_threshold,1,&par_echo);

  ConvertMod (debugmap,mask,&par_echo);
  if ((WriteDebug (debugmap,"firstmask",&par_echo))==ERROR) {
    printf ("%s\n","WriteDebug error");
    return ERROR;
  };

  Erode (mask,6,par_echo.s_z,par_echo.s_y,par_echo.s_x);
  ConvertMod (debugmap,mask,&par_echo);
  if ((WriteDebug (debugmap,"erodedmask",&par_echo))==ERROR) {
    printf ("%s\n","WriteDebug error");
    return ERROR;
  };
 
  Dilate (mask,6,par_echo.s_z,par_echo.s_y,par_echo.s_x);
  ConvertMod (debugmap,mask,&par_echo);
  if ((WriteDebug (debugmap,"Dilatedmask",&par_echo))==ERROR) {
    printf ("%s\n","WriteDebug error");
    return ERROR;
  };

  Write ("convexmask\n");
  convexmask2D (convexmask,mask[0],par_echo.s_y,par_echo.s_x);
  Write ("end convexmask\n");

  for (i=0;i<par_echo.s_x;i++)
    for (j=0;j<par_echo.s_y;j++)
      mask[0][j][i]=1;
  Applyconvmask2D(mask[0],convexmask,par_echo.s_y,par_echo.s_x);

  Write ("end Applyconvmask2D\n");
  par_echo.s_z =1;
  ConvertMod (debugmap,mask,&par_echo);
  if ((WriteDebug (debugmap,"convmask",&par_echo))==ERROR) {
    printf ("%s\n","WriteDebug error");
    return ERROR;
  };
*/
