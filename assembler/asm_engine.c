/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * asm_engine.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <malloc/malloc.h>
#include "asm_engine.h"
#include "asm_debug.h"

/* Function used when a compare is needed between two labels */
static int compareLabels (void *labelA, void *labelB);

/* Function used to produce the header of an object file.
* This function returns ASM_OK if everything went good*/
static int print_header_infos(FILE *fp);

/* Function that translates every code and data segment.
* This function returns ASM_OK if everything went good */
static int translateCode(t_translation_infos *infos, FILE *fp);

/* This function translates a single instruction found into the
* code segment. Returns ASM_OK if everything went good */
static int translateInstruction(t_translation_infos *infos
      , t_asm_instruction *inst, FILE *fp);

/* This function translates a single block of data found into a
* data segment. Returns ASM_OK if everything went good */
static int translateData(t_translation_infos *infos
      , t_asm_data *inst, FILE *fp);

/* The function `getBinaryOpcode' returns the coded opcode that will
* be stored into the object file */
static int getBinaryOpcode(int opcode);

/* This function returns the index of the given instruction/data
* that is stored inside the macro block of code plus data  */
static int getInstructionOrDataIndex(t_translation_infos *infos, void *target);

/* this function is directly called from `finalizeStructures'. The main
 * goal of this function is: free all the memory associated with the
 * labels */
static void finalizeLabels(t_list *labels);


/* This function returns the index of the given instruction/data
* that is stored inside the macro block of code plus data  */
int getInstructionOrDataIndex(t_translation_infos *infos
      , void *target)
{
   t_list	*target_element;
   int position;

   /* the list is empty */
   if (infos->code == NULL)
      return ASM_INVALID_MEMORY_OFFSET;
   
   /* preconditions: infos and inst MUST NOT be NULL references! */
   target_element = findElement(infos->code, target);
   if (target_element == NULL)
      return ASM_INVALID_MEMORY_OFFSET;

   /* retrieve the position of the statement */
   position = getPosition(infos->code, target_element);

   /* the code list doesn't hold target_element */
   if (position == -1)
      return ASM_INVALID_MEMORY_OFFSET;

   /* return the displacement in bytes from the beginning
    * of the instruction segment */
   if (position < infos->codesize)
      return ( ((position * ASM_INSTRUCTION_SIZE) / ASM_ALIGMENT_SIZE)
            + ((((position *ASM_INSTRUCTION_SIZE)
                     % ASM_ALIGMENT_SIZE) > 0)? 1:0) );
   else
   {
      t_list *current_elem;
      t_asm_data *current_data;
      
      int counter = 0;

      current_elem = getElementAt(infos->code, infos->codesize);
      assert(current_elem != NULL);

      counter = ( ((infos->codesize * ASM_INSTRUCTION_SIZE) / ASM_ALIGMENT_SIZE)
            + ((((infos->codesize * ASM_INSTRUCTION_SIZE)
                     % ASM_ALIGMENT_SIZE) > 0)? 1:0) );

      while (current_elem != NULL && LDATA(current_elem) != target)
      {
         current_data = LDATA(current_elem);

         if (current_data->dataType == ASM_WORD)
         {
            counter += (ASM_WORD_SIZE / ASM_ALIGMENT_SIZE)
                  + (((ASM_WORD_SIZE % ASM_ALIGMENT_SIZE) > 0)? 1 : 0);
         }
         else if (current_data->dataType == ASM_SPACE)
         {
            /* precondition always verified */
            assert(current_data->value > 0);
            counter += (current_data->value / ASM_ALIGMENT_SIZE)
                  + (((current_data->value % ASM_ALIGMENT_SIZE) > 0)? 1: 0);
         }

         current_elem = LNEXT(current_elem);
      }

      return counter;
   }

   return -1;
}

