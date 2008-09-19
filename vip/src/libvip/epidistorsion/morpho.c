#include <stdio.h>
#include <stdlib.h>

#include <vip/epidistorsion/dyn_alloc.h>
#include <vip/epidistorsion/morpho.h>
#include <vip/epidistorsion/utils.h>
#include <vip/epidistorsion/convolution.h>

#define DILATE 1
#define ERODE 0

#define MORPHO_DEBUG 0
#define KERNEL_DEBUG 0

int dilateErode (double ***mask, int s_z, int s_y, int s_x,
	    double ***kernel, int k_sz, int k_sy, int k_sx, int direction) {
  /* ----------------------------------------------------------------------
     Dilatation of the given mask ;
     The size of the dilatation is given by the kernel
     direction :
      1 : Dilatation
     -1 : Erosion
     The kernel must contain only positive values, bigger than 0.001
     ---------------------------------------------------------------------- */
  int i,j,k;
  double threshold;

  if (Convolution(mask, s_z, s_y,s_x,kernel,k_sz,k_sy,k_sx) == ERROR) {
    return ERROR;
  }

  threshold = 0;
  for (k=0;k<k_sz;k++)
    for (j=0;j<k_sy;j++)
      for (i=0;i<k_sx;i++)
	threshold += kernel[k][j][i];
  
  if (direction == DILATE) 
    boxcar (mask, 1, 0.001, s_z, s_y, s_x);
  else 
    boxcar (mask, 1, threshold-0.001, s_z, s_y, s_x);
  
  return OK;
}


int Dilate (double ***mask, int s_z, int s_y, int s_x,
	    double ***kernel, int k_sz, int k_sy, int k_sx) {
  /* ----------------------------------------------------- */
  return (dilateErode (mask, s_z, s_y, s_x, kernel, k_sz, k_sy, k_sx, DILATE));
}

int Erode (double ***mask, int s_z, int s_y, int s_x,
	   double ***kernel, int k_sz, int k_sy, int k_sx) {
  /* ----------------------------------------------------- */
  return (dilateErode (mask, s_z, s_y, s_x, kernel, k_sz, k_sy, k_sx, ERODE));
}

void new_entry (int **lifo_list, int index,int k,int j, int i) {
  lifo_list[index][0]=k;
  lifo_list[index][1]=j;
  lifo_list[index][2]=i;
}

int Connect3D (double ***mask, int max_nb_components, int s_z, int s_y, int s_x) {

     /* ------------------------------------------------
	Extracts the biggest 3D-connected component
	of the image
	There must be no more than max_nb_components
	connex components in the image
	------------------------------------------------ */
  int i,j,k,n,i2,j2,k2;
  int ***map;
  int **lifo_list;
  int index = -1;
  int comp_size[max_nb_components+1];
  int current_component = 0;

  if ( (Alloc3Dim ((void ****)&map,sizeof(int),s_z,s_y,s_x) == ERROR)
       ||(Alloc2Dim ((void ***)&lifo_list,sizeof(int), s_z*s_y*s_x, 3) == ERROR))
   {
      return ERROR;
    }
  else
    {
      for (i=0;i<=max_nb_components;i++) comp_size[i]=0;

      for (i=0;i<s_x;i++)
	for (j=0;j<s_y;j++)
	  for (k=0;k<s_z;k++)
	    map[k][j][i]=0;

      for (k=0;k<s_z;k++)
 	for (j=0;j<s_y;j++)
	  for (i=0;i<s_x;i++)
	    if ((mask[k][j][i]>0.5)&&(map[k][j][i]==0))
	      {
		current_component++;
#if MORPHO_DEBUG
		printf("Current_component %d\n",current_component);
#endif
		map[k][j][i]=current_component;
		index=0;
		new_entry(lifo_list,index,k,j,i);
		
		while (index>=0) {
		  n=index;
		  index--;
		  k2=lifo_list[n][0];
		  j2=lifo_list[n][1];
		  i2=lifo_list[n][2];
#if MORPHO_DEBUG
		  printf("k2 %d j2 %d i2 %d index %d\n",k2,j2,i2,index);
#endif
		  if ((k2>0)&&(mask[k2-1][j2][i2]>0.5)&&(map[k2-1][j2][i2]==0)) {
		    map[k2-1][j2][i2]=current_component;
		    comp_size[current_component]++;
		    index++;
		    new_entry(lifo_list,index,k2-1,j2,i2); }
#if MORPHO_DEBUG
		  printf(" 1");
#endif
		  if ((j2>0)&&(mask[k2][j2-1][i2]>0.5)&&(map[k2][j2-1][i2]==0)) {
		    map[k2][j2-1][i2]=current_component;
		    comp_size[current_component]++;
		    index++;
		    new_entry(lifo_list,index,k2,j2-1,i2); }
#if MORPHO_DEBUG
		  printf("2");
#endif	
		  if ((i2>0)&&(mask[k2][j2][i2-1]>0.5)&&(map[k2][j2][i2-1]==0)) {
		    map[k2][j2][i2-1]=current_component;
		    index++;
		    comp_size[current_component]++;
		    new_entry(lifo_list,index,k2,j2,i2-1); }
#if MORPHO_DEBUG
		  printf("3");
#endif	
		  if ((k2<s_z-1)&&(mask[k2+1][j2][i2]>0.5)&&(map[k2+1][j2][i2]==0)) {
		    map[k2+1][j2][i2]=current_component;
		    comp_size[current_component]++;
		    index++;
		    new_entry(lifo_list,index,k2+1,j2,i2); }
#if MORPHO_DEBUG
		  printf("4");
#endif	
		  if ((j2<s_y-1)&&(mask[k2][j2+1][i2]>0.5)&&(map[k2][j2+1][i2]==0)) {
		    map[k2][j2+1][i2]=current_component;
		    comp_size[current_component]++;
		    index++;
		    new_entry(lifo_list,index,k2,j2+1,i2); }
#if MORPHO_DEBUG
		  printf("5  ");
#endif			  
		  if ((i2<s_x-1)&&(mask[k2][j2][i2+1]>0.5)&&(map[k2][j2][i2+1]==0)) {
		    map[k2][j2][i2+1]=current_component;
		    comp_size[current_component]++;
		    index++;
		    new_entry(lifo_list,index,k2,j2,i2+1); }
#if MORPHO_DEBUG
		  printf("index %d\n",index);
#endif			  
		}
	      }
      n = indexmax(comp_size,1,current_component);

      for (i=0;i<s_x;i++)
	for (j=0;j<s_y;j++)
	  for (k=0;k<s_z;k++)
	    mask[k][j][i] = (map[k][j][i]==n) ? 1 : 0;
      
      Free3Dim((void ***)map,s_z,s_y);
      Free2Dim((void **)lifo_list,s_z*s_y*s_x);
      return OK;
    } 
}


