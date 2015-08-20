/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_errors.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "axe_errors.h"
#include "axe_utils.h"
#include "cflow_constants.h"

/* @see Acse.y for these declarations */
extern int num_warning;
extern int num_error;
extern int line_num;
extern int errorcode;
extern int cflow_errorcode;


static void abortProgram();
static void printErrorMessage(int errorcode);

void abortProgram()
{
   if (num_error > 0)
   {
      fprintf(stderr,   "Input file contains some errors. "
                        "No assembly file written\n");
      fprintf(stderr, "**%d error(s) found \n\n", num_error);
   }

   /* finalize all the data structures */
   shutdownCompiler(-1);
}

void printWarningMessage(int warningcode)
{
   char *msg;
   
   switch (warningcode)
   {
      case WARN_DIVISION_BY_ZERO : msg = "warning: division by zero"; break;
      default : msg = "<invalid warning>"; break;
   }
   
   /* print out to the standard error the warning message */
   printMessage(msg);

   /* update the value of num_warning */
   num_warning++;
}

static void printErrorMessage(int errorcode)
{
   char *msg;
  
   switch(errorcode)
   {
      case AXE_OUT_OF_MEMORY : msg = "error: Out of memory"; break;
      case AXE_PROGRAM_NOT_INITIALIZED :
                  msg = "error: Program not initialized"; break;
      case AXE_INVALID_INSTRUCTION :
                  msg = "error: Invalid instruction"; break;
      case AXE_VARIABLE_ID_UNSPECIFIED :
                  msg = "error: Variable ID unspecified"; break;
      case AXE_VARIABLE_ALREADY_DECLARED :
                  msg = "error: Variable already declared"; break;
      case AXE_INVALID_TYPE : msg = "error: Invalid type"; break;
      case AXE_FOPEN_ERROR : msg = "error: fopen failed"; break;
      case AXE_FCLOSE_ERROR : msg = "error: fclose failed"; break;
      case AXE_INVALID_INPUT_FILE : msg = "error: Wrong file pointer"; break;
      case AXE_FWRITE_ERROR : msg = "error: Error while writing on file"; break;
      case AXE_INVALID_DATA_FORMAT : msg = "error: Invalid data format"; break;
      case AXE_INVALID_OPCODE : msg = "error: Invalid opcode found"; break;
      case AXE_INVALID_REGISTER_INFO :
                  msg = "error: Invalid register infos"; break;
      case AXE_INVALID_LABEL : msg = "error: Invalid label found"; break;
      case AXE_INVALID_LABEL_MANAGER :
                  msg = "error: Invalid label manager"; break;
      case AXE_INVALID_ARRAY_SIZE : msg = "error: Invalid array size"; break;
      case AXE_INVALID_VARIABLE : msg = "error: Invalid variable found"; break;
      case AXE_INVALID_ADDRESS : msg = "error: Invalid address"; break;
      case AXE_INVALID_EXPRESSION :
                  msg = "error: Invalid expression found"; break;
      case AXE_UNKNOWN_VARIABLE : msg = "error: Unknown variable found"; break;
      case AXE_SY_TABLE_ERROR :
                  msg = "error: Symbol table returned an errorcode"; break;
      case AXE_NULL_DECLARATION : msg = "error: NULL declaration found"; break;
      case AXE_LABEL_ALREADY_ASSIGNED :
                  msg = "error: label already assigned"; break;
      case AXE_INVALID_CFLOW_GRAPH : msg = "error: Invalid "
                  "control-dataflow graph informations"; break;
      case AXE_INVALID_REG_ALLOC : msg = "error: Invalid "
                  "register allocator instance found"; break;
      case AXE_REG_ALLOC_ERROR : msg = "error: "
                  "register allocation failed"; break;
      case AXE_TRANSFORM_ERROR : msg = "error: "
                  "Invalid operation while modifying the instructions"; break;
      case AXE_SYNTAX_ERROR : msg = "error: "
                  "Syntax error found"; break;
      case AXE_UNKNOWN_ERROR : msg = "error: Unknown error"; break;
      default : msg = "<invalid errorcode>"; break;
   }
   
   /* print out to the standard error the error message */
   printMessage(msg);

   /* update the value of num_error */
   num_error++;
}

void printMessage(const char *msg)
{
   if (line_num != -1)
      fprintf(stderr, "\nAt line %d , %s.\n", line_num, msg);
   else
      fprintf(stderr, "%s.\n", msg);
}

void notifyError(int axe_errorcode)
{
   errorcode = axe_errorcode;
   checkConsistency();
}

void checkConsistency()
{
   /* test if an error occurred */
   if (errorcode != AXE_OK) {
      printErrorMessage(errorcode);
      abortProgram();
   }

   if (cflow_errorcode != CFLOW_OK)
   {
      fprintf(stderr, "An error occurred while working with the "
                 "cflow graph. cflow_errorcode = %d"
                 "\n(see axe_cflow_graph.[ch])\n", cflow_errorcode);
      
      printErrorMessage(AXE_UNKNOWN_ERROR);
      abortProgram();
   }
}
