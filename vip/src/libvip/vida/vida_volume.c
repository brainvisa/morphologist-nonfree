/*****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vida_volume.c        * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 17/01/1997
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
 *              /  /98|  F. POUPON   | Prise en compte des scale et Offset
 *                    |              |  ainsi que du VmdType
 *            18/11/98|  FROUIN      | Prise en compte des start et dur time
 *****************************************************************************/

#ifdef VIDA_LIB
#include <vidaIO/kernel/vidaio.h>
#endif

#include <vip/util.h>
#include <vip/volume.h>
#include <vip/vida.h>
#include <vip/alloc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>
#include <assert.h>

/*                                 static flags for (Volume*)->state  */

#define STATE_DECLARED	1	/* structure declared */
#define STATE_ALLOCATED	2	/* memory allocated for data */
#define STATE_FILLED	3	/* significant data */
#define STATE_FREED	4	/* structure freed */	



/* local functions: declarations */ 
#ifdef VIDA_LIB
static char *imaFile(const char *filename);
#endif
/*static char *hdrFile(const char *filename);
static char *infoFile(const char *filename);
*/


int VipGetVmdtype(const char*);


/*==========================*
 |                          |
 |  input-output functions  |
 |                          |
 *==========================*/


#ifdef VIDA_LIB
/*----------------------------------------------------------------------------*/
static
char *imaFile ( const char *filename )
/*----------------------------------------------------------------------------*/
/* Creates a filename ending on '.vimg' */
{
  size_t l;
  char *filenameIma, *extIma = ".vimg", *extHdr = ".vhdr", *extInfo = ".vinfo";
                                                 

  if ( filename == NULL )
  {
    VipPrintfError ( "filename == NULL, contact a Vip administrator !" );
    VipPrintfExit( "(volume)imaFile" );
    return(NULL);
  }
  l = strlen ( filename );

  filenameIma = VipMalloc ( l + 6, "imaFile : can't malloc" );

  if(filenameIma==NULL) return(NULL);

  strcpy ( filenameIma, filename );

  if ( l <= 6 || strcmp ( filenameIma+l-6, extInfo ) )
  {
  if ( l<=5 || (strcmp ( filenameIma+l-5, extIma ) && strcmp ( filenameIma+l-5, extHdr )) )
        strcat ( filenameIma, extIma );
    else 
    	strcpy ( filenameIma+l-5, extIma );
  }
  else
  	{
	strcpy ( filenameIma+l-6, extIma );
	*(filenameIma+l-1) = (char ) '\0';
	}

  return ( filenameIma );
  
}
#endif

/*----------------------------------------------------------------------------*/
/*static
char *hdrFile (const char *filename )
*/
/*----------------------------------------------------------------------------*/
/* Creates a filename ending on '.vhdr' */
/*{
  size_t l;
  char *filenameIma, *extIma = ".vimg", *extHdr = ".vhdr", *extInfo = ".vinfo";
                                                 

  if ( filename == NULL )
  {
    VipPrintfError ( "filename == NULL, contact a Vip administrator !" );
    VipPrintfExit( "(volume)imaFile" );
  }
  l = strlen ( filename );

  filenameIma = VipMalloc ( l + 6 , "hdrFile : can't malloc");

  strcpy ( filenameIma, filename );

  if ( l <= 6 || strcmp ( filenameIma+l-6, extInfo ) )
  {
  if ( l<=5 || (strcmp ( filenameIma+l-5, extIma ) && strcmp ( filenameIma+l-5, extHdr )) )
        strcat ( filenameIma, extHdr );
    else 
    	strcpy ( filenameIma+l-5, extHdr );
  }
  else
  	{
	strcpy ( filenameIma+l-6, extHdr );
	*(filenameIma+l-1) = (char ) '\0';
	}

  return ( filenameIma );
  
}
*/

/*----------------------------------------------------------------------------*/
/*static
char *infoFile ( const char *filename )
*/
/*----------------------------------------------------------------------------*/
/* Creates a filename ending on '.vinfo' */
/*{
  size_t l;
  char *filenameIma, *extIma = ".vimg", *extHdr = ".vhdr", *extInfo = ".vinfo";
                                                 

  if ( filename == NULL )
  {
    VipPrintfError ( "filename == NULL, contact a Vip administrator !" );
    VipPrintfExit( "(volume)imaFile" );
  }
  l = strlen ( filename );

  filenameIma = VipMalloc ( l + 6 , "infoFile : can't malloc");

  strcpy ( filenameIma, filename );

  if ( l <= 6 || strcmp ( filenameIma+l-6, extInfo ) )
  {
  if ( l<=5 || (strcmp ( filenameIma+l-5, extIma ) && strcmp ( filenameIma+l-5, extHdr )) )
        strcat ( filenameIma, extInfo );
    else 
    	strcpy ( filenameIma+l-5, extInfo );
  }
  else
  	{
	strcpy ( filenameIma+l-6, extInfo );
	}

  return ( filenameIma );
  
}
*/

/*----------------------------------------------------------------------------*/
	
Volume *ReadVidaVolumeDim

