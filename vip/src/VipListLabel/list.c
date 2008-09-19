/****************************************************************************
 * PROJECT     : V.I.P. Command
 * MODULE NAME : list.c               * TYPE     : Command line
 * AUTHOR      : Frouin V.            * CREATION : 15/12/99
 * VERSION     : 1.6                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sparc
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

#include <stdlib.h>
#include <string.h>
#include <vip/volume.h>
#include <vip/nomen.h>





static int Usage()
{
  (void)fprintf(stderr,"Usage: VipListLabel\n");
  (void)fprintf(stderr,"        -i[mage]      {image name}\n");

  return(PB);
}
static int Help()
{

  Usage();

  return(OK);
}

int main(int argc, char *argv[])
{

  VIP_DEC_VOLUME(       etiquette);
  int                   i;
  char                 *image = NULL;
  VipNomenList         *nomenref, *nomenvol;


  for(i=1;i<argc;i++)
    {
      if (!strncmp (argv[i], "-image", 2))
        {
          if(++i >= argc || !strncmp(argv[i],"-",1)) return(Usage());
          image = argv[i];
        }
      else if (!strncmp(argv[i], "-help",2)) return(Help());
      else return(Usage());
    }

  if (image == NULL) 
    {
      VipPrintfError("image argument is required by VipListLabel\n");
      return(Usage());      
    }

  /* Lecture de l'image des labels-------------------------------------*/
  if ((etiquette = VipReadVolume(image)) == (Volume *)PB)
    {
      VipPrintfError("Error while reading volume.");
      VipPrintfExit("(commandline)VipListLabel.");
      return(PB);
    }
  
  nomenref = VipCreateRefNomenList();
  nomenvol = VipCreateNomenListFromVolume( etiquette );
  if (nomenvol == ( (VipNomenList *)PB ))
    {
      VipPrintfExit("(commandline)VipListLabel.");
      return(PB);      
    }
  if ( VipMergeNomenList( nomenvol, nomenref ) == PB ) return( PB );
  VipPrintNomenList(stdout, nomenvol);


  return( OK );
}
