/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * asm_struct.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "asm_struct.h"

t_asm_register * allocRegister(int ID, int indirect)
{
	t_asm_register *result;
	
	result = _ASM_ALLOC_FUNCTION(sizeof(t_asm_register));
   
   /* test the out of memory condition */
   if (result == NULL)
      return NULL;
	
	/* set the fields of the instance of `t_asm_register' */
	result->ID = ID;
	result->indirect = indirect;
	
	/* postconditions */
	return result;
}

t_asm_address * allocAddress(int displacement, t_asm_label *label)
{
	t_asm_address *result;
	
	result = _ASM_ALLOC_FUNCTION(sizeof(t_asm_address));
	
   /* test the out of memory condition */
   if (result == NULL)
      return NULL;
   
	/* set the fields of the instance of `t_asm_address' */
	if (label != NULL)
		result->addr = displacement;
	else
		result->addr = 0;
	result->label = label;
	
	/* postconditions */
	return result;
}

t_asm_label * allocLabel(char *ID, void *data)
{
	t_asm_label *result;
	
   result = _ASM_ALLOC_FUNCTION(sizeof(t_asm_label));
	
   /* test the out of memory condition */
   if (result == NULL)
      return NULL;

	/* set the fields of the instance of `t_asm_label' */
	result->data = data;
	result->ID = ID;
	
	/* postconditions */
	return result;
}

t_asm_instruction * allocInstruction(int opcode)
{
	t_asm_instruction *result;
	
	result = _ASM_ALLOC_FUNCTION(sizeof(t_asm_instruction));
	
   /* test the out of memory condition */
   if (result == NULL)
      return NULL;

	/* set the fields of the instance of `t_asm_instruction' */
	result->opcode = opcode;
	result->reg_1 = NULL;
	result->reg_2 = NULL;
	result->reg_3 = NULL;
	result->immediate = 0;
	result->format = 0;
	result->address = NULL;
	result->user_comment = NULL;
	
	/* postconditions */
	return result;
}

t_asm_instruction * init_opcode3(int opcode, t_asm_register *reg_1
		, t_asm_register *reg_2, t_asm_register *reg_3)
{
	t_asm_instruction *result;
	
	/* create an instance of `t_asm_instruction' */
	result = allocInstruction(opcode);

   /* test `out of memory' */
   if (result == NULL)
      return NULL;

	/* store the correct values for the instruction */
	result->format = ASM_FORMAT_TER;
	result->reg_1 = reg_1;
	result->reg_2 = reg_2;
	result->reg_3 = reg_3;
	
	/* postcondition: return the just created and initialized instance
	 * of `t_asm_instruction' */
	return result;
}

t_asm_instruction * init_opcode2(int opcode, t_asm_register *reg_1
		, t_asm_register *reg_2, int immediate)
{
	t_asm_instruction *result;
	
	/* create an instance of `t_asm_instruction' */
	result = allocInstruction(opcode);
	
   /* test `out of memory' */
   if (result == NULL)
      return NULL;

	/* store the correct values for the instruction */
	result->format = ASM_FORMAT_BIN;
	result->reg_1 = reg_1;
	result->reg_2 = reg_2;
	result->immediate = immediate;
	
	/* postcondition: return the just created and initialized instance
	 * of `t_asm_instruction' */
	return result;
}

t_asm_instruction * init_opcodeI(int opcode, t_asm_register *reg_1
		, t_asm_address *addr)
{
	t_asm_instruction *result;
	
	/* create an instance of `t_asm_instruction' */
	result = allocInstruction(opcode);

   /* test `out of memory' */
   if (result == NULL)
      return NULL;

	/* store the correct values for the instruction */
	result->format = ASM_FORMAT_UNR;
	result->reg_1 = reg_1;
	result->address = addr;
	
	/* postcondition: return the just created and initialized instance
	 * of `t_asm_instruction' */
	return result;
}

t_asm_instruction * init_ccode(int opcode, t_asm_address *addr)
{
	t_asm_instruction *result;
	
	/* create an instance of `t_asm_instruction' */
	result = allocInstruction(opcode);
	
   /* test `out of memory' */
   if (result == NULL)
      return NULL;

	/* store the correct values for the instruction */
	result->format = ASM_FORMAT_JMP;
	result->address = addr;
	
	/* postcondition: return the just created and initialized instance
	 * of `t_asm_instruction' */
	return result;
}

t_asm_instruction * init_halt()
{
	t_asm_instruction *result;
	
	/* create an instance of `t_asm_instruction' */
	result = allocInstruction(HALT_OP);

   /* test `out of memory' */
   if (result == NULL)
      return NULL;

	/* store the correct values for the instruction */
	result->format = ASM_FORMAT_NULL;
	
	/* postcondition: return the just created and initialized instance
	 * of `t_asm_instruction' */
	return result;
}

t_asm_instruction * init_nop()
{
   t_asm_instruction *result;
   
   /* create an instance of `t_asm_instruction' */
   result = allocInstruction(NOP_OP);

   /* test `out of memory' */
   if (result == NULL)
      return NULL;

   /* store the correct values for the instruction */
   result->format = ASM_FORMAT_NULL;
   
   /* postcondition: return the just created and initialized instance
    * of `t_asm_instruction' */
   return result;
}


t_asm_data * allocData(int dataType, int value)
{
	t_asm_data *result;
	
	result = _ASM_ALLOC_FUNCTION(sizeof(t_asm_data));

   /* test the out of memory condition */
   if (result == NULL)
      return NULL;

	/* initialize the fields of `result' */
	result->dataType = dataType;
	result->value = value;
	
	/* postconditions: return result */
	return result;
}

void freeInstruction(t_asm_instruction *inst)
{
	/* preconditions */
	if (inst == NULL)
		return;
	
	/* free memory */
	if (inst->reg_1 != NULL)
		_ASM_FREE_FUNCTION(inst->reg_1);
	if (inst->reg_2 != NULL)
		_ASM_FREE_FUNCTION(inst->reg_2);
	if (inst->reg_3 != NULL)
		_ASM_FREE_FUNCTION(inst->reg_3);
	if (inst->address != NULL)
		_ASM_FREE_FUNCTION(inst->address);
   
	_ASM_FREE_FUNCTION(inst);
}

/* finalize a data info. */
void freeData(t_asm_data *data)
{
	if (data != NULL)
		_ASM_FREE_FUNCTION(data);
}
