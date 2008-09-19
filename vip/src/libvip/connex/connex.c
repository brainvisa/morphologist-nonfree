/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_connex.c         * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 05/10/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 *****************************************************************************
 *
 * DESCRIPTION : tout ce qui concerne la connexite
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include <vip/util.h>
#include <vip/volume.h>
#include <vip/connex.h>
#include <vip/alloc.h>

/*----------------------------------------------------------------------------*/
int VipConvertStringToConnectivity(
char *thestring
)
/*----------------------------------------------------------------------------*/
{
  int connectivity;

  if(!thestring)
    {
      VipPrintfError("NULL string in VipConvertStringToConnectivity");
      VipPrintfExit("VipConvertStringToConnectivity");
      return(PB);
    }
  
  if(!strcmp(thestring,"4"))
    connectivity = CONNECTIVITY_4;
  else if(!strcmp(thestring,"4s"))
    connectivity = CONNECTIVITY_4_SAGITTAL;
  else if(!strcmp(thestring,"4c"))
    connectivity = CONNECTIVITY_4_CORONAL;
  else if(!strcmp(thestring,"8"))
    connectivity = CONNECTIVITY_8;
  else if(!strcmp(thestring,"8s"))
    connectivity = CONNECTIVITY_8_SAGITTAL;
  else if(!strcmp(thestring,"8c"))
    connectivity = CONNECTIVITY_8_CORONAL;
  else if(!strcmp(thestring,"6"))
    connectivity = CONNECTIVITY_6;
   else if(!strcmp(thestring,"18"))
    connectivity = CONNECTIVITY_18;
  else if(!strcmp(thestring,"26"))
    connectivity = CONNECTIVITY_26;
  else if(!strcmp(thestring,"2x26"))
    connectivity = CONNECTIVITY_2x26;
  else if(!strcmp(thestring,"3x26"))
    connectivity = CONNECTIVITY_3x26;
  else if(!strcmp(thestring,"4x26"))
    connectivity = CONNECTIVITY_4x26;
  else
    {
      VipPrintfWarning("Unknown connectivity string");
      return(PB);
    }

  return(connectivity);
 
}

