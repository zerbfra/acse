/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_utils.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _AXE_UTILS_H
#define _AXE_UTILS_H

#include "axe_engine.h"
#include "axe_struct.h"
#include "axe_constants.h"
#include "collections.h"

/* create a variable for each `t_axe_declaration' inside
 * the list `variables'. Each new variable will be of type
 * `varType'. */
extern void set_new_variables(t_program_infos *program
      , int varType, t_list *variables);

/* Given a variable/symbol identifier (ID) this function
 * returns a register location where the value is stored
 * (the value of the variable identified by `ID').
 * If the variable/symbol has never been loaded from memory
 * to a register, first this function searches
 * for a free register, then it assign the variable with the given
 * ID to the register just found.
 * Once computed, the location (a register identifier) is returned
 * as output to the caller.
 * This function generates a LOAD instruction
 * only if the flag `genLoad' is set to 1; otherwise it simply reserve
 * a register location for a new variable in the symbol table.
 * If an error occurs, get_symbol_location returns a REG_INVALID errorcode */
extern int get_symbol_location(t_program_infos *program
         , char *ID, int genLoad);

/* Generate the instruction to load an `immediate' value into a new register.
 * It returns the new register identifier or REG_INVALID if an error occurs */
extern int gen_load_immediate(t_program_infos *program, int immediate);

/* Notify the end of the program. This function is directly called
 * from the parser when the parsing process is ended */
extern void set_end_Program(t_program_infos *program);

/* Once called, this function destroys all the data structures
 * associated with the compiler (program, RA, etc.). This function
 * is typically automatically called before exiting from the main
 * or when the compiler encounters some error. */
extern void shutdownCompiler();

/* Once called, this function initialize all the data structures
 * associated with the compiler (program, RA etc..) and all the
 * global variables in the system. This function
 * is typically automatically called at the beginning of the main
 * and should NEVER be called from the user code */
extern void init_compiler(int argc, char **argv);

#endif
