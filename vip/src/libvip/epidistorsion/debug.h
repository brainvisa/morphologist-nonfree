#ifndef NDEF_PARAM
#define NDEF_PARAM 0
#include <vip/epidistorsion/param.h>
#endif

void ConvertArg (unsigned char ***result, double ***argmap, param_mat *par);
void ConvertMod (unsigned char ***result, double ***argmap, param_mat *par);
void ConvertEpi (unsigned char ***result, double ***argmap, param_mat *par);

int WriteDebug (unsigned char ***result, char *filename,param_mat *par);
