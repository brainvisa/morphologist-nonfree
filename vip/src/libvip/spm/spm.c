/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : spm                  * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 04/01/99
 * VERSION     : 1.4                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Ultra
 *****************************************************************************
 * DESCRIPTION : jfmangin@cea.fr
 
I/O on SPM like analyze format.
NB since the absolute referential underlying SPM use
of Analyze format is different from the absolute
referential used in VIDA format, we have chosen
to flip x, y and z direction after the reading.
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
#include <vip/spm.h>
#include <vip/spm_static.h>


Volume *ReadAnalyzeHeader(char *name, int borderWidth)
{
    char message[500];
    char header_name[500];
    FILE *header_file;
    Volume *vol=PB;
    int n;
    struct analyze_db myhdr;
    short center[5]; /*centre selon SPM*/
    int typecode;
    int flag_byteswap;
    int type;

    memset( center, 0, 5 * sizeof( short ) );
    strcpy (header_name, name);
    header_name[495]='\0';
    strcat (header_name, ".hdr");
    if ((header_file=fopen(header_name,"rb"))==NULL)
	{
	    sprintf(message,"Can not open: %s",header_name);
	    VipPrintfError (message);
	    return(PB);
	}
    if((n=fread((char *)&myhdr,sizeof(struct analyze_db),1,header_file)) !=1)
	{
	    sprintf(message,"Error during the reading of %s",header_name);
	    VipPrintfError (message);
	    return(PB);
	}
    
    fclose(header_file); 

    if (myhdr.key.size > 1000) 
      {
	flag_byteswap = VTRUE;
	VipPrintfInfo("Byte swapping spm header and data!");
      }
    else
      flag_byteswap = VFALSE;

    if (flag_byteswap==VTRUE) 
      {
	VipByteSwapping_S16BIT(myhdr.img.dim, 2, 16);
	VipByteSwapping_VFLOAT(myhdr.img.pixdim, 4, 16);
	VipByteSwapping_S16BIT((short int *)myhdr.his.originator, 2, 5);
      }

    if (myhdr.img.pixdim[4]==0) myhdr.img.pixdim[4]=1.;

    typecode = myhdr.img.dim[15];

    switch( typecode )
      {
      case 2:
	type = U8BIT;
	break;
      case 4:
	type = S16BIT;
	break;
      case 8:
	type = S32BIT;
	break;
      case 16:
	type = VFLOAT;
	break;
      case 64:
	type = VDOUBLE;
	break;
      case 130:
	type = S8BIT;
	break;
      case 132:
	type = U16BIT;
	break;
      case 136:
	type = U32BIT;
	break;
      default:
	VipPrintfError("unrecognized type code - choosing type from bits/voxel\n");
	return(PB);
    }

    vol = VipDeclare4DVolumeStructure(
				      myhdr.img.dim[1],
				      myhdr.img.dim[2],
				      myhdr.img.dim[3],
				      myhdr.img.dim[4],
				      myhdr.img.pixdim[1],
				      myhdr.img.pixdim[2],
				      myhdr.img.pixdim[3],
				      myhdr.img.pixdim[4],
				      type,                         
				      name,
				      borderWidth);

    if(myhdr.his.descrip[0]!='\0')
	{
	    myhdr.his.descrip[79]='\0';
	    memcpy((void *)center,(void *)myhdr.his.originator,10);
	    sprintf(message,"history: %s\n\t origine: %d %d %d\n",
		    myhdr.his.descrip, center[0],center[1],center[2]);
	    VipPrintfInfo(message);
	}
    
    if(vol==PB) return(PB);

    vol->shfj = VipCalloc(1,sizeof(SHFJ_private_struct),"ReadAnalyzeHeader");
    if(vol->shfj)
      {
	vol->shfj->center.x = (float)(center[0]);
	vol->shfj->center.y = (float)(center[1]);
	vol->shfj->center.z = (float)(center[2]);
	if (flag_byteswap==VTRUE)
	  vol->shfj->byte_swapping_actif=VTRUE;
	else
	  vol->shfj->byte_swapping_actif=VFALSE;
      }
    return(vol);
}

