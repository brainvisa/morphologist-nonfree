/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_talairach.c      * TYPE     : Source
 * AUTHOR      : POUPON F.            * CREATION : 07/01/1998
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Genere les fichier de transformation vers Talairach
 *
 ****************************************************************************
 *
 * USED MODULES : Vip_talairach.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <vip/talairach.h> 

int GetCommissureCoordinates(Volume *vol, char *point_filename, VipTalairach *tal,
			     int xCA, int yCA, int zCA, 
			     int xCP, int yCP, int zCP, 
			     int xP, int yP, int zP,
			     int coord_provided)
{

  int i;
  FILE *pointfile;
  char check[1000];

  if(!vol || !tal || !point_filename)
    {
      VipPrintfError("GetCommissureCoordinates bad args");
      return(PB);
    }

  if(strcmp(point_filename,""))
    {
      printf("Reading AC, PC, IH coordinates: %s\n", point_filename);
      pointfile = fopen(point_filename,"r");
      if(!pointfile)
	{
	  strcat(point_filename,".APC");
	  pointfile = fopen(point_filename,"r");
	  if(!pointfile)
	    {
	      VipPrintfWarning("Can not open point file, I will make do with assumptions");
	      printf("%s\n",point_filename);
	    }
	}
      if(pointfile)
	{
	  printf("Reading Coordinate file (AC, PC...)\n");
	  i=fscanf(pointfile,"%s%d%d%d",check,&xCA,&yCA,&zCA);
	  if(i!=4 || strcmp(check,"AC:"))
	    {
	      VipPrintfError("Probleme while reading AC coordinates");
	      printf("Correct format for the commissure coordinate file toto.APC:\n");
	      printf("AC: 91 88 113\nPC: 91 115 109\nIH: 90 109 53\n");
	      return(VIP_CL_ERROR);
	    }
	  i=fscanf(pointfile,"%s%d%d%d",check,&xCP,&yCP,&zCP);
	  if(i!=4 || strcmp(check,"PC:"))
	    {
	      VipPrintfError("Probleme while reading PC coordinates");
	      printf("Correct format for the commissure coordinate file toto.APC:\n");
	      printf("AC: 91 88 113\nPC: 91 115 109\nIH: 90 109 53\n");
	      return(VIP_CL_ERROR);
	    }
	  i=fscanf(pointfile,"%s%d%d%d",check,&xP,&yP,&zP);
	  if(i!=4|| strcmp(check,"IH:"))
	    {
	      VipPrintfError("Probleme while reading inter-hemis point coordinates");
	      printf("Correct format for the commissure coordinate file toto.APC:\n");
	      printf("AC: 91 88 113\nPC: 91 115 109\nIH: 90 109 53\n");
	      return(VIP_CL_ERROR);
	    }
	  fclose(pointfile);
	}
    }
  else if(coord_provided==VFALSE)
    {
      VipPrintfInfo("Computing guess of commissure coordinates");
      xCA = mVipVolSizeX(vol)/2;
      xCP = mVipVolSizeX(vol)/2;
      xP = mVipVolSizeX(vol)/2;
      zCA = (int)(90./mVipVolVoxSizeZ(vol)); 
      zCP = (int)(90./mVipVolVoxSizeZ(vol));
      yCA=113; 
      yCP=141;
      yP=60;
      zP=20;
    }
  tal->AC.x = xCA;
  tal->AC.y = yCA;
  tal->AC.z = zCA;
  tal->PC.x = xCP;
  tal->PC.y = yCP;
  tal->PC.z = zCP;
  tal->Hemi.x = xP;
  tal->Hemi.y = yP;
  tal->Hemi.z = zP;

  printf("Anterior commissure: %d %d %d\n",xCA,yCA,zCA);
  printf("Posterior commissure: %d %d %d\n",xCP,yCP,zCP);
  printf("Random interhemispheric point: %d %d %d\n",xP,yP,zP);
  return(OK);
}

void VipFillTalairachPlane(VipTalairach *tal, int num, float AA, float BB,
			   float CC, float DD)
{
  tal->Plane[num].a = AA;
  tal->Plane[num].b = BB;
  tal->Plane[num].c = CC;
  tal->Plane[num].d = DD;
  tal->Plane[num].num = num;
}

void VipInitTalairachPlanes(VipTalairach *tal, float *minA, float *maxA,
			    float *minS, float *maxS, float *minC, float *maxC)
{
  float AA, BB, CC, DD, PCmm[3], tmp[3];

  /** Dans le repere de Talairach, et en millimetres, d'ou AC=(0,0,0) **/
  tmp[0] = tal->PC.x;
  tmp[1] = tal->PC.y;
  tmp[2] = tal->PC.z;
  VipTalairachTransformation(tmp, tal, PCmm);

  AA = tal->ACPC_vector.x;
  BB = tal->ACPC_vector.y;
  CC = tal->ACPC_vector.z;

  DD = AA*minC[0]+BB*minC[1]+CC*minC[2];
  VipFillTalairachPlane(tal, P_COR_0, AA, BB, CC, DD);

  VipFillTalairachPlane(tal, P_COR_1, AA, BB, CC, 0.0);

  DD = AA*PCmm[0]+BB*PCmm[1]+CC*PCmm[2];
  VipFillTalairachPlane(tal, P_COR_2, AA, BB, CC, DD);

  DD = AA*maxC[0]+BB*maxC[1]+CC*maxC[2];
  VipFillTalairachPlane(tal, P_COR_3, AA, BB, CC, DD);

  AA = tal->Hemi_vector.x;
  BB = tal->Hemi_vector.y;
  CC = tal->Hemi_vector.z;

  DD = AA*minS[0]+BB*minS[1]+CC*minS[2];
  VipFillTalairachPlane(tal, P_SAG_0, AA, BB, CC, DD);

  VipFillTalairachPlane(tal, P_SAG_1, AA, BB, CC, 0.0);

  DD = AA*maxS[0]+BB*maxS[1]+CC*maxS[2];
  VipFillTalairachPlane(tal, P_SAG_2, AA, BB, CC, DD);

  AA = tal->Cross_vector.x;
  BB = tal->Cross_vector.y;
  CC = tal->Cross_vector.z;

  DD = AA*minA[0]+BB*minA[1]+CC*minA[2];
  VipFillTalairachPlane(tal, P_AXI_0, AA, BB, CC, DD);

  VipFillTalairachPlane(tal, P_AXI_1, AA, BB, CC, 0.0);

  DD = AA*maxA[0]+BB*maxA[1]+CC*maxA[2];
  VipFillTalairachPlane(tal, P_AXI_2, AA, BB, CC, DD);
}

