#ifndef NDEF_PARAM
#define NDEF_PARAM 0
#include <vip/epidistorsion/param.h>
#endif


void gravity_center (double ***result, double ***modmap, param_mat *par);

void shifted_gravity_center (double ***result, double ***modmap, param_mat *par, int slice,
			     double ***shift_map, int shift, double cst);
