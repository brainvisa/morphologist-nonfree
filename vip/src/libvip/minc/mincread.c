#ifdef MINC_LIB
#include <volume_io.h>

/* leger probleme, la structure Volume a le meme nom ds vip et minc...,
d'ou des jonglages debiles, je peux pas faire des boucles
avec les structures classique pour ajouter un bord...*/
#define U8BIT  10               /* 8 bit unsigned integer */
#define S8BIT  11               /* 8 bit signed integer */
#define U16BIT 20               /* 16 bit unsigned integer */
#define S16BIT 21               /* 16 bit signed integer */
#define U32BIT 40               /* 32 bit unsigned integer */
#define S32BIT 41               /* 32 bit signed integer */
#define VFLOAT  400             /* simple precision real */
#define VDOUBLE 800             /* double precision real */
/* Main program */



int ReadMinc3DData(int *type,char *name,char **out_ptr,int *xsize, int *ysize, int *zsize,
		float *xvoxsize, float *yvoxsize, float *zvoxsize)
{
   char infile[1024], message[1024];
   Volume volume;
   FILE *thefile;
    int        v1, v2, v3, sizes[MAX_DIMENSIONS];
    int length=0;
    char *ptr;
    STRING dim_names[3];

    dim_names[0] = create_string("zspace");
    dim_names[1] = create_string("yspace");
    dim_names[2] = create_string("xspace");

    

   strcpy (infile, name);
   infile[1023]='\0';
   strcat (infile, ".mnc");
   if ((thefile=fopen(infile,"r"))==NULL)
     {
       sprintf(message,"Can not open: %s",infile);
       printf ("%s\n", message);
       printf("ReadMincVolume\n");
       return(0);
     }
   else fclose(thefile);

   /* Read in input volume */
   /*  3 : prend le premier volume du du fichier,
    MIxspace... est sense orienter tout comme il faut...*/
   printf("Reading Minc volume...\n");
   if( input_volume( infile, 3, dim_names,
		     MI_ORIGINAL_TYPE, TRUE,
		     0.0, 0.0, TRUE, &volume,
		     (minc_input_options *) NULL ) != OK )
     return( 0 );

   if(volume->nc_data_type==NC_BYTE && volume->signed_flag==FALSE)
     {
       *type = U8BIT;
       length=1;
       printf("Unsigned byte\n");
     }
   else if(volume->nc_data_type==NC_BYTE && volume->signed_flag==TRUE)
     {
       *type = S8BIT;
       length=1;
     }  
   else if(volume->nc_data_type==NC_SHORT && volume->signed_flag==FALSE)
     {
       *type = U16BIT;
       length=2;
     }   
   else if(volume->nc_data_type==NC_SHORT && volume->signed_flag==TRUE)
     {
       printf("Signed short\n");
       *type = S16BIT;
       length=2;
     }   
   else if(volume->nc_data_type==NC_LONG && volume->signed_flag==FALSE)
     {
       *type = U32BIT;
       length=4;
     }   
   else if(volume->nc_data_type==NC_LONG && volume->signed_flag==TRUE)
     {
       *type = S32BIT;
       length=4;
     }  
   else if(volume->nc_data_type==NC_FLOAT)
     {
       printf("Float\n");
       *type = VFLOAT;
       length=4;
     }   
   else if(volume->nc_data_type==NC_DOUBLE)
     {
       *type = VDOUBLE;
       length = 8;
     }

    get_volume_sizes( volume, sizes );

    /*I have to recopy the volume because the array seems not
      a one dimensional buffer, like for low level IO*/

    *out_ptr = malloc(sizes[0]*sizes[1]*sizes[2]*length);
    if(!*out_ptr)
      {
	printf("Not enough memory in ReadMincData");
	return(0);
      }

    ptr = *out_ptr;
    switch(*type)
       {
       case U8BIT: 
	 for( v1 = 0;  v1 < sizes[0];  ++v1 ) {
	   for( v2 = 0;  v2 < sizes[1];  ++v2 ) {
	     for( v3 = 0;  v3 < sizes[2];  ++v3 ) {
	      *(unsigned char *)ptr = (unsigned char)rint(get_volume_real_value( volume, v1, v2, v3, 0, 0 ));
	      ptr+=length;
	     }
	   }
	 }
	 break;
       case S8BIT: 
	 for( v1 = 0;  v1 < sizes[0];  ++v1 ) {
	   for( v2 = 0;  v2 < sizes[1];  ++v2 ) {
	     for( v3 = 0;  v3 < sizes[2];  ++v3 ) {
	      *(char *)ptr = (char) get_volume_real_value( volume, v1, v2, v3,0, 0 );
	      ptr+=length;
	     }
	   }
	 }
	 break;
       case U16BIT: 
	 for( v1 = 0;  v1 < sizes[0];  ++v1 ) {
	   for( v2 = 0;  v2 < sizes[1];  ++v2 ) {
	     for( v3 = 0;  v3 < sizes[2];  ++v3 ) {
	      *(unsigned short *)ptr = (unsigned short)get_volume_real_value( volume, v1, v2, v3, 0, 0 );
	      ptr+=length;
	     }
	   }
	 }
	 break;
       case S16BIT: 
	 printf("%f\n",get_volume_real_value( volume, 100, 100, 50,0, 0 ));
	 for( v1 = 0;  v1 < sizes[0];  ++v1 ) {
	   for( v2 = 0;  v2 < sizes[1];  ++v2 ) {
	     for( v3 = 0;  v3 < sizes[2];  ++v3 ) {
	       /*JE ne comprend pas pourquoi le range real de mes IRM monte
a 500000. PEut etre j'ai merde en applicant le passage ds talairach. Pas clair
cette histoire*/
	      *(short *)ptr = (short) get_volume_voxel_value( volume, v1, v2, v3,0, 0 );
	      ptr+=length;
	     }
	   }
	 }
	 break;      
       case U32BIT: 
	 for( v1 = 0;  v1 < sizes[0];  ++v1 ) {
	   for( v2 = 0;  v2 < sizes[1];  ++v2 ) {
	     for( v3 = 0;  v3 < sizes[2];  ++v3 ) {
	      *(unsigned int *)ptr = (unsigned int)get_volume_real_value( volume, v1, v2, v3, 0, 0 );
	      ptr+=length;
	     }
	   }
	 }
	 break;
       case S32BIT: 
	 for( v1 = 0;  v1 < sizes[0];  ++v1 ) {
	   for( v2 = 0;  v2 < sizes[1];  ++v2 ) {
	     for( v3 = 0;  v3 < sizes[2];  ++v3 ) {
	      *(int *)ptr = (int) get_volume_real_value( volume, v1, v2, v3,0, 0 );
	      ptr+=length;
	     }
	   }
	 }
	 break;      
       case VFLOAT: 
	 for( v1 = 0;  v1 < sizes[0];  ++v1 ) {
	   for( v2 = 0;  v2 < sizes[1];  ++v2 ) {
	     for( v3 = 0;  v3 < sizes[2];  ++v3 ) {
	      *(float *)ptr = (float)get_volume_real_value( volume, v1, v2, v3, 0, 0 );
	      ptr+=length;
	     }
	   }
	 }
	 break;
       case VDOUBLE: 
	 for( v1 = 0;  v1 < sizes[0];  ++v1 ) {
	   for( v2 = 0;  v2 < sizes[1];  ++v2 ) {
	     for( v3 = 0;  v3 < sizes[2];  ++v3 ) {
	      *(double *)ptr = (double) get_volume_real_value( volume, v1, v2, v3,0, 0 );
	      ptr+=length;
	     }
	   }
	 }
	 break;     
 }

    *xsize = sizes[2];
    *ysize = sizes[1];
    *zsize = sizes[0];

    printf("Volume dimensions: %d %d %d\n", *xsize, *ysize, *zsize); 
    *xvoxsize = fabs(volume->separations[2]);
    *yvoxsize = fabs(volume->separations[1]);
    *zvoxsize = fabs(volume->separations[0]);
    printf("Voxel sizes: %f %f %f\n", volume->separations[2], volume->separations[1], volume->separations[0]); 

    delete_volume(volume);
 
   return(1);
}

#endif