void BuildKernel (double ***kernel, int s_z, int s_y, int s_x, int dimension, double r_z, double r_y, double r_x) {
  /* -----------------------------------------------------------------------------------
     Builds an ellipsoid kernel of radiuses r_z, r_y, r_x within the matrix.
     you must have : 
     s_i >= 2* r_i +1
     r_i,r_j,r_k > 0 (unit : pixels)
     dimension is the mask of the dimensions that should be used (value between 1 and 7)
     order : zyx (example : dimension = 2 = 010 (binary) means that it is a 1D kernel
     along the y axis)
     Unused radiuses must be set to 0
     ----------------------------------------------------------------------------------- */
  int i,j,k;
  double di,dj,dk,r2z,r2y,r2x,temp;
  int c_x,c_y,c_z;

  c_x=(int) (roundvalue(r_x));
  c_y=(int) (roundvalue(r_y));
  c_z=(int) (roundvalue(r_z));

  r2z = r_z*r_z;
  r2y = r_y*r_y;
  r2x = r_x*r_x;

#if KERNEL_DEBUG
	  printf ("r2x %f, r2y %f, r2z %f\n", r2x,r2y,r2z);
#endif

  for (k=0;k<s_z;k++) 
    for (j=0;j<s_y;j++) 
      for (i=0;i<s_x;i++)
	{
	  di=i-c_x; dj=j-c_y; dk=k-c_z;
#if KERNEL_DEBUG
	  printf ("k %d, j %d, i %d, dz %f, dy %f, dx %f,",k,j,i,dk,dj,di);
#endif  

	  temp = 0.0;
	  if ((dimension & 1) == 1) temp +=  di*di/r2x;
	  if ((dimension & 2) == 2) temp +=  dj*dj/r2y;
	  if ((dimension & 4) == 4) temp +=  dk*dk/r2z;

#if KERNEL_DEBUG
	  printf ("temp %f\n",temp);
#endif
	  kernel[k][j][i] = (temp<=1.0) ? 1.0 : 0.0;
	}
  kernel[c_z][c_y][c_x] = 1;
}


int Erosion (double ***mask, int s_z, int s_y, int s_x, double r_z, double r_y, double r_x) {
  /* ---------------------------------------------------------------------------------------- */
  int k_sz,k_sy,k_sx;
  double ***kernel;
  int flag = 0;

  k_sz = (int) (roundvalue(r_z));
  k_sy = (int) (roundvalue(r_y));
  k_sx = (int) (roundvalue(r_x));

  k_sz = 2*k_sz + 1;
  k_sy = 2*k_sy + 1;
  k_sx = 2*k_sx + 1;

  if ((Alloc3Dim((void ****)&kernel,sizeof(double),k_sz,k_sy,k_sx))==ERROR) {
    printf ("Dilatation : could not allocate kernel\n");
    return ERROR;
  } else {
    
    BuildKernel (kernel, k_sz, k_sy, k_sx, XYZ, r_z, r_y, r_x);

    flag = Erode (mask,s_z,s_y,s_x,kernel,k_sz,k_sy,k_sx);

    Free3Dim((void ***)kernel,k_sz,k_sy);
    
    return flag;
  }
}

int Dilatation (double ***mask, int s_z, int s_y, int s_x, double r_z, double r_y, double r_x) {
  /* ---------------------------------------------------------------------------------------- */
  int k_sz,k_sy,k_sx;
  double ***kernel;
  int flag = 0;

  k_sz = (int) (roundvalue(r_z));
  k_sy = (int) (roundvalue(r_y));
  k_sx = (int) (roundvalue(r_x));

  k_sz = 2*k_sz + 1;
  k_sy = 2*k_sy + 1;
  k_sx = 2*k_sx + 1;

  if ((Alloc3Dim((void ****)&kernel,sizeof(double),k_sz,k_sy,k_sx))==ERROR) {
    printf ("Dilatation : could not allocate kernel\n");
    return ERROR;
  } else {
    
    BuildKernel (kernel, k_sz, k_sy, k_sx,XYZ, r_z, r_y, r_x);
    flag = Dilate (mask,s_z,s_y,s_x,kernel,k_sz,k_sy,k_sx);

    Free3Dim((void ***)kernel,k_sz,k_sy);
    return flag;
  }
}

