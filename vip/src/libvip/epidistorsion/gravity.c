#include <stdio.h>
#include <stdlib.h>

#include <vip/epidistorsion/gravity.h>

void gravity_center (double ***result, double ***modmap, param_mat *par) {
/* -----------------------------------------------------------
   result[k][i][0] = the gravity center of column [k][.][i]
   result[k][i][1] = the intensity sum of that column
   ----------------------------------------------------------- */
  int s_x = par->s_x;
  int s_y = par->s_y;
  int s_z = par->s_z;
  int i,j,k;
  double sum, weights_sum;

  for(k=0; k<s_z; k++)
    for(i=0; i<s_x; i++) {
      
      sum = 0;
      weights_sum =0;
      for (j=0;j<s_y;j++) {
	sum += (modmap[k][j][i]*j);
	weights_sum += modmap[k][j][i];
      }
      result[k][i][0] = (weights_sum > 0.1) ? sum*100/(weights_sum*s_y) : 0;
      result[k][i][1] = weights_sum;
    }
}

void shifted_gravity_center (double ***result, double ***modmap, param_mat *par, int slice,
			     double ***shift_map, int shift, double cst) {
/* ---------------------------------------------------
   same as above for ONE slice,
   except that the modmap(point) is shifted by
   shift*cst + shiftmap(point), along the y-axis;
   --------------------------------------------------- */
  int s_x = par->s_x;
  int s_y = par->s_y;
  int i,j,k;
  double sum, weights_sum, index;
  k = slice;

    for(i=0; i<s_x; i++) {
      
      sum = 0;
      weights_sum =0;
      for (j=0;j<s_y;j++) {
	index = cst*shift + shift_map[k][j][i] + j;
	sum += (modmap[k][j][i]*index);
	weights_sum += modmap[k][j][i];
      }
      result[k][i][0] = (weights_sum>0.1) ? (sum*100)/(weights_sum*s_y) : 0;
      result[k][i][1] = weights_sum;
    }
}
 