void Vip3PlanesIntersection(VipTalairach *tal, int num1, int num2, int num3,
			    float *ptInter)
{
  float AA[3], BB[3], CC[3], DD[3];

  AA[0] = tal->Plane[num1].a;
  AA[1] = tal->Plane[num1].b;
  AA[2] = tal->Plane[num1].c;
  DD[0] = tal->Plane[num1].d;

  BB[0] = tal->Plane[num2].a;
  BB[1] = tal->Plane[num2].b;
  BB[2] = tal->Plane[num2].c;
  DD[1] = tal->Plane[num2].d;

  CC[0] = tal->Plane[num3].a;
  CC[1] = tal->Plane[num3].b;
  CC[2] = tal->Plane[num3].c;
  DD[2] = tal->Plane[num3].d;

  Vip3DPlanesResolution(AA, BB, CC, DD, &ptInter[0], &ptInter[1], &ptInter[2]);
}

void VipTalairachRotation(float *pt, VipTalairach *tal, float *res)
{
  int i, j;

  /** Transformation en coordonnees millimetriques **/
  res[0] = res[1] = res[2] = 0.0;
  for (j=0; j<3; j++)
    for (i=0; i<3; i++)
      res[j] += pt[i]*tal->Rotation[j][i];
}

void VipTalairachInverseRotation(float *pt, VipTalairach *tal, float *res)
{
  int i, j;

  /** Transformation en coordonnees millimetriques **/
  res[0] = res[1] = res[2] = 0.0;
  for (j=0; j<3; j++)
    for (i=0; i<3; i++)
      res[j] += pt[i]*tal->Rotation[i][j];
}

void VipTalairachTranslation(float *pt, VipTalairach *tal, float *res)
{
  int i;

  /** Transformation en coordonnees millimetriques **/
  for (i=0; i<3; i++) res[i] = pt[i]+tal->Translation[i];
}

void VipTalairachInverseTranslation(float *pt, VipTalairach *tal, float *res)
{
  int i;

  /** Transformation en coordonnees millimetriques **/
  for (i=0; i<3; i++)  res[i] = pt[i]-tal->Translation[i];
}

void VipTalairachTransformation(float *pt, VipTalairach *tal, float *res)
{
  int i, j;

  /** Transformation en coordonnees millimetriques **/
  res[0] = res[1] = res[2] = 0.0;
  for (j=0; j<3; j++)
    for (i=0; i<3; i++)
      res[j] += (pt[i]+tal->Translation[i])*tal->Rotation[j][i];
}

void VipTalairachInverseTransformation(float *pt, VipTalairach *tal, 
				       float *res)
{
  int i, j;

  /** Transformation en coordonnees millimetriques **/
  res[0] = res[1] = res[2] = 0.0;
  for (j=0; j<3; j++)
    for (i=0; i<3; i++)
      res[j] += pt[i]*tal->Rotation[i][j];

  for (i=0; i<3; i++)  res[i] -= tal->Translation[i];
}