/* this function translates a single block of data found into a
* data segment. Returns ASM_OK if everything went good */
int translateData(t_translation_infos *infos
      , t_asm_data *data, FILE *fp)
{
   if (data == NULL)
      return ASM_UNDEFINED_DATA;
   
   if (data->dataType == ASM_WORD)
   {
      /* postcondition: print on file the coded data */
      if (fwrite(&(data->value), ASM_WORD_SIZE, 1, fp) != 1)
         return ASM_FWRITE_ERROR;
      return ASM_OK;
   }
   else if (data->dataType == ASM_SPACE)
   {
      char *zero_val;

      /* preconditions: data->value must be >= 0 */
      if (data->value < 0)
         return ASM_INVALID_DATA_FORMAT;

      /* force alignment to 4 bytes */
      if (data->value % ASM_WORD_SIZE != 0)
         data->value = data->value + (data->value % ASM_WORD_SIZE);

      /* initialize the value `zero_val' */
      zero_val = _ASM_ALLOC_FUNCTION(sizeof(char) * data->value);
      if (zero_val == NULL)
         return ASM_OUT_OF_MEMORY;

      /* set the block of memory to zero */
      memset(zero_val, 0, data->value);
      
      /* postcondition: print on file the coded data */
      if (fwrite(zero_val, data->value, 1, fp) != 1)
      {
         _ASM_FREE_FUNCTION(zero_val);
         return ASM_FWRITE_ERROR;
      }

      _ASM_FREE_FUNCTION(zero_val);
      return ASM_OK;
   }
   
   /* default: the data format in unknown */
   return ASM_INVALID_DATA_FORMAT;
}

/* This function translates a single instruction found into the
* code segment. Returns ASM_OK if everything went good */
int translateInstruction(t_translation_infos *infos
      , t_asm_instruction *inst, FILE *fp)
{
   int instruction;
   int pattern;
   int func;
   
   /* preconditions */
   if (inst == NULL)
      return ASM_UNDEFINED_INSTRUCTION;
   
   if (fp == NULL)
      return ASM_INVALID_INPUT_FILE;
   
#ifndef NDEBUG
   fprintf(stderr, "Coding instruction: [opcode == %s, format == %s] --> "
                  , opcode_toString(inst->opcode)
                           , dataFormat_toString(inst->format) );
#endif
   
   /* initialize the instruction */
   instruction = 0;

   /* set the format of instruction */
   if (inst->format == ASM_FORMAT_TER)
   {
      pattern = 0;
   }
   else if (inst->format == ASM_FORMAT_BIN)
   {
      pattern = (1 << 30);
   }
   else if  (   (inst->format == ASM_FORMAT_UNR)
            || (inst->format == ASM_FORMAT_NULL) )
   {
      pattern = (1 << 31);
   }
   else
   {
      assert(inst->format == ASM_FORMAT_JMP);
      pattern = (3 << 30);
   }
   
   /* update the instruction format value */
   instruction = pattern;
   
   /* initialize the opcode information */
   pattern = getBinaryOpcode(inst->opcode);

   /* test if the opcode is valid */
   if (pattern == INVALID_OPCODE)
      return ASM_INVALID_OPCODE;
   
   /* update the instruction format value */
   instruction = instruction + (pattern << 26);
   
   /* initialize the value of `func' */
   func = 0;
   
   if (inst->format == ASM_FORMAT_TER)
   {
      if ((inst->reg_1)->indirect)
         func = 4;
      if ((inst->reg_3)->indirect)
         func = func + 8;
      
      instruction = instruction + ( ((inst->reg_1)->ID) << 21);
      instruction = instruction + ( ((inst->reg_2)->ID) << 16);
      instruction = instruction + ( ((inst->reg_3)->ID) << 11);
      instruction = instruction + func;
   }
   else if (inst->format == ASM_FORMAT_BIN)
   {
      instruction = instruction + ( ((inst->reg_1)->ID) << 21);
      instruction = instruction + ( ((inst->reg_2)->ID) << 16);
      instruction = instruction + ( inst->immediate & 0x0000FFFF);
   }
   else if (inst->format == ASM_FORMAT_UNR)
   {
      instruction = instruction + ( ((inst->reg_1)->ID) << 21);
      if ((inst->address)->label != NULL)
      {
         int destinationIndex;
         
         /* we have to retrieve the position of the requested data */
         destinationIndex = getInstructionOrDataIndex
                  (infos, ((inst->address)->label)->data);

         /* test if the destinationIndex is a valid destination */
         if (destinationIndex == ASM_INVALID_MEMORY_OFFSET)
            return ASM_INVALID_LABEL_FOUND;
         
         /* update the displacement information */
         instruction = instruction
               + ( destinationIndex & 0x000FFFFF);
      }
      else
      {
         instruction = instruction + ( (inst->address)->addr & 0x000FFFFF);
      }
   }
   else if (inst->format == ASM_FORMAT_NULL)
   {
      /* DOES NOTHING */
   }
   else
   {
      /* test a precondition */
      assert(inst->format == ASM_FORMAT_JMP);
      
      if ((inst->address)->label != NULL)
      {
         int currentIndex;
         int destinationIndex;
         
         /* we have to retrieve the displacement between this
         * instruction and the instruction referred by the
         * label */
         currentIndex = getInstructionOrDataIndex(infos, inst);
         destinationIndex = getInstructionOrDataIndex
                  (infos, ((inst->address)->label)->data);
         
         /* test if the destinationIndex is a valid destination */
         if (destinationIndex == ASM_INVALID_MEMORY_OFFSET)
            return ASM_INVALID_LABEL_FOUND;
         
         /* postcondition that MUST be always verified */
         assert(destinationIndex < infos->codesize);
         
         /* update the displacement information */
         instruction = instruction
               + ( (destinationIndex - currentIndex) & 0x000FFFFF);
      }
      else
      {
         instruction = instruction + ( (inst->address)->addr & 0x000FFFFF);
      }
   }
   
   /* postcondition: print on file the coded instruction */
   if (fwrite(&instruction, 4, 1, fp) != 1)
      return ASM_FWRITE_ERROR;

#ifndef NDEBUG
   fprintf(stderr, "Coded Instruction:\t[0x%08x] \n", instruction);
#endif
   return ASM_OK;
}

