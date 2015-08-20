/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * collections.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <stdlib.h>
#include <assert.h>
#include "collections.h"

/* function prototypes */
static t_list * newElement(void *data);


/* remove the first element of the list. Returns the new
 * head of the list */
t_list * removeFirst(t_list *list)
{
   t_list *first_elem;
   
   if (list == NULL)
      return NULL;

   first_elem = list;
   list = LNEXT(list);
   if (list != NULL)
      SET_PREV(list, NULL);
   
   _FREE_FUNCTION(first_elem);

   /* postconditions: return the new head of the list */
   return list;
}

/* add an element `data' to the list `list' at position `pos'. If pos is negative
 * , or is larger than the number of elements in the list, the new element is
 * added on to the end of the list. Function `addElement' returns a pointer
 * to the new head of the list */
t_list * addElement(t_list *list, void * data, int pos)
{
	t_list *result;
	t_list *current_elem;
	t_list *last_elem;
	int	current_pos;
	
	/* initialize the value of `result' */
	result = newElement(data);

	if (list == NULL)
		return result;
	
	if (pos == 0)
	{
		/* update the control informations */
		SET_NEXT(result, list);
		SET_PREV(list, result);
		
		/* return the new head of the list */
		return result;
	}
	
	/* retrieve the last element of the list */
	last_elem = getLastElement(list);
	assert(last_elem != NULL);
	
	if (pos < 0)
	{
		/* update the control informations */
		SET_NEXT(last_elem, result);
		SET_PREV(result, last_elem);
		
		/* update the value of result */
		return list;
	}
	
	/* `pos' is a positive integer */
	current_pos = 0;
	current_elem = list;
	
	while(current_pos < pos)
	{
		if (current_elem == last_elem)
		{
			/* update the control informations */
			SET_NEXT(last_elem, result);
			SET_PREV(result, last_elem);
			
			/* update the value of result */
			return list;
		}
		
		/* update the loop informations */
		current_elem = LNEXT(current_elem);
		current_pos++;
	}

	/* assertions */
	assert(current_elem != NULL);
	
	/* update the control informations */
	SET_NEXT(result, current_elem);
	SET_PREV(result, LPREV(current_elem));
	SET_NEXT(LPREV(current_elem), result);
	SET_PREV(current_elem, result);
	
	/* return the new head of the list */
	return list;
}

/* add an element at the beginning of the list */
t_list * addFirst(t_list *list, void * data)
{
	t_list *result;
	
	/* initialize the value of `result' */
	result = newElement(data);
	
	if (list == NULL)
		return result;
	
	/* postconditions */
	SET_PREV(list, result);
	SET_NEXT(result, list);
	
	/* return the new head of the list */
	return result;
}

/* add an element to the end of the list */
t_list * addLast(t_list *list, void * data)
{
	/* call the `addElement' */
	return addElement(list, data, -1);
}

/* remove an element from the list */
t_list * removeElement(t_list *list, void * data)
{
	t_list *current_elem;
	
	/* preconditions: the list shouldn't be empty */
	if (list == NULL)
		return NULL;
	
	/* intialize the value of `current_elem' */
	current_elem = list;
	while (  (current_elem != NULL)
			   && (LDATA(current_elem) != data))
	{
		current_elem = LNEXT(current_elem);
	}
	
	/* the value hasn't been found */
	if (current_elem == NULL)
		return list;
	
	/* the value is found */
	if (LPREV(current_elem) != NULL)
	{
		SET_NEXT(LPREV(current_elem), LNEXT(current_elem));
		if (LNEXT(current_elem) != NULL)
			SET_PREV(LNEXT(current_elem), LPREV(current_elem));
		
		_FREE_FUNCTION(current_elem);
	}
	else
	{
		/* check the preconditions */
		assert(list == current_elem);
		
		if (LNEXT(current_elem) != NULL)
      {
			SET_PREV(LNEXT(current_elem), NULL);
			
         /* update the new head of the list */
		   list = LNEXT(current_elem);
      }
      else
         list = NULL;
		
		_FREE_FUNCTION(current_elem);
	}
	
	/* postconditions: return the new head of the list */
	return list;
}

