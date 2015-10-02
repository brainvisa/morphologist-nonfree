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

#define _DEFAULT_SOURCE  /* to have unistd.h define readlink and DT_DIR */

#include <vip/util/shelltools.h>
#include <vip/util/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

VipStringListItem* 
VipShellExpandWick( const char* pattern, VipStringList* list, 
                    VipStringListItem* after )
{
  const char		*string, *p, *found, *pi;
  char			*dname, *patexp, *pj, *name = NULL, *pname = NULL;
  VipStringListItem	*firstitem, *item;
  int			cflags;
  regex_t		regex;
  DIR			*dir;
  struct dirent		*dent;
  size_t		nlen = 0, nlen2;

  firstitem = NULL;
  item = NULL;
  found = NULL;

  /* find first unique directory */
  for( pi=pattern, p=pattern, string=pattern; *p!='\0' && found==NULL; )
    {
      string = pi;
      for( p=string; *p!='\0' && *p!='/' && *p!='\\'; ++p )
        if( *p == '*' || *p == '?' )
          found = p;
      if( found == NULL )
        pi = p + 1;
    }
  if( found == NULL )	/* no wildcard: single file/directory */
    {
      if( VipExists( pattern ) )
        firstitem = VipInsertString( list, pattern, after );
      return firstitem;
    }

  /* make directory name pattern -> string */
  if( string == pattern )
    {
      dname = malloc( 3 );
      strcpy( dname, "./" );
    }
  else
    {
      dname = malloc( string - pattern + 1 );
      for( pi=pattern, pj=dname; pi!=string; ++pi, ++pj )
        *pj = *pi;
      *pj = '\0';
    }

  /* make matching regex */
  patexp = malloc( ( p - string ) * 2 + 2 );
  *patexp = '^';
  for( pi=string, pj=patexp + 1; pi!=p; ++pi, ++pj )
    switch( *pi )
      {
      case '*':
        *pj++ = '.';
        *pj = '*';
        break;
      case '?':
        *pj = '.';
        break;
      case '.':
      case '^':
      case '$':
      case '[':
        /* case ']': */
      case '(':
      case ')':
      case '{':
        /* case '}': */
      case '|':
      case '+':
        *pj++ = '\\';
        *pj = *pi;
        break;
      default:
        *pj = *pi;
      }
  *pj++ = '$';
  *pj = '\0';

#ifdef _WIN32
  cflags = REG_ICASE;
#else
  cflags = 0;
#endif

  if( regcomp( &regex, patexp, cflags ) )
    printf( "Warning: VipShellExpand: regcomp() failed for pattern %s\n", 
            patexp );
  else
    {
      /* scan directory */
      dir = opendir( dname );
      if( !dir )
        printf( "Warning: VipShellExpand: opendir() failed for %s\n", dname );
      else
        {
          item = after;

          while( ( dent = readdir( dir ) ) )
            if( strcmp( dent->d_name, "." ) && strcmp( dent->d_name, ".." ) 
                && !regexec( &regex, dent->d_name, 0, NULL, 0 ) )
              {
                nlen2 = strlen( dent->d_name ) + 2 + strlen( pattern ) 
                  + string - p;
                if( nlen2 > nlen )
                  {
                    if( nlen != 0 )
                      free( name );
                    name = malloc( nlen2 );
                    strcpy( name, dname );
                    pname = name + ( string - pattern );
                    nlen = nlen2;
                  }
                strcpy( pname, dent->d_name );
                if( *p == 0 ) /* end point file/dir */
                  {
                    item = VipInsertString( list, name, item );
                    if( firstitem == NULL )
                      firstitem = item;
                  }
                else
#ifdef _DIRENT_HAVE_D_TYPE
                  if( dent->d_type == DT_DIR )
#else
                  if( VipIsDirectory( name ) )
#endif
                    {
                      if( *(p+1) == '\0' ) /* directory/ */
                        {
                          item = VipInsertString( list, name, item );
                          if( firstitem == NULL )
                            firstitem = item;
                        }
                      else /* directory/sth_else */
                        {
                          strcpy( pname + strlen( dent->d_name ), p );
                          /* recurse */
                          item = VipShellExpandWick( name, list, item );
                          if( firstitem == NULL )
                            firstitem = item;
                          if( list )
                            item = list->last;
                          else
                            while( item->next != NULL )
                              item = item->next;
                        }
                    }
              }

          if( nlen != 0 )
            free( name );
          closedir( dir );
        }

      regfree( &regex );
    }

  free( patexp );
  free( dname );
  return firstitem;
}


