/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * cflow_constants.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _CFLOW_CONSTANTS_H
#define _CFLOW_CONSTANTS_H

#define VAR_UNDEFINED -1

/* if this macro is set to 1, the control flow analysis will consider
 * R0 always as a LIVE IN temporary register (i.e. variable) */
#define CFLOW_ALWAYS_LIVEIN_R0 1

/* errorcodes */
#define CFLOW_OK 0
#define CFLOW_GRAPH_UNDEFINED 1
#define CFLOW_INVALID_INSTRUCTION 2
#define CFLOW_INVALID_NODE 3
#define CFLOW_BBLOCK_UNDEFINED 4
#define CFLOW_INVALID_BBLOCK 5
#define CFLOW_INVALID_LABEL_FOUND 6
#define CFLOW_NODE_UNDEFINED 7
#define CFLOW_NODE_ALREADY_INSERTED 8
#define CFLOW_BBLOCK_ALREADY_INSERTED 9
#define CFLOW_INVALID_OPERATION 10
#define CFLOW_INVALID_PROGRAM_INFO 11
#define CFLOW_OUT_OF_MEMORY 12
#define CFLOW_UNKNOWN ERROR -1

#endif