/* function used when a compare is needed between two labels */
int compareLabels (void *labelA, void *labelB)
{
   t_asm_label *asm_labelA;
   t_asm_label *asm_labelB;
   
   /* preconditions */
   if (labelA == NULL)
   {
      if (labelB == NULL)
         return 1;
      return 0;
   }
   
   if (labelB == NULL)
      return 0;
   
   /* initialize labels */
   asm_labelA = (t_asm_label *) labelA;
   asm_labelB = (t_asm_label *) labelB;
   
   /* verify the consistency of this operation */
   assert(asm_labelA->ID != NULL);
   assert(asm_labelB->ID != NULL);

   /* postcondition */
   return (!strcmp(asm_labelA->ID, asm_labelB->ID));
}

/* create an instance of `t_translation_info' initializing the internal data
* of every field of the structure */
t_translation_infos * initStructures(int *errorcode)
{
   t_translation_infos *result;
   
   /* allocate memory for an instance of `t_translation_infos' */
   result = _ASM_ALLOC_FUNCTION(sizeof(t_translation_infos));
   
   /* test the out of memory condition */
   if (result == NULL)
   {
      /* update the value of `errorcode' */
      (*errorcode) = ASM_OUT_OF_MEMORY;
      return NULL;
   }

   /* no errors encountered so far */
   (*errorcode) = ASM_OK;
   
   /* initialize the content of `result' */
   result->code = NULL;
   result->labels = NULL;
   result->codesize = 0;
   
   /* return a new instance of `t_translation_infos' */
   return result;
}