void VipTalairachBox(Volume *vol, VipTalairach *tal)
{
  float pt[3], res[3], minA[3], maxA[3], minS[3], maxS[3], minC[3], maxC[3];
  float ptInter[3], pts[3], mA, mS, mC, ma, ms, mc, scale;
  int xsize, ysize, zsize, i;
  long off_pbl, off_lbs, off_fp;
  Vip_S16BIT *imptr;
  /*squatt de Jeff qui craque*/
  float boxmin[3], boxmax[3];

  for (i=0; i<3; i++)
    {
      minA[i] = minS[i] = minC[i] = boxmin[i] = 1000.0;
      maxA[i] = maxS[i] = maxC[i] = boxmax[i] = -1000.0;
    }

  VipGet3DSize(vol, &xsize, &ysize, &zsize);
  off_pbl = VipOffsetPointBetweenLine(vol);
  off_lbs = VipOffsetLineBetweenSlice(vol);
  off_fp = VipOffsetFirstPoint(vol);
  imptr = VipGetDataPtr_S16BIT(vol)+off_fp;

  printf("%3d%%", 0);
  fflush(stdout);

  for (pt[2]=0.0; pt[2]<(float)zsize; pt[2]++, imptr+=off_lbs)
    {
      for (pt[1]=0.0; pt[1]<(float)ysize; pt[1]++, imptr+=off_pbl)
        for (pt[0]=0.0; pt[0]<(float)xsize; pt[0]++, imptr++)
  	  if (*imptr)
	    {
	      pts[0] = pt[0]*tal->VoxelGeometry.x;
	      pts[1] = pt[1]*tal->VoxelGeometry.y;
	      pts[2] = pt[2]*tal->VoxelGeometry.z;

	      /** recherche des points min est max de la boite dans le repere 
		  de Talairach et en millimetre **/
	      VipTalairachTransformation(pts, tal, res);
	      if(boxmin[0]>res[0]) boxmin[0]=res[0];
	      if(boxmin[1]>res[1]) boxmin[1]=res[1];
	      if(boxmin[2]>res[2]) boxmin[2]=res[2];
	      if(boxmax[0]<res[0]) boxmax[0]=res[0];
	      if(boxmax[1]<res[1]) boxmax[1]=res[1];
	      if(boxmax[2]<res[2]) boxmax[2]=res[2];

	      if (res[2] < minA[2])
		{
		  minA[0] = res[0];
		  minA[1] = res[1];
		  minA[2] = res[2];
		}
	      else if (res[2] > maxA[2])
		{
		  maxA[0] = res[0];
		  maxA[1] = res[1];
		  maxA[2] = res[2];
		}
	      if (res[0] < minS[0])
	        {
		  minS[0] = res[0];
		  minS[1] = res[1];
		  minS[2] = res[2];
		}
	      else if (res[0] > maxS[0])
	        {
		  maxS[0] = res[0];
		  maxS[1] = res[1];
		  maxS[2] = res[2];
		}
	      if (res[1] < minC[1])
	        {
		  minC[0] = res[0];
		  minC[1] = res[1];
		  minC[2] = res[2];
		}
	      else if (res[1] > maxC[1])
	        {
		  maxC[0] = res[0];
		  maxC[1] = res[1];
		  maxC[2] = res[2];
		}
	    }
      printf("\b\b\b\b%3d%%", (int)(100.0*pt[2]/(float)zsize)+1);
      fflush(stdout);
    }
  printf("\n");

  /** Determination des plans dans le repere de Talairach, et en
      coordonnees millimetriques **/
  VipInitTalairachPlanes(tal, minA, maxA, minS, maxS, minC, maxC);

  /** Les points d'intersections obtenus sont en millimetres **/
  Vip3PlanesIntersection(tal, P_COR_1, P_SAG_0, P_AXI_1, ptInter);
  ms = fabs(ptInter[0]);
  VipTalairachInverseTransformation(ptInter, tal, res);
  tal->MinSagittal.x = res[0]/tal->VoxelGeometry.x;
  tal->MinSagittal.y = res[1]/tal->VoxelGeometry.y;
  tal->MinSagittal.z = res[2]/tal->VoxelGeometry.x;

  Vip3PlanesIntersection(tal, P_COR_1, P_SAG_2, P_AXI_1, ptInter);
  mS = fabs(ptInter[0]);
  VipTalairachInverseTransformation(ptInter, tal, res);
  tal->MaxSagittal.x = res[0]/tal->VoxelGeometry.x;
  tal->MaxSagittal.y = res[1]/tal->VoxelGeometry.y;
  tal->MaxSagittal.z = res[2]/tal->VoxelGeometry.x;

  Vip3PlanesIntersection(tal, P_COR_0, P_SAG_1, P_AXI_1, ptInter);
  mc = fabs(ptInter[1]);
  VipTalairachInverseTransformation(ptInter, tal, res);
  tal->MinCoronal.x = res[0]/tal->VoxelGeometry.x;
  tal->MinCoronal.y = res[1]/tal->VoxelGeometry.y;
  tal->MinCoronal.z = res[2]/tal->VoxelGeometry.x;

  Vip3PlanesIntersection(tal, P_COR_3, P_SAG_1, P_AXI_1, ptInter);
  mC = fabs(ptInter[1]);
  VipTalairachInverseTransformation(ptInter, tal, res);
  tal->MaxCoronal.x = res[0]/tal->VoxelGeometry.x;
  tal->MaxCoronal.y = res[1]/tal->VoxelGeometry.y;
  tal->MaxCoronal.z = res[2]/tal->VoxelGeometry.x;

  Vip3PlanesIntersection(tal, P_COR_1, P_SAG_1, P_AXI_0, ptInter);
  ma = fabs(ptInter[2]);
  VipTalairachInverseTransformation(ptInter, tal, res);
  tal->MinAxial.x = res[0]/tal->VoxelGeometry.x;
  tal->MinAxial.y = res[1]/tal->VoxelGeometry.y;
  tal->MinAxial.z = res[2]/tal->VoxelGeometry.x;

  Vip3PlanesIntersection(tal, P_COR_1, P_SAG_1, P_AXI_2, ptInter);
  mA = fabs(ptInter[2]);
  VipTalairachInverseTransformation(ptInter, tal, res);
  tal->MaxAxial.x = res[0]/tal->VoxelGeometry.x;
  tal->MaxAxial.y = res[1]/tal->VoxelGeometry.y;
  tal->MaxAxial.z = res[2]/tal->VoxelGeometry.x;

  scale = tal->VoxelGeometry.z/tal->VoxelGeometry.x;

  tal->MinBox[0] = (int)tal->MinSagittal.x;
  tal->MinBox[1] = (int)tal->MinCoronal.y;
  tal->MinBox[2] = (int)tal->MinAxial.z/scale;
  tal->MaxBox[0] = (int)tal->MaxSagittal.x;
  tal->MaxBox[1] = (int)tal->MaxCoronal.y;
  tal->MaxBox[2] = (int)tal->MaxAxial.z/scale;

  /*les intersections de merdes ci-dessus doivent etre fausses... (JEFF)*/
  /*yen a pas besoin...*/

  printf("Talairach bounding box (mm):\n");
  printf("%f %f %f\n",boxmin[0],boxmin[1],boxmin[2]);
  printf("%f %f %f\n",boxmax[0],boxmax[1],boxmax[2]);

  tal->Scale.x = TALAIRACH_SIZEX/(boxmax[0]-boxmin[0]); /*scaling global -> Talairach atlas size*/
  tal->Scale.y = TALAIRACH_SIZEY/(boxmax[1]-boxmin[1]); /*scaling global*/
  tal->Scale.z = TALAIRACH_SIZEZ/(boxmax[2]-boxmin[2]); /*scaling global*/
  printf("Scaling: x: %f, y:%f, z:%f\n",tal->Scale.x, tal->Scale.y,tal->Scale.z);
							     
  /*
  tal->Scale.x = (ms > mS) ? 1.0/ms : 1.0/mS;
  tal->Scale.y = (mc > mC) ? 1.0/mc : 1.0/mC;
  tal->Scale.z = 1.0/ma;
  */
 /** pour ne pas prendre le cote du cervelet **/
}

