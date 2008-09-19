#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <vip/epidistorsion/dyn_alloc.h>

#include <vip/epidistorsion/param.h>
#define NDEF_PARAM 0

#include <vip/epidistorsion/readwrite.h>
#include <vip/epidistorsion/param_io.h>
#include <vip/epidistorsion/debug.h>
#include <vip/epidistorsion/volume_io.h>
#include <vip/epidistorsion/volume_utils.h>
#include <vip/epidistorsion/utils.h>
#include <vip/epidistorsion/cpp_defs.h>

#if EPI_CORRECT_MAPS
#include <vip/epidistorsion/utils.h>
#endif

void Interpolation (double ***res, double ***image, double ***shiftmap, param_mat *par) {
  /* ------------------------------------------
     linear interpolation of image at points
     given by shiftmap
     res and shiftmap should have the same size
     ------------------------------------------ */
  int i,j,k;
  int size_x = par->s_x;
  int size_y = par->s_y;
  int size_z = par->s_z;

  int l_floor, l_ceil;  /* l stands for local */
  double l_weight,temp;
  double l_value;
#if I_CORRECTION
  double correction; /* according to the shift derivative */
#endif

  for(k=0; k<size_z ; k++)
    for(j=0; j<size_y; j++)
      for(i=0; i<size_x; i++) {

	
	l_value = j-shiftmap[k][j][i];
	temp = floor(l_value);

	/* indexes for interpolation */

	l_floor = (int) temp;
	if ((l_floor < 0)||(l_floor>=(size_y-1))) {res[k][j][i] = 0;}
	else {
	  l_ceil = l_floor + 1;	
	  l_weight = l_value - temp;

#if I_CORRECTION
	  if ((j<1)||(j>=(size_y-1))) correction = 1;
	  else correction = 1 - 0.5*(shiftmap[k][j+1][i]-shiftmap[k][j-1][i]);
#endif

/*	printf ("%s : %d %s : %d %s : %d ","k",k,"j",j,"i",i);                 */
/*	printf ("%s : %f  %s : %d  \n","l_value",l_value,"l_floor",l_floor);   */
	

#if I_CORRECTION
	  res[k][j][i] = correction*( (1-l_weight)*image[k][l_floor][i] + l_weight*image[k][l_ceil][i]);
#else
	  res[k][j][i] = (1-l_weight)*image[k][l_floor][i] + l_weight*image[k][l_ceil][i];
#endif
	}
      }
}


static int arguments(int argc, char* argv[], param_mat *par_epi,param_mat *par_fmap, param *par, param_acq *par_a) {
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
    Write ("epi_correct [parameter_file] distorted_epi corrected_epi phasemap");
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
      if (ReadParam (fid,par,par_a,par_fmap,par_epi) == ERROR) {
	Write (" Incorrect parameter file ; please use edit_param");
	NewLine();
	return ERROR;
      } 
    } else {
      Write ("using default parameters");
      DefaultParam(par,par_a,par_fmap,par_epi);
      NewLine();
    }
    
    (void) strcpy (par->fname_epi,argv[1+pfile]);
    (void) strcpy (par->fname_corr_epi,argv[2+pfile]);
    (void) strcpy (par->fname_fieldmap,argv[3+pfile]);
    
    if (pfile == 1) {(void) fclose (fid);} /* an error won't compromise the execution of the program */
    
    return OK;
  }
} /* end of arguments() */


