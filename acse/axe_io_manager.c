/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_io_manager.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <stdlib.h>
#include "axe_io_manager.h"

static t_io_infos * allocOutputInfos();


t_io_infos * initializeOutputInfos(int argc, char **argv)
{
   t_io_infos *result;

   /* create a new instance of `t_io_infos' */
   result = allocOutputInfos();
   if (result == NULL)
      return NULL;
   
   argc--;
   argv++;

   if (argc > 0)
   {
      result->input_file = fopen(argv[0], "r");
      if (result->input_file == NULL)
      {
         fprintf( stderr, "File not found : %s.\n", argv[0]);
         exit(-1);
      }
#ifndef NDEBUG
      fprintf(stdout, "Input file: %s. \n", argv[0]);
#endif
   }
#ifndef NDEBUG
   else
      fprintf(stdout, "Input file: %s. \n", "standard input");
#endif

   /* update the value of yyin */
   yyin = result->input_file;

   if (argc == 2)
      result->output_file_name = argv[1];
   else
      result->output_file_name = "output.asm";

#ifndef NDEBUG
   fprintf(stdout, "Output will be written on file : "
         "\"%s\". \n", result->output_file_name);
   fprintf(stdout, "The Symbol Table will be written on file : "
         "\"%s\". \n", "sy_table.out");
   fprintf(stdout, "Intermediate code will be written on file : "
         "\"%s\". \n", "output.cfg");
   fprintf(stdout, "control/dataflow informations will "
                   "be written on file : \"%s\". \n", "dataflow.cfg");
   fprintf(stdout, "Output of the register allocator "
                   "will be written on file  : \"%s\". \n\n", "regalloc.out");

   result->reg_alloc_output = fopen("regalloc.out", "w");
   if (result->reg_alloc_output == NULL)
      fprintf( stderr, "WARNING : Unable to create file: %s.\n", "regalloc.out");
   result->cfg_1 = fopen("output.cfg", "w");
   result->cfg_2 = fopen("dataflow.cfg", "w");
   result->syTable_output = fopen("sy_table.out", "w");
   if (result->cfg_1 == NULL)
      fprintf( stderr, "WARNING : Unable to create file: %s.\n", "output.cfg");
   if (result->cfg_2 == NULL)
      fprintf( stderr, "WARNING : Unable to create file: %s.\n", "dataflow.cfg");
   if (result->syTable_output == NULL)
      fprintf( stderr, "WARNING : Unable to create file: %s.\n", "sy_table.out");
#endif

   return result;
}

t_io_infos * allocOutputInfos()
{
   t_io_infos *result;

   /* Allocate memory for an instance of `t_output_infos' */
   result = (t_io_infos *)
         _AXE_ALLOC_FUNCTION(sizeof(t_io_infos));

   /* test if _AXE_ALLOC_FUNCTION returned a null pointer */
   if (result == NULL)
      return NULL;
      
   /* initialize the instance internal data */
   result->output_file_name = NULL;
   result->input_file = stdin;
#ifndef NDEBUG
   result->cfg_1 = stdout;
   result->cfg_2 = stdout;
   result->reg_alloc_output = stdout;
   result->syTable_output = stdout;
#endif

   /* return the result */
   return result;
}

void finalizeOutputInfos(t_io_infos *infos)
{
   if (infos == NULL)
      return;
   if (infos->input_file != NULL)
      fclose(infos->input_file);
#ifndef NDEBUG
   if (infos->cfg_1 != NULL)
      fclose(infos->cfg_1);
   if (infos->cfg_2 != NULL)
      fclose(infos->cfg_2);
   if (infos->reg_alloc_output != NULL)
      fclose(infos->reg_alloc_output);
   if (infos->syTable_output != NULL)
      fclose(infos->syTable_output);
#endif

   _AXE_FREE_FUNCTION(infos);
}
