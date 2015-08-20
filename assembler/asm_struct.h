/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * asm_struct.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _ASM_STRUCT_H
#define _ASM_STRUCT_H

#include <stdlib.h>
#include <stdio.h>
#include <malloc/malloc.h>
#include <string.h>
#include <assert.h>
#include "asm_constants.h"

#ifndef _ASM_ALLOC_FUNCTION
#	define _ASM_ALLOC_FUNCTION malloc
#endif
#ifndef _ASM_FREE_FUNCTION
#	define _ASM_FREE_FUNCTION free
#endif

#define t_asm_comment char

typedef struct t_asm_register
{
	int	ID;
	int	indirect;
}t_asm_register;

typedef struct t_asm_label
{
	char *ID;		/* identifier */
	void *data;	   /* a memory word referenced by this label */
}t_asm_label;

typedef struct t_asm_address
{
	int addr;
	t_asm_label *label;
}t_asm_address;

typedef struct t_asm_instruction
{
	int opcode;
	int format;
	t_asm_register *reg_1;
	t_asm_register *reg_2;
	t_asm_register *reg_3;
	int immediate;
	t_asm_address *address;
	t_asm_comment *user_comment;
}t_asm_instruction;

typedef struct t_asm_data
{
	int dataType;
	int value;
}t_asm_data;

/* create an instance of `t_asm_register' */
extern t_asm_register * allocRegister(int ID, int indirect);

/* create an instance of `t_asm_address' */
extern t_asm_address * allocAddress(int displacement, t_asm_label *label);

/* create an instance of `t_asm_label' */
extern t_asm_label * allocLabel(char *ID, void *data);

/* create an instance of `t_asm_instruction' */
extern t_asm_instruction * allocInstruction(int opcode);

/* create an instance of `t_asm_data' */
extern t_asm_data * allocData(int dataType, int value);

/* initialize an instruction with three operands' */
extern t_asm_instruction * init_opcode3(int opcode, t_asm_register *reg_1
		, t_asm_register *reg_2, t_asm_register *reg_3);

/* initialize an instruction with two operands' */
extern t_asm_instruction * init_opcode2(int opcode, t_asm_register *reg_1
		, t_asm_register *reg_2, int immediate);

/* initialize an instruction with a single operand' */
extern t_asm_instruction * init_opcodeI(int opcode, t_asm_register *reg_1
		, t_asm_address *addr);

/* initialize a branch instruction */
extern t_asm_instruction * init_ccode(int opcode, t_asm_address *addr);

/* initialize a HALT instruction */
extern t_asm_instruction * init_halt();

/* initialize a NOP instruction */
extern t_asm_instruction * init_nop();

/* finalize an instruction info. */
extern void freeInstruction(t_asm_instruction *inst);

/* finalize a data info. */
extern void freeData(t_asm_data *data);

#endif
