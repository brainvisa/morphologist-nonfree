/* constants and prototypes of functions in dyn_alloc.c */
#include <stddef.h>

#define OK 0
#define ERROR 1

/* ----- prototypes ----- */

int Alloc1Dim (void ** T, 		/* OUT */
			size_t size_type,   /* IN */
			int lendim);    	/* IN */
int Alloc2Dim (void ***T, 		/* OUT */
			size_t size_type,	/* IN */
			int len1dim,		/* IN */
			int len2dim); 		/* IN */
int Alloc3Dim (void ****T, 		/* OUT */
			size_t size_type,	/* IN */
			int len1dim, 		/* IN */
			int len2dim,		/* IN */
			int len3dim);		/* IN */
int Alloc4Dim (void *****T, 		/* OUT */
			size_t size_type,	/* IN */
			int len1dim, 		/* IN */
			int len2dim,		/* IN */
			int len3dim,		/* IN */
			int len4dim);		/* IN */
void Free1Dim (void *T);			/* IN / OUT */
void Free2Dim (void **T, 		/* IN / OUT */
			int len1dim);		/* IN */
void Free3Dim (void ***T, 		/* IN / OUT */
			int len1dim,		/* IN */
			int len2dim);		/* IN */
void Free4Dim (void ****T, 		/* IN / OUT */
			int len1dim,		/* IN */
			int len2dim,		/* IN */
			int len3dim);		/* IN */

int Realloc1Dim (void **T,              /* OUT/IN */
		        size_t size_type,       /* IN */
		        int lendim);            /* IN */