/* Insert an instruction inside the `code' list of `infos' */
int addInstruction(t_translation_infos *infos
      , t_asm_instruction *instruction)
{
   /* preconditions */
   if (infos == NULL)
      return ASM_NOT_INITIALIZED_INFO;
   
   if (instruction == NULL)
      return ASM_UNDEFINED_INSTRUCTION;

   /* update the list of instructions */
   infos->code = addElement(infos->code, instruction, infos->codesize);
   
   /* update the codesize */
   infos->codesize++;

   /* notify that everything went correctly */
   return ASM_OK;
}

/* Insert a new label. The label must be initialized externally */
int insertLabel(t_translation_infos *infos, t_asm_label *label)
{
   /* preconditions */
   if (infos == NULL)
      return ASM_NOT_INITIALIZED_INFO;
   
   if (label == NULL)
      return ASM_INVALID_LABEL_FOUND;
   
   /* update the list of labels */
   infos->labels = addFirst(infos->labels, label);
   
   /* notify that everything went correctly */
   return ASM_OK;
}

/* find a label with a given `ID' */
t_asm_label * findLabel(t_translation_infos *infos, char *ID, int *asm_errorcode)
{
   t_asm_label pattern;
   t_list *label_element;
   
   /* preconditions */
   if (infos == NULL && asm_errorcode != NULL)
      (*asm_errorcode) = ASM_NOT_INITIALIZED_INFO;
   
   if (ID == NULL && asm_errorcode != NULL)
      (*asm_errorcode) = ASM_INVALID_LABEL_FOUND;
   
   /* initialize the value of `asm_errorcode' */
   (*asm_errorcode) = ASM_OK;
   
   /* initialize `pattern' */
   pattern.ID = ID;
   pattern.data = NULL;

   /* search the label */
   label_element = CustomfindElement(infos->labels, &pattern, compareLabels);

   /* if not found return a NULL pointer */
   if (label_element == NULL)
      return NULL;

   /* return the label found */
   return (t_asm_label *) LDATA(label_element);
}

/* remove a label */
int removeLabel(t_translation_infos *infos, char *ID)
{
   t_asm_label *result;
   int asm_errorcode;
   
   /* initialize the value of `asm_errorcode' */
   asm_errorcode = ASM_OK;
   
   /* initialize the value of `result' */
   result = findLabel(infos, ID, &asm_errorcode);
   
   /* postconditions */
   if (result == NULL)
      return asm_errorcode;
   
   /* remove the label from the list */
   infos->labels = removeElement(infos->labels, result);
   
   return asm_errorcode;
}

/* add a block of data into the data segment */
int addData(t_translation_infos *infos, t_asm_data *data)
{
   /* preconditions */
   if (infos == NULL)
      return ASM_NOT_INITIALIZED_INFO;
   
   if (data == NULL)
      return ASM_UNDEFINED_DATA;
   
   /* update the list of instructions */
   infos->code = addElement(infos->code, data, -1);

   return ASM_OK;
}

/* finalization of the `infos' structure */
int finalizeStructures(t_translation_infos *infos)
{
   if (infos == NULL)
      return ASM_NOT_INITIALIZED_INFO;
   
   if (infos->code != NULL)
   {
      t_list *current_element;
      t_asm_instruction *current_instr;

      /* initialize `data' */
      current_element = infos->code;
      
      while ((current_element != NULL) && (infos->codesize > 0) )
      {
         current_instr = (t_asm_instruction *) LDATA(current_element);
         
         /* free memory associated with the current instruction */
         freeInstruction(current_instr);

         /* update the value of `current_element' */
         current_element = LNEXT(current_element);
         infos->codesize --;
      }

      while (current_element != NULL)
      {
         /* free memory associated with the current data info. */
         freeData((t_asm_data *) LDATA(current_element));
         
         /* update the value of `current_element' */
         current_element = LNEXT(current_element);
      }

      /* free the code and data segment infos */
      freeList(infos->code);
   }
   
   /* remove labels */
   finalizeLabels(infos->labels);
   
   /* free the memory block associated with `infos' */
   _ASM_FREE_FUNCTION(infos);
   
   return ASM_OK;
}