/*----------------------------------------------------------------------------*/
(
        const char      *filename
)
{

#ifdef VIDA_LIB

   char                 *filenameIma;
   VIDAim		*vp;


   int                  vmdtype;


   VIP_DEC_VOLUME(vol);
   int frame, *start_pt, *dur_pt;


   vmdtype = VipGetVmdtype( filename );
   if(vmdtype==PB) return(PB);
   filenameIma = imaFile ( filename );
   if(filenameIma==PB) return(PB);
   vp = VidaOpen(filenameIma, VidaConsigne(VMODE,       "r",
					   VMDTYPE,      vmdtype,
					   NULL));
   free ( filenameIma );
   if ( vp == NULL )
     {
     VipPrintfError ( "Vida image exists but is unreadable" );
     VipPrintfExit( "(Vida_volume)imaFile" );
     return(NULL);
     }
 
   vol = VipDeclare4DVolumeStructure(
                   VIDA_X(vp),
                   VIDA_Y(vp),
                   VIDA_Z(vp),
                   VIDA_T(vp),
                   VIDA_PX(vp),
                   VIDA_PY(vp),
                   VIDA_PZ(vp),
                   VIDA_PT(vp),
                   S16BIT,                         
                   (char *)filename,
                   0);

   /** rajoute le 22/04/1999 - F. Poupon **/
   vol->shfj->unite = (int) vp->shfj_db->val_calib.active; 

   /* Les trois lignes suivantes donnent les infos sur le scaling des
      images Vida. F. Poupon */
   vol->shfj->VmdType = vmdtype;
   vol->shfj->scale = GetCalib(vp->shfj_db, 0, 0);
   vol->shfj->offset = GetOffset(vp->shfj_db, 0, 0);
   /* La boucle permet d'initialiser les champs concernant les series dyn    */
   /* Le test preliminaire permet de savoir si l'allocation de memoire a ete
      faite ou non pour la zone de stockage de l'info temporelle */
   if (vol->shfj->start_time && vol->shfj->dur_time)
      for (frame=0,
               start_pt = vol->shfj->start_time,
               dur_pt = vol->shfj->dur_time;
           frame < VIDA_T(vp); frame++, start_pt++, dur_pt++)
        {
        *start_pt = GetTimeStart(vp->shfj_db, frame);
        *dur_pt   = GetTimeDura(vp->shfj_db, frame);
        }

   VidaClose(vp);
   return ( vol );

#else
  (void)(filename);
  char message[VIP_NAME_MAXLEN];

  sprintf(message,"Your Vip version has not been compiled to be Vida\n \
          compatible (use #define VIDA_LIB)");
  VipPrintfError(message);
  return(NULL);

#endif
}

/*--------------------------------------------------------------------------*/
        int      VipGetVmdtype

/*--------------------------------------------------------------------------*/
(
       const char      *filename
)
{

#ifdef VIDA_LIB

   char                 *filenameIma;
   VIDAim               *vp;
   int                  fr, sl, flag;


   filenameIma = imaFile ( filename );
   vp = VidaOpen(filenameIma, VidaConsigne(VMODE,               "r",
                                        VMDTYPE,        FX_SHORT_FX,
                                        NULL));
   free ( filenameIma );
   if ( vp == NULL )
     {
     VipPrintfError ( "Vida image exists but is unreadable" );
     VipPrintfExit( "(Vida_volume)imaFile" );
     return(PB);
     }
   flag = VTRUE;
   for (fr = 0; fr < VIDA_T(vp); fr++)
      for (sl = 0; sl < VIDA_Z(vp); sl++)
         {
         flag = flag &&
                 (GetCalib(vp->shfj_db,sl,fr)==GetCalib(vp->shfj_db,0,0));
         flag = flag &&
                 (GetOffset(vp->shfj_db,sl,fr)==GetOffset(vp->shfj_db,0,0));
         }
   VidaClose(vp);
   if (flag)                          /* egalite des scale et offset     */
                                      /* sur la serie 3D ou 4D           */
     {
     return( (int)FX_SHORT_FX);
     }
     else
     {
     return(  (int)FX_SHORT_FL);
     }

#else
  (void)(filename);
  char message[VIP_NAME_MAXLEN];

  sprintf(message,"Your Vip version has not been compiled to be Vida\n \
          compatible (use #define VIDA_LIB)");
  VipPrintfError(message);
  return(PB);

#endif
}

/*----------------------------------------------------------------------------*/

        Volume      *ReadVidaVolume

