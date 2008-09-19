/* fourier.c
   Fourier transform functions

   According to Numer.Recip.			F.H. 27.10.93
----------------------------------------------------------------------- */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <vip/epidistorsion/fourier.h>


#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr;

static void four1(double *data, int nn, int isign);





void fft(double *dat, int n)
/*-----------------------------------------------------
  Forward FFT - Interface to four1(). 
  dat - pointer to complex data vector: re, im, re, im ..
  n -   number of complex points, must be a power of 2
  No scrolling necessary afterwards.
  ----------------------------------------------------*/
{
  int i;

  for(i=2;i<2*n;i+=4)
  {
	dat[i] *= -1.0;
	dat[i+1] *= -1.0;
  }

  four1(dat-1,n,1);
}

void ifft(double *dat, int n)
/*-----------------------------------------------------
  Inverse FFT - Interface to four1(). 
  dat - pointer to complex data vector: re, im, re, im ..
  n -   number of complex points, must be a power of 2
  ----------------------------------------------------*/
{
  int i;
  
  four1(dat-1,n,-1);
  
  for(i=2;i<2*n;i+=4)
  {
	dat[i] *= -1.0;
	dat[i+1] *= -1.0;
  }
 
}

int rfft(double *dat, int n)
/*--------------------------------------------------------
  Calculates ft of a real function. Uses four1().
  At input dat contains n real numbers.
  At output dat contains n/2 complex numbers
  giving the positive half of the complex spectrum.
  n is number of real points and must be power of 2.
  Returns 0-ok, 1-alloc error
  --------------------------------------------------------*/
{
  int i;
  double *buf;
  
  if((buf = (double*) calloc(2*n, sizeof(double))) == NULL)
    return(1);
  for(i=0; i<n; i++)
  {
    buf[2*i] = dat[i];
    buf[2*i+1] = 0.0;
  }
  four1(buf-1,n,1);
  for(i=0; i<n; i++)
    dat[i] = buf[i]/100.0;	/* divided by 100 because of error*/
  free(buf);
  return(0);
}

static void four1(double *data, int nn, int isign)
/*------------------------------------------------------
  Numerical Recipies in C, p.411
  data -  complex data: re,im,re,im. Indexing 1..nn;
  nn -    number of complex points
  isign - +1forward and scaling (1/nn), -1inverse fft,
                                         no scaling.
  WARNING : NO SCALING PROCESSED
  -----------------------------------------------------*/
{
  int n, mmax, m, j, istep, i;
  double wtemp, wr, wpr, wpi,wi,theta;
  double tempr,tempi;

  n = nn<<1;
  j=1;
  for(i=1; i<n; i+=2)
  {
    if(j>i)
    {
	SWAP(data[j],data[i]);
	SWAP(data[j+1],data[i+1]);
    }
    m = n >> 1;
    while( m >= 2 && j>m )
    {
	j -= m;
	m >>= 1;
    }
    j += m;
  }
  mmax = 2;
  while(n > mmax)
  {
    istep = 2*mmax;
    theta = 6.28318530717959/(isign*mmax);
    wtemp = sin(0.5 * theta);
    wpr   = -2.0*wtemp*wtemp;
    wpi   = sin(theta);
    wr    = 1.0;
    wi    = 0.0;
    for(m=1; m<mmax; m+=2)
    {
	for(i=m;i<=n; i+=istep)
	{
	  j = i+ mmax;
	  tempr = wr*data[j] - wi*data[j+1];
	  tempi = wr*data[j+1] + wi*data[j];
	  data[j] = data[i] - tempr;
  	  data[j+1] = data[i+1] - tempi;
	  data[i] += tempr;
	  data[i+1] += tempi;
	}
	wr = (wtemp = wr) * wpr - wi*wpi + wr;
	wi = wi*wpr + wtemp*wpi + wi;
    }
    mmax = istep;
  }
} /* end of four1() */
			 

