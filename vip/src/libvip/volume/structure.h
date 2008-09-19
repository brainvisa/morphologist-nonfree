/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/structure.h      * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 19/01/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : jfmangin@cea.fr
 *
 *****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *              / /   |              |
 *****************************************************************************/


#ifndef VIP_STRUCTURE_H
#define VIP_STRUCTURE_H


/* volume limitation */

#define VIP_IMAGE_MAXSIZE	100000		/* maximum of (Volume*)->size. */
#define VIP_BORDER_MAXWIDTH	16		/* maximum of (Volume*)->borderWidth */
#define VIP_VOXEL_MAXSIZE_MM 100. /*maximum for voxel dimension in mm*/
#define VIP_VOXEL_MAXSIZE_S  1000. /*maximum for time step in seconde*/
#define VIP_NAME_MAXLEN	1024U		/* maximum of strlen((Volume*)->name) */
#define NAME_MAXLEN	1024U		/* same thing: tivoli define*/

/* boolean values */

#ifndef VTRUE
#define VTRUE	1
#endif
#ifndef VFALSE
#define VFALSE	0
#endif

#ifndef YES
#define YES     1
#endif
#ifndef NO
#define NO      0
#endif
/* value returned by testing functions */

#ifndef OK
#define OK	1
#endif
#ifndef PB
#define PB	0
#endif

/* flag modes */

#ifndef ON
#define ON	1
#endif
#ifndef OFF
#define OFF	0
#endif

/* volume structure (should be forgotten...)*/

#include <vip/volume/struct_volume.h>

/* constant for the field (Volume*)->type */

#define U8BIT  10		/* 8 bit unsigned integer */
#define S8BIT  11		/* 8 bit signed integer */
#define U16BIT 20		/* 16 bit unsigned integer */
#define S16BIT 21		/* 16 bit signed integer */
#define U32BIT 40		/* 32 bit unsigned integer */
#define S32BIT 41		/* 32 bit signed integer */
#define VFLOAT  400		/* simple precision real */
#define VDOUBLE 800		/* double precision real */

#define ANY_TYPE 1111
#define ANY_INT  1110
#define ANY_FLOAT 1000

/* corresponding C declaration type */

typedef unsigned char	Vip_U8BIT;
typedef signed char				Vip_S8BIT;
typedef unsigned short	Vip_U16BIT;
typedef signed short				Vip_S16BIT;
typedef unsigned long	Vip_U32BIT;
typedef signed long				Vip_S32BIT;
typedef float				Vip_F32BIT;
typedef float				Vip_FLOAT;
typedef double				Vip_F64BIT;
typedef double				Vip_DOUBLE;

typedef char			Vip_ANY;	/* any type above */

#endif /*VIP_STRUCTURE_H*/

