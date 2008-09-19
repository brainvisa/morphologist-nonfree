/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : spline.c           * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 20/01/99
 * VERSION     : 1.4                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 *
 * DESCRIPTION : Integration des reechantillonage splines de l'equipe de M. Unser
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

#include <vip/volume.h>
#include <vip/util.h>
#include <vip/depla.h>
#include <vip/splineresamp.h>
#include <vip/splineresamp_static.h>

/*------------------------------------------------------------------*/
Volume *VipSplineResamp(
Volume *vol,
Volume *template,
VipDeplacement *dep,
int degree)
/*------------------------------------------------------------------*/
{
    Volume *result;
    Volume *finput, *fresult;
    VipDeplacement invdep;
    int s;
    int dimt;
    char newname[512];
    double transform[4][4];
    double origin[3];
    int t;

    if(!vol || !template || !dep )
	{
	    VipPrintfExit("Argument in VipSplineResamp");
	    return(PB);
	}
    if(degree<0 || degree>7)
	{
	    VipPrintfError("This spline degree is not supported..., try the range [0..7]");
	    VipPrintfExit("Argument in VipSplineResamp");
	    return(PB);
	}
    if(VipVerifyAll(vol)==PB || VipVerifyAll(template)==PB)
	{
	    VipPrintfExit("Argument in VipSplineResamp");
	    return(PB);
	}

    if(mVipVolBorderWidth(vol)!=0 || mVipVolBorderWidth(template)!=0)
	{
	    VipPrintfError("VipSplineResamp requires 0 borderwidth for vol and template");
	    VipPrintfExit("Argument in VipLinearResamp");
	    return(PB);
	}
    if(mVipVolType(vol)!=VFLOAT && mVipVolType(vol)!=U8BIT && mVipVolType(vol)!=S16BIT)
	{
	    VipPrintfError("Sorry, VipSplineResampl thus not support this image type");
	    VipPrintfExit("Argument in VipLinearResamp");
	    return(PB);
	}

    strcpy(newname,mVipVolName(vol));
    strcat(newname,"_splineresamp");

    if(mVipVolType(vol)!=VFLOAT)
	{
	    finput = VipTypeConversionToFLOAT(vol,RAW_TYPE_CONVERSION);
	    if(finput==PB)
		{
		    VipPrintfExit("Conversion in VipSplineResamp");
		    return(NULL);
		}
	}
    else finput = vol;

    fresult = VipDuplicateVolumeStructure( template, newname);
    if(fresult==PB)
	{
	    VipPrintfExit("allocation in VipSplineResamp");
	    return(NULL);
	}
    VipSetBorderWidth(fresult,0);
    VipSetType(fresult,VFLOAT);
    if (VipAllocateVolumeData( fresult)==PB)
	{
	    VipPrintfExit("allocation in VipSplineResamp");
	    return(NULL);
	};

    if (VipInverseDeplacement(dep,&invdep)==PB) 
	{
	    VipPrintfExit("inversion in VipSplineResamp");
	    return(NULL);
	};

    VipScaleDeplacement(dep,vol,fresult);
    VipScaleDeplacement(&invdep,fresult,vol);

    /*
    transform[0][3] = invdep.t.x;
    transform[1][3] = invdep.t.y;
    transform[2][3] = invdep.t.z;
    transform[3][3] = 1.;
    transform[3][0] = 0.;
    transform[3][1] = 0.;
    transform[3][2] = 0.;
    
    transform[0][0] = invdep.r.xx;
    transform[0][1] = invdep.r.xy;
    transform[0][2] = invdep.r.xz;
    transform[1][0] = invdep.r.yx;
    transform[1][1] = invdep.r.yy;
    transform[1][2] = invdep.r.yz;
    transform[2][0] = invdep.r.zx;
    transform[2][1] = invdep.r.zy;
    transform[2][2] = invdep.r.zz;
    */
    transform[0][3] = dep->t.x;
    transform[1][3] = dep->t.y;
    transform[2][3] = dep->t.z;
    transform[3][3] = 1.;
    transform[3][0] = 0.;
    transform[3][1] = 0.;
    transform[3][2] = 0.;
    
    transform[0][0] = dep->r.xx;
    transform[0][1] = dep->r.xy;
    transform[0][2] = dep->r.xz;
    transform[1][0] = dep->r.yx;
    transform[1][1] = dep->r.yy;
    transform[1][2] = dep->r.yz;
    transform[2][0] = dep->r.zx;
    transform[2][1] = dep->r.zy;
    transform[2][2] = dep->r.zz;



    origin[0] = 0.;
    origin[1] = 0.;
    origin[2] = 0.; 

    for ( t = 0; t < mVipVolSizeT( finput ); t++ )
    {

      if ( affineTransformSHFJ( transform, origin,
                                VipGetDataPtr_VFLOAT(finput) + t * VipOffsetVolume( finput ),
                                VipGetDataPtr_VFLOAT(fresult) + t * VipOffsetVolume( fresult ),
			        mVipVolSizeX(finput),
                                mVipVolSizeY(finput),
                                mVipVolSizeZ(finput),
                                mVipVolSizeX(fresult),
                                mVipVolSizeY(fresult),
                                mVipVolSizeZ(fresult), degree, 0 )==PB)
		{
		    VipPrintfError("Sorry, affineTransform problem");
		    VipPrintfExit("Argument in VipSplineResamp");
		    return(PB);
		}

    }

    if(mVipVolType(vol)!=VFLOAT)
	{
	    VipFreeVolume(finput);
	    if(mVipVolType(vol)==U8BIT)
		result = VipTypeConversionToU8BIT(fresult,RAW_TYPE_CONVERSION);
	    else if(mVipVolType(vol)==S16BIT)
		result = VipTypeConversionToS16BIT(fresult,RAW_TYPE_CONVERSION);
	    else
		{
		    VipPrintfError("Sorry, VipSplineResamp thus not support this image type");
		    VipPrintfExit("Argument in VipSplineResamp");
		    return(PB);
		}
	    if(result==PB)
		{
		    VipPrintfExit("Conversion in VipSplineResamp");
		    return(NULL);
		}
	    VipFreeVolume(fresult);
	}
    else result = fresult;

    dimt = mVipVolSizeT(result);
    result->shfj->unite = vol->shfj->unite;
    result->shfj->VmdType = vol->shfj->VmdType;
    result->shfj->scale = vol->shfj->scale;
    result->shfj->offset = vol->shfj->offset;
    if (dimt > 1)
	{
	    if (!result->shfj->start_time)
		result->shfj->start_time = (int *)VipCalloc(dimt,sizeof(int),"");
	    if (!result->shfj->dur_time)
		result->shfj->dur_time = (int *)VipCalloc(dimt,sizeof(int),"");
            if ( vol->shfj->start_time && vol->shfj->dur_time )
            {

              for (s=0; s<dimt; s++)
		{
		    result->shfj->start_time[s] = vol->shfj->start_time[s];
		    result->shfj->dur_time[s] = vol->shfj->dur_time[s];
		}

            }
            else
            {

              for (s=0; s<dimt; s++)
		{
		    result->shfj->start_time[s] = s;
		    result->shfj->dur_time[s] = 1;
		}

            }
	}

    return(result);
}


