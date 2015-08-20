/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_transform.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _AXE_TRANSFORM_H
#define _AXE_TRANSFORM_H

#include "axe_engine.h"
#include "axe_cflow_graph.h"
#include "axe_reg_alloc.h"

/* once executed the liveness analysis, by calling this function
 * we will obtain a graph with the correct load and store
 * instructions */
extern t_cflow_Graph * insertLoadAndStoreInstr
         (t_program_infos *program, t_cflow_Graph *graph);

/* update the instructions within the control flow graph with the
 * informations retrieved from the register allocation */
extern void updateProgramInfos(t_program_infos *program
         , t_cflow_Graph *graph, t_reg_allocator *RA);
         
#endif
