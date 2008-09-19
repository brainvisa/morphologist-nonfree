/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME :                      * TYPE     : Header
 * AUTHOR      :                      * CREATION : 
 * VERSION     :                      * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Linux
 ****************************************************************************
 *
 * DESCRIPTION : 
 *
 ****************************************************************************
 *
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#ifndef VIP_UTIL_STRLIST_H
#define VIP_UTIL_STRLIST_H

#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /* A general list class
     VipStringList is a list of string, simply chained. 
     VipStringListItem is both a storage element and an iterator. It can also 
     be a whole "wick": a list with no size and starting point where you can 
     only go forward.
  */

  typedef struct VipStringListItem VipStringListItem;

  struct VipStringListItem
  {
    char		*string;
    VipStringListItem	*next;
  };

  typedef struct VipStringList VipStringList;

  struct VipStringList
  {
    size_t		size;
    VipStringListItem	*begin;
    VipStringListItem	*last;
  };

  VipStringList* VipCreateStringList();
  void VipFreeStringList( VipStringList* strl );
  /* free a "wick": list without a VipStringList structure */
  void VipFreeStringWick( VipStringListItem* first );
  /* creates a list item and inserts it in a list or wick (list may be NULL). 
     The string is copied. */
  VipStringListItem* VipInsertString( VipStringList* strlist, 
                                      const char* string, 
                                      VipStringListItem* after );
  /* Removes from the list (and frees) the element AFTER the "before" 
     argument. If before is NULL, the first element is erased */
  void VipEraseStringElement( VipStringList* strlist, 
                              VipStringListItem* before );

#ifdef __cplusplus
}
#endif

#endif

