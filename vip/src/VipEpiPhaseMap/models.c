/* basis of models used to interpolate the fieldmap */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "models.h"

#define MODEL_DEBUG 0

static double ***cosmap;
int cosmapsize = 0;
static double ***polymap;
int polymapsize = 0;

/* size of the matrix */ 
extern int sizex, sizey;


/************************** BASIC FUNCTIONS **************************/

double cosinus (double x, int k, int n) {
  /* -------------------------------
     returns cos (pi*k*x/n)
     ------------------------------- */

  double pi = 4*atan(1);
  double freq;
  
  freq = ((double) k)/((double) n);

  return (cos(pi*freq*x));
}

double sinus (double x, int k, int n) {
  /* -------------------------------
     returns sin(pi*k*x/n)
     ------------------------------- */
  double pi = 4*atan(1);
  double freq;
  
  freq = ((double) k)/((double) n);

  return (sin(pi*freq*x));
}

/********************** COSINE BASE ****************************/

void cosbase (int y,int x, double *res, int n) {
  /* ----------------------------------
     res[i] = value of the i-eth function
     of the 2D cosine base of size n,
     at point (x,y)
     i = 0...n-1
     ---------------------------------- */

  int i,j;
  int q, r;
  int nn;
  int flag = OK;

  /* allocation */
  if (n != cosmapsize) {

    if (cosmapsize == 0) {
      
      printf ("cosbase : allocating the cosmap\n");
      flag = Alloc3Dim((void ****)&cosmap,sizeof(double),sizey,sizex,n+1);
      
    } else {
      
      printf ("cosbase : reallocating the cosmap\n");
      
      for (j=0;(flag==OK)&&(j<sizey);j++)
	for (i=0;(flag==OK)&&(i<sizex);i++)
	  flag = Realloc1Dim((void **)(cosmap[j]+i),sizeof(double),n+1);
    }
    
    if (flag == OK) {
      cosmapsize = n;
      for (j=0;j<sizey;j++)
	for (i=0;i<sizex;i++)
	  cosmap[j][i][0]=0;
    } else {
      printf ("allocation failed /n");
    }
  }
  /* returning the values */
#if MODEL_DEBUG
 /*  printf ("%s%d %d\n","cosbasemap ",y,x); */
#endif
  
  if (flag == OK) {
    
    if (cosmap[y][x][0] == 0) {
#if MODEL_DEBUG
      printf ("%s %d %d\n","initializing",y,x); 
#endif  
      
      for(i=0;i<n;i++) {
	nn = sqrt(n);
	
	q = i/nn;
	r = i-q*nn;
	cosmap[y][x][i+1]= (cosinus(y,q,sizey))*(cosinus (x, r,sizex));
      }
      cosmap[y][x][0]=1;
    }
    
    for (i=0;i<n;i++)
      res[i]=cosmap[y][x][i+1];	

  } else {   /* allocation failed */
  
    printf ("%s\n","cosbase : basemap allocation failed ; the program will run slowly");
    
    for(i=0;i<n;i++) {
      nn = sqrt(n);
      
      q = i/nn;
      r = i-q*nn;
      res[i] = (cosinus(y,q,sizey))*(cosinus (x, r,sizex));
    }
  }
}

void diffcosbase (int y,int x, double *res, int n) {
/* ----------------------------------
   at point x,y :
   differenciation of cosbase(   ,1...n+1)
   (we don't want the zero-valued difference
   map of the constant function cosbase(...,0)

   x=0...sizex-1 : differentiation
   along the x-axis
   x=sizex...2*sizex-1 : differenciation
   along the y-axis
   ---------------------------------- */

  double tadd[n+1],tsub[n+1];
  
  int i,x2,y2,x1,y1;

#if MODEL_DEBUG
  /* printf ("%s %d %d\n","diffcosbase :",y,x); */
#endif


  x1=x;
  y1=y;

  if (x<sizex) {
    x2 = (x==0) ? (sizex-1) : (x-1);
    y2 = y;
  } else {
    
    x1=x-sizex;
    x2=x1;
    y2 = (y==0) ? (sizey-1) : (y-1);
  }
  
  cosbase (y1,x1,tadd,n+1);
  cosbase (y2,x2,tsub,n+1);


  for(i=0;i<n;i++) {
    res[i] = tadd[i+1]-tsub[i+1];
  }
}

/********************* POLYNOMIAL BASE **********************/

void polybase (int y, int x, double *res, int n) {
  /* ----------------------------------
     res[i] = value if the i-eth function
     of the 2D polynomial base of size n,
     at point (x,y)
     i = 0...n-1
     ---------------------------------- */

  int i,j;
  int q, r;
  int nn;
  int flag = OK;
  double xx, yy, max;
  
  yy = ((double) y-sizey/2);
  xx = ((double) x-sizex/2);
  
  /* allocation */
  if (n != polymapsize) {

    if (polymapsize == 0) {
      
      printf ("polybase : allocating the polymap\n");
      flag = Alloc3Dim((void ****)&polymap,sizeof(double),sizey,sizex,n+1);
      
    } else {
      
      printf ("polybase : reallocating the polymap\n");
      
      for (j=0;(flag==OK)&&(j<sizey);j++)
	for (i=0;(flag==OK)&&(i<sizex);i++)
	  flag = Realloc1Dim((void **)(polymap[j]+i),sizeof(double),n+1);
    }
    
    if (flag == OK) {
      polymapsize = n;
      for (j=0;j<sizey;j++)
	for (i=0;i<sizex;i++)
	  polymap[j][i][0]=0;
    } else {
      printf ("allocation failed /n");
    }
  }
  /* returning the values */
#if MODEL_DEBUG
  printf ("%s%d %d\n","polybasemap ",y,x);
#endif
  
  if (flag == OK) {

    if (polymap[y][x][0] == 0) {
#if MODEL_DEBUG
      printf ("%s\n","initializing"); 
#endif  
      
      for(i=0;i<n;i++) {
	nn = sqrt(n);
	
	q = i/nn;
	r = i-q*nn;
	max = pow(sizex,q)*pow(sizey,r);
	polymap[y][x][i] = pow(xx,q)*pow(yy,r)/max;
      }
      polymap[y][x][0]=1;
    }
    
    for (i=0;i<n;i++)
      res[i]=polymap[y][x][i+1];	
    
  } else {   /* allocation failed */
    
    printf ("%s\n","polybase : basemap allocation failed ; the program will run slowly");
    
    for(i=0;i<n;i++) {
      nn = sqrt(n);
      
      q = i/nn;
      r = i-q*nn;
      max = pow(sizex,q)*pow(sizey,r);
      res[i] = pow(xx,q)*pow(yy,r)/max;      
    }
  }
}
 

void diffpolybase  (int y, int x, double *res, int n) {

  double tadd[n+1],tsub[n+1];
  
  int i,x2,y2,x1,y1;

  x1=x;
  y1=y;

  if (x<sizex) {
    x2 = (x==0) ? (sizex-1) : (x-1);
    y2 = y;
  } else {
    
    x1=x-sizex;
    x2=x1;
    y2 = (y==0) ? (sizey-1) : (y-1);
  }
  
  polybase (y1,x1,tadd,n+1);
  polybase (y2,x2,tsub,n+1);


  for(i=0;i<n;i++) {
    res[i] = tadd[i+1]-tsub[i+1];
  }
}


void FreeModels () {
  if (cosmapsize != 0) Free3Dim ((void ***)cosmap,sizey,sizex);
  if (polymapsize != 0) Free3Dim ((void ***)polymap,sizey,sizex);
}

