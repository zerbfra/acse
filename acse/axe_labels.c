/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_labels.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "axe_errors.h"
#include "axe_labels.h"
#include "collections.h"


struct t_axe_label_manager
{
   t_list *labels;
   int current_label_ID;
   t_axe_label *label_to_assign;
};

int isAssignedLabel(t_axe_label_manager *lmanager)
{
   /* preconditions: lmanager must be different from NULL */
   if (lmanager == NULL)
      notifyError(AXE_INVALID_LABEL_MANAGER);

   if (  (lmanager->label_to_assign != NULL)
         && ((lmanager->label_to_assign)->labelID != LABEL_UNSPECIFIED) )
   {
      return 1;
   }

   return 0;
}

int compareLabels(t_axe_label *labelA, t_axe_label *labelB)
{
   if ( (labelA == NULL) || (labelB == NULL) )
      return 0;

   if (labelA->labelID == labelB->labelID)
      return 1;
   return 0;
}

/* reserve a new label identifier and return the identifier to the caller */
t_axe_label * newLabelID(t_axe_label_manager *lmanager)
{
   t_axe_label *result;

   /* preconditions: lmanager must be different from NULL */
   assert(lmanager != NULL);
   
   /* initialize a new label */
   result = alloc_label(lmanager->current_label_ID);

   /* update the value of `current_label_ID' */
   lmanager->current_label_ID++;
   
   /* tests if an out of memory occurred */
   if (result == NULL)
      return NULL;

   /* add the new label to the list of labels */
   lmanager->labels = addElement(lmanager->labels, result, -1);

   /* return the new label */
   return result;
}

/* assign the given label identifier to the next instruction. Returns
 * NULL if an error occurred; otherwise the assigned label */
t_axe_label * assignLabelID(t_axe_label_manager *lmanager, t_axe_label *label)
{
   /* precondition: lmanager must be different from NULL */
   assert(lmanager != NULL);

   /* precondition: label must be different from NULL and
    * must always carry a valid identifier */
   if (  (label == NULL)
         || (label->labelID == LABEL_UNSPECIFIED)
         || (label->labelID >= lmanager->current_label_ID))
   {
      notifyError(AXE_INVALID_LABEL);
   }

   /* test if the next instruction has already a label */
   if (  (lmanager->label_to_assign != NULL)
         && ((lmanager->label_to_assign)->labelID != LABEL_UNSPECIFIED) )
   {
      label->labelID = (lmanager->label_to_assign)->labelID;
   }
   else
      lmanager->label_to_assign = label;

   /* all went good */
   return label;
}

/* initialize the memory structures for the label manager */
t_axe_label_manager * initialize_label_manager()
{
   t_axe_label_manager *result;

   /* create an instance of `t_axe_label_manager' */
   result = (t_axe_label_manager *)
         _AXE_ALLOC_FUNCTION (sizeof(t_axe_label_manager));

   if (result == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* initialize the new instance */
   result->labels = NULL;
   result->current_label_ID = 0;
   result->label_to_assign = NULL;

   return result;
}

/* finalize an instance of `t_axe_label_manager' */
void finalize_label_manager(t_axe_label_manager *lmanager)
{
   t_list *current_element;
   t_axe_label *current_label;
   
   /* preconditions */
   if (lmanager == NULL)
      return;

   /* initialize `current_element' to the head of the list
    * of labels */
   current_element = lmanager->labels;

   while (current_element != NULL)
   {
      /* retrieve the current label */
      current_label = (t_axe_label *) LDATA(current_element);
      assert(current_label != NULL);

      /* free the memory associated with the current label */
      _AXE_FREE_FUNCTION(current_label);

      /* fetch the next label */
      current_element = LNEXT(current_element);
   }

   /* free the memory associated to the list of labels */
   freeList(lmanager->labels);

   _AXE_FREE_FUNCTION(lmanager);
}

t_axe_label * assign_label(t_axe_label_manager *lmanager)
{
   t_axe_label *result;
   
   /* precondition: lmanager must be different from NULL */
   if (lmanager == NULL)
      notifyError(AXE_INVALID_LABEL_MANAGER);

   /* the label that must be returned (can be a NULL pointer) */
   result = lmanager->label_to_assign;

   /* update the value of `lmanager->label_to_assign' */
   lmanager->label_to_assign = NULL;

   /* return the label */
   return result;
}

int get_number_of_labels(t_axe_label_manager *lmanager)
{
   if (lmanager == NULL)
      return 0;

   if (lmanager->labels == NULL)
      return 0;

   /* postconditions */
   return getLength(lmanager->labels);
}
