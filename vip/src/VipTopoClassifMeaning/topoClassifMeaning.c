/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipTopoClassifMeaning.c * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 14/04/1997
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultrasparc
 ****************************************************************************
 *
 * DESCRIPTION : 
 *
 ****************************************************************************
 *
 * USED MODULES : 
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/




/*##############################################################################

	I N C L U S I O N S

##############################################################################*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <vip/volume.h>
#include <vip/topology.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  int value = -34567;
  int all = VFALSE; 
  int i;



  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {
      if (!strncmp (argv[i], "-value", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  value = atoi(argv[i]);
	}
      else if (!strncmp (argv[i], "-all", 2)) 
	{
	  all = VTRUE;
	}
      else if (!strncmp (argv[i], "-help",2)) Help();
      else Usage();
 
    }

  /*check that all required arguments have been given*/

  if ((value==-34567)&&(all==VFALSE))
    {
      VipPrintfError("value arg is required by VipTopoClassifMeaning");
      return(Usage());
    }    
  else if (value!=-34567) printf(" Meaning of %d: %s.\n",value,VipTopologicalStringFromDefine(value) );

  if(all==VTRUE)
    {
      printf("Topological classification code:\n");
      value = TOPO_A;
      printf(" %d: %s,\n",value,VipTopologicalStringFromDefine(value) );
      value = TOPO_B;
      printf(" %d: %s,\n",value,VipTopologicalStringFromDefine(value) );
      value = TOPO_C;
      printf(" %d: %s,\n",value,VipTopologicalStringFromDefine(value) );
      value = TOPO_D;
      printf(" %d: %s,\n",value,VipTopologicalStringFromDefine(value) );
      value = TOPO_E;
      printf(" %d: %s,\n",value,VipTopologicalStringFromDefine(value) );
      value = TOPO_F;
      printf(" %d: %s,\n",value,VipTopologicalStringFromDefine(value) );
      value = TOPO_G;
      printf(" %d: %s,\n",value,VipTopologicalStringFromDefine(value) );
      value = TOPO_H;
      printf(" %d: %s,\n",value,VipTopologicalStringFromDefine(value) );
      value = TOPO_I;
      printf(" %d: %s.\n",value,VipTopologicalStringFromDefine(value) );
    }


  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipTopoClassifMeaning\n");
  (void)fprintf(stderr,"        -v[alue] {int value}\n");
  (void)fprintf(stderr,"        -a[ll]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Gives the meaning of a topological classification value");
  (void)printf("\n");
  (void)printf("Usage: VipTopoClassifMeaning\n");
  (void)printf("        -v[alue] {int value}\n");
  (void)printf("        -a[ll]\n");
  (void)printf("        [-h[elp]\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
