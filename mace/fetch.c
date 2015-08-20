/*
 * Giovanni Agosta, Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * fetch.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */
#include <stdlib.h>
#include "decode.h"
#include "fetch.h"
#include "machine.h"

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y) )
#define SIGN(X) ((X) >= 0?1:0)


static int executeTER(decoded_instr *instr);
static int executeBIN(decoded_instr *instr);
static int executeUNR(decoded_instr *instr);
static int executeJMP(decoded_instr *instr);
static int handle_special_instruction(decoded_instr *instr);


/* returns next pc, negative values are error codes, 0 is correct termination */
int fetch_execute(unsigned int *code, int pc){
   int result;
	decoded_instr *instr = decode(code[pc]);

	switch (instr->format) {
		case TER : result = executeTER(instr);
			break;
		case BIN : result = executeBIN(instr);
			break;
		case UNR : result = executeUNR(instr);
			break;
		case JMP : result = executeJMP(instr);
			break;
		default : result = INVALID_INSTR_FORMAT;
	}

   /* free the memory associated with `instr' */
   if (instr != NULL)
      free(instr);

   /* return the new PC (or the value _HALT) as result */
   return result;
}

int executeTER(decoded_instr *instr){
	int *dest, *src1, *src2;
	int carryout=0, overflow=0, negative=0, zero=0;

	/* Manage addressing modes (direct/indirect) */
	if (indirect_dest(instr)) dest=&(mem[reg[instr->dest]]);
	else dest=&(reg[instr->dest]);
	src1=&(reg[instr->src1]);
	if (indirect_src2(instr)) src2=&(mem[reg[instr->src2]]);
	else src2=&(reg[instr->src2]);

   switch (instr->opcode) {
		case ADD  : *dest=*src1 + *src2;
			if (!sign(instr)) {
				if (carry(instr)) *dest= *dest + getflag(CARRY);
				if (*dest<*src1 || (getflag(CARRY) && *dest==*src1)) carryout = 1;
			} else {
				if (SIGN(*dest)!=SIGN(*src1) && SIGN(src1)==SIGN(*src2)) overflow=1;
			}
		 break;
		case SUB  : *dest=*src1 - *src2; 
			if (!sign(instr)) {
				if (carry(instr)) *dest=*dest - getflag(CARRY);
				if (*dest>*src1 || (getflag(CARRY) && *dest==*src1)) carryout = 1;
			} else {
				if (SIGN(*dest)!=SIGN(*src1) && SIGN(src1)==SIGN(*src2)) overflow=1;
			}
		 break;
		case ANDL : *dest = *src1 && *src2 ;
		 break;
		case ORL  : *dest = *src1 || *src2 ;
		 break;
		case EORL : *dest = (*src1 && !*src2) || (!*src1 && *src2);
		 break;
		case ANDB : *dest = *src1 & *src2 ;
		 break;
		case ORB  : *dest = *src1 | *src2 ;
		 break;
		case EORB : *dest = *src1 ^ *src2 ;
		 break;
		case MUL  : *dest=*src1 * *src2; 
			/* TODO: check carry and overflow */
			if (!sign(instr)) {
				if (carry(instr)) *dest= *dest + getflag(CARRY);
				if (*dest<*src1 || (getflag(CARRY) && *dest==*src1)) carryout = 1;
			} else {
				if (SIGN(*dest)!=SIGN(*src1) && SIGN(src1)==SIGN(*src2)) overflow=1;
			}
		 break;
		case DIV  : *dest=*src1 / *src2; 
			/* TODO: check carry and overflow */
			if (!sign(instr)) {
				if (carry(instr)) *dest=*dest - getflag(CARRY);
				if (*dest>*src1 || (getflag(CARRY) && *dest==*src1)) carryout = 1;
			} else {
				if (SIGN(*dest)!=SIGN(*src1) && SIGN(src1)==SIGN(*src2)) overflow=1;
			}
		 break;
		case SHL  : *dest = *src2 << *src1; 
			if ((!sign(instr)) && carry(instr)) *dest=*dest + getflag(CARRY);
			/* TODO: carryout */
		 break;
		case SHR  : *dest = *src2 >> *src1; /* Should use sign bit to implement arithmetic and logic shift */
			if ((!sign(instr)) && carry(instr)) *dest=*dest + getflag(CARRY);
			/* TODO: carryout, signed shift */
		 break;
		case ROTL : ;
		 break;
		case ROTR : ;
		 break;
		case NEG  : *dest = - *src2;
		 break;
		case SPCL : pc=handle_special_instruction(instr) ;
		 break;
		default :
         return INVALID_INSTR;
	}
	zero = (*dest == 0);
	negative = (*dest < 0);

	setflag(CARRY,carryout);
	setflag(OVERFLOW,overflow);
	setflag(ZERO,zero);
	setflag(NEGATIVE,negative);

	return pc+1;
}

