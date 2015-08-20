/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * collections.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _COLLECTIONS_H
#define _COLLECTIONS_H

#include <stdlib.h>
#include <stdio.h>
#include <malloc/malloc.h>
#include <string.h>

/* macros */
#define LNEXT(item) ((item)->next)
#define LPREV(item) ((item)->prev)
#define LDATA(item) ((item)->data)
#define SET_DATA(item, _data) ((item)->data = (_data))
#define SET_NEXT(item, _next) ((item)->next = (_next))
#define SET_PREV(item, _prev) ((item)->prev = (_prev))
#ifndef _ALLOC_FUNCTION
#	define _ALLOC_FUNCTION malloc
#endif
#ifndef _FREE_FUNCTION
#	define _FREE_FUNCTION free
#endif

/* a list element */
typedef struct t_list
{
	void	*data;
	struct t_list *next;
	struct t_list *prev;
}t_list;


/* add an element `data' to the list `list' at position `pos'. If pos is negative
 * , or is larger than the number of elements in the list, the new element is
 * added on to the end of the list. Function `addElement' returns a pointer
 * to the new head of the list */
extern t_list * addElement(t_list *list, void * data, int pos);

/* add sorted */
extern t_list * addSorted(t_list *list, void * data
            , int (*compareFunc)(void *a, void *b));

/* add an element to the end of the list */
extern t_list * addLast(t_list *list, void * data);

/* add an element at the beginning of the list */
extern t_list * addFirst(t_list *list, void * data);

/* remove an element at the beginning of the list */
extern t_list * removeFirst(t_list *list);

/* remove an element from the list */
extern t_list * removeElement(t_list *list, void * data);

/* remove a link from the list `list' */
extern t_list * removeElementLink(t_list *list, t_list *element);

/* find an element inside the list `list'. The current implementation calls the
 * CustomfindElement' passing a NULL reference as `func' */
extern t_list * findElement(t_list *list, void *data);

/* find an element inside the list `list'. */
extern t_list * CustomfindElement(t_list *list, void *data
            , int (*compareFunc)(void *a, void *b));

/* find the position of an `element' inside the `list'. -1 if not found */
extern int getPosition(t_list *list, t_list *element);

/* find the length of `list' */
extern int getLength(t_list *list);

/* remove all the elements of a list */
extern void freeList(t_list *list);

/* get the last element of the list. Returns NULL if the list is empty
 * or list is a NULL pointer */
extern t_list * getLastElement(t_list *list);

/* retrieve the list element at position `position' inside the `list'.
 * Returns NULL if: the list is empty, the list is a NULL pointer or
 * the list holds less than `position' elements. */
extern t_list * getElementAt(t_list *list, unsigned int position);

/* create a new list with the same elements */
extern t_list * cloneList(t_list *list);

/* add a list of elements to another list */
extern t_list * addList(t_list *list, t_list *elements);

/* add a list of elements to a set */
extern t_list * addListToSet(t_list *list, t_list *elements
      , int (*compareFunc)(void *a, void *b), int *modified);


#endif
