#ifndef NDEF_PARAM
#define NDEF_PARAM 0
#include <vip/epidistorsion/param.h>
#endif

/* functions reading and writing volumes */

int ReadEcho (double ***echo1, double ***echo2, FILE* fin, param_mat *par);

int WriteMap (double ***result, FILE* fout, param_mat* par);
int ReadMap (double ***result, FILE* fin, param_mat* par);

int ReadEpi (double ***image, FILE* fin, param_mat *par);
int WriteEpi (double ***image, FILE* fout, param_mat *par);