void ft_3rd_dim(double ***dat, int size2, int size1, int size0)
/*--------------------------------------------------------------------
  Fourier transform in third dimension, 
  ie along last-but-two index (size2)
  data matrix: dat[0..size2-1][0..size1-1][0..2*size0-1]
  -------------------------------------------------------------------*/
{
  int m,i,k;
  double *copy;
  
  if(size2<8)
    return;

  if((copy = calloc(size2*2, sizeof(double))) == NULL) {
    printf("%s\n","alloc failed in ft_3rd_dim");
  }
  
  for(i=0; i<size1; i++) {
    for(k=0; k<2*size0; k+=2) {
      for(m=0; m<size2; m++) {
	copy[2*m]   = dat[m][i][k];
	copy[2*m+1] = dat[m][i][k+1];
      }
      fft(copy, size2);
      
      for(m=0; m<size2; m++) {
	dat[m][i][k]   = copy[2*m];
	dat[m][i][k+1] = copy[2*m+1];
      }
    }
  }
  free(copy);
}


void ft_2nd_dim(double ***dat, int size2, int size1, int size0)
/*--------------------------------------------------------------------
  Fourier transform in second dimension, ie along last-but-one index (size1)
  data matrix: dat[0..size2-1][0..size1-1][0..2*size0-1]
  -------------------------------------------------------------------*/
{
  int m,i,k;
  double *copy;
 
  if((copy = calloc(size1*2, sizeof(double))) == NULL) {
    printf("%s\n","alloc failed in ft_2nd_dim");
  }
  
  for(m=0; m<size2; m++) {
    for(k=0; k<2*size0; k+=2) {
      for(i=0; i<size1; i++) {
	copy[2*i]   = dat[m][i][k];
	copy[2*i+1] = dat[m][i][k+1];
      }
      fft(copy, size1);
      
      for(i=0; i<size1; i++) {
	dat[m][i][k]   = copy[2*i];
	dat[m][i][k+1] = copy[2*i+1];
      }
    }
  }
  free(copy);
  
}

void ift_2nd_dim(double ***dat, int size2, int size1, int size0)
/*--------------------------------------------------------------------
  Inverse Fourier transform in second dimension (size1)
  data matrix: dat[0..size2-1][0..size1-1][0..2*size0-1]
  -------------------------------------------------------------------*/
{
  int m,i,k;
  double *copy;
 
  if((copy = calloc(size1*2, sizeof(double))) == NULL) {
    printf("%s\n","alloc failed in ft_2nd_dim");
  }
  for(m=0; m<size2; m++) {
   for(k=0; k<2*size0; k+=2) {
     for(i=0; i<size1; i++) {
       copy[2*i]   = dat[m][i][k];
       copy[2*i+1] = dat[m][i][k+1];
     }
     ifft(copy, size1);
     
     for(i=0; i<size1; i++) {
       dat[m][i][k]   = copy[2*i];
       dat[m][i][k+1] = copy[2*i+1];
     }
   }
  }   
  free(copy);
  
}


void ft_1st_dim(double ***dat, int size2, int size1, int size0)
/*--------------------------------------------------------------------
  Fourier transform in first dimension (along last index, size0)
  data matrix: dat[0..size2-1][0..size1-1][0..2*size0-1]
  -------------------------------------------------------------------*/
{
  int i,k;
 
  for(k=0; k<size2; k++) {
    for(i=0; i<size1; i++) {
      fft((double*) dat[k][i],size0);
    }
  }
}

void ift_1st_dim(double ***dat, int size2, int size1, int size0)
/*--------------------------------------------------------------------
  Inverse Fourier transform in first dimension (along last index, size0)
  data matrix: dat[0..size2-1][0..size1-1][0..2*size0-1]
  -------------------------------------------------------------------*/
{
  int i,k;
  for(k=0; k<size2; k++)
    for(i=0; i<size1; i++)
	ifft((double*) dat[k][i],size0);
}
 
