/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : topology_test.c  * TYPE     : Function
 * AUTHOR      : POUPON F.            * CREATION : 22/01/1997
 * VERSION     : 1.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Test topological classification
 *
 ****************************************************************************
 *
 * USED MODULES : math.h - alloc.h - topology.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <vip/topology.h>
#include <vip/topology_static.h>

void VipDisplayTopologySingularityType(int C18, int C26)
{
  int falseC18, falseC26;

  falseC18 = (C18 >= 3) ? 3 : C18;
  falseC26 = (C26 >= 3) ? 3 : C26;

  printf("C18=%d\tC26=%d\tTopology singularity type : %s\n", C18, C26,
	 TopologyTable[falseC26][falseC18]);
  fflush(stdout);
}

Volume *VipCreateTopologyTestVolume(int type)
{ VIP_DEC_VOLUME(vol);
  int i, j, k;
  Vip_S16BIT *volptr;

  vol = VipCreate3DVolume(5, 5, 5, 1.0, 1.0, 1.0, S16BIT, "", 0);
  if (!vol) 
  { VipPrintfError("Error while creating volume");
    VipPrintfExit("(commandline)VipCreateTopologyTestVolume");
    return((Volume *)NULL);
  }

  switch(type)
  { case TOPO_A : volptr = VipGetDataPtr_S16BIT(vol)+31;
                  for (k=1; k<4; k++, volptr+=10)
                    for (j=1; j<4; j++, volptr+=2)
                      for (i=1; i<4; i++)  *volptr++ = 1;
                  break;
    case TOPO_B : volptr = VipGetDataPtr_S16BIT(vol)+62;
                  *volptr = 1;
                  break;
    case TOPO_C : volptr = VipGetDataPtr_S16BIT(vol);
                  *(volptr+62) = *(volptr+63) = *(volptr+88) = *(volptr+113) = 1;
                  for (i=2; i<5; i++)  *(volptr+104+i*5) = 1;
                  break;
    case TOPO_D : volptr = VipGetDataPtr_S16BIT(vol);
                  *(volptr+62) = *(volptr+12) = *(volptr+13) = *(volptr+37) = 1;
                  *(volptr+87) = *(volptr+112) = *(volptr+113) = 1;
                  break;
    case TOPO_E : volptr = VipGetDataPtr_S16BIT(vol);
                  for (i=0; i<5; i++)  *(volptr+50+6*i) = 1;
                  for (i=1; i<3; i++) *(volptr+60+i) = 1;
                  *(volptr+5) = *(volptr+35) = *(volptr+83) = *(volptr+104) = 1;
                  break;
    case TOPO_F : volptr = VipGetDataPtr_S16BIT(vol);
                  for (j=5; j--;)
                    for (i=5; i--;)  *(volptr+26*j+5*i) = 1;
                  break;
    case TOPO_G : volptr = VipGetDataPtr_S16BIT(vol);
                  for (i=3;i--;)
		  { *(volptr+31+25*i) = *(volptr+33+25*i) = 1;
                    *(volptr+41+25*i) = *(volptr+43+25*i) = 1;
		  }
                  *(volptr+38) = *(volptr+88) = *(volptr+92) = *(volptr+62) = 1;
                  break;
    case TOPO_H : volptr = VipGetDataPtr_S16BIT(vol);
                  for (j=0; j<5; j++)
                    for (i=0; i<5; i++) 
                      *(volptr+j*26+5*i) = *(volptr+24*j+5*i+4) = 1;
                  break;
    case TOPO_I : volptr = VipGetDataPtr_S16BIT(vol);
	          for (i=3; i--;)  
                    *(volptr+31+25*i) = *(volptr+33+25*i) = *(volptr+43+25*i) = 1;
                  *(volptr+32) = *(volptr+37) = *(volptr+41) = *(volptr+62) = 1;
                  *(volptr+82) = *(volptr+88) = *(volptr+91) = 1;
                  break;
  }

  return(vol);
}