/*********************************************************************/
Volume *ReadSPMVolume(char *name, int borderWidth)
/*********************************************************************/
{
    Volume *vol, *converted;
    FILE  *thefile;
    int n;
    char message[500];
    char file_name[500];
    VipOffsetStruct *vos;
    int iy, iz, it;
    Vip_S16BIT *ptr;
    Vip_U8BIT *ptr8;
    Vip_FLOAT *ptrF;
    float xfov, yfov, zfov;


    vol = ReadAnalyzeHeader(name,borderWidth);
    if(vol==PB) return(PB);


    strcpy (file_name, name);
    file_name[495]='\0';
    strcat (file_name, ".img");
    if ((thefile=fopen(file_name,"rb"))==NULL)
	{
	    sprintf(message,"Can not open: %s",file_name);
	    VipPrintfError (message);
	    VipPrintfExit("ReadSPMVolume");
	    return(PB);
	}

    if( VipAllocateVolumeData(vol) == PB)
	{
	    VipPrintfExit("ReadSPMVolume");
	    return(PB);
	}


    if(mVipVolBorderWidth(vol)!=0) VipSetBorderLevel(vol, 0);

    vos = VipGetOffsetStructure(vol);

    if(mVipVolType(vol)==VFLOAT)
	{
	    ptrF = VipGetDataPtr_VFLOAT( vol ) + vos->oFirstPoint;
	    
	    for ( it = mVipVolSizeT(vol); it-- ; )               /* loop on volumes */
		{
		    for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
			{
			    for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
				{
				    n = fread(ptrF,sizeof(Vip_FLOAT),  mVipVolSizeX(vol), thefile);
				    if(n!=mVipVolSizeX(vol))
					{
					    sprintf(message,
						    "Error during reading of: %s (x:%d, y:%d, z:%d)",
						    file_name, n, iy, iz);
					    VipPrintfError (message);
					    VipPrintfExit("ReadSPMVolume");
					    VipFreeVolumeData(vol);
					    return(PB);
					}
				    if(vol->shfj->byte_swapping_actif==VTRUE)
				      	VipByteSwapping_VFLOAT(ptrF, 4, mVipVolSizeX(vol));
				    ptrF += vos->oLine;  
				}
			    ptrF += vos->oLineBetweenSlice; /*skip border lines*/
			}
		    ptrF += vos->oSliceBetweenVolume; /*skip border slices*/
		}
	}
    else if(mVipVolType(vol)==S16BIT)
	{
	    ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;
	    
	    for ( it = mVipVolSizeT(vol); it-- ; )               /* loop on volumes */
		{
		    for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
			{
			    for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
				{
				    n = fread(ptr,sizeof(Vip_S16BIT),  mVipVolSizeX(vol), thefile);
				    if(n!=mVipVolSizeX(vol))
					{
					    sprintf(message,
						    "Error during reading of: %s (x:%d, y:%d, z:%d)",
						    file_name, n, iy, iz);
					    VipPrintfError (message);
					    VipPrintfExit("ReadSPMVolume");
					    VipFreeVolumeData(vol);
					    return(PB);
					}
				    if(vol->shfj->byte_swapping_actif==VTRUE)
				      	VipByteSwapping_S16BIT(ptr, 2, mVipVolSizeX(vol));
				    ptr += vos->oLine;  
				}
			    ptr += vos->oLineBetweenSlice; /*skip border lines*/
			}
		    ptr += vos->oSliceBetweenVolume; /*skip border slices*/
		}
	}
    else if(mVipVolType(vol)==U8BIT)
	{
	    ptr8 = VipGetDataPtr_U8BIT( vol ) + vos->oFirstPoint;
	    
	    for ( it = mVipVolSizeT(vol); it-- ; )               /* loop on volumes */
		{
		    for ( iz = mVipVolSizeZ(vol); iz-- ; )               /* loop on slices */
			{
			    for ( iy = mVipVolSizeY(vol); iy-- ; )            /* loop on lines */
				{
				    n = fread(ptr8,sizeof(Vip_U8BIT),  mVipVolSizeX(vol), thefile);
				    if(n!=mVipVolSizeX(vol))
					{
					    sprintf(message,
						    "Error during reading of: %s (x:%d, y:%d, z:%d)",
						    file_name, n, iy, iz);
					    VipPrintfError (message);
					    VipPrintfExit("ReadSPMVolume");
					    VipFreeVolumeData(vol);
					    return(PB);
					}
				    ptr8 += vos->oLine;  
				}
			    ptr8 += vos->oLineBetweenSlice; /*skip border lines*/
			}
		    ptr8 += vos->oSliceBetweenVolume; /*skip border slices*/
		}
	}
    else
	{
	    VipPrintfError("Up to now, only signed16bit and unsigne8bit  SPM format are implemented in VIP for reading");
	    VipPrintfExit("ReadSPMVolume");
	    VipFreeVolumeData(vol);
	    return(PB);
	}
    fclose(thefile);


    xfov = mVipVolSizeX(vol) * mVipVolVoxSizeX(vol);
    yfov = mVipVolSizeY(vol) * mVipVolVoxSizeY(vol);
    zfov = mVipVolSizeZ(vol) * mVipVolVoxSizeZ(vol);

    /*Talairach system usual field of view...(we did not find more robust
      clues of SPM normalization...)*/    
    if((xfov>152) && (xfov<165) && (yfov>185) && (yfov<195) && (zfov>130) && (zfov<145))
	{
	    vol->shfj->spm_normalized = VTRUE;
	}
    else if((xfov>178) && (xfov<185) && (yfov>215) && (yfov<220) && (zfov>178) && (zfov<185))
	{
	    vol->shfj->spm_normalized = VTRUE;
	}
    else
	{
	    vol->shfj->spm_normalized = VFALSE;
	}
    /*to go to absolute referential of VIDA format*/
    if(vol->shfj->spm_normalized == VTRUE)
	{
	    printf("This volume has been normalized\n");
	    converted = VipFlipVolume( vol, FLIP_XXYYZZ); 
	}
    else
	{
	    converted = VipFlipVolume( vol, FLIP_YYZZ); 

	}

    converted->shfj = vol->shfj;
    vol->shfj = NULL;
    VipFreeVolume( vol);

    return(converted);
}