/*----------------------------------------------------------------------------*/
(
        const char	*filename,
        int		borderWidth
)
{

#ifdef VIDA_LIB

   char                 *filenameIma;
   VIDAim		*vp;

   short int		*buffer, *ptbuff;
   Vip_S16BIT		*ptvol;
   int                  frame, slice, line;
   long                 oL, oLbS, oSbV;
   int                  vmdtype;
   int  		*start_pt, *dur_pt;
   int mot;



   VIP_DEC_VOLUME(vol);

   vmdtype = VipGetVmdtype( filename );
   if(vmdtype==PB) return(PB);
   filenameIma = imaFile ( filename );
   if(filenameIma==PB) return(PB);

 
   vp = VidaOpen(filenameIma, VidaConsigne(VMODE,       "r",
					   VMDTYPE,      vmdtype,
					   NULL));
   free ( filenameIma );

   if ( vp == NULL )
     {
     VipPrintfError ( "Vida image exists but is unreadable" );
     VipPrintfExit( "(Vida_volume)imaFile" );
     return(NULL);

     }
   if( VIDA_T(vp) <= 1 )
      {
      vol = VipCreate3DVolume(
                   VIDA_X(vp),
                   VIDA_Y(vp),
                   VIDA_Z(vp),
                   VIDA_PX(vp),
                   VIDA_PY(vp),
                   VIDA_PZ(vp),
                   S16BIT,                          /* Valeur de depart */
                   (char *)filename,
                   borderWidth);
      if ( vol == NULL) goto abort;
      }
      else                                          /*Cas 4D */
      {
      vol = VipCreate4DVolume(
                   VIDA_X(vp),
                   VIDA_Y(vp),
                   VIDA_Z(vp),
                   VIDA_T(vp),
                   VIDA_PX(vp),
                   VIDA_PY(vp),
                   VIDA_PZ(vp),
                   VIDA_PT(vp),
                   S16BIT,                          /* Valeur de depart */
                   (char *)filename,
                   borderWidth);
      if ( vol == NULL) goto abort;
      }
   
                                                    /* On suppose d'emblee */
                                                    /* du 16 bit signe     */


   /*elimination de VidaAlloc qui alloue un buffer de la taille totale
     du volume, JFM*/
   switch(vp->consign->type)
  { case FL_FLOAT_FL :
    case FX_INT_FX   :
    case FX_INT_FL   : mot = 4;break;
    case FX_SHORT_FX :
    case FX_SHORT_FL : mot = 2;break;
    default              : fprintf(stderr, "VidaAlloc: unknown VIDAim type\n");
                           return(NULL);
                           break;
  }	
   
   if(vp->ana_db==NULL) return(NULL);

   buffer = (short int *) malloc(vp->ana_db->img.dim[1] * vp->ana_db->img.dim[2] * mot);
   if (!buffer) 
       {
	   VipPrintfExit( "(Vida_volume)VipReadVidaVolume : can't malloc\n");
	   return(NULL);
       }
   ptvol = VipGetDataPtr_S16BIT(vol) + VipOffsetFirstPoint(vol);
   oL    = VipOffsetPointBetweenLine(vol);
   oLbS  = VipOffsetLineBetweenSlice(vol);
   oSbV  = VipOffsetSliceBetweenVolume(vol);

   for(frame=0; frame < VIDA_T(vp); ++frame)
     {
     for(slice=0; slice < VIDA_Z(vp); ++slice)
       {
       if ( VidaRead((char *)buffer, slice, frame, vp) == 0)
	   {
	       VipPrintfExit( "libvidaio : VidaRead : Pb to read short image\n");
	       return(NULL);
	   }
       ptbuff = buffer;
       for(line=0; line < VIDA_Y(vp); ++line)
         {
         memcpy( (char *)ptvol, (char *)ptbuff, VIDA_X(vp)*sizeof(*ptvol) );
         ptvol  += oL + VIDA_X(vp);
         ptbuff += VIDA_X(vp);
         }
       ptvol += oLbS;          
       }
     ptvol  += oSbV;
     fprintf(stderr, ".");
     }
   fprintf(stderr, "\n");
   /** rajoute le 22/04/1999 - F. Poupon **/
   vol->shfj->unite = (int) vp->shfj_db->val_calib.active; 

   /* Les trois lignes suivantes donnent les infos sur le scaling des
      images Vida. F. Poupon */
   vol->shfj->VmdType = vmdtype;
   vol->shfj->scale = GetCalib(vp->shfj_db, 0, 0);
   vol->shfj->offset = GetOffset(vp->shfj_db, 0, 0);
   /* La boucle permet d'initialiser les champs concernant les series dyn    */
   /* Le test preliminaire permet de savoir si l'allocation de memoire a ete
      faite ou non pour la zone de stockage de l'info temporelle */
   if (vol->shfj->start_time && vol->shfj->dur_time)
      for (frame=0,
               start_pt = vol->shfj->start_time,
               dur_pt = vol->shfj->dur_time;
           frame < VIDA_T(vp); frame++, start_pt++, dur_pt++)
        {
        *start_pt = GetTimeStart(vp->shfj_db, frame);
        *dur_pt   = GetTimeDura(vp->shfj_db, frame);
        }

   free(buffer); /* <---- OOOOhhhhh c'est pas propre de l'oublier!!!  F.P.*/

   VidaClose(vp);
   return ( vol );

abort:    
   VidaClose(vp);
   VipPrintfExit ( "(volume)VipReadVidaVolume" );
   return ( NULL );
#else
  (void)(filename);
  (void)(borderWidth);
  char message[VIP_NAME_MAXLEN];

  sprintf(message,"Your Vip version has not been compiled to be Vida\n \
          compatible (use #define VIDA_LIB)");
  VipPrintfError(message);
  return(NULL);

#endif
}

/*----------------------------------------------------------------------------*/
	
    Volume *ReadVidaFloatVolumeWithBorder