void VipTalairachRotationAndTranslation(VipTalairach *tal)
{
  /** A partir des valeurs en millimetres **/
  tal->Rotation[0][0] = tal->Hemi_vector.x;
  tal->Rotation[0][1] = tal->Hemi_vector.y;
  tal->Rotation[0][2] = tal->Hemi_vector.z;

  tal->Rotation[1][0] = tal->ACPC_vector.x;
  tal->Rotation[1][1] = tal->ACPC_vector.y;
  tal->Rotation[1][2] = tal->ACPC_vector.z;

  tal->Rotation[2][0] = -tal->Cross_vector.x;
  tal->Rotation[2][1] = -tal->Cross_vector.y;
  tal->Rotation[2][2] = -tal->Cross_vector.z;

  tal->Translation[0] = -tal->AC.x;
  tal->Translation[1] = -tal->AC.y;
  tal->Translation[2] = -tal->AC.z;
}

void VipComputeDeterminant(float A[3], float B[3], float C[3], float *sol)
{
  int i, j, k, l, carry;

  *sol = 0;
  j = 0;
  k = 1;
  l = 2;
   
  for (i=0; i<3; i++)
    {
      *sol += A[j]*B[k]*C[l];
      carry = j;
      j = k;
      k = l;
      l = carry;

    }

  j = 2;
  k = 1;
  l = 0;

  for (i=0; i<3; i++)
    {
      *sol -= A[j]*B[k]*C[l];
      carry = j;
      j = k;
      k = l;
      l = carry;
    }
}

void Vip3DPlanesResolution(float a[3], float b[3], float c[3], float d[3], 
			   float *x, float *y, float *z)
{
  float det, tempA, tempB, tempC, sol[3];
  int i;

  VipComputeDeterminant(a, b, c, &det);

  for (i=0; i<3; i++)
    {
      tempA = a[i];
      tempB = b[i];
      tempC = c[i];

      a[i] = d[0];
      b[i] = d[1];
      c[i] = d[2];

      VipComputeDeterminant(a, b, c, &sol[i]);

      a[i] = tempA;
      b[i] = tempB;
      c[i] = tempC;
    }

  *x = sol[0]/det;
  *y = sol[1]/det;
  *z = sol[2]/det;
}

void VipInitTalairachVectors(VipTalairach *tal)
{
  float J[3], K[3], L[3], M[3], norme;

  tal->ACPC_vector.x = -(tal->AC.x-tal->PC.x);
  tal->ACPC_vector.y = -(tal->AC.y-tal->PC.y);
  tal->ACPC_vector.z = -(tal->AC.z-tal->PC.z);

  J[0] = tal->AC.x;
  J[1] = tal->AC.y;
  J[2] = tal->AC.z;

  K[0] = tal->PC.x;
  K[1] = tal->PC.y;
  K[2] = tal->PC.z;

  L[0] = tal->Hemi.x;
  L[1] = tal->Hemi.y;
  L[2] = tal->Hemi.z;

  M[0] = M[1] = M[2] = 1.0;

  Vip3DPlanesResolution(J, K, L, M, &(tal->Hemi_vector.x), 
			&(tal->Hemi_vector.y), &(tal->Hemi_vector.z));

  tal->Cross_vector.x = (tal->Hemi_vector.z * tal->ACPC_vector.y)-
    (tal->Hemi_vector.y * tal->ACPC_vector.z);
  tal->Cross_vector.y = (tal->Hemi_vector.x * tal->ACPC_vector.z)-
    (tal->Hemi_vector.z * tal->ACPC_vector.x);
  tal->Cross_vector.z = (tal->Hemi_vector.y * tal->ACPC_vector.x)-
    (tal->Hemi_vector.x * tal->ACPC_vector.y);

  norme = sqrt(mVipSq(tal->ACPC_vector.x)+mVipSq(tal->ACPC_vector.y)+
	       mVipSq(tal->ACPC_vector.z));
  tal->ACPC_vector.x /= norme;
  tal->ACPC_vector.y /= norme;
  tal->ACPC_vector.z /= norme;

  norme = sqrt(mVipSq(tal->Hemi_vector.x)+mVipSq(tal->Hemi_vector.y)+
	       mVipSq(tal->Hemi_vector.z));
  tal->Hemi_vector.x /= norme;
  tal->Hemi_vector.y /= norme;
  tal->Hemi_vector.z /= norme;

  norme = sqrt(mVipSq(tal->Cross_vector.x)+mVipSq(tal->Cross_vector.y)+
	       mVipSq(tal->Cross_vector.z));
  tal->Cross_vector.x /= norme;
  tal->Cross_vector.y /= norme;
  tal->Cross_vector.z /= norme;
}

