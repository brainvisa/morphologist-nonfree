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

#include <vip/util/strlist.h>
#include <stdlib.h>

VipStringList* VipCreateStringList()
{
  VipStringList	*sl = (VipStringList *) malloc( sizeof( VipStringList ) );
  sl->size = 0;
  sl->begin = 0;
  sl->last = 0;
  return sl;
}


void VipFreeStringList( VipStringList* strl )
{
  VipStringListItem	*item, *item2;
  for( item=strl->begin; item; item=item2 )
    {
      item2 = item->next;
      free( item->string );
      free( item );
    }
  free( strl );
}


void VipFreeStringWick( VipStringListItem* item )
{
  VipStringListItem	*item2;

  while( item != NULL )
    {
      item2 = item->next;
      free( item->string );
      free( item );
    }
}

/* #include <stdio.h> */
VipStringListItem* VipInsertString( VipStringList* strlist, 
                                    const char* string, 
                                    VipStringListItem* after )
{
  VipStringListItem	*item;

  /* printf( "insertString: %s in %p after %p\n", string, strlist, after ); */

  item = (VipStringListItem *) malloc( sizeof( VipStringListItem ) );
  if( after != NULL )
    {
      item->next = after->next;
      after->next = item;
    }
  else
    {
      if( strlist != NULL )
        {
          item->next = strlist->begin;
          strlist->begin = item;
        }
      else
        item->next = NULL;
    }
  item->string = malloc( strlen( string ) + 1 );
  strcpy( item->string, string );

  if( strlist != NULL )
    {
      ++strlist->size;
      if( strlist->last == after )
        strlist->last = item;
    }

  return item;
}


void VipEraseStringElement( VipStringList* strlist, VipStringListItem* before )
{
  VipStringListItem	*item;

  if( before == NULL )
    {
      item = strlist->begin;
      strlist->begin = item->next;
    }
  else
    {
      item = before->next;
      before->next = item->next;
    }

  if( strlist )
    {
      --strlist->size;
      if( strlist->last == item )
        strlist->last = before;
    }

  free( item->string );
  free( item );
}