/*----------------------------------------------------------------------------*/
(
        const char	*filename,
        int		borderWidth
)
{

#ifdef VIDA_LIB

   char                 *filenameIma;
   VIDAim               *vp;

   float               *buffer, *ptbuff;
   Vip_FLOAT           *ptvol;
   int                  frame, slice, line;
   long                 oL, oLbS, oSbV;
   int 			*start_pt, *dur_pt;
   int mot;


   VIP_DEC_VOLUME(vol);

   if(borderWidth!=0)
     {
     VipPrintfError ( "Sorry, float volume lecture with border not implemented" );
     VipPrintfExit( "(Vida_volume)imaFile" );
     return(NULL);
     }

   filenameIma = imaFile ( filename );
   vp = VidaOpen(filenameIma, VidaConsigne(VMODE,       "r",
                                           VMDTYPE,      FL_FLOAT_FL,
                                           NULL));
   free ( filenameIma );
   if ( vp == NULL )
     {
     VipPrintfError ( "Vida image exists but is unreadable" );
     VipPrintfExit( "(Vida_volume)imaFile" );
     return(NULL);
     }
   if( VIDA_T(vp) <= 1 )
      {
      vol = VipCreate3DVolume(
                   VIDA_X(vp),
                   VIDA_Y(vp),
                   VIDA_Z(vp),
                   VIDA_PX(vp),
                   VIDA_PY(vp),
                   VIDA_PZ(vp),
                   VFLOAT,                          /* Valeur de depart */
                   (char *)filename,
                   0);
      if ( vol == NULL) goto abort;
      }
      else                                          /*Cas 4D */
      {
      vol = VipCreate4DVolume(
                   VIDA_X(vp),
                   VIDA_Y(vp),
                   VIDA_Z(vp),
                   VIDA_T(vp),
                   VIDA_PX(vp),
                   VIDA_PY(vp),
                   VIDA_PZ(vp),
                   VIDA_PT(vp),
                   VFLOAT,                          /* Valeur de depart */
                   (char *)filename,
                   0);
      if ( vol == NULL) goto abort;
      }

   vol->shfj->unite = (int) vp->shfj_db->val_calib.active;

   /*elimination de VidaAlloc qui alloue un buffer de la taille totale
     du volume, JFM*/
   switch(vp->consign->type)
       { case FL_FLOAT_FL :
       case FX_INT_FX   :
       case FX_INT_FL   : mot = 4;break;
       case FX_SHORT_FX :
       case FX_SHORT_FL : mot = 2;break;
       default              : fprintf(stderr, "VidaAlloc: unknown VIDAim type\n");
	   return(NULL);
	   break;
       }	

   if(vp->ana_db==NULL) return(NULL);

   buffer = (float *) malloc(vp->ana_db->img.dim[1] * vp->ana_db->img.dim[2] * mot);
   if (!buffer)
       {
	   VipPrintfExit( "ReadVidaFloatVolumeWithBorder : can't malloc\n");
	   return(NULL);
       }
   ptvol = VipGetDataPtr_VFLOAT(vol) + VipOffsetFirstPoint(vol);
   oL    = VipOffsetPointBetweenLine(vol);
   oLbS  = VipOffsetLineBetweenSlice(vol);
   oSbV  = VipOffsetSliceBetweenVolume(vol);

   for(frame=0; frame < VIDA_T(vp); frame++)
     {
     for(slice=0; slice < VIDA_Z(vp); slice++)
       {
       if ( VidaRead((char *)buffer, slice, frame, vp) == 0)
	   {
	       VipPrintfExit( "libvidaio : VidaRead : Pb to read float image\n");
	       return(NULL);
	   }
       ptbuff = buffer;
       for(line=0; line < VIDA_Y(vp); line++)
         {
         memcpy( (char *)ptvol, (char *)ptbuff, VIDA_X(vp)*sizeof(*ptvol) );
         ptvol  += oL + VIDA_X(vp);
         ptbuff += VIDA_X(vp);
         }
       ptvol += oLbS;     
       }
     ptvol += oSbV; /* Offset intervolume*/
     }

   /* Les trois lignes suivantes donnent les infos sur le scaling des
      images Vida. F. Poupon */
   vol->shfj->VmdType = (int)FL_FLOAT_FL;
   vol->shfj->scale = GetCalib(vp->shfj_db, 0, 0);
   vol->shfj->offset = GetOffset(vp->shfj_db, 0, 0);
   /* La boucle permet d'initialiser les champs concernant les series dyn    */
   /* Le test preliminaire permet de savoir si l'allocation de memoire a ete
      faite ou non pour la zone de stockage de l'info temporelle */
   if (vol->shfj->start_time && vol->shfj->dur_time)
      for (frame=0,
               start_pt = vol->shfj->start_time,
               dur_pt = vol->shfj->dur_time;
           frame < VIDA_T(vp); frame++, start_pt++, dur_pt++)
        {
        *start_pt = GetTimeStart(vp->shfj_db, frame);
        *dur_pt   = GetTimeDura(vp->shfj_db, frame);
        }

   free(buffer); /* <---- OOOOhhhhh c'est pas propre de l'oublier!!!  F.P.*/

   VidaClose(vp);
   return ( vol );

abort:   
   VidaClose(vp);
   VipPrintfExit ( "(volume)VipReadVidaVolumeWithBorder" );
   return ( NULL );

#else
  (void)(filename);
  (void)(borderWidth);
  char message[VIP_NAME_MAXLEN];

  sprintf(message,"Your Vip version has not been compiled to be Vida\n \
          compatible (use #define VIDA_LIB)");
  VipPrintfError(message);
  return(NULL);

#endif
}
/*----------------------------------------------------------------------------*/
	
    Volume *ReadVidaFrame
