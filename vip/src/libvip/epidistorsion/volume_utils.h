#ifndef NDEF_PARAM
#define NDEF_PARAM 0
#include <vip/epidistorsion/param.h>
#endif

void Divide (double ***res,double ***a,double ***b,param_mat *par);


void rMultiply (double ***res,double ***x, double ***y,double factor,param_mat *par);
void cstMultiply (double ***res, double ***arg, double factor,param_mat *par);

void GetPhase (double ***res,double ***m, param_mat *par);

void GetMod (double ***res, double ***m, param_mat *par);

void Resize (double ***res, double ***m, param_mat *par_result, param_mat *par_m);

void ApplyMask (double ***m, double ***mask, param_mat *par);
