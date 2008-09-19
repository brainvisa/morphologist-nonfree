#ifndef NDEF_PARAM
#define NDEF_PARAM 0
#include <vip/epidistorsion/param.h>
#endif

double corrmax3D (double ***map, int throw, int keep, int s_z, int s_y, int s_x);

void GetMask (double ***res, double ***m, double noise_threshold, int contrast, param_mat *par);

int Segmentation (double ***mask, double ***image, param *par, param_mat *par_echo);

int GetSegmentedMask (double ***res, param *par, param_mat *par_echo);