void VipComputeTalairachTransformation(Volume *vol, VipTalairach *tal)
{
  VipGet3DVoxelSize(vol, &(tal->VoxelGeometry.x), &(tal->VoxelGeometry.y), 
		    &(tal->VoxelGeometry.z));

  /** Tout est passe en coordonnees millimetriques **/
  tal->AC.x *= tal->VoxelGeometry.x;
  tal->AC.y *= tal->VoxelGeometry.y;
  tal->AC.z *= tal->VoxelGeometry.z;

  tal->PC.x *= tal->VoxelGeometry.x;
  tal->PC.y *= tal->VoxelGeometry.y;
  tal->PC.z *= tal->VoxelGeometry.z;

  tal->Hemi.x *= tal->VoxelGeometry.x;
  tal->Hemi.y *= tal->VoxelGeometry.y;
  tal->Hemi.z *= tal->VoxelGeometry.z;

  VipInitTalairachVectors(tal);
  VipTalairachRotationAndTranslation(tal);
  VipTalairachBox(vol, tal);
}

int VipWriteTalairachRefFile(char *filename, VipTalairach *tal)
{
  char refName[VIP_NAME_MAXLEN];
  FILE *theFile;

  if (strstr(filename, ".vimg"))  *strstr(filename, ".vimg") = '\0';

  strcpy(refName, filename);
  strcat(refName, ".ref");

  theFile = fopen(refName, VIP_WRITE_TEXT);
  if (!theFile)
    {
      VipPrintfError("Enable to create Talairach reference file.");
      VipPrintfExit("(Vip_talairach.c)VipWriteTalairachRefFile");
      return(PB);
    }

  printf("Writting file : %s\n", refName);
  fprintf(theFile, "rotation 3D:\n");
  fprintf(theFile, "%f %f %f\n", tal->Translation[0], tal->Translation[1], 
	  tal->Translation[2]);
  fprintf(theFile, "%f %f %f\n", tal->Rotation[0][0], tal->Rotation[0][1], 
	  tal->Rotation[0][2]);
  fprintf(theFile, "%f %f %f\n", tal->Rotation[1][0], tal->Rotation[1][1], 
	  tal->Rotation[1][2]);
  fprintf(theFile, "%f %f %f\n", tal->Rotation[2][0], tal->Rotation[2][1], 
	  tal->Rotation[2][2]);
  fprintf(theFile, "scale factors:\n");
  fprintf(theFile, "%f %f %f\n", tal->Scale.x, tal->Scale.y, tal->Scale.z);
  fprintf(theFile, "box:\n");
  fprintf(theFile, "%d %d %d\n", tal->MinBox[0], tal->MinBox[1], 
	  tal->MinBox[2]);
  fprintf(theFile, "%d %d %d\n", tal->MaxBox[0], tal->MaxBox[1], 
	  tal->MaxBox[2]);
  fprintf(theFile, "voxel geometry (mm)\n");
  fprintf(theFile, "%f %f %f\n", tal->VoxelGeometry.x, tal->VoxelGeometry.y, 
	  tal->VoxelGeometry.z);
  fclose(theFile);

  return(OK);
}

int VipWriteTalairachTalFile(char *filename, VipTalairach *tal)
{
  float sx, sy;
  char talName[VIP_NAME_MAXLEN];
  FILE *theFile;

  if (strstr(filename, ".vimg"))  *strstr(filename, ".vimg") = '\0';

  strcpy(talName, filename);
  strcat(talName, ".tal");

  theFile = fopen(talName, VIP_WRITE_TEXT);
  if (!theFile)
    {
      VipPrintfError("Enable to create Talairach information file.");
      VipPrintfExit("(Vip_talairach.c)VipWriteTalairachTalFile");
      return(PB);
    }

  sx = tal->VoxelGeometry.x;
  sy = tal->VoxelGeometry.y;

  printf("Writting file : %s\n", talName);
  fprintf(theFile, "CA\n");
  fprintf(theFile, "%f\n%f\n%f\n", tal->AC.x/sx, tal->AC.y/sy, tal->AC.z/sx);
  fprintf(theFile, "CP\n");
  fprintf(theFile, "%f\n%f\n%f\n", tal->PC.x/sx, tal->PC.y/sy, tal->PC.z/sx);
  fprintf(theFile, "Pt_hemi\n");
  fprintf(theFile, "%f\n%f\n%f\n", tal->Hemi.x/sx, tal->Hemi.y/sy, 
	  tal->Hemi.z/sx);
  fprintf(theFile, "Pt_cor_min\n");
  fprintf(theFile, "%f\n%f\n%f\n", tal->MinCoronal.x, tal->MinCoronal.y, 
	  tal->MinCoronal.z);
  fprintf(theFile, "Pt_cor_max\n");
  fprintf(theFile, "%f\n%f\n%f\n", tal->MaxCoronal.x, tal->MaxCoronal.y, 
	  tal->MaxCoronal.z);
  fprintf(theFile, "Pt_axi_min\n");
  fprintf(theFile, "%f\n%f\n%f\n", tal->MinAxial.x, tal->MinAxial.y, 
	  tal->MinAxial.z);
  fprintf(theFile, "Pt_axi_max\n");
  fprintf(theFile, "%f\n%f\n%f\n", tal->MaxAxial.x, tal->MaxAxial.y, 
	  tal->MaxAxial.z);
  fprintf(theFile, "Pt_sag_min\n");
  fprintf(theFile, "%f\n%f\n%f\n", tal->MinSagittal.x, tal->MinSagittal.y, 
	  tal->MinSagittal.z);
  fprintf(theFile, "Pt_sag_max\n");
  fprintf(theFile, "%f\n%f\n%f\n", tal->MaxSagittal.x, tal->MaxSagittal.y, 
	  tal->MaxSagittal.z);
  fprintf(theFile, "Vec_hemi\n");
  fprintf(theFile, "%f\n%f\n%f\n", tal->Hemi_vector.x, tal->Hemi_vector.y, 
	  tal->Hemi_vector.z);
  fprintf(theFile, "Vec_CACP\n");
  fprintf(theFile, "%f\n%f\n%f\n", tal->ACPC_vector.x, tal->ACPC_vector.y, 
	  tal->ACPC_vector.z);
  fprintf(theFile, "Vec_autre\n");
  fprintf(theFile, "%f\n%f\n%f\n", tal->Cross_vector.x, tal->Cross_vector.y, 
	  tal->Cross_vector.z);
  fprintf(theFile, "Rot_total\n");
  fprintf(theFile, "%f %f %f\n", tal->Rotation[0][0], tal->Rotation[0][1], 
	  tal->Rotation[0][2]);
  fprintf(theFile, "%f %f %f\n", tal->Rotation[1][0], tal->Rotation[1][1], 
	  tal->Rotation[1][2]);
  fprintf(theFile, "%f %f %f\n", tal->Rotation[2][0], tal->Rotation[2][1], 
	  tal->Rotation[2][2]);
  fprintf(theFile, "Dim\n");
  fprintf(theFile, "%f\n%f\n%f\n", tal->VoxelGeometry.x, tal->VoxelGeometry.y, 
	  tal->VoxelGeometry.z);
  fclose(theFile);

  return(OK);
}

