#ifndef DYN_ALLOC
#define DYN_ALLOC 0
#include <vip/epidistorsion/dyn_alloc.h>
#endif

double cosinus (double x, int k, int n) ;
double sinus (double x, int k, int n) ;

void cosbase (int y,int x, double *res, int n) ;
void diffcosbase (int y,int x, double *res, int n) ;

void polybase (int y, int x, double *res, int n) ;
void diffpolybase  (int y, int x, double *res, int n) ;

void FreeModels ();
