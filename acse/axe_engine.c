/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_engine.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <assert.h>
#include <string.h>
#include <malloc/malloc.h>
#include "axe_engine.h"
#include "symbol_table.h"
#include "axe_errors.h"

/* global variable errorcode */
int errorcode;

/* Function used when a compare is needed between two labels */
static int compareVariables (void *Var_A, void *Var_B);

/* Finalize the memory associated with an instruction */
static void finalizeInstructions(t_list *instructions);

/* Finalize the data segment */
static void finalizeDataSegment(t_list *dataDirectives);

/* finalize the informations associated with all the variables */
static void finalizeVariables(t_list *variables);

/* Translate the assembler directives (definitions inside the data segment */
static void translateDataSegment(t_program_infos *program, FILE *fp);

/* Translate all the instructions within the code segment */
static void translateCodeSegment(t_program_infos *program, FILE *fp);

/* print out to the file `fp' an opcode */
static void printOpcode(int opcode, FILE *fp);

/* print out to the file `fp' a register information */
static void printRegister(t_axe_register *reg, FILE *fp);

/* add a variable to the program */
static void addVariable(t_program_infos *program, t_axe_variable *variable);

      
/* create a new variable */
void createVariable(t_program_infos *program, char *ID
      , int type, int isArray, int arraySize, int init_val)
{
   t_axe_variable *var;
         
   /* test the preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   /* initialize a new variable */
   var = alloc_variable(ID, type, isArray, arraySize, init_val);
   if (var == NULL)
      notifyError(AXE_OUT_OF_MEMORY);
   
   /* assign a new label to the newly created variable `var' */
   var->labelID = newLabel(program);

   /* add the new variable to program */
   addVariable(program, var);
}