/************************** main procedure *****************************/
int main (int argc, char* argv[]) {
  
  param_mat par_epi;
  param_mat par_fmap;
  param par;
  param_acq par_a;

  FILE *fin,*fout,*ffield;
  double ***image, ***result, ***map, ***shift_map;
  double factor, factor2pi;
  double twopi = 2*M_PI;
  int l;

#if EPI_CORRECT_MAPS
  param_mat par_debug;
  unsigned char ***debugmap;
  double ***debugresult;
#endif
  
  if ((arguments (argc,argv,&par_epi, &par_fmap, &par, &par_a)) == ERROR) {
    Write ("phasemap : an error occured ; exiting...");
    NewLine();
    return ERROR;
  };

#if EPI_CORRECT_MAPS
    par_debug.s_t=par_epi.s_t;
  par_debug.s_z=par_epi.s_z;
  par_debug.s_y=par_epi.s_y;
  par_debug.s_x=2*par_epi.s_x;

  if (( (Alloc3Dim((void ****)&debugresult,sizeof(double),par_debug.s_z,par_debug.s_y,par_debug.s_x))==ERROR )
      ||( (Alloc3Dim((void ****)&debugmap,sizeof(unsigned char),par_debug.s_z,par_debug.s_y,par_debug.s_x))==ERROR ) ) {
    Write ("Epi_correct, debug maps : alloc failed\n");
    return ERROR;
  };
#endif
  if  ((fin = fopen (par.fname_epi,"r")) == NULL) {
    printf ("epi_correct : could not open epi image file %s\n",par.fname_epi);
    return ERROR;
  }
  if ((fout = fopen (par.fname_corr_epi,"w")) == NULL) {
    printf ("epi_correct : could not open result file %s\n",par.fname_corr_epi);
    return ERROR;
  }
  if ((ffield = fopen (par.fname_fieldmap,"r")) == NULL) {
    printf ("epi_correct : could not open phasemap file %s\n",par.fname_fieldmap);
    return ERROR;
  };

  if ( ((Alloc3Dim((void ****)&image,sizeof(double),par_epi.s_z,par_epi.s_y,par_epi.s_x))==ERROR)
       ||((Alloc3Dim((void ****)&result,sizeof(double),par_epi.s_z,par_epi.s_y,par_epi.s_x))==ERROR)
       ||((Alloc3Dim((void ****)&map,sizeof(double),par_fmap.s_z,par_fmap.s_y,par_fmap.s_x))==ERROR)
       ||((Alloc3Dim((void ****)&shift_map,sizeof(double),par_epi.s_z,par_epi.s_y,par_epi.s_x))==ERROR) ) {
    printf("%s\n","alloc failed");
    return ERROR;
  };
  
  Write ("ReadMap");
  NewLine();
  if ((ReadMap (map,ffield,&par_fmap)) == ERROR) {
    Write ("error reading fieldmap");
    NewLine();
    return ERROR;
  }
     
  /* core of the algorithm */
  
#if EPI_CORRECT_DEBUG
  Write ("Resize\n");
#endif
  
  Resize (shift_map, map, &par_epi, &par_fmap);
  
  factor2pi = (par_epi.s_x)*(par_a.dwell_time)/(par_a.inter_echo_time);
  factor = factor2pi/twopi;
  
#if EPI_CORRECT_DEBUG
  Write ("cstMultiply\n");
#endif
  
  cstMultiply (shift_map, shift_map,factor, &par_epi);
  
#if EPI_CORRECT_MAPS
    ConvertEpi (debugmap,shift_map,&par_epi);
  
  if ((WriteDebug (debugmap,"shiftmap",&par_epi))==ERROR) {
    Write("WriteDebug error\n");
    return ERROR;
  };
#endif
  
  /* Interpolation */
  for (l=0;l<par_epi.s_t;l++) { 
        
    NewLine();
    if ((ReadEpi (image,fin,&par_epi)) == ERROR) {
      Write ("error reading epi data\n");
      return ERROR;
    }
    printf ("Correcting volume %d\n",l);
    
    Interpolation (result, image, shift_map, &par_epi);
    
    if ((WriteEpi (result,fout,&par_epi))==ERROR) {
      Write ("WriteEpi error\n");
      return ERROR;
    };
  }
#if EPI_CORRECT_MAPS
  copy3D (debugresult,result,par_epi.s_z,par_epi.s_y,par_epi.s_x,0,0,0);
  copy3D (debugresult,image,par_epi.s_z,par_epi.s_y,par_epi.s_x,0,0,par_epi.s_x);
  
  ConvertEpi (debugmap,result,&par_epi);
  if ((WriteDebug (debugmap,"corr_epi",&par_epi))==ERROR) {
    printf ("%s\n","WriteDebug error");
    return ERROR;
  };

  ConvertEpi (debugmap,image,&par_epi);
  if ((WriteDebug (debugmap,"epi",&par_epi))==ERROR) {
    printf ("%s\n","WriteDebug error");
    return ERROR;
  };
  
  ConvertEpi (debugmap,debugresult,&par_debug);
  if ((WriteDebug (debugmap,"epi_compare",&par_debug))==ERROR) {
    printf ("%s\n","WriteDebug error");
    return ERROR;
  };

  Free3Dim ((void ***)debugresult,par_debug.s_z,par_debug.s_y);
  Free3Dim ((void ***)debugmap,par_debug.s_z,par_debug.s_y);
#endif

  Free3Dim((void ***)image,par_epi.s_z,par_epi.s_y);
  Free3Dim((void ***)result,par_epi.s_z,par_epi.s_y);
  Free3Dim((void ***)map,par_fmap.s_z,par_fmap.s_y);
  Free3Dim((void ***)shift_map,par_epi.s_z,par_epi.s_y);

  return OK;
  
}

