/************************************************************************/
/*																		*/
/* Author			Philippe Thevenaz									*/
/* Affiliation		Ecole polytechnique federale de Lausanne			*/
/*					DMT/IOA/BIG											*/
/*					P.O. Box 127										*/
/*					CH-1015 Lausanne									*/
/*					Switzerland											*/
/* Telephone		+41(21)693.51.89									*/
/* Telefax			+41(21)693.37.01									*/
/* Email			philippe.thevenaz@epfl.ch							*/
/*																		*/
/************************************************************************/

/************************************************************************/
/* Public implementation follows										*/
/************************************************************************/

/*--- Defines ----------------------------------------------------------*/

#undef				FALSE
#define				FALSE			((int)(0 != 0))

#undef				TRUE
#define				TRUE			((int)(!FALSE))

#undef				ERROR
#define				ERROR			((int)FALSE)
/*jeff*/
/*--- Types ------------------------------------------------------------*/

struct	trsfStruct {
  double			dx[3];
  double			skew[3][3];
};


int					affineTransformSHFJ	(double				transform[][4],
									 double				origin[],
									 float				*inPtr,
									 float				*outPtr,
									 long				innx,
									 long				inny,
									 long				innz,
									 long				outnx,
									 long				outny,
									 long				outnz,
									 int				interpolationDegree,
									 float				background);
/************************************************************************/
/* Externals															*/
/*----------------------------------------------------------------------*/
/* access.c																*/
/************************************************************************/

/* access.c */
extern	void		errorReport		(char				*errorMessage);
