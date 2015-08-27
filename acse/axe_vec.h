/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_array.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _AXE_EXPRESSION_H
#define _AXE_EXPRESSION_H

#include "axe_engine.h"
#include "axe_array.h"
#include "axe_gencode.h"
#include "symbol_table.h"
#include "axe_utils.h"
#include "axe_errors.h"
#include "axe_constants.h"

/* Genera l'istruzione per la somma e la sottrazione tra array */
extern void handle_vec_op(t_program_infos *program, char *destID, char *sourceID1, char *sourceID2, int vec_op);

#endif
