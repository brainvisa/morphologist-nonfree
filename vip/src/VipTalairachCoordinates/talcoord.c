/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipTalairachCoordinates.c * TYPE     : Command line
 * AUTHOR      : POUPON F.                 * CREATION : 09/03/2001
 * VERSION     : 0.1                       * REVISION :
 * LANGUAGE    : C++                       * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Exprime des coordonnes dans le repere AC-PC
 *
 ****************************************************************************
 *
 * USED MODULES : string.h - vip/volume.h - vip/transform.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <vip/talairach.h>

#include <stdlib.h>
#include <string.h>

static void Usage()
{ 
    (void)fprintf(stderr,"Usage: VipTalairachCoordinates\n");
    (void)fprintf(stderr,"\t\t-x {input point X}\n");
    (void)fprintf(stderr,"\t\t-y {input point Y}\n");
    (void)fprintf(stderr,"\t\t-z {input point Z}\n");
    (void)fprintf(stderr,"\t\t-xa[c] {AC point X}\n");
    (void)fprintf(stderr,"\t\t-ya[c] {AC point Y}\n");
    (void)fprintf(stderr,"\t\t-za[c] {AC point Z}\n");
    (void)fprintf(stderr,"\t\t-xp[c] {PC point X}\n");
    (void)fprintf(stderr,"\t\t-yp[c] {PC point Y}\n");
    (void)fprintf(stderr,"\t\t-zp[c] {PC point Z}\n");
    (void)fprintf(stderr,"\t\t-xh[emi] {Hemispheric point X}\n");
    (void)fprintf(stderr,"\t\t-yh[emi] {Hemispheric point Y}\n");
    (void)fprintf(stderr,"\t\t-zh[emi] {Hemispheric point Z}\n");
    (void)fprintf(stderr,"\t\t[-h[elp]]\n");
    exit(-1);
}

static void Help()
{ 
    VipPrintfInfo("Provide the coordinates of a point in the Talairach frame.");
    (void)printf("\n");
    (void)fprintf(stderr,"Usage: VipTalairachCoordinates\n");
    (void)fprintf(stderr,"\t\t-X {input point X}\n");
    (void)fprintf(stderr,"\t\t-Y {input point Y}\n");
    (void)fprintf(stderr,"\t\t-Z {input point Z}\n");
    (void)fprintf(stderr,"\t\t-xa[c] {AC point X}\n");
    (void)fprintf(stderr,"\t\t-ya[c] {AC point Y}\n");
    (void)fprintf(stderr,"\t\t-za[c] {AC point Z}\n");
    (void)fprintf(stderr,"\t\t-xp[c] {PC point X}\n");
    (void)fprintf(stderr,"\t\t-yp[c] {PC point Y}\n");
    (void)fprintf(stderr,"\t\t-zp[c] {PC point Z}\n");
    (void)fprintf(stderr,"\t\t-xh[emi] {Hemispheric point X}\n");
    (void)fprintf(stderr,"\t\t-yh[emi] {Hemispheric point Y}\n");
    (void)fprintf(stderr,"\t\t-zh[emi] {Hemispheric point Z}\n");
    (void)fprintf(stderr,"\t\t[-h[elp]]\n");
    exit(-1);
}

int main(int argc, char *argv[])
{ 
    int i;
    int x=-1, y=-1, z=-1;
    int xa=-1, ya=-1, za=-1;
    int xp=-1, yp=-1, zp=-1;
    int xh=-1, yh=-1, zh=-1;
    float pt[3], res[3];

    VipTalairach tal;

    /***** gestion des arguments *****/
    for (i=1; i<argc; i++)
	{ 
	    if (!strncmp(argv[i], "-X", 2))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    x = atoi( argv[i] );
		}
	    else if (!strncmp(argv[i], "-Y", 2))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    y = atoi( argv[i] );
		}
	    else if (!strncmp(argv[i], "-Z", 2))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    z = atoi( argv[i] );
		}
	    else if (!strncmp(argv[i], "-xac", 3))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    xa = atoi( argv[i] );
		}
	    else if (!strncmp(argv[i], "-yac", 3))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    ya = atoi( argv[i] );
		}
	    else if (!strncmp(argv[i], "-zac", 3))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    za = atoi( argv[i] );
		}
	    else if (!strncmp(argv[i], "-xpc", 3))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    xp = atoi( argv[i] );
		}
	    else if (!strncmp(argv[i], "-ypc", 3))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    yp = atoi( argv[i] );
		}
	    else if (!strncmp(argv[i], "-zpc", 3))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    zp = atoi( argv[i] );
		}
	    else if (!strncmp(argv[i], "-xhemi", 3))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    xh = atoi( argv[i] );
		}
	    else if (!strncmp(argv[i], "-yhemi", 3))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    yh = atoi( argv[i] );
		}
	    else if (!strncmp(argv[i], "-zhemi", 3))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    zh = atoi( argv[i] );
		}
	    else if (!strncmp(argv[i], "-help", 2))  Help();
	    else Usage();
	}

    if (x==-1 || y==-1 || z==-1 || xa==-1 || ya==-1 || za==-1 ||
    	xp==-1 || yp==-1 || zp==-1 || xh==-1 || yh==-1 || zh==-1)
	{
	    VipPrintfError("Wrong arg passed to VipTalairachCoordinates");
	    Usage();
	}

  tal.AC.x = xa;
  tal.AC.y = ya;
  tal.AC.z = za;
  tal.PC.x = xp;
  tal.PC.y = yp;
  tal.PC.z = zp;
  tal.Hemi.x = xh;
  tal.Hemi.y = yh;
  tal.Hemi.z = zh;

  VipInitTalairachVectors( &tal );
  VipTalairachRotationAndTranslation( &tal );

  pt[ 0 ] = (float)x;
  pt[ 1 ] = (float)y;
  pt[ 2 ] = (float)z;

  VipTalairachTransformation( pt, &tal, res );
  
  printf( "\nInitial point: (%d,%d,%d)\n", x, y, z );
  printf( "In Talairach frame: (%f,%f,%f)\n", res[ 0 ], res[ 1 ], res[ 2 ] );

  return 0;
}