void VipDisplayTopologyTestVolume(Volume *vol)
{ int i;
  Vip_S16BIT *volptr;

  volptr = VipGetDataPtr_S16BIT(vol);

  for (i=0; i<5; i++)
  { printf("%1d%1d%1d%1d%1d  ", *(volptr+5*i), *(volptr+5*i+1), *(volptr+5*i+2), 
                                *(volptr+5*i+3), *(volptr+5*i+4));
    printf("%1d%1d%1d%1d%1d  ", *(volptr+25+5*i), *(volptr+25+5*i+1), *(volptr+25+5*i+2), 
                                *(volptr+25+5*i+3), *(volptr+25+5*i+4));
    printf("%1d%1d%1d%1d%1d  ", *(volptr+50+5*i), *(volptr+50+5*i+1), *(volptr+50+5*i+2), 
                                *(volptr+50+5*i+3), *(volptr+50+5*i+4));
    printf("%1d%1d%1d%1d%1d  ", *(volptr+75+5*i), *(volptr+75+5*i+1), *(volptr+75+5*i+2), 
                                *(volptr+75+5*i+3), *(volptr+75+5*i+4));
    printf("%1d%1d%1d%1d%1d\n", *(volptr+100+5*i), *(volptr+100+5*i+1), *(volptr+100+5*i+2), 
                                *(volptr+100+5*i+3), *(volptr+100+5*i+4));
  }
}

long *VipGetTopologyTypeTable(Topology26Neighborhood *topo)
{ int *X;
  long i, j, C18, C26;
  long *typeTable;
  long tempTable[26][26];

  X = topo->ordered_values;
  X[0] = 0;

  typeTable = (long *)VipCalloc(9, sizeof(long), NULL);
  if (!typeTable)
  { VipPrintfError("Error while allocating typeTable");
    VipPrintfExit("(topology.c)VipGetTopologyTypeTable");
    return((long *)NULL);
  }

  for (j=26;j--;)
    for (i=26;i--;)  tempTable[j][i] = 0;

  printf("Remaining combinations : %8d", 67108864); /** 2^26 **/
  fflush(stdout);
  for (i=67108864; i;) 
  { do
    { for (i--, j=0; j<26; j++)  X[j+1] = ((i >> j) & 1) ? 1 : 0;
      
      C26 = VipGetNumberOfComponentsIn26Neighborhood(topo,CONNECTIVITY_26,CC_26_ADJACENT);
      VipFlipTopology26NeighborhoodValueTable(topo);
      C18 = VipGetNumberOfComponentsIn18Neighborhood(topo,CONNECTIVITY_6,CC_6_ADJACENT);
 
      tempTable[C26][C18]++;
    }
    while (i & 0x3FFF);
    printf("\b\b\b\b\b\b\b\b%8ld", i);
    fflush(stdout);
  }
  printf("\n");

  for (i=26; i--;)  
  { typeTable[0] += tempTable[i][0];
    typeTable[1] += tempTable[0][i];
  }
  typeTable[2] = tempTable[1][1];
  typeTable[3] = tempTable[2][1];
  for (i=3; i<26; i++)  
  { typeTable[4] += tempTable[i][1];
    typeTable[7] += tempTable[1][i];
  }
  typeTable[5] = tempTable[1][2];
  for (i=2; i<26; i++)  typeTable[6] += tempTable[i][2];
  for (j=2; j<26; j++)
    for (i=3; i<26; i++)  typeTable[8] += tempTable[j][i];
  
  return(typeTable);
}

void VipDisplayTopologyTypeTable(char *filename, long *typeTable)
{ int i, sum=0;

/* these procedures were written for test purpose,
   you can find correct numbers in G. Malandain PhD thesis */
  if (filename != NULL)  VipDisplayTopologyTypeTableInFile(filename, typeTable);
  else
  { for (i=0; i<9; sum+=typeTable[i], i++)  printf("%c : %ld\n", (char)(i+65), typeTable[i]);
    printf("sum = %d\n", sum);
  }
}

void VipDisplayTopologyTypeTableInFile(char *filename, long *typeTable)
{ int i, sum=0;
  FILE *file;

  file = fopen(filename, "wt");
  for (i=0; i<9; sum+= typeTable[i], i++)  fprintf(file, "%c : %ld\n", (char)(i+65), typeTable[i]);
  fprintf(file, "sum = %d\n", sum);

  fclose(file);
}