/* remove all the elements of a list */
void freeList(t_list *list)
{
	/* verify the preconditions */
	if (list == NULL)
		return;

	/* recursively call the freeList */
	freeList(LNEXT(list));
	
	/* deallocate memory for the current element of the list */
	_FREE_FUNCTION(list);
}

t_list * newElement(void *data)
{
	t_list * result;
	
	/* create an instance of t_list in memory */
	result = (t_list *) _ALLOC_FUNCTION(sizeof(t_list));

   /* verify the out of memory condition */
   if (result == NULL)
   {
      fprintf(stderr, "COLLECTIONS.C:: _ALLOC_FUNCTION returned a NULL pointer \n");
      abort();
   }

	/* set the internal value of the just created t_list element */
	SET_DATA(result, data);
	SET_PREV(result, NULL);
	SET_NEXT(result, NULL);
	
	/* postconditions : return the element */
	return result;
}

t_list * getLastElement(t_list *list)
{
	/* preconditions */
	if (list == NULL)
		return NULL;
	
	/* test if the current element is the last element of the list */
	if (LNEXT(list) == NULL)
		return list;
	
	/* recursively call the getLastElement on the next item of the list */
	return getLastElement(LNEXT(list));
}

/* remove a link from the list `list' */
extern t_list * removeElementLink(t_list *list, t_list *element)
{
	t_list *current_elem;
	
	/* preconditions */
	if (list == NULL || element == NULL)
		return list;
	
	if ((LPREV(element) == NULL) && (element != list))
		return list;
	
	/* intialize the value of `current_elem' */
	current_elem = list;
	while(	(LDATA(current_elem) != LDATA(element))
			|| (LPREV(current_elem) != LPREV(element))
			|| (LNEXT(current_elem) != LNEXT(element)) )
	{
		/* retrieve the next element from the list */
		current_elem = LNEXT(current_elem);
		
		/* test if we reached the end of the list */
		if (current_elem == NULL)
			return list;
	}

   if (LPREV(element) == NULL)
   {
      assert(list == element);
      
      if (LNEXT(element) != NULL)
      {
         list = LNEXT(element);
         SET_PREV(LNEXT(element), NULL);
      }
      else
         list = NULL;

      /* remove the allocated memory of element */
      _FREE_FUNCTION(element);
      return list;
   }
   
	/* we found the element */
	if (LPREV(element) != NULL)
	{
		/* we found the element, and it is the top of the list */
		SET_NEXT(LPREV(element), LNEXT(element));
	}
	
	if (LNEXT(element) != NULL)
		SET_PREV(LNEXT(element), LPREV(element));
	
	/* remove the allocated memory of element */
	_FREE_FUNCTION(element);
	
	/* return the new top of the list */
	return list;
}

/* find an element inside the list `list'. The current implementation calls the
 * CustomfindElement' passing a NULL reference as `func' */
t_list * findElement(t_list *list, void *data)
{
	t_list *current_elem;
	
	/* if the list is empty returns a NULL pointer */
	if (list == NULL)
		return NULL;

	/* intialize the value of `current_elem' */
	current_elem = list;
	while (	(current_elem != NULL)
			&& (	LDATA(current_elem) != data) )
	{
		current_elem = LNEXT(current_elem);
	}
	
	/* postconditions */
	return current_elem;
}

/* find an element inside the list `list'. */
t_list * CustomfindElement(t_list *list, void *data
		, int (*compareFunc)(void *a, void *b))
{
	t_list *current_elem;	
	
	/* preconditions */
	if (compareFunc == NULL)
		return findElement(list, data);
	
	if (list == NULL)
		return NULL;
	
	/* intialize the value of `current_elem' */
	current_elem = list;
	while (current_elem != NULL)
	{
      void *other_Data;

      other_Data = LDATA(current_elem);

      if (compareFunc(other_Data, data))
         break;
      
		current_elem = LNEXT(current_elem);
	}	

	/* postconditions */
	return current_elem;
}

