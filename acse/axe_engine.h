/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_engine.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _AXE_ENGINE_H
#define _AXE_ENGINE_H

#include "axe_struct.h"
#include "axe_labels.h"
#include "collections.h"
#include "symbol_table.h"

typedef struct t_program_infos
{
  t_list *variables;
  t_list *instructions;
  t_list *data;
  t_axe_label_manager *lmanager;
  t_symbol_table *sy_table;
  int current_register;
} t_program_infos;


/* initialize the informations associated with the program. This function is
 * called at the beginning of the translation process. This function
 * is called once: its only purpouse is to initialize an instance of the struct
 * `t_program_infos' that will contain all the informations about the program
 * that will be compiled */
extern t_program_infos * allocProgramInfos();

/* add a new instruction to the current program. This function is directly
 * called by all the functions defined in `axe_gencode.h' */
extern void addInstruction(t_program_infos *program, t_axe_instruction *instr);

/* reserve a new label identifier and return the identifier to the caller */
extern t_axe_label * newLabel(t_program_infos *program);

/* assign the given label identifier to the next instruction. Returns
 * the label assigned; otherwise (an error occurred) LABEL_UNSPECIFIED */
extern t_axe_label * assignLabel(t_program_infos *program, t_axe_label *label);

/* reserve and fix a new label. It returns either the label assigned or the
 * value LABEL_UNSPECIFIED if an error occurred */
extern t_axe_label * assignNewLabel(t_program_infos *program);

/* add a variable to the program */
extern void createVariable(t_program_infos *program
      , char *ID, int type, int isArray, int arraySize, int init_val);

/* get a previously allocated variable */
extern t_axe_variable * getVariable
      (t_program_infos *program, char *ID);

/* get the label that marks the starting address of the variable
 * with name "ID" */
extern t_axe_label * getLabelFromVariableID
            (t_program_infos *program, char *ID);

/* get a register still not used. This function returns
 * the ID of the register found*/
extern int getNewRegister(t_program_infos *program);

/* finalize all the data structures associated with `program' */
extern void finalizeProgramInfos(t_program_infos *program);

/* write the corresponding assembly for the given program */
extern void writeAssembly(t_program_infos *program, char *output_file);

#endif
