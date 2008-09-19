/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : vip/nomen.h          * TYPE     : Header
 * AUTHOR      : Frouin V.            * CREATION : 20/09/1999
 * VERSION     : 1.5                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC
 *****************************************************************************
 *
 * DESCRIPTION : header for Nomenclature management
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


#ifndef VIP_NOMEN_DEF_H
#define VIP_NOMEN_DEF_H

#ifdef __cplusplus
  extern "C" {
#endif

/* general includes -------------------------------------------------------  */


/* application includes ---------------------------------------------------  */


/* extern declaration   ---------------------------------------------------  */
#define NO_NAME " No Name"
#define VIP_NOMEN_INIT_INCR 16

typedef struct {
  int    size;
  int    capacity;
  int    *datalabel;
  char   **dataname;
} VipNomenList;


#ifdef __cplusplus
  }
#endif

#endif