/*----------------------------------------------------------------------------*/
(
        const char	*filename,
        int		fr1,               /* 1ere frame a lire      */
        int		fr2                /* derniere frame a lire  */
)
{

#ifdef VIDA_LIB

   char                 *filenameIma;
   VIDAim               *vp;

   short int            *buffer, *ptbuff;
   Vip_S16BIT           *ptvol;

   int                  frame, slice, line, fr;
   long                 oL, oLbS, oSbV;
   int                  vmdtype;
   int 			*start_pt, *dur_pt;


   VIP_DEC_VOLUME(vol);

   vmdtype = VipGetVmdtype( filename );
   if(vmdtype==PB) return(PB);
   filenameIma = imaFile ( filename );
   if(filenameIma==PB) return(PB);

   vp = VidaOpen(filenameIma, VidaConsigne(VMODE,       "r",
                                           VMDTYPE,      vmdtype,
                                           VUNITE,       VIDA_SANSUNITE,
                                           NULL));

   free ( filenameIma );
   if ( vp == NULL )
     {
     VipPrintfError ( "Vida image exists but is unreadable" );
     VipPrintfExit( "(Vida_volume)imaFile" );
     fflush(stdout);
     return(NULL);
     }
   if ( fr1 > fr2 ){
     fr = fr2;
     fr2 = fr1;
     fr1 = fr;
   }
   if( (fr1 < 0) || (fr2 >= VIDA_T(vp)) ){
     VipPrintfError ( "Numero de frame incorrect" );
     VipPrintfExit( "(Vida_volume)imaFile" );
     fflush(stdout);
     return(NULL);
   }
   if( fr1 == fr2 )
      {
      vol = VipCreate3DVolume(
                   VIDA_X(vp),
                   VIDA_Y(vp),
                   VIDA_Z(vp),
                   VIDA_PX(vp),
                   VIDA_PY(vp),
                   VIDA_PZ(vp),
                   S16BIT,                          
                   (char *)filename,
                   0);
      if ( vol == NULL) goto abort;
      }
      else                                        
      {
      vol = VipCreate4DVolume(
                   VIDA_X(vp),
                   VIDA_Y(vp),
                   VIDA_Z(vp),
                   fr2-fr1+1,
                   VIDA_PX(vp),
                   VIDA_PY(vp),
                   VIDA_PZ(vp),
                   VIDA_PT(vp),
                   S16BIT,                          /* Valeur de depart */
                   (char *)filename,
                   0);
      if ( vol == NULL) goto abort;
      }

   buffer = (short int *) VidaAlloc(vp);
   if (!buffer)
       {
	   VipPrintfExit( "ReadVidaFrame : can't malloc\n");
	   return(NULL);
       }
   ptvol = VipGetDataPtr_S16BIT(vol) + VipOffsetFirstPoint(vol);
   oL    = VipOffsetPointBetweenLine(vol);
   oLbS  = VipOffsetLineBetweenSlice(vol);
   oSbV  = VipOffsetSliceBetweenVolume(vol);

   for(frame=fr1; frame <= fr2; frame++)
     {
     for(slice=0; slice < VIDA_Z(vp); slice++)
       {
       if ( VidaRead((char *)buffer, slice, frame, vp) == 0)
	   {
	       VipPrintfExit( "libvidaio : VidaRead : Pb to read float image\n");
	       return(NULL);
	   }
       ptbuff = buffer;
       for(line=0; line < VIDA_Y(vp); line++)
         {
         memcpy( (char *)ptvol, (char *)ptbuff, VIDA_X(vp)*sizeof(*ptvol) );
         ptvol  += oL + VIDA_X(vp);
         ptbuff += VIDA_X(vp);
         }
       ptvol += oLbS;     
       }
     ptvol += oSbV; /* Offset intervolume*/
     }

   /** rajoute le 22/04/1999 - F. Poupon **/
   vol->shfj->unite = (int) vp->shfj_db->val_calib.active; 

   /* Les trois lignes suivantes donnent les infos sur le scaling des
      images Vida. F. Poupon */
   vol->shfj->VmdType = vmdtype;
   vol->shfj->scale = GetCalib(vp->shfj_db, 0, 0);
   vol->shfj->offset = GetOffset(vp->shfj_db, 0, 0);
   /* La boucle permet d'initialiser les champs concernant les series dyn    */
   /* Le test preliminaire permet de savoir si l'allocation de memoire a ete
      faite ou non pour la zone de stockage de l'info temporelle */
   if (vol->shfj->start_time && vol->shfj->dur_time)
      for (frame=fr1,
               start_pt = vol->shfj->start_time,
               dur_pt = vol->shfj->dur_time;
           frame < fr2; frame++, start_pt++, dur_pt++)
        {
        *start_pt = GetTimeStart(vp->shfj_db, frame);
        *dur_pt   = GetTimeDura(vp->shfj_db, frame);
        }

   free(buffer);
   VidaClose(vp);
   return ( vol );

abort:   
   VidaClose(vp);
   (void) fprintf ( stderr, "Vida file: '%s'\n",
                filenameIma );
   VipPrintfExit ( "(volume)VipReadVidaVolume" );
   return ( NULL );

#else
  (void)(filename);
  (void)(fr1);
  (void)(fr2);
  char message[VIP_NAME_MAXLEN];

  sprintf(message,"Your Vip version has not been compiled to be Vida\n \
          compatible (use #define VIDA_LIB)");
  VipPrintfError(message);
  return(NULL);

#endif
}

/*----------------------------------------------------------------------------*/
	
    Volume *ReadVidaFloatFrame