int executeBIN(decoded_instr *instr)
{
	int *dest, *src1, *src2;
	int carryout=0, overflow=0, negative=0, zero=0;

	/* Manage addressing modes (direct only) */
	dest=&(reg[instr->dest]);
	src1=&(reg[instr->src1]);
	src2=&(instr->imm);

	switch (instr->opcode) {
		case ADDI : *dest=*src1 + *src2; 
				if (SIGN(*dest)!=SIGN(*src1) && SIGN(src1)==SIGN(*src2)) overflow=1;
		 break;
		case SUBI : *dest=*src1 - *src2; 
				if (SIGN(*dest)!=SIGN(*src1) && SIGN(src1)!=SIGN(*src2)) overflow=1;
		 break;
		case ANDLI : *dest = *src1 && *src2 ;
		 break;
		case ORLI  : *dest = *src1 || *src2 ;
		 break;
		case EORLI : *dest = (*src1 && !*src2) || (!*src1 && *src2);
		 break;
		case ANDBI : *dest = *src1 & *src2 ;
		 break;
		case ORBI  : *dest = *src1 | *src2 ;
		 break;
		case EORBI : *dest = *src1 ^ *src2 ;
		 break;
		case MULI  : *dest=*src1 * *src2; 
				if (SIGN(*dest)!=SIGN(*src1) && SIGN(src1)==SIGN(*src2)) overflow=1;
		 break;
		case DIVI  : *dest=*src1 / *src2; 
				if (SIGN(*dest)!=SIGN(*src1) && SIGN(src1)==SIGN(*src2)) overflow=1;
		 break;
		case SHLI  : *dest = *src1 << *src2; 
		 break;
		case SHRI  : *dest = *src1 >> *src2; 
		 break;
		case ROTLI: ;
		 break;
		case ROTRI: ;
		 break;
		case NOTL : *dest = !*src1 ;
		 break;
		case NOTB : *dest = ~*src1 ;
		 break;
		default : return INVALID_INSTR;
	}
	zero = (*dest == 0);
	negative = (*dest < 0);

	setflag(CARRY,carryout);
	setflag(OVERFLOW,overflow);
	setflag(ZERO,zero);
	setflag(NEGATIVE,negative);
	
	return pc+1;
}