/*----------------------------------------------------------------------------*/
VipConnectivityStruct *VipGetConnectivityStruct(
Volume *volume,
int connectivity)
/*----------------------------------------------------------------------------*/
{
	char error[1024];
	int nb_neighbors;
	VipConnectivityStruct *vcs;
	int xsize, slicesize;
	int i,j,k;
	int dist;

	if ( !VipVerifyAll ( volume ) )
	{
	 	VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
		return(NULL);
	}
	if ((connectivity!=CONNECTIVITY_4)
		&& (connectivity!=CONNECTIVITY_4_SAGITTAL)
		&& (connectivity!=CONNECTIVITY_4_CORONAL)
		&& (connectivity!=CONNECTIVITY_6)
		&& (connectivity!=CONNECTIVITY_8)
		&& (connectivity!=CONNECTIVITY_8_SAGITTAL)
		&& (connectivity!=CONNECTIVITY_8_CORONAL)
		&& (connectivity!=CONNECTIVITY_18)
		&& (connectivity!=CONNECTIVITY_26)
		&& (connectivity!=CONNECTIVITY_2x26)
		&& (connectivity!=CONNECTIVITY_3x26)
		&& (connectivity!=CONNECTIVITY_4x26))

	{
		sprintf(error,"Unknown connectivity: %d",connectivity);
		VipPrintfError(error);
	 	VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
		return(NULL);
	}
	switch(connectivity)
	{
		case CONNECTIVITY_4: 
		case CONNECTIVITY_4_SAGITTAL: 
		case CONNECTIVITY_4_CORONAL:  nb_neighbors = 4; break;
		case CONNECTIVITY_6: nb_neighbors = 6; break;
		case CONNECTIVITY_8:
		case CONNECTIVITY_8_SAGITTAL:
		case CONNECTIVITY_8_CORONAL:  nb_neighbors = 8; break;
		case CONNECTIVITY_18: nb_neighbors = 18; break;
		case CONNECTIVITY_26: nb_neighbors = 26; break;
		case CONNECTIVITY_2x26: nb_neighbors = 5*5*5-1; break;
		case CONNECTIVITY_3x26: nb_neighbors = 7*7*7-1; break;
		case CONNECTIVITY_4x26: nb_neighbors = 9*9*9-1; break;
		default:
		{
			sprintf(error,"Unknown connectivity: %d",connectivity);
			VipPrintfError(error);
	 		VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
			return(NULL);
		}
	}

	vcs = (VipConnectivityStruct *)VipMalloc(sizeof(VipConnectivityStruct),"VipGetConnectivityStruct");
	if(vcs==NULL) return(NULL);
	vcs->offset = (int *)VipMalloc(sizeof(int)*nb_neighbors,"VipGetConnectivityStruct");
	if(vcs->offset==NULL) return(NULL);
	vcs->connectivity = connectivity;
	vcs->nb_neighbors = 0;
	vcs->xyz_offset = (Vip3DPoint_S16BIT *) 
		VipMalloc(sizeof(Vip3DPoint_S16BIT)*nb_neighbors,
		          "VipGetConnectivityStruct");
	if(vcs->xyz_offset==NULL) return(NULL);

	xsize = VipOffsetLine(volume);
	slicesize = VipOffsetSlice(volume);

/* Here we have not chosen the trick which consist in nested neighborhood */
/* Warning: the neighbor order could have consequences on some algorithms */


	switch(connectivity)
	{
		case CONNECTIVITY_4: nb_neighbors = 4;
			vcs->xyz_offset[vcs->nb_neighbors].x = -1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = -1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y;
			vcs->nb_neighbors++;
			if (vcs->nb_neighbors!=nb_neighbors)
			{
				VipPrintfError("Incoherence in switch");
		 		VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
				return(NULL);
			}
			break;

		case CONNECTIVITY_4_SAGITTAL: nb_neighbors = 4;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = -1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].y*xsize
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = -1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].y*xsize
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].y*xsize
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].y*xsize
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			if (vcs->nb_neighbors!=nb_neighbors)
			{
				VipPrintfError("Incoherence in switch");
		 		VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
				return(NULL);
			}
			break;

		case CONNECTIVITY_4_CORONAL: nb_neighbors = 4;
			vcs->xyz_offset[vcs->nb_neighbors].x = -1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = -1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			if (vcs->nb_neighbors!=nb_neighbors)
			{
				VipPrintfError("Incoherence in switch");
		 		VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
				return(NULL);
			}
			break;

		case CONNECTIVITY_6: nb_neighbors = 6;
			vcs->xyz_offset[vcs->nb_neighbors].x = -1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y
				+ slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = -1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y
				+ slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = -1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y
				+ slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y
				+ slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y
				+ slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y
				+ slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			if (vcs->nb_neighbors!=nb_neighbors)
			{
				VipPrintfError("Incoherence in switch");
		 		VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
				return(NULL);
			}
			 break;

		case CONNECTIVITY_8: nb_neighbors = 8;
			vcs->xyz_offset[vcs->nb_neighbors].x = -1;
			vcs->xyz_offset[vcs->nb_neighbors].y = -1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = -1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = -1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = -1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 1;
			vcs->xyz_offset[vcs->nb_neighbors].y = -1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + xsize * vcs->xyz_offset[vcs->nb_neighbors].y;
			vcs->nb_neighbors++;
			if (vcs->nb_neighbors!=nb_neighbors)
			{
				VipPrintfError("Incoherence in switch");
		 		VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
				return(NULL);
			}
			break;

		case CONNECTIVITY_8_SAGITTAL: nb_neighbors = 8;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = -1;
			vcs->xyz_offset[vcs->nb_neighbors].z = -1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].y*xsize
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = -1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].y*xsize
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = -1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].y*xsize
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = -1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].y*xsize
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].y*xsize
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 1;
			vcs->xyz_offset[vcs->nb_neighbors].z = -1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].y*xsize
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].y*xsize
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 1;
			vcs->xyz_offset[vcs->nb_neighbors].z = 1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].y*xsize
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			if (vcs->nb_neighbors!=nb_neighbors)
			{
				VipPrintfError("Incoherence in switch");
		 		VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
				return(NULL);
			}
			break;

		case CONNECTIVITY_8_CORONAL: nb_neighbors = 8;
			vcs->xyz_offset[vcs->nb_neighbors].x = -1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = -1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = -1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = -1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = -1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 0;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = -1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 0;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			vcs->xyz_offset[vcs->nb_neighbors].x = 1;
			vcs->xyz_offset[vcs->nb_neighbors].y = 0;
			vcs->xyz_offset[vcs->nb_neighbors].z = 1;
			vcs->offset[vcs->nb_neighbors] =
            vcs->xyz_offset[vcs->nb_neighbors].x
            + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
			vcs->nb_neighbors++;
			if (vcs->nb_neighbors!=nb_neighbors)
			{
				VipPrintfError("Incoherence in switch");
		 		VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
				return(NULL);
			}
			break;

		case CONNECTIVITY_18: nb_neighbors = 18;
			for (i= -1; i<=1; i++)
			for (j= -1; j<=1; j++)
			for (k= -1; k<=1; k++)
			{
				dist = abs(i)+abs(j)+abs(k);
				if((dist==1)||(dist==2))
				{
               vcs->xyz_offset[vcs->nb_neighbors].x = i;
               vcs->xyz_offset[vcs->nb_neighbors].y = j;
               vcs->xyz_offset[vcs->nb_neighbors].z = k;
               vcs->offset[vcs->nb_neighbors] =
               vcs->xyz_offset[vcs->nb_neighbors].x
               + xsize * vcs->xyz_offset[vcs->nb_neighbors].y
               + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
				   vcs->nb_neighbors++;
				}
			}
			if (vcs->nb_neighbors!=nb_neighbors)
			{
				VipPrintfError("Incoherence in switch");
		 		VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
				return(NULL);
			}

			break;

		case CONNECTIVITY_26: nb_neighbors = 26;
			for (i= -1; i<=1; i++)
			for (j= -1; j<=1; j++)
			for (k= -1; k<=1; k++)
			{
				dist = abs(i)+abs(j)+abs(k);
				if((dist>=1)&&(dist<=3))
				{
               vcs->xyz_offset[vcs->nb_neighbors].x = i;
               vcs->xyz_offset[vcs->nb_neighbors].y = j;
               vcs->xyz_offset[vcs->nb_neighbors].z = k;
               vcs->offset[vcs->nb_neighbors] =
               vcs->xyz_offset[vcs->nb_neighbors].x
               + xsize * vcs->xyz_offset[vcs->nb_neighbors].y
               + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
				   vcs->nb_neighbors++;
				}
			}
			if (vcs->nb_neighbors!=nb_neighbors)
			{
				VipPrintfError("Incoherence in switch");
		 		VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
				return(NULL);
			}
 			break;
	case CONNECTIVITY_2x26: nb_neighbors = 5*5*5-1;
			for (i= -2; i<=2; i++)
			for (j= -2; j<=2; j++)
			for (k= -2; k<=2; k++)
			{
				dist = mVipMax(abs(i),abs(j));
				dist = mVipMax(abs(k),dist);
				if((dist>=1)&&(dist<=2))
				{
               vcs->xyz_offset[vcs->nb_neighbors].x = i;
               vcs->xyz_offset[vcs->nb_neighbors].y = j;
               vcs->xyz_offset[vcs->nb_neighbors].z = k;
               vcs->offset[vcs->nb_neighbors] =
               vcs->xyz_offset[vcs->nb_neighbors].x
               + xsize * vcs->xyz_offset[vcs->nb_neighbors].y
               + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
				   vcs->nb_neighbors++;
				}
			}
			if (vcs->nb_neighbors!=nb_neighbors)
			{
				VipPrintfError("Incoherence in switch");
		 		VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
				return(NULL);
			}
 			break;
	case CONNECTIVITY_3x26: nb_neighbors = 7*7*7-1;
			for (i= -3; i<=3; i++)
			for (j= -3; j<=3; j++)
			for (k= -3; k<=3; k++)
			{
				dist = mVipMax(abs(i),abs(j));
				dist = mVipMax(abs(k),dist);
				if((dist>=1)&&(dist<=3))
				{
               vcs->xyz_offset[vcs->nb_neighbors].x = i;
               vcs->xyz_offset[vcs->nb_neighbors].y = j;
               vcs->xyz_offset[vcs->nb_neighbors].z = k;
               vcs->offset[vcs->nb_neighbors] =
               vcs->xyz_offset[vcs->nb_neighbors].x
               + xsize * vcs->xyz_offset[vcs->nb_neighbors].y
               + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
				   vcs->nb_neighbors++;
				}
			}
			if (vcs->nb_neighbors!=nb_neighbors)
			{
				VipPrintfError("Incoherence in switch");
		 		VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
				return(NULL);
			}
 			break;
	case CONNECTIVITY_4x26: nb_neighbors = 9*9*9-1;
			for (i= -4; i<=4; i++)
			for (j= -4; j<=4; j++)
			for (k= -4; k<=4; k++)
			{
				dist = mVipMax(abs(i),abs(j));
				dist = mVipMax(abs(k),dist);
				if((dist>=1)&&(dist<=4))
				{
               vcs->xyz_offset[vcs->nb_neighbors].x = i;
               vcs->xyz_offset[vcs->nb_neighbors].y = j;
               vcs->xyz_offset[vcs->nb_neighbors].z = k;
               vcs->offset[vcs->nb_neighbors] =
               vcs->xyz_offset[vcs->nb_neighbors].x
               + xsize * vcs->xyz_offset[vcs->nb_neighbors].y
               + slicesize * vcs->xyz_offset[vcs->nb_neighbors].z;
				   vcs->nb_neighbors++;
				}
			}
			if (vcs->nb_neighbors!=nb_neighbors)
			{
				VipPrintfError("Incoherence in switch");
		 		VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
				return(NULL);
			}
 			break;
		default:
		{
			sprintf(error,"Unknown connectivity: %d",connectivity);
			VipPrintfError(error);
	 		VipPrintfExit ( "(Vip_connex.c)VipGetConnectivityStruct" );
			return(NULL);
		}
	}


	return(vcs);
}

/*----------------------------------------------------------------------------*/
void VipFreeConnectivityStruct(
  VipConnectivityStruct *vcs
)
/*----------------------------------------------------------------------------*/
{
  if (vcs)
  { 
    if (vcs->xyz_offset)  VipFree(vcs->xyz_offset);
    if (vcs->offset)  VipFree(vcs->offset);
    VipFree(vcs);
  }
}