/*----------------------------------------------------------------------------*/
(
        const char	*filename,
        int		fr1,               /* 1ere frame a lire      */
        int		fr2                /* derniere frame a lire  */
)
{

#ifdef VIDA_LIB

   char                 *filenameIma;
   VIDAim               *vp;

   float               *buffer, *ptbuff;
   Vip_FLOAT           *ptvol;
   int                  frame, slice, line, fr;
   long                 oL, oLbS, oSbV;
   int			*start_pt, *dur_pt;

   VIP_DEC_VOLUME(vol);

   filenameIma = imaFile ( filename );
   vp = VidaOpen(filenameIma, VidaConsigne(VMODE,       "r",
                                           VMDTYPE,      FL_FLOAT_FL,
                                           NULL));
   free ( filenameIma );
   if ( vp == NULL )
     {
     VipPrintfError ( "Vida image exists but is unreadable" );
     VipPrintfExit( "(Vida_volume)imaFile" );
     return(NULL);
     }
   if ( fr1 > fr2 ){
     fr = fr2;
     fr2 = fr1;
     fr1 = fr;
   }
   if( (fr1 < 0) || (fr2 >= VIDA_T(vp)) ){
     VipPrintfError ( "Numero de frame incorrect" );
     VipPrintfExit( "(Vida_volume)imaFile" );
     return(NULL);
   }
   if( fr1 == fr2 )
      {
      vol = VipCreate3DVolume(
                   VIDA_X(vp),
                   VIDA_Y(vp),
                   VIDA_Z(vp),
                   VIDA_PX(vp),
                   VIDA_PY(vp),
                   VIDA_PZ(vp),
                   VFLOAT,                          
                   (char *)filename,
                   0);
      if ( vol == NULL) goto abort;
      }
      else                                        
      {
      vol = VipCreate4DVolume(
                   VIDA_X(vp),
                   VIDA_Y(vp),
                   VIDA_Z(vp),
                   fr2-fr1+1,
                   VIDA_PX(vp),
                   VIDA_PY(vp),
                   VIDA_PZ(vp),
                   VIDA_PT(vp),
                   VFLOAT,                          /* Valeur de depart */
                   (char *)filename,
                   0);
      if ( vol == NULL) goto abort;
      }

   vol->shfj->unite = (int) vp->shfj_db->val_calib.active;

   buffer = (float *) VidaAlloc(vp);
   if (!buffer) 
       {
	   VipPrintfExit( "ReadVidaFloatFrame : can't malloc\n");
	   return(NULL);
       }
   ptvol = VipGetDataPtr_VFLOAT(vol) + VipOffsetFirstPoint(vol);
   oL    = VipOffsetPointBetweenLine(vol);
   oLbS  = VipOffsetLineBetweenSlice(vol);
   oSbV  = VipOffsetSliceBetweenVolume(vol);

   for(frame=fr1; frame <= fr2; frame++)
     {
     for(slice=0; slice < VIDA_Z(vp); slice++)
       {
       if ( VidaRead((char *)buffer, slice, frame, vp) == 0)
	   {
	       VipPrintfExit( "libvidaio : VidaRead : Pb to read float image\n");
	       return(NULL);
	   }
       ptbuff = buffer;
       for(line=0; line < VIDA_Y(vp); line++)
         {
         memcpy( (char *)ptvol, (char *)ptbuff, VIDA_X(vp)*sizeof(*ptvol) );
         ptvol  += oL + VIDA_X(vp);
         ptbuff += VIDA_X(vp);
         }
       ptvol += oLbS;     
       }
     ptvol += oSbV; /* Offset intervolume*/
     }

   /* Les trois lignes suivantes donnent les infos sur le scaling des
      images Vida. Pour le type FL_FLOAT_FL, on a normalement scale=1.0 et 
      offset=0.0. F. Poupon */
   vol->shfj->VmdType = (int)FL_FLOAT_FL;
   vol->shfj->scale = GetCalib(vp->shfj_db, 0, 0);
   vol->shfj->offset = GetOffset(vp->shfj_db, 0, 0);
   /* La boucle permet d'initialiser les champs concernant les series dyn    */
   /* Le test preliminaire permet de savoir si l'allocation de memoire a ete
      faite ou non pour la zone de stockage de l'info temporelle */
   if (vol->shfj->start_time && vol->shfj->dur_time)
      for (frame=0,
               start_pt = vol->shfj->start_time,
               dur_pt = vol->shfj->dur_time;
           frame < VIDA_T(vp); frame++, start_pt++, dur_pt++)
        {
        *start_pt = GetTimeStart(vp->shfj_db, frame);
        *dur_pt   = GetTimeDura(vp->shfj_db, frame);
        }

   free(buffer);
   VidaClose(vp);
   return ( vol );

abort:   
   VidaClose(vp);
   (void) fprintf ( stderr, "Vida file: '%s'\n",
                filenameIma );
   VipPrintfExit ( "(volume)VipReadVidaVolume" );
   return ( NULL );

#else
  (void)(filename);
  (void)(fr1);
  (void)(fr2);
  char message[VIP_NAME_MAXLEN];

  sprintf(message,"Your Vip version has not been compiled to be Vida\n \
          compatible (use #define VIDA_LIB)");
  VipPrintfError(message);
  return(NULL);

#endif
}

/*----------------------------------------------------------------------------*/
    int                    WriteVidaVolume