/* translate each instruction in his assembler symbolic representation */
void translateCodeSegment(t_program_infos *program, FILE *fp)
{
   t_list *current_element;
   t_axe_instruction *current_instruction;
   int _error;
   
   /* preconditions */
   if (fp == NULL)
      notifyError(AXE_INVALID_INPUT_FILE);

   if (program == NULL)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   }

   /* initialize the current_element */
   current_element = program->instructions;

   /* write the .text directive */
   if (current_element != NULL)
   {
      if (fprintf(fp, "\t.text\n") < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }
   }

   while (current_element != NULL)
   {
      /* retrieve the current instruction */
      current_instruction = (t_axe_instruction *) LDATA(current_element);
      assert(current_instruction != NULL);
      assert(current_instruction->opcode != INVALID_OPCODE);

      if (current_instruction->labelID != NULL)
      {
            /* create a string identifier for the label */
            if (  fprintf(fp, "L%d : \t"
                     , (current_instruction->labelID)->labelID ) < 0)
            {
              _error = fclose(fp);
               if (_error == EOF)
                  notifyError(AXE_FCLOSE_ERROR);
               notifyError(AXE_FWRITE_ERROR);
            }
      }
      else
      {
            /* create a string identifier for the label */
            if (fprintf(fp, "\t") < 0)
            {
              _error = fclose(fp);
               if (_error == EOF)
                  notifyError(AXE_FCLOSE_ERROR);
               notifyError(AXE_FWRITE_ERROR);
            }
      }

      /* print the opcode */
      printOpcode(current_instruction->opcode, fp);

      if (  (current_instruction->opcode == HALT)
            || (current_instruction->opcode == NOP) )
      {
         if (fprintf(fp, "\n") < 0)
         {
              _error = fclose(fp);
               if (_error == EOF)
                  notifyError(AXE_FCLOSE_ERROR);
               notifyError(AXE_FWRITE_ERROR);
         }

         /* update the current_element */
         current_element = LNEXT(current_element);
         continue;
      }
      
      if (fputc(' ', fp) == EOF)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }

      if (current_instruction->reg_1 != NULL)
      {
         printRegister(current_instruction->reg_1, fp);
         
         if (fputc(' ', fp) == EOF)
         {
            _error = fclose(fp);
            if (_error == EOF)
               notifyError(AXE_FCLOSE_ERROR);
            notifyError(AXE_FWRITE_ERROR);
         }
      }
      if (current_instruction->reg_2 != NULL)
      {
         printRegister(current_instruction->reg_2, fp);
         if (errorcode != AXE_OK)
            return;

         if (fputc(' ', fp) == EOF)
         {
            _error = fclose(fp);
            if (_error == EOF)
               notifyError(AXE_FCLOSE_ERROR);
            notifyError(AXE_FWRITE_ERROR);
         }
      }
      if (current_instruction->reg_3 != NULL)
      {
         printRegister(current_instruction->reg_3, fp);

         if (fprintf(fp, "\n") < 0) {
            _error = fclose(fp);
            if (_error == EOF)
               notifyError(AXE_FCLOSE_ERROR);
            notifyError(AXE_FWRITE_ERROR);
         }

         /* update the current_element */
         current_element = LNEXT(current_element);
         continue;
      }

      if (current_instruction->address != NULL)
      {
         if ((current_instruction->address)->type == ADDRESS_TYPE)
         {
            if (fprintf(fp, "%d", (current_instruction->address)->addr) < 0)
            {
               _error = fclose(fp);
               if (_error == EOF)
                  notifyError(AXE_FCLOSE_ERROR);
               notifyError(AXE_FWRITE_ERROR);
            }
         }
         else
         {
            assert((current_instruction->address)->type == LABEL_TYPE);
            if (  fprintf(fp, "L%d"
                     , ((current_instruction->address)->labelID)
                              ->labelID) < 0)
            {
               _error = fclose(fp);
               if (_error == EOF)
                  notifyError(AXE_FCLOSE_ERROR);
               notifyError(AXE_FWRITE_ERROR);
            }
         }
         
         if (fprintf(fp, "\n") < 0) {
            _error = fclose(fp);
            if (_error == EOF)
               notifyError(AXE_FCLOSE_ERROR);
            notifyError(AXE_FWRITE_ERROR);
         }

         /* update the current_element */
         current_element = LNEXT(current_element);
         continue;
      }

      if (fprintf(fp, "#%d", current_instruction->immediate) < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }

      if (fprintf(fp, "\n") < 0) {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }

      /* loop termination condition */
      current_element = LNEXT(current_element);
   }
}

void translateDataSegment(t_program_infos *program, FILE *fp)
{
   t_list *current_element;
   t_axe_data *current_data;
   int _error;
   int fprintf_error;
   
   /* preconditions */
   if (fp == NULL)
      notifyError(AXE_INVALID_INPUT_FILE);

   /* initialize the local variable `fprintf_error' */
   fprintf_error = 0;
   
   if (program == NULL)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);

      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   }

   /* initialize the value of `current_element' */
   current_element = program->data;

   /* write the .data directive */
   if (current_element != NULL)
   {
      if (fprintf(fp, "\t.data\n") < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }
   }

   /* iterate all the elements inside the data segment */
   while (current_element != NULL)
   {
      /* retrieve the current data element */
      current_data = (t_axe_data *) LDATA(current_element);

      /* assertions */
      assert (current_data->directiveType != DIR_INVALID);

      /* create a string identifier for the label */
      if ( (current_data->labelID != NULL)
            && ((current_data->labelID)->labelID != LABEL_UNSPECIFIED) )
      {
         fprintf_error = fprintf(fp, "L%d : \t"
                  , (current_data->labelID)->labelID);
      }
      else
      {
         fprintf_error = fprintf(fp, "\t");
      }

      /* test if an error occurred while executing the `fprintf' function */
      if (fprintf_error < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }

      /* print the directive identifier */
      if (current_data->directiveType == DIR_WORD)
      {
         if (fprintf(fp, ".WORD ") < 0)
         {
            _error = fclose(fp);
            if (_error == EOF)
               notifyError(AXE_FCLOSE_ERROR);
            notifyError(AXE_FWRITE_ERROR);
         }
      }
      
      else if (current_data->directiveType == DIR_SPACE)
      {
         if (fprintf(fp, ".SPACE ") < 0)
         {
            _error = fclose(fp);
            if (_error == EOF)
               notifyError(AXE_FCLOSE_ERROR);
            notifyError(AXE_FWRITE_ERROR);
         }
      }

      /* print the value associated with the directive */
      if (fprintf(fp, "%d\n", current_data->value) < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }

      /* loop termination condition */
      current_element = LNEXT(current_element);
   }
}

