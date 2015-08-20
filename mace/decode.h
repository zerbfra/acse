/*
 * Giovanni Agosta, Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * decode.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */
#ifndef _DECODE_H
#define _DECODE_H

#include <stdio.h>
#include "getbits.h"

#define FORMAT_TER 0
#define FORMAT_BIN 1
#define FORMAT_UNR 2
#define FORMAT_CC 3

typedef struct _instr {
	int opcode;
	int dest;
	int src1;
	int src2;
	int addr;
	int imm;
	int func;
	int format;
} decoded_instr;



/*
 * Formats ([..] optional elements, default at 0):
 *  opcode dest src1 src2 [func]
 *  opcode dest src1 [imm]
 *  opcode dest [addr]
 *  ccode [dest] addr
 */
decoded_instr *decode(int the_instr);

int carry(decoded_instr *instr);

int sign(decoded_instr *instr);

int indirect_dest(decoded_instr *instr);

int indirect_src2(decoded_instr *instr);

void print(FILE* file, decoded_instr *instr);

void normalizeValues(decoded_instr *instr);
/*

Formats
00 opcode dest src1 [src2] [func]
	0000 ADD
	0001 SUB
	0010 ANDL
	0011 ORL
	0100 XORL
	0101 ANDB
	0110 ORB
	0111 XORB
	1000 MUL
	1001 DIV
	1010 SHL
	1011 SHR
	1100 ROTL
	1101 ROTR
	1110 NEG
	1111 (SPCL, used for extensions)

func bits
	0 add carry
	1 signed/unsigned
	2 indirect addressing dest
	3 indirect addressing src2
	4
	5
	6
	7
	8
	9
	10
	11

01 opcode dest src1 [imm]
	0000 ADDI
	0001 SUBI
	0010 ANDLI
	0011 ORLI
	0100 XORLI
	0101 ANDBI
	0110 ORBI
	0111 XORBI
	1000 MULI
	1001 DIVI
	1010 SHLI
	1011 SHRI
	1100 ROTLI
	1101 ROTRI
	1110 NOTL
	1111 NOTB

10 opcode [dest] [addr]
	0000 NOP
	0001 MOVA
	0010 JSR
	0011 RET
	0100 LOAD
	0101 STORE
	0110 HALT
	0111 SEQ
	1000 SGE
	1001 SGT
	1010 SLE
	1011 SLT
	1100 SNE
	1101 READ
	1110 WRITE
	1111 

11 ccode [dest] addr
	Branch to:
	0000 T
	0001 F
	0010 HI
	0011 LS
	0100 CC
	0101 CS
	0110 NE
	0111 EQ
	1000 VC
	1001 VS
	1010 PL
	1011 MI
	1100 GE
	1101 LT
	1110 GT
	1111 LE
	
*/
#endif /* _DECODE_H */