int getPosition(t_list *list, t_list *element)
{
	int counter;
	
	/* preconditions */
	if (list == NULL || element == NULL)
		return -1;
	
	/* initialize the local variable `counter' */
	counter = 0;
	
	if (list == element)
		return counter;
	
	/* update values */
	counter++;
	list = LNEXT(list);
	
	while (list != NULL)
	{
		if (list == element)
			return counter;
		
		counter++;
		list = LNEXT(list);
	}
	
	return -1;
}

int getLength(t_list *list)
{
	int counter;
	
   /* initialize the local variable `counter' */
   counter = 0;

   /* precondition */
	if (list == NULL)
		return counter;
	
	while (list != NULL)
	{
		counter++;
		list = LNEXT(list);
	}
	
	/* postconditions: return the length of the list */
	return counter;
}

t_list * cloneList(t_list *list)
{
   t_list *result;
   t_list *current_element;

   /* initialize the local variables */
   result = NULL;
   current_element = list;

   /* preconditions */
   if (current_element == NULL)
      return result;

   while(current_element != NULL)
   {
      /* add an element to the new list */
      result = addElement(result, LDATA(current_element), -1);

      /* retrieve the next element of the list */
      current_element = LNEXT(current_element);
   }

   /* return the new list */
   return result;
}

t_list * getElementAt(t_list *list, unsigned int position)
{
   t_list *result;
   t_list *current_element;
   unsigned int current_pos;
   
   if (list == NULL)
      return NULL;

   /* initialize the local variables */
   result = NULL;
   current_element = list;
   current_pos = 0;
   while((current_element != NULL) && (current_pos < position))
   {
      current_element = LNEXT(current_element);
      current_pos++;
   }

   /* return the element at the requested position */
   return current_element;
}

t_list * addList(t_list *list, t_list *elements)
{
   t_list *current_element;
   void *current_data;

   /* if the list of elements is null, this function
    * will return `list' unmodified */
   if (elements == NULL)
      return list;

   /* initialize the value of `current_element' */
   current_element = elements;
   while (current_element != NULL)
   {
      /* retrieve the data associated with the current element */
      current_data = LDATA(current_element);
      list = addElement(list, current_data, -1);

      /* retrieve the next element in the list */
      current_element = LNEXT(current_element);
   }

   /* return the new list */
   return list;
}

t_list * addListToSet(t_list *list, t_list *elements
      , int (*compareFunc)(void *a, void *b), int *modified)
{
   t_list *current_element;
   void *current_data;

   /* if the list of elements is NULL returns the current list */
   if (elements == NULL)
      return list;

   /* initialize the value of `current_element' */
   current_element = elements;
   while (current_element != NULL)
   {
      /* retrieve the data associated with the current element */
      current_data = LDATA(current_element);

      /* Test if the element was already inserted. */
      if (CustomfindElement(list, current_data, compareFunc) == NULL)
      {
         list = addElement(list, current_data, -1);
         if (modified != NULL)
            (* modified) = 1;
      }

      /* retrieve the next element in the list */
      current_element = LNEXT(current_element);
   }

   /* return the new list */
   return list;
}

/* add sorted */
t_list * addSorted(t_list *list, void * data
            , int (*compareFunc)(void *a, void *b))
{
   t_list *current_element;
   void *current_data;
   int counter;
   
   /* preconditions */
   if (list == NULL)
      return addFirst(list, data);

   counter = 0;
   current_element = list;
   while(current_element != NULL)
   {
      /* get the current interval informations */
      current_data = LDATA(current_element);
      assert(current_data != NULL);

      if (compareFunc(current_data, data) >= 0)
      {
         list = addElement(list, data, counter);
         return list;
      }
         
      /* retrieve the next element */
      current_element = LNEXT(current_element);

      /* update the value of counter */
      counter++;
   }

   return addElement(list, data, -1);
}