/* begin the translation process */
int asm_writeObjectFile(t_translation_infos *infos, char *output_file)
{
   FILE *fp;
   int errorcode;

   if (output_file == NULL)
   {
      /* set "output.o" as output file name */
      output_file = "output.o";
   }

#ifndef NDEBUG
   fprintf(stdout, "\n\n*******************************************\n");
   fprintf(stdout, "INITIALIZE OUTPUT FILE: %s. \n", output_file);
   fprintf(stdout, "CODE SEGMENT has a size of %d instructions \n", infos->codesize);
   fprintf(stdout, "DATA SEGMENT has a size of %d elements \n"
         , (getLength(infos->code) - infos->codesize) );
   fprintf(stdout, "NUMBER OF LABELS : %d. \n", getLength(infos->labels));
   fprintf(stdout, "*******************************************\n\n");
#endif
   
   /* open a new file */
   fp = fopen(output_file, "w");
   if (fp == NULL)
      return ASM_FOPEN_ERROR;
   
   /* print the header of the object file */
   errorcode = print_header_infos(fp);
   
   if (errorcode != ASM_OK)
   {
      if (fclose(fp) == EOF)
         return ASM_FCLOSE_ERROR;
      return errorcode;
   }
   
   /* print the code and data segment */
   errorcode = translateCode(infos, fp);
   if (errorcode != ASM_OK)
   {
      if (fclose(fp) == EOF)
         return ASM_FCLOSE_ERROR;
      return errorcode;
   }

   /* print the trailer informations */
   //DOES NOTHING

   /* close the file and return */
   errorcode = fclose(fp);
   if (errorcode == EOF)
      return ASM_FCLOSE_ERROR;
   
   return ASM_OK;
}

/* Function that translates every code and data segment.
* This function returns ASM_OK if everything went good */
int translateCode(t_translation_infos *infos, FILE *fp)
{
   int instruction_counter;
   t_list *current_instruction;
   void *instruction_or_data;
   int errorcode;
   
   /* unchecked preconditions: pf and infos are different from NULL */
   
   if (infos->code == NULL)
      return ASM_CODE_NOT_PRESENT;
   
   /* initialize the instruction_counter */
   instruction_counter = 0;
   current_instruction = infos->code;
   errorcode = ASM_OK;
   
   /* translate the instruction segment */
   while (instruction_counter < infos->codesize)
   {
      instruction_or_data = LDATA(current_instruction);
      assert(instruction_or_data != NULL);
      
      /* translate every single instruction */
      errorcode = translateInstruction
            (infos, (t_asm_instruction *) instruction_or_data, fp);

      /* verify the errorcode */
      if (errorcode != ASM_OK)
         return errorcode;
      
      /* update the instruction counter and the current instruction data */
      instruction_counter++;
      current_instruction = LNEXT(current_instruction);
   }

#ifndef NDEBUG
   fprintf(stderr, "\n");
#endif
   
   /* translate the data segment */
   while (current_instruction != NULL)
   {
      instruction_or_data = LDATA(current_instruction);
      assert(instruction_or_data != NULL);
      
      /* translate every single element of data */
#ifndef NDEBUG
      fprintf(stderr, "Adding data into the data segment [datatype == %s \t; "
            , dataType_toString(((t_asm_data *)instruction_or_data)->dataType) );
      fprintf(stderr, "value == 0x%08x] \n"
            , ((t_asm_data *)instruction_or_data)->value);
#endif
      
      errorcode = translateData(infos, (t_asm_data *) instruction_or_data, fp);
      
      /* verify the errorcode */
      if (errorcode != ASM_OK)
         return errorcode;
      
      current_instruction = LNEXT(current_instruction);
   }
   
#ifndef NDEBUG
   fprintf(stderr, "\n");
#endif

   return ASM_OK;
}