void finalizeDataSegment(t_list *dataDirectives)
{
   t_list *current_element;
   t_axe_data *current_data;

   /* nothing to finalize */
   if (dataDirectives == NULL)
      return;

   current_element = dataDirectives;
   while(current_element != NULL)
   {
      /* retrieve the current instruction */
      current_data = (t_axe_data *) LDATA(current_element);
      if (current_data != NULL)
         free_Data(current_data);

      current_element = LNEXT(current_element);
   }

   /* free the list of instructions */
   freeList(dataDirectives);
}

void finalizeInstructions(t_list *instructions)
{
   t_list *current_element;
   t_axe_instruction *current_instr;

   /* nothing to finalize */
   if (instructions == NULL)
      return;

   current_element = instructions;
   while(current_element != NULL)
   {
      /* retrieve the current instruction */
      current_instr = (t_axe_instruction *) LDATA(current_element);
      if (current_instr != NULL)
         free_Instruction(current_instr);

      current_element = LNEXT(current_element);
   }

   /* free the list of instructions */
   freeList(instructions);
}

int compareVariables (void *Var_A, void *Var_B)
{
   t_axe_variable *va;
   t_axe_variable *vb;
   
   if (Var_A == NULL)
   {
      if (Var_B == NULL)
         return 1;
   }

   if (Var_B == NULL)
      return 0;

   va = (t_axe_variable *) Var_A;
   vb = (t_axe_variable *) Var_B;

   /* test if the name is the same */
   return (!strcmp(va->ID, vb->ID));
}

t_axe_variable * getVariable
      (t_program_infos *program, char *ID)
{
   t_axe_variable search_pattern;
   t_list *elementFound;
   
   /* preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   if (ID == NULL)
      notifyError(AXE_VARIABLE_ID_UNSPECIFIED);

   /* initialize the pattern */
   search_pattern.ID = ID;
   
   /* search inside the list of variables */
   elementFound = CustomfindElement
         (program->variables, &search_pattern, compareVariables);

   /* if the element is found return it to the caller. Otherwise return NULL. */
   if (elementFound != NULL)
      return (t_axe_variable *) LDATA(elementFound);

   return NULL;
}

