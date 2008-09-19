/****************************************************************************
 * PROJECT     : V.I.P. Library
 ****************************************************************************
 *
 *
 ****************************************************************************
 *
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/


#include <vip/util/file.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/*-------------------------------------------------------------------------*/
char VipFileSeparator()
/*-------------------------------------------------------------------------*/
{
#ifdef _WIN32
  return '\\';
#else
  return '/';
#endif
}

/*-------------------------------------------------------------------------*/
int VipExists( const char *filename )
/*-------------------------------------------------------------------------*/
{
  struct stat	s;
  return !stat( filename, &s );
}

/*-------------------------------------------------------------------------*/
int VipIsDirectory( const char *dirname )
/*-------------------------------------------------------------------------*/
{
  DIR	*dir = opendir( dirname );
  if( !dir )
    return 0;
  else
    closedir( dir );
  return 1;
}

/*-------------------------------------------------------------------------*/
int VipIsSymlink( const char * filename )
/*-------------------------------------------------------------------------*/
{
#ifdef _WIN32

  return 0;

#else

  char	buf[10000];
  return( readlink( filename, buf, 9999 ) > 0 );

  /* strange, why doesnt' this work ? 
  struct stat	s;
  if( stat( filename, &s ) )
    return 0;
  printf( "IsSymlink( %s ): %d, mode: %d, dir: %d\n", filename, 
          S_ISLNK( s.st_mode ), 
          s.st_mode, S_ISDIR( s.st_mode ) );
  return S_ISLNK( s.st_mode );
  */

#endif
}

/*-------------------------------------------------------------------------*/
int VipMkdir( const char *dirname )
/*-------------------------------------------------------------------------*/
{
  DIR	*dir = opendir( dirname );
  if( !dir )
    {
#if defined( _WIN32 )
      if( mkdir( dirname ) )
#else
      if( mkdir( dirname, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) )
#endif
        return 0;
      return 1;
    }
  else
    closedir( dir );
  return 0;
}


/*-------------------------------------------------------------------------*/
void VipBasename( const char* orig, char* dest )
/*-------------------------------------------------------------------------*/
{
  const char *root1, *root2;
  root2 = orig;
  root1 = strrchr( orig, '/' );
#ifdef _WIN32
  root2 = strrchr( orig, '\\' );
  if( root2 != NULL && (root1 == NULL || root1 < root2 ) )
    root1 = root2;
#endif
  if( root1 != 0 )
    ++root1;
  else
    root1 = orig;
  strcpy( dest, root1 );
}


/*-------------------------------------------------------------------------*/
void VipDirname( const char* orig, char* dest )
/*-------------------------------------------------------------------------*/
{
  const char *root1, *root2;
  root2 = orig;
  root1 = strrchr( orig, '/' );
#ifdef _WIN32
  root2 = strrchr( orig, '\\' );
  if( root2 != NULL && (root1 == NULL || root1 < root2 ) )
    root1 = root2;
#endif
  if( root1 != NULL )
    {
      strcpy( dest, orig );
      dest[ root1 - orig ] = '\0';
    }
  else
    {
      if( strcmp( orig, ".." ) == 0 )
        strcpy( dest, ".." );
      else
        strcpy( dest, "." );
    }
}


/*-------------------------------------------------------------------------*/
void VipUnixFilename( const char* orig, char* dest )
/*-------------------------------------------------------------------------*/
{
  const char	*root1;
  char		*root2;
  root1 = orig;
  root2 = dest;
  do
    {
      if( *root1 == '\\' )
        *root2 = '/';
      else
        *root2 = *root1;
    }
  while( *root1 );
}


/*-------------------------------------------------------------------------*/
const char *VipTmpDirectory()
/*-------------------------------------------------------------------------*/
{
  static char	*tmp = NULL;

  if( tmp == NULL )
    {
#ifndef _WIN32
      tmp = "/tmp";
#else
      tmp = getenv( "TMP" );
      if( tmp == NULL )
        {
          tmp = getenv( "TEMP" );
          if( tmp == NULL )
            {
              tmp = "C:\\TEMP";
              VipMkdir( tmp );
            }
        }
#endif
    }

  return tmp;
}


/*-------------------------------------------------------------------------*/
void VipUnlink( const char* file )
/*-------------------------------------------------------------------------*/
{
  unlink( file );
}


/*-------------------------------------------------------------------------*/
void VipRename( const char* src, const char* dst )
/*-------------------------------------------------------------------------*/
{
  rename( src, dst );
}


/*-------------------------------------------------------------------------*/
void VipFileCat( const char* src, const char* dst )
/*-------------------------------------------------------------------------*/
{
  FILE		*fw, *fi;
  char		buf[ 10000 ];
  size_t	sz;

  fi = fopen( src, "rb" );
  if( fi == NULL )
    {
      printf( "Cannot read %s\n", src );
      return;
    }
  fw = fopen( dst, "wb" );
  if( fw == NULL )
    {
      printf( "Cannot write %s\n", dst );
      return;
    }
  while( !feof( fi ) )
    {
      sz = fread( buf, 1, 10000, fi );
      fwrite( buf, 1, sz, fw );
    }
}