int VipWriteTalairachFiles(char *filename, VipTalairach *tal)
{
  if (!VipWriteTalairachRefFile(filename, tal))  return(PB);
  if (!VipWriteTalairachTalFile(filename, tal))  return(PB);

  return(OK);
}

int VipReadTalairachRefFile(char *filename, VipTalairach *tal)
{
  char refName[VIP_NAME_MAXLEN], buf[256];
  FILE *theFile;

  if (strstr(filename, ".vimg"))  *strstr(filename, ".vimg") = '\0';

  strcpy(refName, filename);
  strcat(refName, ".ref");

  theFile = fopen(refName, VIP_READ_TEXT);
  if (!theFile)
    {
      VipPrintfError("Enable to read Talairach reference file.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile");
      return(PB);
    }

  printf("Reading file : %s\n", refName);
  if ( !fgets(buf, 256, theFile) )
  {
    VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile : Corrupted file");
    return(PB);
  }
  if (!strncmp(buf, "rotation 3D:", 12))
    {
      if ( fscanf(theFile, "%f%f%f", &tal->Translation[0], &tal->Translation[1], 
	                &tal->Translation[2]) != 3 )
      {
        VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile : Corrupted file");
        return(PB);
      }
      if ( fscanf(theFile, "%f%f%f", &tal->Rotation[0][0], &tal->Rotation[0][1],
	                &tal->Rotation[0][2]) != 3 )
      {
        VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile : Corrupted file");
        return(PB);
      }
      if ( fscanf(theFile, "%f%f%f", &tal->Rotation[1][0], &tal->Rotation[1][1],
	                &tal->Rotation[1][2]) != 3 )
      {
        VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile : Corrupted file");
        return(PB);
      }
      if ( fscanf(theFile, "%f%f%f\n", &tal->Rotation[2][0], &tal->Rotation[2][1],
	         &tal->Rotation[2][2]) != 3 )
      {
        VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile : Corrupted file");
        return(PB);
      }
    }
  else
    {
      VipPrintfError("Talairach reference file corrupted (rotation 3D).");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile");
      return(PB);
    }

  if ( !fgets(buf, 256, theFile) )
  {
    VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile : Corrupted file");
    return(PB);
  }
  if (!strncmp(buf, "scale factors:", 14))
    if ( fscanf(theFile, "%f%f%f\n", &tal->Scale.x, &tal->Scale.y, &tal->Scale.z) != 3 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile : Corrupted file");
      return(PB);
    }
  else 
    {
      VipPrintfError("Talairach reference file corrupted (scale factors).");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile");
      return(PB);      
    }

  if ( !fgets(buf, 256, theFile) )
  {
    VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile : Corrupted file");
    return(PB);
  }
  if (!strncmp(buf, "box:", 4))
    {
      if ( fscanf(theFile, "%d%d%d", &tal->MinBox[0], &tal->MinBox[1], 
	                &tal->MinBox[2]) != 3 )
      {
        VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile : Corrupted file");
        return(PB);
      }
      if ( fscanf(theFile, "%d%d%d\n", &tal->MaxBox[0], &tal->MaxBox[1],
	                &tal->MaxBox[2]) != 3 )
      {
        VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile : Corrupted file");
        return(PB);
      }
    }
  else 
    {
      VipPrintfError("Talairach reference file corrupted (box).");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile");
      return(PB);      
    }

  if ( !fgets(buf, 256, theFile) )
  {
    VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile : Corrupted file");
    return(PB);
  }
  if (!strncmp(buf, "voxel geometry (mm)", 19))
    if ( fscanf(theFile, "%f%f%f\n", &tal->VoxelGeometry.x, &tal->VoxelGeometry.y,
	              &tal->VoxelGeometry.z) != 3 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile : Corrupted file");
      return(PB);
    }
 else 
    {
      VipPrintfError("Talairach reference file corrupted (voxel geometry).");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachRefFile");
      return(PB);      
    }

  fclose(theFile);

  return(OK);
}

