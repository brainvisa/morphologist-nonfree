/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : VipTransform.c       * TYPE     : Command line
 * AUTHOR      : POUPON F.            * CREATION : 24/10/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Effectue une transformation 3D a un volume
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

#include <vip/transform.h>

#include <string.h>

static void Usage()
{ 
    (void)fprintf(stderr,"Usage: VipTransform\n");
    (void)fprintf(stderr,"\t\t-i[nput] {volume name}\n");
    (void)fprintf(stderr,"\t\t[-o[utput] {transformed volume file (default:\"transfo\")}]\n");
    (void)fprintf(stderr,"\t\t[-xs[cale] {X scale factor (default=1.0)}]\n");
    (void)fprintf(stderr,"\t\t[-ys[cale] {Y scale factor (default=1.0)}]\n");
    (void)fprintf(stderr,"\t\t[-zs[cale] {Z scale factor (default=1.0)}]\n");
    (void)fprintf(stderr,"\t\t[-xt[ranslate] {X translation value (default=0.0)}]\n");
    (void)fprintf(stderr,"\t\t[-yt[ranslate] {Y translation value (default=0.0)}]\n");
    (void)fprintf(stderr,"\t\t[-zt[ranslate] {Z translation value (default=0.0)}]\n");
    (void)fprintf(stderr,"\t\t[-xr[otate] {X rotation angle in degree (default=0.0)}]\n");
    (void)fprintf(stderr,"\t\t[-yr[otate] {Y rotation angle in degree (default=0.0)}]\n");
    (void)fprintf(stderr,"\t\t[-zr[otate] {Z rotation angle in degree (default=0.0)}]\n");
    (void)fprintf(stderr,"\t\t[-xg[ravity] {X rotation center position (default=dimx/2)}]\n");
    (void)fprintf(stderr,"\t\t[-yg[ravity] {Y rotation center position (default=dimy/2)}]\n");
    (void)fprintf(stderr,"\t\t[-zg[raviy] {Z rotation center position (default=dimz/2)}]\n");
    (void)fprintf(stderr,"\t\t[-h[elp]]\n");
    exit(-1);
}

static void Help()
{ 
    VipPrintfInfo("Translation, scale, and rotation volume transformation.");
    (void)printf("\n");
    (void)fprintf(stderr,"Usage: VipTransform\n");
    (void)fprintf(stderr,"\t\t-i[nput] {volume name}\n");
    (void)fprintf(stderr,"\t\t[-o[utput] {transformed volume file (default:\"transfo\")}]\n");
    (void)fprintf(stderr,"\t\t[-xs[cale] {X scale factor (default=1.0)}]\n");
    (void)fprintf(stderr,"\t\t[-ys[cale] {Y scale factor (default=1.0)}]\n");
    (void)fprintf(stderr,"\t\t[-zs[cale] {Z scale factor (default=1.0)}]\n");
    (void)fprintf(stderr,"\t\t[-xt[ranslate] {X translation value (default=0.0)}]\n");
    (void)fprintf(stderr,"\t\t[-yt[ranslate] {Y translation value (default=0.0)}]\n");
    (void)fprintf(stderr,"\t\t[-zt[ranslate] {Z translation value (default=0.0)}]\n");
    (void)fprintf(stderr,"\t\t[-xr[otate] {X rotation angle in degree (default=0.0)}]\n");
    (void)fprintf(stderr,"\t\t[-yr[otate] {Y rotation angle in degree (default=0.0)}]\n");
    (void)fprintf(stderr,"\t\t[-zr[otate] {Z rotation angle in degree (default=0.0)}]\n");
    (void)fprintf(stderr,"\t\t[-xg[ravity] {X rotation center position (default=dimx/2)}]\n");
    (void)fprintf(stderr,"\t\t[-yg[ravity] {Y rotation center position (default=dimy/2)}]\n");
    (void)fprintf(stderr,"\t\t[-zg[raviy] {Z rotation center position (default=dimz/2)}]\n");
    (void)fprintf(stderr,"\t\t[-h[elp]]\n");
    exit(-1);
}