/*********************************************************************/
/*----------------------------------------------------------------------------*/
int WriteSPMVolume
/*----------------------------------------------------------------------------*/
(
 Volume	    *vol,
 char    *name
)
{
    FILE  *newfile;
    int n;
    char file_name[1000];
    char message[1000];
    VipOffsetStruct *vos;
    int iy, iz, it;
    Vip_S16BIT *ptr;
    Vip_U8BIT *ptr8;
    Vip_FLOAT *ptrF;
    Volume *converted;
    float xfov, yfov, zfov;

    if( vol==NULL || name==NULL)
	{
	    VipPrintfError("Null argument in WriteSPMVolume");
	    return(PB);
	}

    if(WriteAnalyzeHeader(vol, name)==PB) return(PB);

    xfov = mVipVolSizeX(vol) * mVipVolVoxSizeX(vol);
    yfov = mVipVolSizeY(vol) * mVipVolVoxSizeY(vol);
    zfov = mVipVolSizeZ(vol) * mVipVolVoxSizeZ(vol);

    if((xfov>152) && (xfov<165) && (yfov>185) && (yfov<195) && (zfov>130) && (zfov<145))
	{
	    /*to go to absolute referential of normalized SPM format*/
	    converted = VipFlipVolume( vol, FLIP_XXYYZZ);
	}
    else if((xfov>178) && (xfov<185) && (yfov>215) && (yfov<220) && (zfov>178) && (zfov<185))
	{
	    /*to go to absolute referential of normalized SPM format*/
	    converted = VipFlipVolume( vol, FLIP_XXYYZZ);
	}
    else
	{
	    converted = VipFlipVolume( vol, FLIP_YYZZ);
	}
   /*to go to absolute referential of SHFJ non normalized SPM format*/
    VipFreeVolume( vol);
    vol = converted;

    strcpy(file_name, name);
    file_name[995]='\0';
    strcat(file_name, ".img");

    newfile = fopen(file_name,"wb");
    if(!newfile)
	{
 	  sprintf (message,"Can not write: %s\n",file_name);
	  VipPrintfError(message);
	  VipPrintfExit("WriteSPMVolume");
 	  return(PB);
	}

   vos = VipGetOffsetStructure(vol);

   if(mVipVolType(vol)==VFLOAT)
     {
       ptrF = VipGetDataPtr_VFLOAT( vol ) + vos->oFirstPoint;

       for ( it = mVipVolSizeT(vol); it-- ; )               /* loop on volumes */
	 {
	   for ( iz = mVipVolSizeZ(vol); iz-- ; )             /* loop on slices */
	     {
	       for ( iy = mVipVolSizeY(vol); iy-- ; )          /* loop on lines */
		 {
		   n = fwrite(ptrF,sizeof(Vip_FLOAT),mVipVolSizeX(vol),newfile);
		   if (n!=mVipVolSizeX(vol))
		     {
		       sprintf (message,"Error while writing in: %s\n",file_name);
		       VipPrintfError(message);
		       VipPrintfExit("WriteSPMVolume");
		       return(PB);
		       fclose(newfile);
		     }
		   ptrF += vos->oLine;        /*skip one line*/
		 }
	       ptrF += vos->oLineBetweenSlice; /*skip border lines*/
	     }
	   ptrF += vos->oSliceBetweenVolume; /*skip border slices*/
	 }
     }
   else if(mVipVolType(vol)==U8BIT)
     {
       ptr8 = VipGetDataPtr_U8BIT( vol ) + vos->oFirstPoint;

       for ( it = mVipVolSizeT(vol); it-- ; )               /* loop on volumes */
	 {
	   for ( iz = mVipVolSizeZ(vol); iz-- ; )             /* loop on slices */
	     {
	       for ( iy = mVipVolSizeY(vol); iy-- ; )          /* loop on lines */
		 {
		   n = fwrite(ptr8,sizeof(Vip_U8BIT),mVipVolSizeX(vol),newfile);
		   if (n!=mVipVolSizeX(vol))
		     {
		       sprintf (message,"Error while writing in: %s\n",file_name);
		       VipPrintfError(message);
		       VipPrintfExit("WriteSPMVolume");
		       return(PB);
		       fclose(newfile);
		     }
		   ptr8 += vos->oLine;        /*skip one line*/
		 }
	       ptr8 += vos->oLineBetweenSlice; /*skip border lines*/
	     }
	   ptr8 += vos->oSliceBetweenVolume; /*skip border slices*/
	 }
     }
   else if(mVipVolType(vol)==S16BIT)
     {
       ptr = VipGetDataPtr_S16BIT( vol ) + vos->oFirstPoint;

       for ( it = mVipVolSizeT(vol); it-- ; )               /* loop on volumes */
	 {
	   for ( iz = mVipVolSizeZ(vol); iz-- ; )             /* loop on slices */
	     {
	       for ( iy = mVipVolSizeY(vol); iy-- ; )          /* loop on lines */
		 {
		   n = fwrite(ptr,sizeof(Vip_S16BIT),mVipVolSizeX(vol),newfile);
		   if (n!=mVipVolSizeX(vol))
		     {
		       sprintf (message,"Error while writing in: %s\n",file_name);
		       VipPrintfError(message);
		       VipPrintfExit("WriteSPMVolume");
		       return(PB);
		       fclose(newfile);
		     }
		   ptr += vos->oLine;        /*skip one line*/
		 }
	       ptr += vos->oLineBetweenSlice; /*skip border lines*/
	     }
	   ptr += vos->oSliceBetweenVolume; /*skip border slices*/
	 }
     }

   fclose(newfile);

   return( OK );
}