/* initialize an instance of `t_program_infos' */
t_program_infos * allocProgramInfos()
{
   t_program_infos *result;

   /* initialize the local variable `result' */
   result = (t_program_infos *)
         _AXE_ALLOC_FUNCTION(sizeof(t_program_infos));

   /* verify if an error occurred during the memory allocation
    * process */
   if (result == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* initialize the new instance of `result' */
   result->variables = NULL;
   result->instructions = NULL;
   result->data = NULL;
   result->current_register = 1; /* we are excluding the register R0 */
   result->lmanager = initialize_label_manager();

   if (result->lmanager == NULL)
   {
      finalizeProgramInfos(result);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   result->sy_table = initialize_sy_table();
   
   /* test if the sy_table is a NULL pointer */
   if (result->sy_table == NULL)
   {
      finalizeProgramInfos(result);
      notifyError(AXE_OUT_OF_MEMORY);
   }
   
   /* postcondition: return an instance of `t_program_infos' */
   return result;
}

/* add an instruction at the tail of the list `program->instructions'.
 * Returns an error code. */
void addInstruction(t_program_infos *program, t_axe_instruction *instr)
{
   /* test the preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   
   if (instr == NULL)
      notifyError(AXE_INVALID_INSTRUCTION);

   if (program->lmanager == NULL)
      notifyError(AXE_INVALID_LABEL_MANAGER);

   instr->labelID = assign_label(program->lmanager);

   /* update the list of instructions */
   program->instructions = addElement(program->instructions, instr, -1);
}

/* reserve a new label identifier for future uses */
t_axe_label * newLabel(t_program_infos *program)
{
   /* test the preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   if (program->lmanager == NULL)
      notifyError(AXE_INVALID_LABEL_MANAGER);

   return newLabelID(program->lmanager);
}

/* assign a new label identifier to the next instruction */
t_axe_label * assignLabel(t_program_infos *program, t_axe_label *label)
{
   /* test the preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   if (program->lmanager == NULL)
      notifyError(AXE_INVALID_LABEL_MANAGER);

   /* fix the label */
   return assignLabelID(program->lmanager, label);
}

/* reserve a new label identifier */
t_axe_label * assignNewLabel(t_program_infos *program)
{
   t_axe_label * reserved_label;

   /* reserve a new label */
   reserved_label = newLabel(program);
   if (reserved_label == NULL)
      return NULL;

   /* fix the label */
   return assignLabel(program, reserved_label);
}

void addVariable(t_program_infos *program, t_axe_variable *variable)
{
   t_axe_variable *variableFound;
   t_axe_data *new_data_info;
   int sy_error;
   
   /* test the preconditions */
   if (variable == NULL)
      notifyError(AXE_INVALID_VARIABLE);

   if (program == NULL)
   {
      free_variable(variable);
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   }

   if (variable->ID == NULL)
   {
      free_variable(variable);
      notifyError(AXE_VARIABLE_ID_UNSPECIFIED);
   }

   if (variable->type == UNKNOWN_TYPE)
   {
      free_variable(variable);
      notifyError(AXE_INVALID_TYPE);
   }

   if (variable->isArray)
   {
      if (variable->arraySize <= 0)
      {
         free_variable(variable);
         notifyError(AXE_INVALID_ARRAY_SIZE);
      }
   }
   
   if (variable->labelID == NULL)
   {
      free_variable(variable);
      notifyError(AXE_INVALID_LABEL);
   }
   
   /* we have to test if already exists a variable with the same ID */
   variableFound = getVariable(program, variable->ID);

   if (variableFound != NULL)
   {
      free_variable(variable);
      notifyError(AXE_VARIABLE_ALREADY_DECLARED);
   }

   /* now we can add the new variable to the program */
   program->variables = addElement(program->variables, variable, -1);

   /* create an instance of `t_axe_data' */
   if (variable->type == INTEGER_TYPE)
   {
      if (variable->isArray)
      {
         new_data_info = alloc_data
               (DIR_SPACE, (variable->arraySize * 4), variable->labelID);
         
         if (new_data_info == NULL)
            notifyError(AXE_OUT_OF_MEMORY);
      }
      else
      {
         new_data_info = alloc_data
            (DIR_WORD, variable->init_val, variable->labelID);
         
         if (new_data_info == NULL)
            notifyError(AXE_OUT_OF_MEMORY);
      }
   }

   /* update the list of directives */
   program->data = addElement(program->data, new_data_info, -1);

   /* update the content of the symbol table */
   sy_error = putSym(program->sy_table, variable->ID, variable->type);
     
   if (sy_error != SY_TABLE_OK)
      notifyError(AXE_SY_TABLE_ERROR);
}

int getNewRegister(t_program_infos *program)
{
   int result;
   
   /* test the preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   result = program->current_register;
   program->current_register++;
   
   /* return the current label identifier */
   return result;
}

void finalizeProgramInfos(t_program_infos *program)
{
   if (program == NULL)
      return;
   if (program->variables != NULL)
      finalizeVariables(program->variables);
   if (program->instructions != NULL)
      finalizeInstructions(program->instructions);
   if (program->data != NULL)
      finalizeDataSegment(program->data);
   if (program->lmanager != NULL)
      finalize_label_manager(program->lmanager);
   if (program->sy_table != NULL)
      finalize_sy_table(program->sy_table);

   _AXE_FREE_FUNCTION(program);
}

void writeAssembly(t_program_infos *program, char *output_file)
{
   FILE *fp;
   int _error;

   /* test the preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   /* If necessary, set the value of `output_file' to "output.asm" */
   if (output_file == NULL)
   {
      /* set "output.o" as output file name */
      output_file = "output.asm";
   }

#ifndef NDEBUG
   fprintf(stdout, "\n\n*******************************************\n");
   fprintf(stdout, "INITIALIZING OUTPUT FILE: %s. \n", output_file);
   fprintf(stdout, "CODE SEGMENT has a size of %d instructions \n"
         , getLength(program->instructions));
   fprintf(stdout, "DATA SEGMENT has a size of %d elements \n"
         , getLength(program->data));
   fprintf(stdout, "NUMBER OF LABELS : %d. \n"
         , get_number_of_labels(program->lmanager));
   fprintf(stdout, "*******************************************\n\n");
#endif
   
   /* open a new file */
   fp = fopen(output_file, "w");
   if (fp == NULL)
      notifyError(AXE_FOPEN_ERROR);

   /* print the data segment */
   translateDataSegment(program, fp);

   /* print the code segment */
   translateCodeSegment(program, fp);

   /* close the file and return */
   _error = fclose(fp);
   if (_error == EOF)
      notifyError(AXE_FCLOSE_ERROR);
}

void printOpcode(int opcode, FILE *fp)
{
   char *opcode_to_string;
   int _error;
   
   /* preconditions: fp must be different from NULL */
   if (fp == NULL)
      notifyError(AXE_INVALID_INPUT_FILE);

   switch(opcode)
   {
      case ADD : opcode_to_string = "ADD"; break;
      case SUB : opcode_to_string = "SUB"; break;
      case ANDL : opcode_to_string = "ANDL"; break;
      case ORL : opcode_to_string = "ORL"; break;
      case EORL : opcode_to_string = "EORL"; break;
      case ANDB : opcode_to_string = "ANDB"; break;
      case ORB : opcode_to_string = "ORB"; break;
      case EORB : opcode_to_string = "EORB"; break;
      case MUL : opcode_to_string = "MUL"; break;
      case DIV : opcode_to_string = "DIV"; break;
      case SHL : opcode_to_string = "SHL"; break;
      case SHR : opcode_to_string = "SHR"; break;
      case ROTL : opcode_to_string = "ROTL"; break;
      case ROTR : opcode_to_string = "ROTR"; break;
      case NEG : opcode_to_string = "NEG"; break;
      case SPCL : opcode_to_string = "SPCL"; break;
      case ADDI : opcode_to_string = "ADDI"; break;
      case SUBI : opcode_to_string = "SUBI"; break;
      case ANDLI : opcode_to_string = "ANDLI"; break;
      case ORLI : opcode_to_string = "ORLI"; break;
      case EORLI : opcode_to_string = "EORLI"; break;
      case ANDBI : opcode_to_string = "ANDBI"; break;
      case ORBI : opcode_to_string = "ORBI"; break;
      case EORBI : opcode_to_string = "EORBI"; break;
      case MULI : opcode_to_string = "MULI"; break;
      case DIVI : opcode_to_string = "DIVI"; break;
      case SHLI : opcode_to_string = "SHLI"; break;
      case SHRI : opcode_to_string = "SHRI"; break;
      case ROTLI : opcode_to_string = "ROTLI"; break;
      case ROTRI : opcode_to_string = "ROTRI"; break;
      case NOTL : opcode_to_string = "NOTL"; break;
      case NOTB : opcode_to_string = "NOTB"; break;
      case NOP : opcode_to_string = "NOP"; break;
      case MOVA : opcode_to_string = "MOVA"; break;
      case JSR : opcode_to_string = "JSR"; break;
      case RET : opcode_to_string = "RET"; break;
      case HALT : opcode_to_string = "HALT"; break;
      case BT : opcode_to_string = "BT"; break;
      case BF : opcode_to_string = "BF"; break;
      case BHI : opcode_to_string = "BHI"; break;
      case BLS : opcode_to_string = "BLS"; break;
      case BCC : opcode_to_string = "BCC"; break;
      case BCS : opcode_to_string = "BCS"; break;
      case BNE : opcode_to_string = "BNE"; break;
      case BEQ : opcode_to_string = "BEQ"; break;
      case BVC : opcode_to_string = "BVC"; break;
      case BVS : opcode_to_string = "BVS"; break;
      case BPL : opcode_to_string = "BPL"; break;
      case BMI : opcode_to_string = "BMI"; break;
      case BGE : opcode_to_string = "BGE"; break;
      case BLT : opcode_to_string = "BLT"; break;
      case BGT : opcode_to_string = "BGT"; break;
      case BLE : opcode_to_string = "BLE"; break;
      case LOAD : opcode_to_string = "LOAD"; break;
      case STORE : opcode_to_string = "STORE"; break;
      case SEQ : opcode_to_string = "SEQ"; break;
      case SGE : opcode_to_string = "SGE"; break;
      case SGT : opcode_to_string = "SGT"; break;
      case SLE : opcode_to_string = "SLE"; break;
      case SLT : opcode_to_string = "SLT"; break;
      case SNE : opcode_to_string = "SNE"; break;
      case AXE_READ : opcode_to_string = "READ"; break;
      case AXE_WRITE : opcode_to_string = "WRITE"; break;
      default :
         /* close the file and return */
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_INVALID_OPCODE);
   }
      
   /* postconditions */
   if (fprintf(fp, "%s", opcode_to_string) < 0)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);
      notifyError(AXE_FWRITE_ERROR);
   }
}

