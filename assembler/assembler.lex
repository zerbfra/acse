
%{
/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * assembler.lex
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "asm_struct.h"
#include "collections.h"
#include "assembler.tab.h"

extern int line_num;
extern int num_error;

%}

%x comment
%s DATA
%option noyywrap

DIGIT	[0-9]
ID	[a-zA-Z_][a-zA-Z0-9_]*

%%
[ \t\f\v]+		{ /* Ignore whitespace. */ }
"/*"                    {  BEGIN(comment); return BEGIN_COMMENT; }
<comment>[^*]*          { yylval.svalue = strdup(yytext); return COMMENT; }
<comment>"*"+[^*/]*     { yylval.svalue = strdup(yytext); return COMMENT; }
<comment>"*"+"/"        {  BEGIN(INITIAL); return END_COMMENT; }


"("		{ return LPAR; }
")"		{ return RPAR; }
":"		{ return COLON; }
"#"		{ return BEGIN_IMMEDIATE; }
"-"      { return MINUS; }

"add"|"ADD"		{yylval.opcode = ADD_OP; return OPCODE3; }
"sub"|"SUB"		{yylval.opcode = SUB_OP; return OPCODE3; }
"andl"|"ANDL"	{yylval.opcode = ANDL_OP; return OPCODE3; }
"orl"|"ORL"		{yylval.opcode = ORL_OP; return OPCODE3; }
"xorl"|"XORL"	{yylval.opcode = XORL_OP; return OPCODE3; }
"andb"|"ANDB"	{yylval.opcode = ANDB_OP; return OPCODE3; }
"orb"|"ORB"		{yylval.opcode = ORB_OP; return OPCODE3; }
"xorb"|"XORB"	{yylval.opcode = XORB_OP; return OPCODE3; }
"mul"|"MUL"		{yylval.opcode = MUL_OP; return OPCODE3; }
"div"|"DIV"		{yylval.opcode = DIV_OP; return OPCODE3; }
"shl"|"SHL"		{yylval.opcode = SHL_OP; return OPCODE3; }
"shr"|"SHR"		{yylval.opcode = SHR_OP; return OPCODE3; }
"rotl"|"ROTL"	{yylval.opcode = ROTL_OP; return OPCODE3; }
"rotr"|"ROTR"	{yylval.opcode = ROTR_OP; return OPCODE3; }
"neg"|"NEG"		{yylval.opcode = NEG_OP; return OPCODE3; }
"spcl"|"SPCL"		{yylval.opcode = SPCL_OP; return OPCODE3; }
"addi"|"ADDI"		{yylval.opcode = ADDI_OP; return OPCODE2; }
"subi"|"SUBI"		{yylval.opcode = SUBI_OP; return OPCODE2; }
"andli"|"ANDLI"	{yylval.opcode = ANDLI_OP; return OPCODE2; }
"orli"|"ORLI"		{yylval.opcode = ORLI_OP; return OPCODE2; }
"xorli"|"XORLI"	{yylval.opcode = XORLI_OP; return OPCODE2; }
"andbi"|"ANDBI"	{yylval.opcode = ANDBI_OP; return OPCODE2; }
"orbi"|"ORBI"		{yylval.opcode = ORBI_OP; return OPCODE2; }
"xorbi"|"XORBI"	{yylval.opcode = XORBI_OP; return OPCODE2; }
"muli"|"MULI"		{yylval.opcode = MULI_OP; return OPCODE2; }
"divi"|"DIVI"		{yylval.opcode = DIVI_OP; return OPCODE2; }
"shli"|"SHLI"		{yylval.opcode = SHLI_OP; return OPCODE2; }
"shri"|"SHRI"		{yylval.opcode = SHRI_OP; return OPCODE2; }
"rotli"|"ROTLI"	{yylval.opcode = ROTLI_OP; return OPCODE2; }
"rotri"|"ROTRI"	{yylval.opcode = ROTRI_OP; return OPCODE2; }
"notl"|"NOTL"		{yylval.opcode = NOTL_OP; return OPCODE2; }
"notb"|"NOTB"		{yylval.opcode = NOTB_OP; return OPCODE2; }
"nop"|"NOP"		   {yylval.opcode = NOP_OP; return NOP; }
"mova"|"MOVA"		{yylval.opcode = MOVA_OP; return OPCODEI; }
"load"|"LOAD"     {yylval.opcode = LOAD_OP; return OPCODEI; }
"store"|"STORE"   {yylval.opcode = STORE_OP; return OPCODEI; }
"jsr"|"JSR"		   {yylval.opcode = JSR_OP; return OPCODEI; }
"ret"|"RET"		   {yylval.opcode = RET_OP; return OPCODEI; }
"seq"|"SEQ"       {yylval.opcode = SEQ_OP; return OPCODEI; }
"sge"|"SGE"       {yylval.opcode = SGE_OP; return OPCODEI; }
"sgt"|"SGT"       {yylval.opcode = SGT_OP; return OPCODEI; }
"sle"|"SLE"       {yylval.opcode = SLE_OP; return OPCODEI; }
"slt"|"SLT"       {yylval.opcode = SLT_OP; return OPCODEI; }
"sne"|"SNE"       {yylval.opcode = SNE_OP; return OPCODEI; }
"read"|"READ"     {yylval.opcode = READ_OP; return OPCODEI; }
"write"|"WRITE"   {yylval.opcode = WRITE_OP; return OPCODEI; }
"halt"|"HALT"	   {yylval.opcode = HALT_OP; return HALT; }
"bt"|"BT"		   {yylval.opcode = BT_OP; return CCODE; }
"bf"|"BF"		   {yylval.opcode = BF_OP; return CCODE; }
"bhi"|"BHI"		   {yylval.opcode = BHI_OP; return CCODE; }
"bls"|"BLS"		   {yylval.opcode = BLS_OP; return CCODE; }
"bcc"|"BCC"		   {yylval.opcode = BCC_OP; return CCODE; }
"bcs"|"BCS"		   {yylval.opcode = BCS_OP; return CCODE; }
"bne"|"BNE"		   {yylval.opcode = BNE_OP; return CCODE; }
"beq"|"BEQ"		   {yylval.opcode = BEQ_OP; return CCODE; }
"bvc"|"BVC"		   {yylval.opcode = BVC_OP; return CCODE; }
"bvs"|"BVS"		   {yylval.opcode = BVS_OP; return CCODE; }
"bpl"|"BPL"		   {yylval.opcode = BPL_OP; return CCODE; }
"bmi"|"BMI"		   {yylval.opcode = BMI_OP; return CCODE; }
"bge"|"BGE"		   {yylval.opcode = BGE_OP; return CCODE; }
"blt"|"BLT"		   {yylval.opcode = BLT_OP; return CCODE; }
"bgt"|"BGT"		   {yylval.opcode = BGT_OP; return CCODE; }
"ble"|"BLE"		   {yylval.opcode = BLE_OP; return CCODE; }

".data"|".DATA"            BEGIN(DATA); {line_num++; }
".text"|".TEXT"            BEGIN(INITIAL); {line_num++; }
<DATA>".word"|".WORD"		{return _WORD;}
<DATA>".space"|".SPACE"    {return _SPACE;}

["R"|"r"]{DIGIT}+    { yylval.immediate = atoi(&yytext[1]); return REG; }

\n						{ /* DOES NOTHING */ }
{DIGIT}+          { yylval.immediate = atoi(yytext); return IMM; };
{ID}              { yylval.svalue = strdup(yytext); return ETI; };
.						{ return(yytext[0]); num_error++; }
%%