VipStringList* VipShellExpand( const char* pattern )
{
  VipStringList	*list = VipCreateStringList();
  VipShellExpandWick( pattern, list, NULL );
  return list;
}


VipStringList* VipListDirectory( const char* path )
{
  VipStringList		*list;
  DIR			*dir;
  struct dirent		*dent;
  VipStringListItem	*item = NULL;

  dir = opendir( path );
  if( dir == NULL )
    return NULL;

  list = VipCreateStringList();
  while( ( dent = readdir( dir ) ) )
    if( strcmp( dent->d_name, "." ) && strcmp( dent->d_name, ".." ) )
      item = VipInsertString( list, dent->d_name, item );

  closedir( dir );
  return list;
}



int VipCopyFile( const char* src, const char* dst, int flags )
{
  FILE			*fi, *fo;
  char			buf[ 10000 ];
  size_t		sz, szo;
  int			code = 0;
  struct stat		s;
  struct utimbuf	tb;

#ifndef _WIN32

  if( flags & VipSymlinks )
    {
      code = readlink( src, buf, 9999 );
      if( code > 0 )
        {
          buf[ code ] = '\0';
          code = symlink( buf, dst );
#ifndef __APPLE_CC__
          /* MacOS X doesn't seem to have the lchown() function */
          if( flags & VipPreserve )
            {
              lstat( src, &s );
              lchown( dst, s.st_uid, s.st_gid );
            }
#endif
          return code;
        }
      code = 0;
    }

#endif

  fi = fopen( src, "rb" );
  if( fi == NULL )
    {
      fprintf( stderr, "VipCopyFile: cannot read %s\n", src );
      return 1;
    }
  fo = fopen( dst, "wb" );
  if( fo == NULL )
    {
      fprintf( stderr, "VipCopyFile: cannot write %s\n", dst );
      code = 2;
    }
  else
    {
      while( !feof( fi ) )
        {
          sz = fread( buf, 1, 10000, fi );
          szo = fwrite( buf, 1, sz, fo );
          if( szo != sz )
            {
              fprintf( stderr, "VipCopyFile: write failed for %s\n", src );
              code = 3;
            }
        }
      fclose( fo );
      if( flags & VipPreserve )
        {
          fstat( fileno( fi ), &s );
          chmod( dst, s.st_mode );
#ifndef _WIN32
          chown( dst, s.st_uid, s.st_gid );
#endif
          tb.actime = s.st_atime;
          tb.modtime = s.st_mtime;
          utime( dst, &tb );
        }
    }

  fclose( fi );
  return code;
}


