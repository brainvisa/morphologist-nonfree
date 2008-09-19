
#include <errno.h>
#include <string.h>
#include <vip/epidistorsion/readwrite.h>

#define READWRITE_DEBUG 0

/* -----------------------------------------------
   functions reading command line arguments 
   ----------------------------------------------- */
int ReadInt ( int *res, FILE *stream) {
  char buf[100];

  
  if (fgets (buf, 100, stream) == NULL) {return 1;}
  else {
    errno = 0;
    *res = (int) strtol(buf, (char**)NULL, 10);
    if ((*res == 0)||(errno != 0)) {return 1;}
    else { return 0;}
  }
}


int ReadDouble ( double *res, FILE *stream) {
  char buf[50];
  errno = 0;
  
  if (fgets (buf, 100, stream) == NULL) {return 1;}
  else {
    *res = atof(buf);
    if ((*res == 0)||(errno != 0)) {return 1;}
    else {return 0;}
  }
}

int ReadString ( char *res, FILE *stream) {
  char buf[200];
  int n;
  
  if (fgets (buf, 200, stream) == NULL) {
#if READWRITE_DEBUG
    printf ("ReadString : fgets  error\n");
#endif
    return 1;
  } else {
    n = sscanf (buf,"%s",res);
    if ((n == EOF)||(n==0)) {
#if READWRITE_DEBUG
      Write ("ReadString : sscanf error\n");
#endif
      return 1;
    } else 
      {return 0;}
  }
}

void Write (char *msg) { printf ("%s ",msg);}

void WriteInt (int n) { printf ("%d ",n);}
  
void WriteDouble (double x) { printf ("%f ",(float) x);}
 
void NewLine () { printf ("%s","\n");}


int fWrite (char *msg, FILE *stream) {
  if (fprintf(stream,"%s\n",msg) < 0) return 1;
  else return 0;
}
  
int fWriteInt (int n, FILE *stream) {
  if (fprintf(stream, "%d\n",n) < 0) return 1;
  else return 0;
}
  
int fWriteDouble (double x, FILE *stream) {
  if (fprintf(stream,"%f\n",(float) x) <0) return 1;
  else return 0;
}


/* --------------------------------------------
   functions getting args either in argv or
   interactively
   -------------------------------------------- */

void GetInt (int *res, char *name, int argc, char *argv[], int pos) {
  int temp_int;
  int dflt = *res;
 
  printf ("%s (default : %d) : ",name,dflt);
  if (argc > pos) {
    errno = 0;
    temp_int = (int) strtol(argv[pos], (char**)NULL, 10);
    if ((temp_int == 0)||(errno != 0)) {
      NewLine();
      printf ("incorrect command line entry, please re-enter (default : %d) : ",dflt);
      if (ReadInt (&temp_int, stdin) != 1) {*res = temp_int;}
    } else {
      *res = temp_int;
    }
  } else 
    if (ReadInt (&temp_int,stdin) != 1) {*res = temp_int;}
  WriteInt (*res);
  NewLine();
}

void GetDouble (double *res, char *name, int argc, char *argv[], int pos) {
  double temp;
  double dflt = *res;

  printf ("%s (default : %f) : ",name,dflt);
  if (argc > pos) {
    errno = 0;
    temp = atof(argv[pos]);
    if (temp == 0) {
      NewLine();
      printf ("incorrect command line entry, please re-enter (default : %f) : ",dflt);
      if (ReadDouble (&temp,stdin) != 1) {*res = temp;}
    } else {
      *res = temp;
    }
  } else 
    if (ReadDouble (&temp,stdin) != 1) {*res = temp;}
  WriteDouble(*res);
  NewLine();
}

void GetString (char *res, char *name, int argc, char *argv[], int pos) {
  char temp[200];
  char *dflt = res;

  printf ("%s (default : %s) : ",name,dflt);
  if (argc > pos) {
    (void) strcpy (res, argv[pos]);
  }
  else 
    if (ReadString (temp,stdin) != 1) {(void) strcpy(res,temp);}
  Write (res);
  NewLine();

}
  

