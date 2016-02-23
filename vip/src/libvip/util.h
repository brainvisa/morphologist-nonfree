/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/util.h           * TYPE     : Header
 * AUTHOR      : MANGIN J.F.          * CREATION : 19/01/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : Diverses constantes et procedures relativement generales
 *               jfmangin@cea.fr
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



#ifndef VIP_UTIL_H
#define VIP_UTIL_H

#ifndef __has_feature
  #define __has_feature(x) 0  /* Compatibility with non-clang compilers. */
#endif
#ifndef CLANG_ANALYZER_NORETURN
  #if __has_feature(attribute_analyzer_noreturn)
    #define CLANG_ANALYZER_NORETURN __attribute__((analyzer_noreturn))
  #else
    #define CLANG_ANALYZER_NORETURN
  #endif
#endif

#ifdef __cplusplus
  extern "C" {
#endif

#include <math.h>
#include <vip/volume.h>
#include <vip/util/util_gen.h>
#include <vip/util/util2_gen.h>
#include <vip/util/slice.h>

/*A few constant for thresholding functions*/

#define GREATER_THAN 66
#define GREATER_OR_EQUAL_TO 67
#define EQUAL_TO 68
#define LOWER_THAN 69
#define LOWER_OR_EQUAL_TO 70
#define NOT_EQUAL_TO 71

#define VIP_BETWEEN 76
#define VIP_BETWEEN_OR_EQUAL_TO 77
#define VIP_OUTSIDE 78
#define VIP_OUTSIDE_OR_EQUAL_TO 79

#define BINARY_RESULT 88
#define GREYLEVEL_RESULT 89

#define RAW_TYPE_CONVERSION 11

#define LOG_NEP 12
#define LOG_TEN 13

#define VIP_MERGE_SAME_VALUES 94
#define VIP_MERGE_ONE_TO_ONE 95
#define VIP_MERGE_ALL_TO_ONE 96
#define VIP_MERGE_ALL_TO_MAXP1 97
#define VIP_MERGE_ONE_TO_MAXP1 98

#define FLIP_XX 111
#define FLIP_YY 112
#define FLIP_ZZ 113
#define FLIP_XY 114
#define FLIP_XZ 115
#define FLIP_YZ 116
#define FLIP_XXZZ 117
#define FLIP_YYZZ 118
#define FLIP_XXYYZZ 119

#define VIP_CL_ERROR 111

/*Definitions pour les gestions de fichier*/

/*----------------------------------------------------------------------------*/
FILE *VipOpenFile(
char *filename,
char *mode,
char *caller);
/*----------------------------------------------------------------------------*/

#define VIP_READ_TEXT          "r"
#define VIP_WRITE_TEXT         "w"
#define VIP_APPEND_TEXT        "a"
#define VIP_READ_TEXT_UPDATE   "r+"
#define VIP_WRITE_TEXT_UPDATE  "w+"
#define VIP_APPEND_TEXT_UPDATE "a+"
#define VIP_READ_BIN           "rb"
#define VIP_WRITE_BIN          "wb"
#define VIP_APPEND_BIN         "ab"
#define VIP_READ_BIN_UPDATE    "rb+"
#define VIP_WRITE_BIN_UPDATE   "wb+"
#define VIP_APPEND_BIN_UPDATE  "ab+"



/*Macros classiques*/

/*----------------------------------------------------------------------------*/

#define mVipMin(a,b)        ((a)<(b)?(a):(b))
#define mVipMax(a,b)        ((a)>(b)?(a):(b))
#define mVipRint(a)         ((int)((a)+0.5))
#define mVipSign(a)         (((a) < 0.0) ? -1.0 : 1.0)
#define mVipSq(a)           ((a)*(a))
#define mVipSq3(a)          (mVipSq(a)*(a))
#define mVipRootN(a,n)      (mVipSign(a)*pow(fabs(a), 1.0/(double)n))
#define mVipHypotenuse(a,b) sqrt(mVipSq(a)+mVipSq(b))

/*----------------------------------------------------------------------------*/

/*==============================*
 |                              |
 |  error processing functions  |
 |                              |
 *==============================*/

/*----------------------------------------------------------------------------*/
	extern
	void			VipPrintfExit
/*
	abort program execution after printing the name of the function
	where problems occure
*/
/*----------------------------------------------------------------------------*/
(
	const char		    *fname
) CLANG_ANALYZER_NORETURN;

/*----------------------------------------------------------------------------*/
	extern
	void			VipPrintfError
/*
	Vip formatted error printing	
*/
/*----------------------------------------------------------------------------*/
(
	const char		    *s
);

/*----------------------------------------------------------------------------*/
	extern
	void			VipPrintfWarning
/*
	Vip formatted warning	
*/
/*----------------------------------------------------------------------------*/
(
	const char		    *fname
);

/*----------------------------------------------------------------------------*/
	extern
	void			VipPrintfInfo
/*
	Vip formatted information
*/
/*----------------------------------------------------------------------------*/
(
	const char		    *fname
);

/*---------------------------------------------------------------*/
extern int VipComputeReversedSignVolume(Volume *vol);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
extern int VipComputeLogVolume(Volume *vol, int logmode);
/*---------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif

#endif /* VIP_UTIL_H */
