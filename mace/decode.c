/*
 * Giovanni Agosta, Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * decode.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */
#include <stdlib.h>
#include "decode.h"

/* Code Names for debug printing  */
char *names[4][16] = { 
	{ "ADD", "SUB", "ANDL", "ORL", "EORL", "ANDB", "ORB", "EORB", "MUL", "DIV", "SHL", "SHR", "ROTL", "ROTR", "NEG", "SPCL" },
	{ "ADDI", "SUBI", "ANDLI", "ORLI", "EORLI", "ANDBI", "ORBI", "EORBI", "MULI", "DIVI", "SHLI", "SHRI", "ROTLI", "ROTRI", "NOTL", "NOTB" },
	{ "NOP", "MOVA", "JSR", "RET", "LOAD", "STORE", "HALT", "SEQ", "SGE", "SGT", "SLE", "SLT", "SNE", "READ", "WRITE", "ILL15" },
	{ "BT", "BF", "BHI", "BLS", "BCC", "BCS", "BNE", "BEQ", "BVC", "BVS", "BPL", "BMI", "BGE", "BLT", "BGT", "BLE" } 
};

decoded_instr *decode(int the_instr) {
		decoded_instr *instr = (decoded_instr *) malloc(sizeof(decoded_instr));
		/* decodifica */
		instr->format = getbits(the_instr,31,30); /* Instruction format (2 bits) */
		instr->opcode = getbits(the_instr,29,26); /* Opcode (4 bits) */
		instr->dest   = getbits(the_instr,25,21); /* Destination register */
		instr->src1   = getbits(the_instr,20,16); /* Source register 1 */
		instr->src2   = getbits(the_instr,15,11); /* Source register 2 */
		instr->imm    = getbits(the_instr,15, 0); /* 16 bits immediate value */
		instr->addr   = getbits(the_instr,19, 0); /* 20 bits immediate address */
		instr->func   = getbits(the_instr,10, 0); /* 11 extra bits, used to encode extra info */
      normalizeValues(instr);
		return instr;
}		

int carry(decoded_instr *instr){
	return instr->func & 1;
}

int sign(decoded_instr *instr){
	return instr->func & 2;
}

int indirect_dest(decoded_instr *instr){
	return instr->func & 4;
}

int indirect_src2(decoded_instr *instr){
	return instr->func & 8;
}

/* Debug printf: print out the current instruction */
void print(FILE* file, decoded_instr *instr)
{
   /* default value for file is stderr */
   if (file == NULL)
      file = stderr;

   /* precondition: instr should be different from NULL */
   if (instr == NULL)
   {
      fprintf(file,"[NULL POINTER] \n");
      return;
   }

   switch (instr->format)
   {
      case FORMAT_TER :
         fprintf(file,"%s ", names[instr->format][instr->opcode]);
         if (indirect_dest(instr))
            fprintf(file,"(R%d) ", instr->dest);
         else
            fprintf(file,"R%d ", instr->dest);
         fprintf(file,"R%d ", instr->src1);
         if (indirect_src2(instr))
            fprintf(file,"(R%d) \n", instr->src2);
         else
            fprintf(file,"R%d \n", instr->src2);
         break;
      case FORMAT_BIN :
         fprintf(file,"%s R%d, R%d, #%d \n"
            , names[instr->format][instr->opcode], instr->dest
                  , instr->src1, instr->imm);
         break;
      case FORMAT_UNR :
         if (instr->opcode == 0 || instr->opcode == 6)
            fprintf(file,"%s \n", names[instr->format][instr->opcode]);
         else if (instr->opcode == 13 || instr->opcode == 14)
            fprintf(file,"%s R%d \n"
                  , names[instr->format][instr->opcode], instr->dest);
         else
            fprintf(file,"%s R%d, $%d \n", names[instr->format][instr->opcode]
                        , instr->dest, instr->addr);
         break;
      case FORMAT_CC :
         fprintf(file,"%s $%d \n"
            , names[instr->format][instr->opcode], instr->addr);
         break;
      default :
         fprintf(file,"[UNKNOWN INSTRUCTION FORMAT] \n");
         break;
   }
}

void normalizeValues(decoded_instr *instr)
{
   instr->imm = (instr->imm << 16);
   instr->imm = (instr->imm >> 16);
   instr->addr = (instr->addr << 12);
   instr->addr = (instr->addr >> 12);
}