int main(int argc, char *argv[])
{ 
    VIP_DEC_VOLUME(vol);
    VIP_DEC_VOLUME(volout);
    char *input=NULL, output[VIP_NAME_MAXLEN]="transfo";
    VipTransform_VDOUBLE transf;
    int i;

    transf.gravity.x = transf.gravity.y = transf.gravity.z = -1.0;
    transf.scale.x = transf.scale.y = transf.scale.z = 1.0;
    transf.translation.x = transf.translation.y = transf.translation.z = 0.0;
    transf.rotation.x = transf.rotation.y = transf.rotation.z = 0.0;

    /***** gestion des arguments *****/
    for (i=1; i<argc; i++)
	{ 
	    if (!strncmp(argv[i], "-input", 2))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    input = argv[i];
		}
	    else if (!strncmp(argv[i], "-output", 2))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    strcpy(output, argv[i]);
		}
	    else if (!strncmp(argv[i], "-xscale", 3))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    transf.scale.x = atof(argv[i]);
		}
	    else if (!strncmp(argv[i], "-yscale", 3))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    transf.scale.y = atof(argv[i]);
		}
	    else if (!strncmp(argv[i], "-zscale", 3))
		{
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    transf.scale.z = atof(argv[i]);
		}
	    else if (!strncmp(argv[i], "-xtranslate", 3))
		{ 
		    if (++i >= argc)  Usage();
		    transf.translation.x = atof(argv[i]);
		}
	    else if (!strncmp(argv[i], "-ytranslate", 3))
		{ 
		    if (++i >= argc)  Usage();
		    transf.translation.y = atof(argv[i]);
		}
	    else if (!strncmp(argv[i], "-ztranslate", 3))
		{ 
		    if (++i >= argc)  Usage();
		    transf.translation.z = atof(argv[i]);
		}
	    else if (!strncmp(argv[i], "-xrotate", 3))
		{ 
		    if (++i >= argc)  Usage();
		    transf.rotation.x = atof(argv[i]);
		}
	    else if (!strncmp(argv[i], "-yrotate", 3))
		{ 
		    if (++i >= argc)  Usage();
		    transf.rotation.y = atof(argv[i]);
		}
	    else if (!strncmp(argv[i], "-zrotate", 3))
		{ 
		    if (++i >= argc)  Usage();
		    transf.rotation.z = atof(argv[i]);
		}
	    else if (!strncmp(argv[i], "-xgravity", 3))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    transf.gravity.x = atof(argv[i]);
		}
	    else if (!strncmp(argv[i], "-ygravity", 3))
		{ 
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    transf.gravity.y = atof(argv[i]);
		}
	    else if (!strncmp(argv[i], "-zgravity", 3))
		{
		    if ((++i >= argc) || !strncmp(argv[i], "-", 1))  Usage();
		    transf.gravity.z = atof(argv[i]);
		}
	    else if (!strncmp(argv[i], "-help", 2))  Help();
	    else Usage();
	}

    if (input == NULL)
	{
	    VipPrintfError("input arg is required by VipTranform");
	    Usage();
	}

    printf("Reading file : %s...\n", input);
    vol = VipReadVolume(input);    

    if (transf.gravity.x == -1.0) transf.gravity.x = (double)mVipVolSizeX(vol)/2.0;
    if (transf.gravity.y == -1.0) transf.gravity.y = (double)mVipVolSizeY(vol)/2.0;
    if (transf.gravity.z == -1.0) transf.gravity.z = (double)mVipVolSizeZ(vol)/2.0;

    printf("Processing transformation...\n");
    volout = VipTransformation_VDOUBLE(vol, &transf);

    printf("Writting file : %s...\n", output);
    VipWriteVolume(volout, output);

    VipFreeVolume(vol);

  return 0;
}