int VipCopyFileOrDir( const char* src, const char* dst, int flags )
{
  VipStringList		*list;
  VipStringListItem	*item;
  char			*sname, *dname;
  size_t		slen, dlen;
  char			sep;
  int			code = 0;
  struct stat		s;
  struct utimbuf	tb;

  if( !VipIsDirectory( src ) 
      || ( ( flags & VipSymlinks ) && VipIsSymlink( src ) ) )
    return VipCopyFile( src, dst, flags );
  else
    if( flags & VipRecursive )
      {
        VipMkdir( dst );
        list = VipListDirectory( src );
        if( list == NULL )
          fprintf( stderr, "VipCp: cannot read directory %s\n", src );
        else
          {
            slen = strlen( src );
            dlen = strlen( dst );
            sep = VipFileSeparator();
            for( item=list->begin; item; item=item->next )
              {
                sname = malloc( slen + strlen( item->string ) + 2 );
                dname = malloc( dlen + strlen( item->string ) + 2 );
                sprintf( sname, "%s%c%s", src, sep, item->string );
                sprintf( dname, "%s%c%s", dst, sep, item->string );
                code |= VipCopyFileOrDir( sname, dname, flags );
                free( dname );
                free( sname );
              }
            VipFreeStringList( list );
          }
        if( flags & VipPreserve )
          {
            stat( src, &s );
            chmod( dst, s.st_mode );
#ifndef _WIN32
            chown( dst, s.st_uid, s.st_gid );
#endif
            tb.actime = s.st_atime;
            tb.modtime = s.st_mtime;
            utime( dst, &tb );
          }
      }
    else
      printf( "VipCp: omiting directory `%s'\n", src );

  return code;
}


void VipCp( const char* srcpatt, const char* dst, int flags )
{
  VipStringList		*files = VipShellExpand( srcpatt );
  VipStringListItem	*item;
  char			*str;
  size_t		dlen = strlen( dst ) + 1;

  if( !VipIsDirectory( dst ) )
    if( files->size > 1 )
      fprintf( stderr, "VipCp: destination must be a directory\n" );
    else
      VipCopyFileOrDir( files->begin->string, dst, flags );
  else
    for( item=files->begin; item; item=item->next )
      {
        str = malloc( strlen( item->string ) + dlen + 1 );
        sprintf( str, "%s%c", dst, VipFileSeparator() );
        VipBasename( item->string, str + dlen );
        if( VipCopyFileOrDir( item->string, str, flags ) )
          fprintf( stderr, "VipCp: cannot copy %s to %s\n", item->string, 
                   str );
        free( str );
      }

  VipFreeStringList( files );
}


void VipMv( const char* srcpatt, const char* dst )
{
  VipStringList		*files = VipShellExpand( srcpatt );
  VipStringListItem	*item;
  char			*str;
  size_t		dlen = strlen( dst ) + 1;

  if( files == NULL || files->begin == NULL )
    return;
  if( !VipIsDirectory( dst ) )
    if( files->size > 1 )
      fprintf( stderr, "VipMv: destination must be a directory\n" );
    else
      rename( files->begin->string, dst );
  else
    for( item=files->begin; item; item=item->next )
      {
        str = malloc( strlen( item->string ) + dlen + 1 );
        sprintf( str, "%s%c", dst, VipFileSeparator() );
        VipBasename( item->string, str + dlen );
        /* printf( "rename %s %s\n", item->string, str ); */
        if( rename( item->string, str ) )
          fprintf( stderr, "VipMv: cannot move %s to %s\n", item->string, 
                   str );
        free( str );
      }

  VipFreeStringList( files );
}


void VipRm( const char* pattern, int flags )
{
  VipStringList		*files = VipShellExpand( pattern ), *dir;
  VipStringListItem	*item, *ditem;
  char			*name;
  size_t		dlen;
  char			sep = VipFileSeparator();

  for( item=files->begin; item; item=item->next )
    {
      if( ( flags & VipRecursive ) && !VipIsSymlink( item->string ) 
          && VipIsDirectory( item->string ) )
        {
          dlen = strlen( item->string );
          dir = VipListDirectory( item->string );
          if( dir )
            {
              for( ditem=dir->begin; ditem; ditem=ditem->next )
                {
                  name = malloc( dlen + strlen( ditem->string ) + 2 );
                  sprintf( name, "%s%c%s", item->string, sep, ditem->string );
                  VipRm( name, flags );
                  free( name );
                }
            }
        }
      if( VipIsDirectory( item->string ) )
        rmdir( item->string );
      else
        unlink( item->string );
    }

  VipFreeStringList( files );
}