/* Function used to produce the header of an object file.
* This function returns ASM_OK if everything went good*/
int print_header_infos(FILE *fp)
{
   char begin_header[4] = {'L', 'F', 'C', 'M'};
   char other_data[16];

   /* preconditions */
   if (fp == NULL)
      return ASM_INVALID_INPUT_FILE;
   
   /* write the starting string `LFCM' without the end of string '\0' */
   fputc(begin_header[0], fp);
   fputc(begin_header[1], fp);
   fputc(begin_header[2], fp);
   fputc(begin_header[3], fp);
   
   /* set `other_data' */
   memset(other_data, 0, 16);
   
   /* write the other_data infos */
   if (fwrite(other_data, 1, 16, fp) != 16)
      return ASM_FWRITE_ERROR;
   
   return ASM_OK;
}

/* The function `getBinaryOpcode' returns the coded opcode that will
* be stored into the object file */
int getBinaryOpcode(int opcode)
{
   switch(opcode)
   {
      case ADD_OP: return 0;
      case SUB_OP:  return 1;
      case ANDL_OP: return 2;
      case ORL_OP: return 3;
      case XORL_OP: return 4;
      case ANDB_OP: return 5;
      case ORB_OP: return 6;
      case XORB_OP: return 7;
      case MUL_OP: return 8;
      case DIV_OP: return 9;
      case SHL_OP: return 10;
      case SHR_OP: return 11;
      case ROTL_OP: return 12;
      case ROTR_OP: return 13;
      case NEG_OP: return 14;
      case SPCL_OP: return 15;
      case ADDI_OP: return 0;
      case SUBI_OP:  return 1;
      case ANDLI_OP: return 2;
      case ORLI_OP: return 3;
      case XORLI_OP: return 4;
      case ANDBI_OP: return 5;
      case ORBI_OP: return 6;
      case XORBI_OP: return 7;
      case MULI_OP: return 8;
      case DIVI_OP: return 9;
      case SHLI_OP: return 10;
      case SHRI_OP: return 11;
      case ROTLI_OP: return 12;
      case ROTRI_OP: return 13;
      case NOTL_OP: return 14;
      case NOTB_OP: return 15;
      case NOP_OP: return 0;
      case MOVA_OP: return 1;
      case LOAD_OP: return 4;
      case STORE_OP: return 5;
      case JSR_OP: return 2;
      case RET_OP: return 3;
      case HALT_OP: return 6;
      case SEQ_OP: return 7;
      case SGE_OP: return 8;
      case SGT_OP: return 9;
      case SLE_OP: return 10;
      case SLT_OP: return 11;
      case SNE_OP: return 12;
      case READ_OP: return 13;
      case WRITE_OP: return 14;
      case BT_OP: return 0;
      case BF_OP: return 1;
      case BHI_OP: return 2;
      case BLS_OP: return 3;
      case BCC_OP: return 4;
      case BCS_OP: return 5;
      case BNE_OP: return 6;
      case BEQ_OP: return 7;
      case BVC_OP: return 8;
      case BVS_OP: return 9;
      case BPL_OP: return 10;
      case BMI_OP: return 11;
      case BGE_OP: return 12;
      case BLT_OP: return 13;
      case BGT_OP: return 14;
      case BLE_OP: return 15;
      default: return INVALID_OPCODE;
   }
}

void finalizeLabels(t_list *labels)
{
   t_list *current_element;
   t_asm_label *current_label;
   
   if (labels == NULL)
      return;

   current_element = labels;
   while(current_element != NULL)
   {
      current_label = (t_asm_label *) LDATA(current_element);
      if (current_label != NULL)
      {
         if (current_label->ID != NULL)
            free(current_label->ID);
         _ASM_FREE_FUNCTION(current_label);
      }
      
      current_element = LNEXT(current_element);
   }

   freeList(labels);
}
