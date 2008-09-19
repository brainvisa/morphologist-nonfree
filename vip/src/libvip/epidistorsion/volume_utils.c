#include <stdio.h>
#include <stdlib.h>

#include <vip/epidistorsion/utils.h>
#include <vip/epidistorsion/volume_utils.h>

void Divide (double ***res,double ***a,double ***b,param_mat *par) {
/* --------------------------------------------
   term-to-term complex division
   res[k][j][i] = 1 if b[k][j][i] == 0
   -------------------------------------------- */
  int i,j,k;
  int size_x = par->s_x;
  int size_y = par->s_y;
  int size_z = par->s_z;

  
  for (k=0;k<size_z;k++) {
    for (j=0;j<size_y;j++) {
      for (i=0;i<size_x;i++) {
	cdiv (res[k][j]+2*i,a[k][j]+2*i,b[k][j]+2*i);
      }
    }
  }
}
 
void GetPhase (double ***res, double ***z, param_mat *par) {
  int i,j,k;
  int size_x = par->s_x;
  int size_y = par->s_y;
  int size_z = par->s_z;

  for (k=0;k<size_z;k++) {
    for (j=0;j<size_y;j++) {
      for (i=0;i<size_x;i++) {
	cphase (res[k][j]+i,z[k][j]+2*i);
	
      }
    }
  }
  
}

void GetMod (double ***res, double ***m, param_mat *par) {
  int i,j,k;
  int size_x = par->s_x;
  int size_y = par->s_y;
  int size_z = par->s_z;

  for (k=0;k<size_z;k++) {
    for (j=0;j<size_y;j++) {
      for (i=0;i<size_x;i++) {
	cmod (res[k][j]+i,m[k][j]+2*i);
	
      }
    }
  }
  
}

void rMultiply (double ***res,double ***x, double ***y, double factor, param_mat *par) {
/* ---------------------------------
   real term-to-term multiplication,
   with constant factor
   --------------------------------- */
  int i,j,k;
  int size_x = par->s_x;
  int size_y = par->s_y;
  int size_z = par->s_z;

  for (k=0;k<size_z;k++) {
    for (j=0;j<size_y;j++) {
      for (i=0;i<size_x;i++) {
	res[k][j][i] = factor*(x[k][j][i])*(y[k][j][i]);
      }
    }
  }
}
	
void cstMultiply (double ***res, double ***arg, double factor, param_mat *par) {
/* -----------------------------------
   multiplication by a constant factor
   ----------------------------------- */
  int i,j,k;
  int size_x = par->s_x;
  int size_y = par->s_y;
  int size_z = par->s_z;

  for (k=0;k<size_z;k++) {
    for (j=0;j<size_y;j++) {
      for (i=0;i<size_x;i++) {
	res[k][j][i] = factor*(arg[k][j][i]);
      }
    }
  }
}

void Resize (double ***res, double ***m, param_mat *par_result, param_mat *par_m) {
/* ------------------------------------------------------------------
   reduces the size of m to size_z size_y size_x
   the result is the mean of the overlapped values
   WARNING : m sizes should be multiples of res sizes !
   ------------------------------------------------------------------ */
  int i,j,k,ii,jj,kk;
  double value;
  int size_x = par_result->s_x;
  int size_y = par_result->s_y;
  int size_z = par_result->s_z;

  int factor_x = (par_m->s_x)/size_x;
  int factor_y = (par_m->s_y)/size_y;
  int factor_z = (par_m->s_z)/size_z;
  
  for(k=0; k<size_z; k++) {  
    for(j=0; j<size_y; j++) {
      for(i=0; i<size_x; i++) {
	
	value = 0;
	
	for (kk=0;kk<factor_z;kk++) {
	  for (jj=0;jj<factor_y;jj++) {
	    for (ii=0;ii<factor_x;ii++) {
	      value += m[factor_z*k+kk][factor_y*j+jj][factor_x*i+ii];
	    }
	  }
	}
	res[k][j][i] = value/(factor_x*factor_y*factor_z);
      }
    }
  }
}

void ApplyMask (double ***m, double ***mask, param_mat *par) {
  /* ----------------------------------------------
     m[j][i] = m[j][i]*mask[j][i];
     interface for rMultiply
     ---------------------------------------------- */
   
  rMultiply (m, m, mask, 1, par);
}
	
