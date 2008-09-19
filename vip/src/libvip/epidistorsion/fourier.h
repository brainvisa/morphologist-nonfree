/* fourier.h
   ___________________________________________________________________ */


void fft(double *dat, int n);
int rfft(double *dat, int n);
void ifft(double *dat, int n);

void ft_1st_dim(double ***dat, int size2, int size1, int size0);
void ft_2nd_dim(double ***dat, int size2, int size1, int size0);
void ift_1st_dim(double ***dat, int size2, int size1, int size0);
void ift_2nd_dim(double ***dat, int size2, int size1, int size0);
void ft_3rd_dim(double ***dat, int size2, int size1, int size0);
