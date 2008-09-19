#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <vip/epidistorsion/debug.h>
#include <vip/epidistorsion/utils.h>

void ConvertArg (unsigned char ***result, double ***argmap,param_mat *par) {
/* --------------------------------------------
   linear map of arguments between [-2pi,2pi[
   into [0,256[
   -------------------------------------------- */
  int i,j,k;
  double twopi = 8*atan(1);
  double temp;
  double index,step;
  int size_x = par->s_x;
  int size_y = par->s_y;
  int size_z = par->s_z;
  
  step = 256.0/size_y;

  for (k=0;k<size_z;k++) {
    index = -1;
    for (j=0;j<size_y;j++) {
      index+= step;
      for (i=0;i<4;i++)     /* writing an index */
	result[k][j][i]=(unsigned char) index;
      for (;i<size_x;i++) {
	temp = ( 128 + (128*argmap[k][j][i]/twopi));
	if (temp < 0) temp = 0;
	else if (temp > 255) temp = 255;
	result[k][j][i] = (unsigned char) temp;
      }
    }  
  }
}

void ConvertMod (unsigned char ***result, double ***argmap,param_mat *par) {
/* -------------------------------------------
   linear map of modulus range [0,max2D[
   into [0,256[ FOR EACH SLICE SEPARATELY
   ------------------------------------------- */
  int i,j,k;
  double max = 0;
  double value = 0;
  int size_x = par->s_x;
  int size_y = par->s_y;
  int size_z = par->s_z;

  for (k=0;k<size_z;k++) {
    max = max2D (argmap[k],size_y,size_x);

    for (j=0;j<size_y;j++) {
      for (i=0;i<size_x;i++) {
	value = argmap[k][j][i];
	/* value = (value<=max) ? value : max;*/
	result[k][j][i]= (unsigned char) (256*value/(max+1));
      }
    }
  }
}

void ConvertEpi (unsigned char ***result, double ***argmap, param_mat *par) {
/* --------------------------------------------
   linear map of [min_value, max_value[
   into [0,256[
   -------------------------------------------- */
  int i,j,k;
  double max,min;
  int size_x = par->s_x;
  int size_y = par->s_y;
  int size_z = par->s_z;


  max = max3D (argmap,size_z,size_y,size_x);
  min = min3D (argmap,size_z,size_y,size_x);
  

  for (k=0;k<size_z;k++) {
    for (j=0;j<size_y;j++) {
      for (i=0;i<size_x;i++) {
	result[k][j][i] = (unsigned char) (floor (256*(argmap[k][j][i]-min)/(max-min+1)) );
      }
    }
  }
}

int WriteDebug (unsigned char ***result, char *filename,param_mat *par) {
/* ----------------------------------------------------
   writes size_z debug images of size size_x*size_y
   filenames : results/filename$n.gray
               $n belongs to [0,size_z-1]
   ---------------------------------------------------- */
  int size_x = par->s_x;
  int size_y = par->s_y;
  int size_z = par->s_z;

  int j,k;
  char absfname[200];
  FILE* fout;
  
  for (k=0;k<size_z;k++) {
    
    (void) sprintf (absfname,"epi_corr_debug/%s%d.gray",filename,k);
    
    if ((fout = fopen (absfname,"w")) == NULL) {
      (void) printf ("%s%s","WriteDebug : cannot open ",absfname);
      return 1;
    };
    
    for (j=0;j<size_y;j++) {
      
      if( (fwrite((void *) result[k][j], sizeof(unsigned char),(size_t) size_x,fout)) != (size_t) size_x) {
	return 1;
      };
      
    }
    if ((fclose (fout)) !=0) {
      printf ("%s\n","WriteDebug : fclose error");
      return 1;
    }
    
  }
  return 0;
}