/*************************************************************************/

int WriteAnalyzeHeader( 
    Volume *vol,
   char *name)
{
  char error[1000];
	struct analyze_db *myhdr;
	FILE *header_file;
	int n;
	char header_name[1000], message[1000];
	float vmin, vmax;
	short center[5]; /*centre selon SPM*/

	if( vol==NULL || name==NULL)
	{
	    VipPrintfError("Null argument in WriteAnalyzeHeader");
	    return(PB);
	}

	strcpy (header_name, name);
	header_name[995]='\0';
	strcat (header_name, ".hdr");
	if ((header_file=fopen(header_name,"wb"))==NULL)
	{
 	  sprintf (message,"Can not write: %s\n",header_name);
	  VipPrintfError(message);
	  VipPrintfExit("WriteAnalyzeHeader");
 	  return(PB);
	}

	myhdr = (struct analyze_db *)calloc(1, sizeof(struct analyze_db));
	if(myhdr==PB)
	  {
	    VipPrintfError("No more memory");
	    return(PB);
	  }
	  
	if((mVipVolSizeX(vol)%2)==0) center[0] =mVipVolSizeX(vol)/2 +1;
	else center[0] =(mVipVolSizeX(vol)+1)/2 ;
	if((mVipVolSizeY(vol)%2)==0) center[1] =mVipVolSizeY(vol)/2 +1;
	else center[1] =(mVipVolSizeY(vol)+1)/2 ;
	if((mVipVolSizeZ(vol)%2)==0) center[2] =mVipVolSizeZ(vol)/2 +1;
	else center[2] =(mVipVolSizeZ(vol)+1)/2 ;	  
	sprintf(error,"Setting standard center for SPM: %d, %d, %d\n",center[0],
		center[1], center[2]);
	VipPrintfWarning(error);
	memcpy((void *)myhdr->his.originator,(void *)center,10);

	myhdr->img.dim[0] = 4;
	myhdr->key.regular = 'r';

	if(mVipVolType(vol)!=S16BIT && mVipVolType(vol)!=U8BIT && mVipVolType(vol)!=VFLOAT)
	    {
		VipPrintfError("Up to now, writing of SPM format is only supported in short u8bit/s16bit/float, use TIVOLI format, sorry...");
		VipPrintfExit("WriteAnalyzeHeader");
		return(PB);
	    }

	myhdr->img.dim[1] = mVipVolSizeX(vol);
	myhdr->img.dim[2] = mVipVolSizeY(vol);
	myhdr->img.dim[3] = mVipVolSizeZ(vol);
	myhdr->img.dim[4] = mVipVolSizeT(vol);
	if(mVipVolType(vol)==VFLOAT)
	  {
	    myhdr->img.bitpix = 32;
	    myhdr->img.dim[15] = 16;
	  }
	else if(mVipVolType(vol)==U8BIT)
	  {
	    myhdr->img.bitpix = 8;
	    myhdr->img.dim[15] = 2;
	  }
	else if(mVipVolType(vol)==S16BIT)
	  {
	    myhdr->img.bitpix = 16;
	    myhdr->img.dim[15] = 4;
	  }

	myhdr->img.pixdim[1] = mVipVolVoxSizeX(vol);
	myhdr->img.pixdim[2] = mVipVolVoxSizeY(vol);
	myhdr->img.pixdim[3] = mVipVolVoxSizeZ(vol);
	myhdr->img.pixdim[4] = mVipVolVoxSizeT(vol);

	myhdr->img.pixdim[10] = 1.; /*scale*/


	vmin = VipGetVolumeMin(vol);
	vmax = VipGetVolumeMax(vol);
	myhdr->img.glmax = (int)vmax+1;
	myhdr->img.glmin = (int)vmin;

	myhdr->key.size = sizeof(struct analyze_db);
	sprintf (myhdr->key.data_type, "%.1s", "\0");
	sprintf (myhdr->key.name, "%.1s", "\0");


	sprintf (myhdr->his.descrip, "%s", "Copyright: Jeff");
	sprintf (myhdr->his.aux_file, "%.1s", "\0");
	myhdr->his.orient = ' ';
	sprintf  (myhdr->his.generated, "%.1s", "\0");
	sprintf (myhdr->his.scannum, "%.1s", "\0");
	sprintf (myhdr->his.patient_id, "%.1s", "\0");
	sprintf (myhdr->his.exp_date, "%.1s", "\0");
	sprintf (myhdr->his.exp_time, "%.1s", "\0");
	sprintf (myhdr->his.hist_un0, "%.1s", "\0");

	myhdr->his.views = 0;
	myhdr->his.start_field = 32768;
	myhdr->his.field_skip=8192;
	myhdr->his.omax = 0;
	myhdr->his.omin = 0;
	myhdr->his.smax = 32;
	myhdr->his.smin = 0;

	if((n=fwrite((char *)myhdr,sizeof(struct analyze_db),1,header_file)) !=1)
	{
	    sprintf(message,"error writing to %s \n",header_name);
	    VipPrintfError("message");
	    return(PB);
	}
	fclose(header_file);

	return(OK);
}
