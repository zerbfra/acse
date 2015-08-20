/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_io_manager.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _AXE_IO_MANAGER_H
#define _AXE_IO_MANAGER_H

#include <stdio.h>
#include "axe_struct.h"

extern FILE *yyin;

typedef struct t_io_infos
{
   char *output_file_name;
   FILE *input_file;
#ifndef NDEBUG
   FILE *cfg_1;
   FILE *cfg_2;
   FILE *reg_alloc_output;
   FILE *syTable_output;
#endif
}t_io_infos;


extern void finalizeOutputInfos(t_io_infos *infos);
extern t_io_infos * initializeOutputInfos(int argc, char **argv);

      
#endif
