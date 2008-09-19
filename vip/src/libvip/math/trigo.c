/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_trigo.c          * TYPE     : Function
 * AUTHOR      : POUPON F.            * CREATION : 31/01/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Genere des tables de Sinus, Cosinus et Tangentes
 *
 ****************************************************************************
 *
 * USED MODULES : math.h - values.h - Vip_alloc.h - Vip_util.h - Vip_trigo.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <math.h>
#include <float.h>

#include <vip/alloc.h>
#include <vip/util.h>
#include <vip/math.h>

float *VipCreateCosinusTable(float nb_angle, float mul)
{ float *cosin, step=360.0/nb_angle, angle, *cosptr;
  char   message[256];
 
  cosin = (float *)VipCalloc((int)nb_angle+1, sizeof(float), NULL);
  if (!cosin)
  { sprintf(message, "Allocation error for Cos(%dt) table", (int)mul);
    VipPrintfError(message);
    VipPrintfExit("(Vip_trigo.c)VipCreateCosinusTable");
    return((float *)NULL);
  }

  for (cosptr=cosin, angle=0.0; angle<=nb_angle*step; angle+=step)
    *cosptr++ = cos(mul*angle*M_PI/180.0);

  return(cosin);
}

float *VipCreateSinusTable(float nb_angle, float mul)
{ float *sinus, step=360.0/nb_angle, angle, *sinptr;
  char   message[256];

  sinus = (float *)VipCalloc((int)nb_angle+1, sizeof(float), NULL);
  if (!sinus)
  { sprintf(message, "Allocation error for Sin(%dt) table", (int)mul);
    VipPrintfError(message);
    VipPrintfExit("(Vip_trigo.c)VipCreateSinusTable");
    return((float *)NULL);
  }

  for (sinptr=sinus, angle=0.0; angle<=nb_angle*step; angle+=step)
    *sinptr++ = sin(mul*angle*M_PI/180.0);

  return(sinus);
}

float *VipCreateTangenteTable(float nb_angle, float mul)
{ float *sinus, step=360.0/nb_angle, angle, *sinptr;
  char   message[256];

  sinus = (float *)VipCalloc((int)nb_angle+1, sizeof(float), NULL);
  if (!sinus)
  { sprintf(message, "Allocation error for Tan(%dt) table", (int)mul);
    VipPrintfError(message);
    VipPrintfExit("(Vip_trigo.c)VipCreateTangenteTable");
    return((float *)NULL);
  }

  for (sinptr=sinus, angle=0.0; angle<=nb_angle*step; angle+=step)
    *sinptr++ = tan(mul*angle*M_PI/180.0);

  return(sinus);
}

void VipFreeCosinusTable(float *table)
{ VipFree((void *)table);
}

void VipFreeSinusTable(float *table)
{ VipFree((void *)table);
}

void VipFreeTangenteTable(float *table)
{ VipFree((void *)table);
}
