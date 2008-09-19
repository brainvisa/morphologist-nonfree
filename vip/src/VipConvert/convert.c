/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipConvert.c         * TYPE     : Command line
 * AUTHOR      : MANGIN J.F.          * CREATION : 06/11/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
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
 *              / /   |   
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
#include <vip/util.h>

/*------------------------------------------------------------------*/
static int Usage();
static int Help();
/*-----------------------------------------------------------------*/




int main(int argc, char *argv[])
{     

  /*declarations and initializations*/

  VIP_DEC_VOLUME(vol);
  VIP_DEC_VOLUME(converted);  
  char *input = NULL;
  char output[VIP_NAME_MAXLEN] = "converted";
  char outfile[VIP_NAME_MAXLEN];
  int mode = RAW_TYPE_CONVERSION;
  int readlib, writelib;
  char *type = NULL;
  int letype;
  int thelib;
  char split = 'n';
  int nbf;
  int l;
 
  int i;

  readlib = OK;
  writelib = OK;

  /*loop on command line arguments*/

  for(i=1;i<argc;i++)
    {	
      if (!strncmp (argv[i], "-input", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  input = argv[i];
	}
      else if (!strncmp (argv[i], "-output", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  strcpy(output,argv[i]);
	}
      else if (!strncmp (argv[i], "-type", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  type = argv[i];
	}
      else if (!strncmp (argv[i], "-mode", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(!strcmp(argv[i],"r")) mode = RAW_TYPE_CONVERSION;	  
	  else
	    {
	      (void)fprintf(stderr,"Unknown mode: %s\n",argv[i]);
	      return(Usage());
	    }
	}
      else if (!strncmp (argv[i], "-readformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') readlib = TIVOLI;
	  else if(argv[i][0]=='v') readlib = VIDA;
	  else if(argv[i][0]=='s') readlib = SPM;
	  else
	    {
	      (void)fprintf(stderr,"This format is not implemented for reading\n");
	      Usage();
	    }
	}
      else if (!strncmp (argv[i], "-split", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='y') split = 'y';
	  else if(argv[i][0]=='n') split = 'n';
	  else
	    {
	      (void)fprintf(stderr,"Unknown split mode\n");
	      Usage();
	    }
	}
      else if (!strncmp (argv[i], "-writeformat", 2)) 
	{
	  if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
	  if(argv[i][0]=='t') writelib = TIVOLI;
	  else if(argv[i][0]=='s') writelib = SPM;
	  else if(argv[i][0]=='v') writelib = VIDA;
	  else
	    {
	      (void)fprintf(stderr,"This format is not implemented for writing\n");
	      return(Usage());
	    }    
	}
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  /*check that all required arguments have been given*/

  if (input==NULL)
    {
      (void)fprintf(stderr,"input arg is required by VipConvert\n");
      return(Usage());
    }
  if (type==NULL) letype = S16BIT;
  else if (!strcmp(type,"U8")) letype = U8BIT;
  else if (!strcmp(type,"S16")) letype = S16BIT;
  else if (!strcmp(type,"F")) letype = VFLOAT;
  else
    {
      VipPrintfError("This conversion is not implemented");
      return(VIP_CL_ERROR);
    }

  if ((thelib=VipTestImageFileExist(input))==PB)
    {
      (void)fprintf(stderr,"Can not open this image: %s\n",input);
      return(VIP_CL_ERROR);
    }

  if(letype!=VFLOAT && writelib==VIDA)
    {
      VipPrintfWarning("You try to trigger scaled writing but the conversion mode is not float!!!");
      return(Help());
    }

  if(split=='y') /*stem from VipSplit command line...*/
    {
      if (readlib == TIVOLI || readlib==SPM)
	{
	  VipPrintfWarning("Split not implemented yet for tivoli and analyse input!");
	  return(-1);
	}
      else vol = VipReadVidaHeader(input);

      nbf = mVipVolSizeT(vol);
      printf("Splitting in %d frames...\n",nbf);
      printf("---------------------------");
      fflush(stdout);
      /* manque un free...*/
      /* Boucle sur chacune des frames					*/

      for(l=1;l<=nbf;l++)
	{

	/* Creation d'une structure Vip avec une seule frame			*/
	/* Transfert des data du volume en entree vers le volume cree		*/

	  printf("Extraction frame %d\n",l);

	  if( letype==VFLOAT )
	    vol = VipReadVidaFloatFrame(input,l-1,l-1);
	  else
	    vol = VipReadVidaFrame(input,l-1,l-1);
	  
	  if (vol==PB)
	    {
	      VipPrintfError("VipReadVidaFrame: erreur");
	      return(VIP_CL_ERROR);
	    }

	  /* Ecriture sur disque du volume cree mono-frame			*/

	  sprintf(outfile,"%s%03d",output,l);

	  if (writelib == TIVOLI)
	    {
	      if(VipWriteTivoliVolume(vol,outfile)==PB) return(VIP_CL_ERROR);
	    }
	  else if (writelib == SPM)
	    {
	      if(VipWriteSPMVolume(vol,outfile)==PB) return(VIP_CL_ERROR);
	    }
	  else if(writelib==OK)
	    {
	      if(letype!=S16BIT)
		VipPrintfWarning("Be careful, I am writing a S16BIT unscaled vida volume!\n");
	      if(VipWriteVolume(vol,outfile)==PB) return(VIP_CL_ERROR);
	    }
	  else
	    {   
	      if(VipWriteVidaScaledVolume(vol,outfile)==PB) return(VIP_CL_ERROR);
	    }

	  /* Desallocation du volume en memoire					*/

	  /*VipFreeVolume(vol); Do not understand why, JFM...*/
	}
    }
  else
    {
      if (readlib == TIVOLI)
	vol = VipReadTivoliVolumeWithBorder(input,0);
      else if (readlib == SPM)
	vol = VipReadSPMVolumeWithBorder(input,0);
      else if (readlib==OK && thelib==VIDA)
	{
	  vol = VipReadVidaFloatVolumeWithBorder(input,0);
	  if(vol==NULL)
	    {
	      VipPrintfWarning("Vida scaled reading failed, I try usual one...(no problem if input is in tivoli format)\n");
	      vol = VipReadVolumeWithBorder(input,0); 
	      if (vol==NULL)
		{
		  VipPrintfError("This one fails also, sorry...\n");
		  return(Help());
		}
	    }
	}
      else 	  vol = VipReadVolumeWithBorder(input,0); 
      if (vol==NULL)
	{
	  VipPrintfError("Reading problems, sorry...\n");
	  return(VIP_CL_ERROR);
	}
      
      if (letype==U8BIT) converted = VipTypeConversionToU8BIT(vol,mode);
      else if (letype==VFLOAT) converted = VipTypeConversionToFLOAT(vol,mode);
      else converted = VipTypeConversionToS16BIT(vol,mode);
      
      if(converted==PB) return(VIP_CL_ERROR);
      
      if (writelib == TIVOLI)
	{
	  if(VipWriteTivoliVolume(converted,output)==PB) return(VIP_CL_ERROR);
	}
      else if (writelib == SPM)
	{
	  if(VipWriteSPMVolume(converted,output)==PB) return(VIP_CL_ERROR);
	}
      else if(writelib==OK)
	{
	  if(letype!=S16BIT)
	    VipPrintfWarning("Be careful, I am writing a S16BIT unscaled vida volume!\n");
	  if(VipWriteVolume(converted,output)==PB) return(VIP_CL_ERROR);
	}
      else
	{   
	  if(VipWriteVidaScaledVolume(converted,output)==PB) return(VIP_CL_ERROR);
	}
    }
  return(0);

}
/*-----------------------------------------------------------------------------------------*/

static int Usage()
{
  (void)fprintf(stderr,"Usage: VipConvert\n");
  (void)fprintf(stderr,"        -i[nput] {image name}\n");
  (void)fprintf(stderr,"        [-t[ype] {U8/S16/F (default:S16)}]\n");
  (void)fprintf(stderr,"        [-o[utput] {image name (default:\"converted\")}]\n");
  (void)fprintf(stderr,"        [-s[plit] {y/n to create SPM dynamic format (default n)}]\n");
  (void)fprintf(stderr,"        [-m[ode] {r (default:r (raw))}]\n");
  (void)fprintf(stderr,"        [-r[eadformat] {char: v, s or t (default: v scaled)}]\n");
  (void)fprintf(stderr,"        [-w[riteformat] {char: v, s or t (default: unscaled vida)}]\n");
  (void)fprintf(stderr,"        [-h[elp]\n");
  return(VIP_CL_ERROR);

}
/*****************************************************/

static int Help()
{
 
  VipPrintfInfo("Converts any volume to another type");
  (void)printf("\n");
  (void)printf("Usage: VipConvert\n");
  (void)printf("        -i[nput] {image name}\n");
  (void)printf("        [-t[ype] {U8/S16/F (default:S16)}]\n");
  (void)printf("Be aware that the default reading and writing format is vida unscaled (S16BIT)\n");
  (void)printf("        [-o[utput] {image name (default:\"converted\")}]\n");
  (void)printf("        [-s[plit] {y/n to create SPM dynamic format (default n)}]\n");
  (void)printf("put this flag to s to trigger conversion of a dynamic image to SPM format\n");
  (void)printf("        [-m[ode] {r (default:r (raw))}]\n");
  (void)printf("with r mode, the conversion is a simple cast\n");
  (void)printf("        [-r[eadformat] {char: v, s or t (default: scaled vida)}]\n");
  (void)printf("Forces the reading of VIDA, SPM or TIVOLI image file format\n");
  (void)printf("By default, the vida scaled reading to float volume is used\n");
  (void)printf("When -r v is asked, the vida reading to S16BIT is used\n");
  (void)printf("        [-w[riteformat] {char: v, s or t (default: unscaled vida)}]\n");
  (void)printf("Forces the writing of VIDA, SPM or TIVOLI image file format\n");
  (void)printf("the v choice forces scaled writing of float volume to vida format\n");
  (void)printf("the default mode do not scaled the volume\n");
  (void)printf("        [-h[elp]\n");
  (void)printf("Examples:\n");
  (void)printf("-----------------------------------------------------\n");
  (void)printf("Conversion d'une image d'index DTI (trace, volume ratio...)\n");
  (void)printf("L'image a convertir est au format TIVOLI (ima/dim) en float (verifiez dans le .dim)\n");
  (void)printf("-->VipConvert -i trace -o traceSPM -w s -t F\n");
  (void)printf("-----------------------------------------------------\n");
  (void)printf("Conversion d'une serie dynamique type IRMf pour SPM\n");
  (void)printf("L'image a convertir est au format VIDA\n");
  (void)printf("-->VipConvert -i raw -o frame -w s -s yn\n");
  (void)printf("-----------------------------------------------------\n");
  return(VIP_CL_ERROR);

}

/******************************************************/