/* The volume is writen in VIDA format without scale factors    */
/*----------------------------------------------------------------------------*/
(
        Volume      *volume,
        char        *ima_name
)
{

#ifdef VIDA_LIB

   char                 *filenameIma;
   VIDAim               *vp;
   int                  frame, slice, line, col;
   long                 oL, oLbS, oSbV;
   int                  type;
  

   Vip_S16BIT           *sptvol;
   Vip_FLOAT            *fptvol;
   Vip_DOUBLE           *dptvol;

   short                *s_pt, *s_ima;



                                           /* FX_SHORT_FX ONLY */
   switch( VipTypeOf(volume) )
    {
    case VFLOAT  :
    case VDOUBLE :
    case S16BIT :
       type = FX_SHORT_FX;
       break;
    default:
       VipPrintfError ( "Vip Image type is not writable in Vida format\n" );
       VipPrintfExit( "(Vida_volume)WriteVidaVolume" );
       return(PB);
    }
 

   filenameIma = imaFile ( ima_name );
   vp = VidaOpen(filenameIma, VidaConsigne(VMODE,               "w",
                  VMDTYPE,   type,
                  VSIZE_X,   VipSizexOf(volume),
                  VSIZE_Y,   VipSizeyOf(volume),
                  VSIZE_Z,   VipSizezOf(volume),
                  VSIZE_T,   VipSizetOf(volume),
                  VSIZE_PX,  mVipVolVoxSizeX(volume),
                  VSIZE_PY,  mVipVolVoxSizeY(volume),
                  VSIZE_PZ,  mVipVolVoxSizeZ(volume),
                  VSIZE_PT,  mVipVolVoxSizeT(volume),
                  VUNITE,    mVipVolShfjUnite(volume), /*VIDA_SANSUNITE,*/
                  NULL));                              /* le 22/04/1999 */
   free ( filenameIma );                               /* F. Poupon */
   if ( vp == NULL )
     {
     VipPrintfError ( "Vida image is unwritable\n" );
     VipPrintfExit( "(Vida_volume)WriteVidaVolume" );
     return(PB);
     }


   oL    = VipOffsetPointBetweenLine(volume);
   oLbS  = VipOffsetLineBetweenSlice(volume);
   oSbV  = VipOffsetSliceBetweenVolume(volume);


   s_ima = (short int *)VidaAlloc(vp);
   if (!s_ima)
        {
        VipPrintfError ( "Can't malloc\n" );
        VipPrintfExit( "(Vida_volume)WriteVidaVolume" );
        return(PB);
        }
   switch ( VipTypeOf(volume) )
     {
     case S16BIT :
         sptvol = VipGetDataPtr_S16BIT(volume) + VipOffsetFirstPoint(volume);
         for (frame = 0; frame < VIDA_T(vp); frame++)
            {
            for (slice = 0; slice < VIDA_Z(vp); slice++)
               {
               s_pt = s_ima;
               for(line=0; line < VIDA_Y(vp); line++)
                 {
                 memcpy((char *)s_pt,(char *)sptvol,VIDA_X(vp)*sizeof(*sptvol));
                 sptvol += oL + VIDA_X(vp);
                 s_pt  += VIDA_X(vp);
                 }
               if (VidaWrite((char *)s_ima, slice, frame, vp) == -1)
                 {
                 VipPrintfExit("libvidaio : VidaWrite : Pb to write image\n");
                 return(PB);
                 }
               sptvol += oLbS;
               }
               sptvol += oSbV; /* Offset intervolume*/
            }
         free(s_ima);
         break;
     case VFLOAT :
         fptvol = VipGetDataPtr_VFLOAT(volume) + VipOffsetFirstPoint(volume);
         for (frame = 0; frame < VIDA_T(vp); frame++)
            {
            for (slice = 0; slice < VIDA_Z(vp); slice++)
               {
               s_pt = s_ima;
               for(line=0; line < VIDA_Y(vp); line++)
                 {
                 for(col=0; col < VIDA_X(vp); col++, s_pt++, fptvol++)
                    {
                    if (*fptvol > 32767.0)       *s_pt = 32767; 
                    else if (*fptvol < -32768.0) *s_pt = -32768;
                    else          *s_pt = (short) floor( (double) *fptvol);
                    }
                  fptvol += oL;
                  }
               if (VidaWrite((char *)s_ima, slice, frame, vp) == -1)
                 {
                 VipPrintfExit("libvidaio : VidaWrite : Pb to write image\n");
                 return(PB);
                 }
               fptvol += oLbS;
               }
            fptvol += oSbV;
            }
         break;
     case VDOUBLE :
         dptvol = VipGetDataPtr_VDOUBLE(volume) + VipOffsetFirstPoint(volume);
         for (frame = 0; frame < VIDA_T(vp); frame++)
            {
            for (slice = 0; slice < VIDA_Z(vp); slice++)
               {
               s_pt = s_ima;
               for(line=0; line < VIDA_Y(vp); line++)
                 {
                 for(col=0; col < VIDA_X(vp); col++, s_pt++, dptvol++)
                    {
                    if (*dptvol > 32767.0)       *s_pt = 32767;
                    else if (*dptvol < -32768.0) *s_pt = -32768;
                    else          *s_pt = (short) floor(*dptvol);
                    }
                  dptvol += oL;
                  }
               if (VidaWrite((char *)s_ima, slice, frame, vp) == -1)
                 {
                 VipPrintfExit("libvidaio : VidaWrite : Pb to write image\n");
                 return(PB);
                 }
               dptvol += oLbS;
               }
            dptvol += oSbV;
            }
         break;
     default:
        VipPrintfError ( "Vip Image type is not writable in Vida format\n" );
        VipPrintfExit( "(Vida_volume)WriteVidaVolume" );
        return(PB);
     }


   VidaClose(vp);
   return(OK);

#else
  (void)(volume);
  (void)(ima_name);
  char message[VIP_NAME_MAXLEN];

  sprintf(message,"Your Vip version has not been compiled to be Vida\n \
          compatible (use #define VIDA_LIB)");
  VipPrintfError(message);
  return(PB);

#endif
}

/*----------------------------------------------------------------------------*/
    int                    WriteVidaScaledVolume
