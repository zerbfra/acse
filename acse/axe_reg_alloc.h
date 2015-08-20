/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_reg_alloc.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _AXE_REG_ALLOC_H
#define _AXE_REG_ALLOC_H

#include "axe_struct.h"
#include "axe_engine.h"
#include "collections.h"
#include "axe_cflow_graph.h"

typedef struct t_live_interval
{
   int varID;     /* a variable identifier */
   int startPoint;  /* the index of the first instruction
                   * that make use of (or define) this variable */
   int endPoint;   /* the index of the last instruction
                   * that make use of (or define) this variable */
}t_live_interval;

typedef struct t_reg_allocator
{
   t_list *live_intervals;    /* an ordered list of live intervals */
   int regNum;                /* the number of registers of the machine */
   int varNum;                /* number of variables */
   int *bindings;             /* an array of bindings of kind : varID-->register.
                               * If a certain variable X need to be spilled
                               * in memory, the value of `register' is set
                               * to the value of the macro RA_SPILL_REQUIRED */
   t_list *freeRegisters;     /* a list of free registers */
}t_reg_allocator;


/* Initialize the internal structure of the register allocator */
extern t_reg_allocator * initializeRegAlloc(t_cflow_Graph *graph);

/* finalize all the data structure associated with the given register allocator */
extern void finalizeRegAlloc(t_reg_allocator *RA);

/* execute the register allocation algorythm (Linear Scan) */
extern int execute_linear_scan(t_reg_allocator *RA);

#endif
