#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <vip/epidistorsion/volume_io.h>

/* --------------------------------------------
   functions reading and writing volumes
   -------------------------------------------- */
int ReadEcho (double ***echo1,double ***echo2,FILE *fin,param_mat *par) {
  int i,j,k,slice;
  int ii, jj;

  int s_x = par->s_x;
  int s_y = par->s_y;
  int s_z = par->s_z;
  long vect[4*s_x];

  for (jj=0;jj<s_y;jj++) {
    j = (par->reverse_y == 1) ? jj : (s_y-1-jj); /* reversing */
    
    for (k=0;k<s_z;k++) {
      
      slice = 2*k;
      slice = ((slice)>=s_z) ? (slice+1-s_z) : slice;
      
      slice = (par->reverse_z == 1) ? slice : (s_z-1-slice);  /* reversing */

      if ( (fread((void *) &vect, sizeof(long),(size_t) 4*s_x,fin)) 
           != 4*(size_t)s_x) {
	return 1;
      };
      for (ii=0;ii<s_x;ii++) {

	i = (par->reverse_x == 1) ? ii : (s_x -1 - ii); /* reversing */

	echo1[slice][j][2*i] = (double) vect[2*ii];
	echo1[slice][j][2*i+1] = (double) vect[2*ii+1];
	echo2[slice][j][2*i] = (double) vect[2*s_x+2*ii];
	echo2[slice][j][2*i+1] = (double) vect[2*s_x+2*ii+1];
      }
    }
  }
  return 0;
}

int WriteMap (double ***result, FILE* fout, param_mat *par) {
  int j,k;
  int s_x = par->s_x;
  int s_y = par->s_y;
  int s_z = par->s_z;

  for (k=0;k<s_z;k++) {
    for (j=0;j<s_y;j++) {
	if ( (fwrite((void *) result[k][j],sizeof(double),(size_t) s_x,fout)) 
             != (size_t)s_x) {
	  return 1;
	}
    }
  } 
  return 0;
}

int ReadMap (double ***map, FILE* fin, param_mat* par) {
  int j,k;
  int s_x = par->s_x;
  for (k=0;k<par->s_z;k++) {
    for (j=0;j<par->s_y;j++) {
      if ( (fread((void *) map[k][j],sizeof(double),(size_t) s_x,fin)) 
           != (size_t)s_x) {
	  return 1;
	}
    }
  } 
  return 0;
}

int ReadEpi (double ***image, FILE* fin, param_mat *par) {
  /* reads ONE REPETITION of an epi file */
  int s_x = par->s_x,
    s_y = par->s_y,
    s_z = par->s_z;
  
  short vect[s_x];
  
  int i,j,k, ii, jj, kk;
  
  for (kk=0;kk<s_z;kk++) {
    k = (par->reverse_z == 1) ? kk : (s_z-1 -kk); /* reversing */

      
    for (jj=0;jj<s_y;jj++) {
      j = (par->reverse_y == 1) ? jj : (s_y-1 -jj); /* reversing */
      
      if ( (fread((void *) &vect,sizeof(short),(size_t) s_x,fin)) 
           != (size_t)s_x) {
	return 1;
      }
      for (ii=0;ii<s_x;ii++) {
	i = (par->reverse_x == 1) ? ii : (s_x-1 -ii); /* reversing */

	image[k][j][i] = (double) vect[ii]; 
      }
      
    } 
  }
  
  return 0;
}

int WriteEpi (double ***image, FILE* fout, param_mat *par) {

  int s_x = par->s_x,
    s_y = par->s_y,
    s_z = par->s_z;

  short vect[s_x];

  int i,j,k,ii,jj,kk;

  for (kk=0;kk<s_z;kk++) {
    k = (par->reverse_z == 1) ? kk : (s_z-1 -kk);  /* reversing */
      
    for (jj=0;jj<s_y;jj++) {
      j = (par->reverse_y == 1) ? jj : (s_y-1 -jj);  /* reversing */
      
      for (ii=0;ii<s_x;ii++) {
	i = (par->reverse_x == 1) ? ii : (s_x-1 -ii);  /* reversing */

	vect[i] = (short) image[k][j][ii];
      }
      if ( (fwrite((void *) vect,sizeof(short),(size_t) s_x,fout)) 
           != (size_t)s_x) {
	  return 1;
      } 
    }
  }
  return 0;
}