void printRegister(t_axe_register *reg, FILE *fp)
{
   int _error;
   
   /* preconditions: fp must be different from NULL */
   if (fp == NULL)
      notifyError(AXE_INVALID_INPUT_FILE);
   if (reg == NULL)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);
      notifyError(AXE_INVALID_REGISTER_INFO);
   }
   if (reg->ID == REG_INVALID)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);
      notifyError(AXE_INVALID_REGISTER_INFO);
   }

   if (reg->indirect)
   {
      if (fprintf(fp, "(R%d)", reg->ID) < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }
   }
   else
   {
      if (fprintf(fp, "R%d", reg->ID) < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }
   }
}

t_axe_label * getLabelFromVariableID(t_program_infos *program, char *ID)
{
   t_axe_variable *var;
   
   var = getVariable(program, ID);
   if (var == NULL)
      return NULL;

   /* test the postconditions */
   assert(var->labelID != NULL);
   
   return var->labelID;
}

void finalizeVariables(t_list *variables)
{
   t_list *current_element;
   t_axe_variable *current_var;

   if (variables == NULL)
      return;

   /* initialize the `current_element' */
   current_element = variables;
   while(current_element != NULL)
   {
      current_var = (t_axe_variable *) LDATA(current_element);
      if (current_var != NULL)
      {
         if (current_var->ID != NULL)
            free(current_var->ID);
         
         _AXE_FREE_FUNCTION(current_var);
      }
      
      current_element = LNEXT(current_element);
   }

   /* free the list of variables */
   freeList(variables);
}
