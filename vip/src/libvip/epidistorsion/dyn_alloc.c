#include <stdlib.h>
#include <vip/epidistorsion/dyn_alloc.h>

/* -----------------------------------------------
	procedures pour l'allocation dynamique
-------------------------------------------------- */

/* -----------------------------------------------
	Alloc1Dim(): allocation dynamic d'un tableau de 1 dimension

declaration:
	<type> *T;
appelle:
  Alloc1Dim ((void **)&T, sizeof(<type>), len);
utilisation:
  T[i] = x;

*/

int Alloc1Dim (void ** T, 	/* OUT */
			size_t size_type, /* IN */
			int lendim)    /* IN */
{
  if (((int)size_type <= 0) || (lendim <= 0)) return ERROR;
  (*T) = (void *) calloc ((size_t)lendim, size_type);
  if ((*T) == NULL) return ERROR;
  return OK;
}


/* -----------------------------------------------
	Alloc2Dim(): allocation dynamic d'un tableau de 2 dimensions
declaration:
			<type> **T;
appelle:
			Alloc2Dim ((void ***) &T, sizeof(type), len1dim, len2dim);
utilisation:
			*(*(T+i)+j) = x;

*/

int Alloc2Dim (void ***T, 	/* OUT */
			size_t size_type,	/* IN */
			int len1dim,	/* IN */
			int len2dim) 	/* IN */
{
int i;

  if (((int)size_type <= 0) || (len1dim <= 0) || (len2dim <= 0)) return ERROR;	

  (*T) = (void **) calloc ((size_t)len1dim, sizeof(void *));
  if ((*T) == NULL) return ERROR;

  for (i=0; i<len1dim; i++)
	{
	  if (Alloc1Dim ((*T)+i, size_type, len2dim) == ERROR)
	  	{
		Free2Dim ((*T), i);
	  	return ERROR;
	  	}
	}
  return OK;
}



/* -----------------------------------------------
	Alloc3Dim(): allocation dynamic d'un tableau de 3 dimensions
declaration:
			<type> ***T;
appelle:
			Alloc3Dim ((void ****) &T, sizeof(<type>), len1dim, len2dim, len3dim);
utilisation:
			*(*(*(T+i)+j)+k)
*/

int Alloc3Dim (void ****T, 	/* OUT */
			size_t size_type,	/* IN */
			int len1dim, 	/* IN */
			int len2dim,	/* IN */
			int len3dim)	/* IN */
{
int i;

  if (((int)size_type <= 0) || (len1dim <= 0) 
	|| (len2dim <= 0) || (len3dim <= 0)) return ERROR;	

  (*T) = (void ***) calloc ((size_t)len1dim, sizeof(void **));
  if ((*T) == NULL) return ERROR;

  for (i=0; i<len1dim; i++)
	{
 	if (Alloc2Dim ((*T)+i, size_type, len2dim, len3dim) == ERROR) 
		{
		Free3Dim ((*T), i, len2dim);
		return ERROR;
		}
	}
  return OK;
}


/* -----------------------------------------------
	Alloc4Dim(): allocation dynamic d'un tableau de 4 dimensions
declaration:
			<type> ****T;
appelle:
			Alloc4Dim ((void *****) &T, sizeof(<type>), len1dim, len2dim, len3dim, len4dim);
utilisation:
			*(*(*(*(T+i)+j)+k)+l)
*/

int Alloc4Dim (void *****T, 	/* OUT */
			size_t size_type,	/* IN */
			int len1dim, 	/* IN */
			int len2dim,	/* IN */
			int len3dim,	/* IN */
			int len4dim)	/* IN */
{
int i;

  if (((int)size_type <= 0) || (len1dim <= 0) 
	|| (len2dim <= 0) || (len3dim <= 0) || (len4dim <= 0)) return ERROR;	

  (*T) = (void ****) calloc ((size_t)len1dim, sizeof(void ***));
  if ((*T) == NULL) return ERROR;

  for (i=0; i<len1dim; i++)
	{
	if (Alloc3Dim ((*T)+i, size_type, len2dim, len3dim, len4dim) == ERROR) 
		{
		Free4Dim ((*T), i, len2dim, len3dim);
		return ERROR;
		}	
	}
  return OK;
}


/* -----------------------------------------------
	Free1Dim():
		liberation de memoire du tableau alloue par Alloc1Dim()
*/

void Free1Dim (void *T)
{
  free (T);
}


/* -----------------------------------------------
	Free2Dim():
		liberation de memoire du tableau alloue par Alloc2Dim()
*/

void Free2Dim (void **T, 	/* IN / OUT */
			int lendim)	/* IN */
{
int i;

  for (i=0; i<lendim; i++)
	Free1Dim (*(T+i));
  Free1Dim (T);
}


/* -----------------------------------------------
	Free3Dim():
		liberation de memoire du tableau alloue par Alloc3Dim()
*/

void Free3Dim (void ***T, 	/* IN / OUT */
				int len1dim,	/* IN */
				int len2dim)	/* IN */
{
int i;

  for (i=0; i<len1dim; i++)
	Free2Dim (*(T+i), len2dim);
  Free1Dim (T);
}

/* -----------------------------------------------
	Free4Dim():
		liberation de memoire du tableau alloue par Alloc4Dim()
*/

void Free4Dim (void ****T, 	/* IN / OUT */
			int len1dim,	/* IN */
			int len2dim,	/* IN */
			int len3dim)	/* IN */
{
int i;

  for (i=0; i<len1dim; i++)
	Free3Dim (*(T+i), len2dim, len3dim);
  Free1Dim (T);
}

/* -----------------------------------------------
        Realloc1Dim();
	       reallocation dynamique d'un tableau alloue par Alloc1Dim()
*/

int Realloc1Dim (void **T,              /* OUT/IN */
		        size_t size_type,       /* IN */
		        int lendim)             /* IN */
{
  int length = lendim * ((int) size_type);
  
  if (((int)size_type <= 0) || (lendim <= 0)) return ERROR;
  (*T) = (void *) realloc (*T,(size_t)length);
  if ((*T) == NULL) return ERROR;
  return OK;  
}
