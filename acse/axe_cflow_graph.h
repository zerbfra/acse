/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_cflow_graph.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _AXE_CFLOW_GRAPH_H
#define _AXE_CFLOW_GRAPH_H

#include <stdio.h>
#include "axe_struct.h"
#include "collections.h"

extern int cflow_errorcode;


/* a variable of the intermediate code */
typedef struct t_cflow_var
{
   int ID;   /* variable identifier */
} t_cflow_var;

/* A Node exists only in a basic block. It defines a list of
 * def-uses and it is associated with a specific instruction
 * inside the code */
typedef struct t_cflow_Node
{
   t_cflow_var *def;        /* A variable that is defined by this node */
   t_cflow_var *uses[3];    /* set of variables that will be used by this node */
   t_axe_instruction *instr;  /* a pointer to the instruction associated
                               * with this node */
   t_list *in;             /* variables that are live-in the current node */
   t_list *out;            /* variables that are live-out the current node */
} t_cflow_Node;

/* an ordered list of nodes with only one predecessor and one successor */
typedef struct t_basic_block
{
   t_list *pred;  /* predecessors : a list of basic blocks */
   t_list *succ;  /* successors : a list of basic blocks */
   t_list *nodes; /* an ordered list of instructions */
} t_basic_block;

/* a control flow graph */
typedef struct t_cflow_Graph
{
   t_basic_block *startingBlock; /* the starting basic block of code */
   t_basic_block *endingBlock;   /* the last block of the graph */
   t_list *blocks;               /* an ordered list of all the basic blocks */
   t_list *cflow_variables;      /* a list of all the variable identifiers */
} t_cflow_Graph;


/* functions that are used in order to allocate/deallocate instances
 * of basic blocks, instruction nodes and flow graphs */
extern t_cflow_Node * allocNode
      (t_cflow_Graph *graph, t_axe_instruction *instr);
extern void finalizeNode(t_cflow_Node *node);
extern t_basic_block * allocBasicBlock();
extern void finalizeBasicBlock(t_basic_block *block);
extern t_cflow_Graph * allocGraph();
extern void finalizeGraph(t_cflow_Graph *graph);

/* working with basic blocks */
extern void setPred(t_basic_block *block, t_basic_block *pred);
extern void setSucc(t_basic_block *block, t_basic_block *succ);
extern void insertNode(t_basic_block *block, t_cflow_Node *node);
extern void insertNodeBefore(t_basic_block *block
      , t_cflow_Node *before_node, t_cflow_Node *new_node);
extern void insertNodeAfter(t_basic_block *block
      , t_cflow_Node *after_node, t_cflow_Node *new_node);
extern t_list * getLiveINVars(t_basic_block *bblock);
extern t_list * getLiveOUTVars(t_basic_block *bblock);

/* working with the control flow graph */
extern void insertBlock(t_cflow_Graph *graph, t_basic_block *block);
extern t_cflow_Graph * createFlowGraph(t_list *instructions);

/* dataflow analysis */
extern void performLivenessAnalysis(t_cflow_Graph *graph);


#endif
