/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_debug.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _AXE_DEBUG_H
#define _AXE_DEBUG_H

#include <stdio.h>
#include "axe_struct.h"
#include "axe_cflow_graph.h"
#include "axe_reg_alloc.h"

/* a debug routine used to print on a specific output file the string
 * representation of an instruction */
extern void debug_printInstruction(t_axe_instruction *instr, FILE *fout);

/* print debug informations about the control flow graph */
extern void printGraphInfos(t_cflow_Graph *graph, FILE *fout, int verbose);

/* print debug informations about register allocation infos */
extern void printRegAllocInfos(t_reg_allocator *RA, FILE *fout);

/* returns a string that represent the codedType given as input */
extern char * dataTypeToString(int codedType);

#endif
