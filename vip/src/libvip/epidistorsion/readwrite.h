#ifndef NDEF_PARAM
#define NDEF_PARAM 0
#include <vip/epidistorsion/param.h>
#endif

#include <stdio.h>
#include <stdlib.h>

/* functions reading command line arguments */

int ReadInt ( int *res, FILE *stream);
int ReadDouble ( double *res, FILE *stream);
int ReadString ( char *res, FILE *stream);

void Write (char *msg);
void WriteInt (int n);
void WriteDouble (double x);

void NewLine ();

int fWrite (char *msg, FILE *stream);
int fWriteInt (int n, FILE *stream);
int fWriteDouble (double x, FILE *stream);

/* functions getting args either in argv or interactively */

void GetInt (int *res, char *name, int argc, char *argv[], int pos);
void GetDouble (double *res, char *name, int argc, char *argv[], int pos);
void GetString (char *res, char *name, int argc, char *argv[], int pos);