/* The volume is writen in VIDA format with optimal scale factors*/
/*----------------------------------------------------------------------------*/
(
        Volume      *volume,
        char        *ima_name
)
{

#ifdef VIDA_LIB

   char                 *filenameIma;
   VIDAim               *vp;
   int                  frame, slice, line;
   long                 oL, oLbS, oSbV;
   int                  type;
   int calib_unite;
   
   
   Vip_S16BIT		      *sptvol;
   short                *s_pt, *s_ima;

   Vip_FLOAT            *fptvol;
   float                *f_pt, *f_ima;


                                           /* Deux modes d'ecritures seuls    */
                                           /* FX_SHORT_FX ou FL_FLOAT_FL      */
   switch( VipTypeOf(volume) )
    { 
    case VFLOAT :
       type = FL_FLOAT_FL;
       if ( volume->shfj->unite != VIDA_NOCALIB )
          calib_unite = volume->shfj->unite;
       else 
          calib_unite = VIDA_SANSUNITE;
       break;
    case S16BIT :
       type = FX_SHORT_FX;
       calib_unite = volume->shfj->unite;
       /* calib_unite = VIDA_NOCALIB; le 22/04/1999 - F. Poupon */
       break;
    default:
       VipPrintfError ( "Vip Image type is not writable in Vida format\n" );
       VipPrintfExit( "(Vida_volume)WriteVidaScaledVolume" );
       return(PB);
    } 

      
   filenameIma = imaFile ( ima_name );
   vp = VidaOpen(filenameIma, VidaConsigne(VMODE,               "w",
                  VMDTYPE,   type,
		  VSIZE_X,   VipSizexOf(volume), 
		  VSIZE_Y,   VipSizeyOf(volume),
		  VSIZE_Z,   VipSizezOf(volume),
		  VSIZE_T,   VipSizetOf(volume),
		  VSIZE_PX,  mVipVolVoxSizeX(volume),
		  VSIZE_PY,  mVipVolVoxSizeY(volume),
		  VSIZE_PZ,  mVipVolVoxSizeZ(volume),
		  VSIZE_PT,  mVipVolVoxSizeT(volume),
		  VUNITE, calib_unite,
		  NULL));
   free ( filenameIma );
   if ( vp == NULL )
     {
     VipPrintfError ( "Vida image is unwritable\n" );
     VipPrintfExit( "(Vida_volume)WriteVidaScaledVolume" );
     return(PB);
     }
   

   oL    = VipOffsetPointBetweenLine(volume);
   oLbS  = VipOffsetLineBetweenSlice(volume);
   oSbV  = VipOffsetSliceBetweenVolume(volume);

  switch(VIDA_TYPE(vp) )
     {
     case FL_FLOAT_FL :
        f_ima = (float *)VidaAlloc(vp);
        if (!f_ima) 
              {
              VipPrintfError ( "Can't malloc\n" );
              VipPrintfExit( "(Vida_volume)WriteVidaScaledVolume" );
	      return(PB);
              }
        fptvol = VipGetDataPtr_VFLOAT(volume)+VipOffsetFirstPoint(volume);
        for (frame = 0; frame < VIDA_T(vp); frame++)
           {
           for (slice = 0; slice < VIDA_Z(vp); slice++)
              {
              f_pt = f_ima;
              for(line=0; line < VIDA_Y(vp); line++)
                 {
                 memcpy((char *)f_pt,(char *)fptvol,VIDA_X(vp)*sizeof(*fptvol));
                 fptvol += oL + VIDA_X(vp);
                 f_pt  += VIDA_X(vp);
                 }
              if (VidaWrite((char *)f_ima, slice, frame, vp) == -1)
		{
		  VipPrintfExit("libvidaio : VidaWrite : Pb to write image\n");
		  return(PB);
		}
              fptvol += oLbS;
              }
           fptvol +=  oSbV; /* Varible d'offset inter volume*/
           }
        free(f_ima);
        break;   
     case FX_SHORT_FX :
        s_ima = (short int *)VidaAlloc(vp);
        if (!s_ima)
              {
              VipPrintfError ( "Can't malloc\n" );
              VipPrintfExit( "(Vida_volume)WriteVidaScaledVolume" );
	      return(PB);
              }
        sptvol = VipGetDataPtr_S16BIT(volume) + VipOffsetFirstPoint(volume);
        for (frame = 0; frame < VIDA_T(vp); frame++)
           {
           for (slice = 0; slice < VIDA_Z(vp); slice++)
              {
              s_pt = s_ima;
              for(line=0; line < VIDA_Y(vp); line++)
                 {
                 memcpy((char *)s_pt,(char *)sptvol,VIDA_X(vp)*sizeof(*sptvol));
                 sptvol += oL + VIDA_X(vp);
                 s_pt  += VIDA_X(vp);
                 }
              if (VidaWrite((char *)s_ima, slice, frame, vp) == -1)
		{
		  VipPrintfExit("libvidaio : VidaWrite : Pb to write image\n");
		  fflush(stdout);
		  return(PB);
		}
              sptvol += oLbS;
              } 
           sptvol += oSbV; /* Offset intervolume*/
           }
        free(s_ima);
        break;   
     default :
        VipPrintfError ( "Vida image is unexpectedly non-typed\n" );
        VipPrintfExit( "(Vida_volume)WriteVidaScaledVolume" );
	fflush(stdout);
	return(PB);
     }

   VidaClose(vp);
   return(OK);

#else
  (void)(volume);
  (void)(ima_name);
   char message[VIP_NAME_MAXLEN];

  sprintf(message,"Your Vip version has not been compiled to be Vida\n \
          compatible (use #define VIDA_LIB)");
  VipPrintfError(message);
  fflush(stdout);
  return(PB);

#endif
}