int VipReadTalairachTalFile(char *filename, VipTalairach *tal)
{
  char talName[VIP_NAME_MAXLEN], buf[20];
  FILE *theFile;

  if (strstr(filename, ".vimg"))  *strstr(filename, ".vimg") = '\0';

  strcpy(talName, filename);
  strcat(talName, ".tal");

  theFile = fopen(talName, VIP_READ_TEXT);
  if (!theFile)
    {
      VipPrintfError("Enable to read Talairach information file.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);
    }

  printf("Reading file : %s\n", talName);
  if ( fscanf(theFile, "%s", buf) != 1 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }

  if (!strcmp(buf, "CA"))
    if ( fscanf(theFile, "%f%f%f", &tal->AC.x, &tal->AC.y, &tal->AC.z) != 3 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  else 
    {
      VipPrintfError("Talairach information file corrupted.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);      
    }

  if ( fscanf(theFile, "%s", buf) != 1 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  if (!strcmp(buf, "CP"))
    if ( fscanf(theFile, "%f%f%f", &tal->PC.x, &tal->PC.y, &tal->PC.z) != 3 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  else 
    {
      VipPrintfError("Talairach information file corrupted.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);      
    }

  if ( fscanf(theFile, "%s", buf) != 1 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  if (!strcmp(buf, "Pt_hemi"))
    if ( fscanf(theFile, "%f%f%f", &tal->Hemi.x, &tal->Hemi.y, &tal->Hemi.z) != 3 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  else 
    {
      VipPrintfError("Talairach information file corrupted.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);      
    }

  if ( fscanf(theFile, "%s", buf) != 1 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  if (!strcmp(buf, "Pt_cor_min"))
    if ( fscanf(theFile, "%f%f%f", &tal->MinCoronal.x, &tal->MinCoronal.y, 
	              &tal->MinCoronal.z) != 3 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  else 
    {
      VipPrintfError("Talairach information file corrupted.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);      
    }

  if ( fscanf(theFile, "%s", buf) != 1 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  if (!strcmp(buf, "Pt_cor_max"))
    if ( fscanf(theFile, "%f%f%f", &tal->MaxCoronal.x, &tal->MaxCoronal.y, 
	              &tal->MaxCoronal.z) != 3 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  else 
    {
      VipPrintfError("Talairach information file corrupted.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);      
    }

  if ( fscanf(theFile, "%s", buf) != 1 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  if (!strcmp(buf, "Pt_axi_min"))
    if ( fscanf(theFile, "%f%f%f", &tal->MinAxial.x, &tal->MinAxial.y,
	       &tal->MinAxial.z) != 3 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  else 
    {
      VipPrintfError("Talairach information file corrupted.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);      
    }

  if ( fscanf(theFile, "%s", buf) != 1 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  if (!strcmp(buf, "Pt_axi_max"))
    if ( fscanf(theFile, "%f%f%f", &tal->MaxAxial.x, &tal->MaxAxial.y,
	              &tal->MaxAxial.z) != 3 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  else 
    {
      VipPrintfError("Talairach information file corrupted.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);      
    }

  if ( fscanf(theFile, "%s", buf) != 1 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  if (!strcmp(buf, "Pt_sag_min"))
    if ( fscanf(theFile, "%f%f%f", &tal->MinSagittal.x, &tal->MinSagittal.y,
	              &tal->MinSagittal.z) != 3 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  else 
    {
      VipPrintfError("Talairach information file corrupted.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);      
    }

  if ( fscanf(theFile, "%s", buf) != 1 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  if (!strcmp(buf, "Pt_sag_max"))
    if ( fscanf(theFile, "%f%f%f", &tal->MaxSagittal.x, &tal->MaxSagittal.y,
	              &tal->MaxSagittal.z) != 3 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  else 
    {
      VipPrintfError("Talairach information file corrupted.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);      
    }

  if ( fscanf(theFile, "%s", buf) != 1 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  if (!strcmp(buf, "Vec_hemi"))
    if ( fscanf(theFile, "%f%f%f",  &tal->Hemi_vector.x, &tal->Hemi_vector.y,
	              &tal->Hemi_vector.z) != 3 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  else 
    {
      VipPrintfError("Talairach information file corrupted.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);      
    }

  if ( fscanf(theFile, "%s", buf) != 1 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  if (!strcmp(buf, "Vec_CACP"))
    if ( fscanf(theFile, "%f%f%f", &tal->ACPC_vector.x, &tal->ACPC_vector.y,
	              &tal->ACPC_vector.z) != 3 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  else 
    {
      VipPrintfError("Talairach information file corrupted.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);      
    }

  if ( fscanf(theFile, "%s", buf) != 1 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  if (!strcmp(buf, "Vec_autre"))
    if ( fscanf(theFile, "%f%f%f", &tal->Cross_vector.x, &tal->Cross_vector.y,
	              &tal->Cross_vector.z) != 3 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  else 
    {
      VipPrintfError("Talairach information file corrupted.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);      
    }

  if ( fscanf(theFile, "%s", buf) != 1 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  if (!strcmp(buf, "Rot_total"))
    {
      if ( fscanf(theFile, "%f%f%f", &tal->Rotation[0][0], &tal->Rotation[0][1],
	                &tal->Rotation[0][2]) != 3 )
      {
        VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
        return(PB);
      }
      if ( fscanf(theFile, "%f%f%f", &tal->Rotation[1][0], &tal->Rotation[1][1],
	                &tal->Rotation[1][2]) != 3 )
      {
        VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
        return(PB);
      }
      if ( fscanf(theFile, "%f%f%f", &tal->Rotation[2][0], &tal->Rotation[2][1],
	                &tal->Rotation[2][2]) != 3 )
      {
        VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
        return(PB);
      }
    }
  else 
    {
      VipPrintfError("Talairach information file corrupted.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);      
    }

  if ( fscanf(theFile, "%s", buf) != 1 )
    {
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
      return(PB);
    }
  if (!strcmp(buf, "Dim"))
    {
      if ( fscanf(theFile, "%f%f%f", &tal->VoxelGeometry.x, &tal->VoxelGeometry.y,
	                &tal->VoxelGeometry.z) != 3 )
      {
        VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile : Corrupted file");
        return(PB);
      }
    }
  else 
    {
      VipPrintfError("Talairach information file corrupted.");
      VipPrintfExit("(Vip_talairach.c)VipReadTalairachTalFile");
      return(PB);      
    }

  fclose(theFile);

  tal->AC.x *= tal->VoxelGeometry.x;
  tal->AC.y *= tal->VoxelGeometry.y;
  tal->AC.z *= tal->VoxelGeometry.x;

  tal->PC.x *= tal->VoxelGeometry.x;
  tal->PC.y *= tal->VoxelGeometry.y;
  tal->PC.z *= tal->VoxelGeometry.x;

  tal->Hemi.x *= tal->VoxelGeometry.x;
  tal->Hemi.y *= tal->VoxelGeometry.y;
  tal->Hemi.z *= tal->VoxelGeometry.x;

  return(OK);
}

int VipReadTalairachFiles(char *filename, VipTalairach *tal)
{
  if (!VipReadTalairachRefFile(filename, tal)) return(PB);
  if (!VipReadTalairachTalFile(filename, tal)) return(PB);

  return(OK);
}

void VipPrintTalairach(VipTalairach *tal)
{
  printf("***  Talairach structure informations  ***\n\n");

  printf("+++ The three Talairach basic points +++\n");
  printf("\tAC point coordinates (mm) :\n");
  printf("\tx = %f  y = %f  z = %f\n", tal->AC.x, tal->AC.y, tal->AC.z);
  printf("\tPC point coordinates (mm) :\n");
  printf("\tx = %f  y = %f  z = %f\n", tal->PC.x, tal->PC.y, tal->PC.z);
  printf("\tInter-hemispheric point coordinates (mm) :\n");
  printf("\tx = %f  y = %f  z = %f\n", tal->Hemi.x, tal->Hemi.y, tal->Hemi.z);

  printf("+++ Intersections of the three Talairach planes +++\n");
  printf("\tMinimum Sagittal coordinates (Z normalized by sz/sx) :\n");
  printf("\tx = %f  y = %f  z = %f\n", tal->MinSagittal.x, tal->MinSagittal.y,
	 tal->MinSagittal.z);
  printf("\tMaximum Sagittal coordinates (Z normalized by sz/sx) :\n");
  printf("\tx = %f  y = %f  z = %f\n", tal->MaxSagittal.x, tal->MaxSagittal.y,
	 tal->MaxSagittal.z);
  printf("\tMinimum Coronal coordinates (Z normalized by sz/sx) :\n");
  printf("\tx = %f  y = %f  z = %f\n", tal->MinCoronal.x, tal->MinCoronal.y,
	 tal->MinCoronal.z);
  printf("\tMaximum Coronal coordinates (Z normalized by sz/sx) :\n");
  printf("\tx = %f  y = %f  z = %f\n", tal->MaxCoronal.x, tal->MaxCoronal.y,
	 tal->MaxCoronal.z);
  printf("\tMinimum Axial coordinates (Z normalized by sz/sx) :\n");
  printf("\tx = %f  y = %f  z = %f\n", tal->MinAxial.x, tal->MinAxial.y,
	 tal->MinAxial.z);
  printf("\tMaximum Axial coordinates (Z normalized by sz/sx) :\n");
  printf("\tx = %f  y = %f  z = %f\n", tal->MaxAxial.x, tal->MaxAxial.y,
	 tal->MaxAxial.z);

  printf("+++ Talairach basis vectors +++\n");
  printf("\tHemispheric vector (equivalent to X) :\n");
  printf("\tx = %f  y = %f  z = %f\n", tal->Hemi_vector.x, tal->Hemi_vector.y,
	 tal->Hemi_vector.z);
  printf("\tAC-PC vector (equivalent to Y) :\n");
  printf("\tx = %f  y = %f  x = %f\n", tal->ACPC_vector.x, tal->ACPC_vector.y,
	 tal->ACPC_vector.z);
  printf("\tCross vector (equivalent to Z) :\n");
  printf("\tx = %f  y = %f  z = %f\n", tal->Cross_vector.x, 
	 tal->Cross_vector.y, tal->Cross_vector.z);

  printf("+++ Talairach transformation parameters +++\n");
  printf("\tScale factors (for unitary box normalization) :\n");
  printf("\tx = %f  y = %f  z = %f\n", tal->Scale.x, tal->Scale.y, 
	 tal->Scale.z);
  printf("\tTranslation vector (mm) :\n");
  printf("\tTx = %f  Ty = %f  Tz = %f\n", tal->Translation[0],
	 tal->Translation[1], tal->Translation[2]);
  printf("\tRotation matrix :\n");
  printf("\tRxx = %f  Rxy = %f  Rxz = %f\n", tal->Rotation[0][0],
	 tal->Rotation[0][1], tal->Rotation[0][2]);
  printf("\tRyx = %f  Ryy = %f  Ryz = %f\n", tal->Rotation[1][0],
	 tal->Rotation[1][1], tal->Rotation[1][2]);
  printf("\tRzx = %f  Rzy = %f  Rzz = %f\n", tal->Rotation[2][0],
	 tal->Rotation[2][1], tal->Rotation[2][2]);

  printf("+++ Talairach box +++\n");
  printf("\tMinimum box point :\n");
  printf("\tx = %d  y = %d  z = %d\n", tal->MinBox[0], tal->MinBox[1],
	 tal->MinBox[2]);
  printf("\tMaximum box point :\n");
  printf("\tx = %d  y = %d  z = %d\n", tal->MaxBox[0], tal->MaxBox[1],
	 tal->MaxBox[2]);

  printf("+++ Data volume informations +++\n");
  printf("\tVoxel geometry (mm) :\n");
  printf("\tsx = %f  sy = %f  sz = %f\n", tal->VoxelGeometry.x, 
	 tal->VoxelGeometry.y, tal->VoxelGeometry.z);
}