int executeUNR(decoded_instr *instr){
	int *dest, src ;

	/* Manage addressing modes (direct only) */
	dest=&reg[instr->dest];
	src=instr->addr;

	switch (instr->opcode) {
		case NOP : /* NOP */
			break;
		case HALT : /* HALT */
			return _HALT;
			break;
		case MOVA : *dest = src ; /* Move a 20-bit constant to a register */
			break;
      case LOAD : *dest = mem[src];
         break;
      case STORE : mem[src] = *dest;
         break;
		case JSR : /* Not implemented yet */ ;
			break;
		case RET : /* Not implemented yet */ ;
			break;
      case SEQ :  *dest = getflag(ZERO);
                  setflag(ZERO, (*dest == 0));
                  setflag(NEGATIVE, 0);
                  setflag(CARRY, 0);
                  setflag(OVERFLOW, 0);
                  break;
      case SGT :  *dest = ( ( getflag(NEGATIVE) && getflag(OVERFLOW)
                              && (!getflag(ZERO)))
                           || (!getflag(NEGATIVE) && !getflag(OVERFLOW)
                              && !getflag(ZERO)) );
                  setflag(ZERO, (*dest == 0));
                  setflag(NEGATIVE, 0);
                  setflag(CARRY, 0);
                  setflag(OVERFLOW, 0);
                  break;
      case SGE :  *dest = ((getflag(NEGATIVE) && getflag(OVERFLOW))
                           || (!getflag(NEGATIVE) && !getflag(OVERFLOW)));
                  setflag(ZERO, (*dest == 0));
                  setflag(NEGATIVE, 0);
                  setflag(CARRY, 0);
                  setflag(OVERFLOW, 0);
                  break;
      case SLE :  *dest = (getflag(ZERO) || (getflag(NEGATIVE) && !getflag(OVERFLOW))
                               || (!getflag(NEGATIVE) && getflag(OVERFLOW)));
                  setflag(ZERO, (*dest == 0));
                  setflag(NEGATIVE, 0);
                  setflag(CARRY, 0);
                  setflag(OVERFLOW, 0);
                  break;
      case SLT :  *dest = ( (getflag(NEGATIVE) && !getflag(OVERFLOW))
                               || (!getflag(NEGATIVE) && getflag(OVERFLOW)));
                  setflag(ZERO, (*dest == 0));
                  setflag(NEGATIVE, 0);
                  setflag(CARRY, 0);
                  setflag(OVERFLOW, 0);
                  break;
      case SNE :  *dest = !getflag(ZERO);
                  setflag(ZERO, (*dest == 0));
                  setflag(NEGATIVE, 0);
                  setflag(CARRY, 0);
                  setflag(OVERFLOW, 0);
                  break;
      case READ : fputs("int value? >", stdout);
                  fscanf(stdin, "%d", dest);
         break;
      case WRITE : fprintf(stdout, "%d\n", (*dest) );
         break;
		default : return INVALID_INSTR;
	}

   /* update the value of program counter */
	return pc+1;
}

int executeJMP(decoded_instr *instr){
	/*
	 * Handles Bcc instructions 
	 * (see M68000 docs for an overview of the possible branches)
	 */
	int src;
   int taken;

   /* initialize src */
   src = instr->addr;
   taken = 0;
   
	switch (instr->opcode) {
		case BT : if (1) taken = 1;
			break;
		case BF : if (0) taken = 1;
			break;
		case BHI : if (!(getflag(CARRY)||getflag(ZERO))) taken = 1;
			break;
		case BLS : if (getflag(CARRY)||getflag(ZERO)) taken = 1;
			break;
		case BCC : if (!getflag(CARRY)) taken = 1;
			break;
		case BCS : if (getflag(CARRY)) taken = 1;
			break;
		case BNE : if (!getflag(ZERO)) taken = 1;
			break;
		case BEQ : if (getflag(ZERO)) taken = 1;
			break;
		case BVC : if (!getflag(OVERFLOW)) taken = 1;
			break;
		case BVS : if (getflag(OVERFLOW)) taken = 1;
			break;
		case BPL : if (!getflag(NEGATIVE)) taken = 1;
			break;
		case BMI : if (getflag(NEGATIVE)) taken = 1;
			break;
		case BGE : if (!(getflag(NEGATIVE)^getflag(OVERFLOW))) taken = 1;
			break;
		case BLT : if (getflag(NEGATIVE)^getflag(OVERFLOW)) taken = 1;
			break;
		case BGT : if (   !(getflag(ZERO)
                        ||(getflag(NEGATIVE)^getflag(OVERFLOW)))) taken = 1;
			break;
		case BLE : if (getflag(ZERO)||(getflag(NEGATIVE)^getflag(OVERFLOW))) taken = 1;
			break;
		default : return INVALID_INSTR;
	}

   /* test if the branch is taken or not */
   if (taken)
	   pc += src;
   else
      pc++;

   return pc;
}


int handle_special_instruction(decoded_instr *instr){
	/* here should be inserted code to handle special instructions
	 * using the function bits
	 */
	return INVALID_INSTR;
}
