#ifndef NDEF_PARAM
#define NDEF_PARAM 0
#include <vip/epidistorsion/param.h>
#endif

int ReadParam(FILE *fid,param *par,param_acq *par_a,param_mat *par_echo,param_mat *par_epi);

int WriteParam(FILE *fid,param *par,param_acq *par_a,param_mat *par_echo,param_mat *par_epi);

void DefaultParam (param *par,param_acq *par_a,param_mat *par_echo,param_mat *par_epi);


