#define XY 3
#define XYZ 7

int Dilate (double ***mask, int s_z, int s_y, int s_x,
	    double ***kernel, int k_sz, int k_sy, int k_sx);
int Erode  (double ***mask, int s_z, int s_y, int s_x,
	    double ***kernel, int k_sz, int k_sy, int k_sx);

int Connect3D (double ***mask, int max_nb_components, int s_z, int s_y, int s_x);

void BuildKernel (double ***kernel, int s_z, int s_y, int s_x,int dimension,
		  double r_z, double r_y, double r_x);

int Erosion (double ***mask, int s_z, int s_y, int s_x,
	     double r_z, double r_y, double r_x);

int Dilatation (double ***mask, int s_z, int s_y, int s_x,
		double r_z, double r_y, double r_x);
