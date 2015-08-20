/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_utils.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <stdio.h>
#include "axe_utils.h"
#include "axe_gencode.h"
#include "symbol_table.h"
#include "axe_labels.h"
#include "axe_cflow_graph.h"
#include "axe_reg_alloc.h"
#include "axe_io_manager.h"
#include "axe_errors.h"

extern int errorcode;
extern int line_num;
extern int num_error;
extern int num_warning;
extern t_program_infos *program;
extern t_cflow_Graph *graph;
extern t_reg_allocator *RA;
extern t_io_infos *file_infos;

static void free_new_variables(t_list *variables)
{
   t_list *current_element;
   t_axe_declaration *current_decl;

   /* preconditions */
   assert(variables != NULL);

   /* initialize the value of `current_element' */
   current_element = variables;
   while(current_element != NULL)
   {
      current_decl = (t_axe_declaration *) LDATA(current_element);
      if (current_decl != NULL)
         _AXE_FREE_FUNCTION(current_decl);

      current_element = LNEXT(current_element);
   }

   /* free the memory associated with the list `variables' */
   freeList(variables);
}

void set_new_variables(t_program_infos *program
            , int varType, t_list *variables)
{
   t_list *current_element;
   t_axe_declaration *current_decl;
   int counter;

   /* preconditions */
   if (program == NULL){
      free_new_variables(variables);
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   }
   
   /* initialize `current_element' */
   current_element = variables;
   counter = 1;

   while (current_element != NULL)
   {
      /* retrieve the current declaration infos */
      current_decl = (t_axe_declaration *) LDATA(current_element);
      if (current_decl == NULL) {
         free_new_variables(variables);
         notifyError(AXE_NULL_DECLARATION);
      }

      /* create and assign a new variable to program */
      createVariable(program, current_decl->ID, varType, current_decl->isArray
            , current_decl->arraySize, current_decl->init_val);

      /* update the value of `current_element' */
      current_element = LNEXT(current_element);
   }

   /* free the linked list */
   /* initialize `current_element' */
   current_element = variables;

   while (current_element != NULL)
   {
      /* retrieve the current declaration infos */
      current_decl = (t_axe_declaration *) LDATA(current_element);

      /* assertion -- must always be verified */
      assert(current_decl != NULL);

      /* add a load instruction for each declared variable
       * that is not an array type */
      if (!(current_decl->isArray))
         get_symbol_location(program, current_decl->ID, 0);

      /* free the memory associated with the current declaration */
      _AXE_FREE_FUNCTION(current_decl);

      /* update the value of `current_element' */
      current_element = LNEXT(current_element);
   }

   freeList(variables);
}

int get_symbol_location(t_program_infos *program, char *ID, int genLoad)
{
   int sy_error;
   int location;

   /* preconditions: ID and program shouldn't be NULL pointer */
   if (ID == NULL)
      notifyError(AXE_VARIABLE_ID_UNSPECIFIED);

   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   
   /* get the location of the symbol with the given ID */
   location = getLocation(program->sy_table, ID, &sy_error);
   if (sy_error != SY_TABLE_OK)
      notifyError(AXE_INVALID_VARIABLE);

   /* verify if the variable was previously loaded into
   * a register. This check is made by looking to the
   * value of `location'.*/
   if (location == REG_INVALID)
   {
      /* we have to load the variable with the given
      * identifier (ID) */
      t_axe_label *label;
      int sy_errorcode;

      /* retrieve the label associated with ID */
      label = getLabelFromVariableID(program, ID);

      /* fetch an unused register where to store the
       * result of the load instruction */
      location = getNewRegister(program);

      /* assertions */
      assert(location != REG_INVALID);
      assert(  (label != NULL)
               && (label->labelID != LABEL_UNSPECIFIED) );
      
      /* load the value of IDENTIFIER from the
       * given label to a register */
      if (genLoad)
         gen_load_instruction(program, location, label, 0);

      /* update the symbol table */
      sy_errorcode = setLocation(program->sy_table, ID, location);

      if (sy_errorcode != SY_TABLE_OK)
         notifyError(AXE_SY_TABLE_ERROR);

#ifndef NDEBUG
      /* get the location of the symbol with the given ID */
      location = getLocation(program->sy_table, ID, &sy_error);
#endif
   }

   /* test the postconditions */
   assert(location != REG_INVALID);

   return location;
}

int gen_load_immediate(t_program_infos *program, int immediate)
{
   int imm_register;

   imm_register = getNewRegister(program);

   /* load the immediate into a register */
   gen_addi_instruction(program, imm_register, REG_0, immediate);

   return imm_register;
}

void set_end_Program(t_program_infos *program)
{
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
      
   if (isAssignedLabel(program->lmanager))
   {
      gen_halt_instruction(program);
      return;
   }

   if (program->instructions != NULL)
   {
      t_axe_instruction *last_instr;
      t_list *last_element;

      /* get the last element of the list */
      last_element = getLastElement(program->instructions);
      assert(last_element != NULL);

      /* retrieve the last instruction */
      last_instr = (t_axe_instruction *) LDATA(last_element);
      assert(last_instr != NULL);

      if (last_instr->opcode == HALT)
         return;
   }

   gen_halt_instruction(program);
   return;
}

void shutdownCompiler(int exitStatus)
{
#ifndef NDEBUG
   fprintf(stdout, "Finalizing the compiler data structures.. \n");
#endif
   
   /* shutdown the asm engine */
   finalizeProgramInfos(program);
   /* finalize the control flow graph informations */
   finalizeGraph(graph);
   /* finalize the register allocator */
   finalizeRegAlloc(RA);
   /* close all the files used by the compiler */
   finalizeOutputInfos(file_infos);
   
#ifndef NDEBUG
   fprintf(stdout, "Done. \n");
#endif
   
   exit(exitStatus);
}

void init_compiler(int argc, char **argv)
{

#ifndef NDEBUG
   fprintf(stdout, "Starting the compilation process. \n\n");
#endif

   /* initialize all the global variables */
   errorcode = AXE_OK;
   line_num = -1;
   num_error = 0;
   num_warning = 0;
   program = NULL;
   graph = NULL;
   RA = NULL;
   file_infos = NULL;

#ifndef NDEBUG
   fprintf(stdout, "Initialize the compiler internal data structures. \n");
#endif
   
   /* initialize all the files used by the compiler */
   file_infos = initializeOutputInfos(argc, argv);
   if (file_infos == NULL)
      errorcode = AXE_OUT_OF_MEMORY;

   /* initialize the translation infos */
   program = allocProgramInfos(&errorcode);
   
   /* initialize the line number */
   line_num = 1;

   checkConsistency();
}
