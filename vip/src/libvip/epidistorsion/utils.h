#define ABS(x)         ((x) < 0 ? -(x) : (x))

double roundvalue (double x);

int indexmax (int *array,int fst,int lst);

double max2D (double **map,int size_y, int size_x);
double max3D (double ***map,int size_z,int size_y,int size_x);
double min2D (double **map,int size_y, int size_x);
double min3D (double ***map,int size_z,int size_y,int size_x);


void hsort (double ra[], int n);
double mean (double vect[], int fst, int lst);

void cmod (double *res, double *z);
void cdiv (double* res, double *a, double *b);
void cmult (double* res, double *a, double *b);
void cphase (double *res,double *z);

double pvalue (double phi);

void copy3D (double ***res,double ***src, int s_z, int s_y, int s_x, int start_z, int start_y, int start_x);
void boxcar (double ***mask, int contrast, double threshold, int s_z, int s_y, int s_x);

double sum (double ***image, int s_z,int s_y,int s_x);

void avevar (double *ave, double *var, double ***data, int s_z, int s_y, int s_x);

