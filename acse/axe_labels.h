/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_labels.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _AXE_LABELS_H
#define _AXE_LABELS_H

#include "axe_struct.h"


struct t_axe_label_manager;

/* Typedef for the struct t_axe_label_manager */
typedef struct t_axe_label_manager t_axe_label_manager;

/* reserve a new label identifier and return the identifier to the caller */
extern t_axe_label * newLabelID(t_axe_label_manager *lmanager);

/* assign the given label identifier to the next instruction. Returns
 * FALSE if an error occurred; otherwise true */
extern t_axe_label * assignLabelID(t_axe_label_manager *lmanager, t_axe_label *label);

/* initialize the memory structures for the label manager */
extern t_axe_label_manager * initialize_label_manager();

/* retrieve the label that will be assigned to the next instruction */
extern t_axe_label * assign_label(t_axe_label_manager *lmanager);

/* finalize an instance of `t_axe_label_manager' */
extern void finalize_label_manager(t_axe_label_manager *lmanager);

/* get the number of labels inside the list of labels */
extern int get_number_of_labels(t_axe_label_manager *lmanager);

/* return TRUE if the two labels hold the same identifier */
extern int compareLabels(t_axe_label *labelA, t_axe_label *labelB);

/* test if a label will be assigned to the next instruction */
extern int isAssignedLabel(t_axe_label_manager *lmanager);

#endif
