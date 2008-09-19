/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : spm                  * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 11/2001
 * VERSION     : 2.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 *****************************************************************************
 * DESCRIPTION : jfmangin@cea.fr
 
I/O on MNI minc format.
NB since the absolute referential underlying minc may
be different from the absolute
referential used in VIDA format, we have chosen
to flip x, y or z direction after the reading.
Then the image orientation in memory is always
the same (unfortunately, a non direct referential...)
*****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------

 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <vip/volume.h>
#include <vip/util.h>
#include <vip/alloc.h>

#ifdef MINC_LIB
#include <vip/minc.h>
#endif

extern int ReadMinc3DData(int *type,char *name,char **out_ptr,int *xsize, int *ysize, int *zsize,
		    float *xvoxsize, float *yvoxsize, float *zvoxsize);

/*********************************************************************/
Volume *VipReadMincVolumeWithBorder(char *name, int borderWidth)
/*********************************************************************/
{

#ifdef MINC_LIB

    Volume *vol, *converted;
    char *out_ptr;
    int xsize, ysize, zsize;
    float xvoxsize, yvoxsize, zvoxsize;
    float xfov, yfov, zfov;
    int type=0;
    if (ReadMinc3DData(&type,name,&out_ptr, &xsize, &ysize, &zsize,
			&xvoxsize, &yvoxsize, &zvoxsize)==PB) return(PB);

    if (borderWidth!=0)
      {
	VipPrintfWarning("Border not implemented yet for minc format");
      }
    vol = VipDeclare3DVolumeStructure( xsize, ysize, zsize, 
				       xvoxsize, yvoxsize, zvoxsize,
				       type,
				       name,				       
				       borderWidth );

    vol->data = out_ptr;
    vol->state = STATE_FILLED;

    xfov = mVipVolSizeX(vol) * mVipVolVoxSizeX(vol);
    yfov = mVipVolSizeY(vol) * mVipVolVoxSizeY(vol);
    zfov = mVipVolSizeZ(vol) * mVipVolVoxSizeZ(vol);

 
    /*to go to absolute referential of VIDA format from minc one*/

    converted = VipFlipVolume( vol, FLIP_XXYYZZ); 
    converted->shfj = vol->shfj;
    vol->shfj = NULL;
    VipFreeVolume( vol);
    vol = converted;
           
    return(vol);

#endif

    VipPrintfError("Your vip version has not been compiled with minc library ...");
    printf("%s, %d border\n",name,borderWidth);
    return(PB);
}


/*********************************************************************/


