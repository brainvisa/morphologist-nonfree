#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vip/epidistorsion/utils.h>
#include <vip/epidistorsion/sort.h>
#include <vip/epidistorsion/dyn_alloc.h>
#include <vip/epidistorsion/cpp_defs.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

double roundvalue (double x) {
  double fl, rem, res;

  fl = floor(x);
  rem = x - fl;
  res = (rem >= 0.5) ? (fl+1) : fl;
  return res;
}

int indexmax (int *array,int fst,int lst) {
  int n,max;

  max = fst;
  for (n=fst+1;n<=lst;n++)
    if(array[n]>array[max])
      max = n;
  return max;
}

double max2D (double **map,int size_y, int size_x) {
  int i,j;
  double max = map[0][0];
  double value;
  
  for (i=0;i<size_x;i++) {
    for (j=0;j<size_y;j++) {
      value = map[j][i];
      max = (value>max) ? value : max;
    }
  }
  return max;
}

double max3D (double ***map,int size_z,int size_y,int size_x) {
  int k;
  double max = map[0][0][0];
  double value;

  for (k=0;k<size_z;k++) {
    value = max2D (map[k],size_y,size_x);
    max = (value>max) ? value : max;
  }
  return max;
}


double min2D (double **map,int size_y, int size_x) {
  int i,j;
  double min = map[0][0];
  double value;
  
  for (i=0;i<size_x;i++) {
    for (j=0;j<size_y;j++) {
      value = map[j][i];
      min = (value<min) ? value : min;
    }
  }
  return min;
}

double min3D (double ***map,int size_z,int size_y,int size_x) {
  int k;
  double min = map[0][0][0];
  double value;

  for (k=0;k<size_z;k++) {
    value = min2D (map[k],size_y,size_x);
    min = (value<min) ? value : min;
  }
  return min;
}


void hsort (double ra[], int n) {
/* -----------------------------
   heap sort of ra[0...n-1];
   interface to sort (from NRC)
   ----------------------------- */
  double *vect = ra-1;
  sort (n,vect);
}

double mean (double vect[], int fst, int lst) {
/* ---------------------------------------
   mean of the (lst-fst) values of vect,
   located between the indexes fst and lst
   --------------------------------------- */
  double sum = 0;
  int i;

  for (i=fst;i<lst;i++) sum += vect[i];
  sum /= (lst-fst);
  return sum;
}

void cmod (double *res, double *z) {
/* -----------------------------
   modulus of a complex value
   ----------------------------- */
  double x,y;
  x=z[0]; y=z[1];
  *res = x*x+y*y;
}
  
  

void cdiv (double* res, double *a, double *b) {
/* ------------------------------
   complex division    res = a/b
   1 is returned if b=0
   -----------------------------*/
  double x1,y1,x2,y2,m;

  x1=a[0]; y1=a[1];
  x2=b[0]; y2=-b[1];

  m=x2*x2+y2*y2;
  
  if (m==0) {
    res[0]=1;res[1]=0;
  } else {
    res[0]=(x1*x2-y1*y2)/m;
    res[1]=(x1*y2+x2*y1)/m;
  }
  
}

void cmult (double* res, double *a, double *b) {
/* ------------------------------
   complex multiplication res = a*b
   -----------------------------*/
  double x1,y1,x2,y2;

  x1=a[0]; y1=a[1];
  x2=b[0]; y2=b[1];
  
  res[0]=(x1*x2-y1*y2);
  res[1]=(x1*y2+x2*y1);
  
}

void cphase (double *res,double *z) {
/* -----------------------------
   argument of a complex number
   result in range [-pi,pi[
   -----------------------------*/
  double x,y,phi,pi2;
  pi2=M_PI_2;

  x=z[0];
  y=z[1];
  phi = (x==0) ? pi2 : (atan(y/x));
  if (x>=0) {
    res[0] = phi;
  } else {
    res[0] = (y<=0) ? (phi-2*pi2) : (phi + 2*pi2);
  }
}

double pvalue (double phi) {
  /* -------------------------
     principal value of phi :
     result = phi mod(2pi)
     result in [-pi,pi[
     ------------------------- */
  double pi=M_PI;
  double res=phi;
  
  while (res>=pi) res-=(2*pi);
  while (res<-pi) res+=(2*pi);
  return res;
}

void copy3D (double ***res,double ***src, int s_z, int s_y, int s_x, int start_z, int start_y, int start_x) {
  int i,j,k;

   for (k=start_z;k<(start_z+s_z);k++) 
    for (j=start_y;j<(start_y+s_y);j++) 
      for (i=start_x;i<(start_x+s_x);i++) 
	res[k][j][i]=src[k][j][i];
}
    
void boxcar (double ***mask, int contrast, double threshold, int s_z, int s_y, int s_x) {
  /* ------------------------------------------------------------------
     sets all the values of the matrix "mask" to contrast or 1-contrast
     mask[k][j][i] = contrast if mask[k][j][i]>threshold,
                     1 - contrast otherwise
     ------------------------------------------------------------------ */
  int i,j,k;

  for (k=0;k<s_z;k++) 
    for (j=0;j<s_y;j++) 
      for (i=0;i<s_x;i++) 
	if (mask[k][j][i] > threshold)
	  mask[k][j][i]=contrast;
	else
	  mask[k][j][i] = (1-contrast);
}

double sum (double ***image, int s_z,int s_y,int s_x) {
  /*--------------------------------------------------*/
  int i,j,k;
  double result = 0;

  for (k=0;k<s_z;k++) 
    for (j=0;j<s_y;j++) 
      for (i=0;i<s_x;i++) 
	result += image[k][j][i];
  return result;
}


void avevar (double *ave, double *var, double ***data, int s_z, int s_y, int s_x) {
  /* -----------------------------------------------------------------
     calculates the mean and the variance of the right 10 %
     of the volume
     ----------------------------------------------------------------- */

  int i,j,k,start_x, n;
  double s,ep;

#if UTILS_DEBUG
  printf ("\navevar start\n");
#endif
  
  start_x = 0.9*s_x;
  n = s_z*s_y*(s_x-start_x);

#if UTILS_DEBUG
  printf ("starting point along x axis : %d\n",start_x);
  printf ("number of points : %d\n",n);
#endif
  
  *ave = 0.0;

  for(k=0; k<s_z; k++)
    for(j=0; j<s_y; j++)
      for(i=start_x; i<s_x; i++) 
	*ave += data[k][j][i];
  
  *ave /= n;
  *var=ep=0.0;
  for(k=0; k<s_z; k++)
    for(j=0; j<s_y; j++)
      for(i=start_x; i<s_x; i++) {
	s=data[k][j][i]-(*ave);
	ep += s;
	*var += s*s;
      }
  *var=(*var-ep*ep/n)/(n-1);
}
/* Adapted from Numerical Recipes */


