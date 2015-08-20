%{
/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * assembler.y
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc/malloc.h>
#include "asm_struct.h"
#include "asm_engine.h"

/* Variables declared for error tracking */
int line_num;
int num_error;

/* other global variables */
t_translation_infos *infos;

/* functions declared into assembler.y */
char * AsmErrorToString(int errorcode);

%}

%expect 3
      
%union{
	char *svalue;
	int immediate;
	int opcode;
	t_asm_address *address;
	t_asm_data *dataVal;
   t_asm_instruction *instr;
   t_asm_label *label;
   t_asm_register *reg;
}

%start asm

/* definitions */
%token <opcode> OPCODE3
%token <opcode> OPCODE2
%token <opcode> OPCODEI
%token <opcode> CCODE
%token <opcode> HALT
%token <opcode> NOP
%token _WORD
%token _SPACE
%token <immediate>REG
%token LPAR
%token RPAR
%token COLON
%token MINUS
%token BEGIN_IMMEDIATE
%token BEGIN_COMMENT
%token END_COMMENT
%token <svalue> COMMENT
%token <svalue> ETI
%token <immediate> IMM

%type <dataVal> data_value
%type <reg> register
%type <immediate> immediate
%type <address> address
%type <svalue> comment
%type <label> label_decl
%type <instr> instr

%%

asm :       asm data_segm instruction_segm { /* DOES NOTHING */}
            | data_segm instruction_segm     { /* DOES NOTHING */}
            | instruction_segm               { /* DOES NOTHING */}
;

instruction_segm :   instruction_segm instruction   { line_num++; }
                     | instruction                  { line_num++; }
;

instruction :  instr comment           { /* DOES NOTHING */}
            | label_decl instr comment {
               /* assign the label to the current instruction */
               $1->data = (void *) $2;
            }
            | instr            { /* DOES NOTHING */}
            | label_decl instr {
               /* assign the label to the current instruction */
               $1->data = (void *) $2;
            }
;

instr : OPCODE3 register REG register  {
         t_asm_register *reg;
         int asm_errorcode;
         
         /* alloc memory for a register info. */
         reg = allocRegister($3, 0);
   
         /* register shouldn't be a NULL pointer */
         if (reg == NULL)
         {
            /* an out of memory occurred */
            yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
            /* stop the parser */
            YYABORT;
         }

         /* initialize an instruction with three operands' */
         $$ = init_opcode3($1, $2, reg, $4);
         
         /* $$ shouldn't be a NULL pointer */
         if ($$ == NULL)
         {
            /* an out of memory occurred */
            yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
            /* stop the parser */
            YYABORT;
         }

         /* add the newly created instruction to the code segment */
         asm_errorcode = addInstruction(infos, $$);
         if (asm_errorcode != ASM_OK)
         {
               /* an error occurred */
               yyerror(AsmErrorToString(asm_errorcode));
         }
      }
      | OPCODE2 REG REG immediate {
         t_asm_register *register_1;
         t_asm_register *register_2;
         int asm_errorcode;
         
         /* alloc memory for a register info. */
         register_1 = allocRegister($2, 0);
         register_2 = allocRegister($3, 0);
   
         /* registers shouldn't be a NULL pointer */
         if (register_1 == NULL || register_2 == NULL)
         {
            /* an out of memory occurred */
            yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
            /* stop the parser */
            YYABORT;
         }

         /* initialize an instruction with two operands' */
         $$ = init_opcode2($1, register_1, register_2, $4);
         
         /* $$ shouldn't be a NULL pointer */
         if ($$ == NULL)
         {
            /* an out of memory occurred */
            yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
            /* stop the parser */
            YYABORT;
         }

         /* add the newly created instruction to the code segment */
         asm_errorcode = addInstruction(infos, $$);
         if (asm_errorcode != ASM_OK)
         {
               /* an error occurred */
               yyerror(AsmErrorToString(asm_errorcode));
         }
      }
      | OPCODEI REG address {
         t_asm_register *reg;
         int asm_errorcode;
         
         /* alloc memory for a register info. */
         reg = allocRegister($2, 0);
   
         /* register shouldn't be a NULL pointer */
         if (reg == NULL)
         {
            /* an out of memory occurred */
            yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
            /* stop the parser */
            YYABORT;
         }

         /* initialize an instruction with a single operand' */
         $$ = init_opcodeI($1, reg, $3);
         
         /* $$ shouldn't be a NULL pointer */
         if ($$ == NULL)
         {
            /* an out of memory occurred */
            yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
            /* stop the parser */
            YYABORT;
         }

         /* add the newly created instruction to the code segment */
         asm_errorcode = addInstruction(infos, $$);
         if (asm_errorcode != ASM_OK)
         {
               /* an error occurred */
               yyerror(AsmErrorToString(asm_errorcode));
         }
      }
      | CCODE address {
         int asm_errorcode;
         
         /* initialize a branch instruction */
         $$ = init_ccode($1, $2);
         
         /* $$ shouldn't be a NULL pointer */
         if ($$ == NULL)
         {
            /* an out of memory occurred */
            yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
            /* stop the parser */
            YYABORT;
         }

         /* add the newly created instruction to the code segment */
         asm_errorcode = addInstruction(infos, $$);
         if (asm_errorcode != ASM_OK)
         {
               /* an error occurred */
               yyerror(AsmErrorToString(asm_errorcode));
         }
      }
      | HALT {
         int asm_errorcode;
         
         /* initialize a HALT instruction */
         $$ = init_halt();

         /* $$ shouldn't be a NULL pointer */
         if ($$ == NULL)
         {
            /* an out of memory occurred */
            yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
            /* stop the parser */
            YYABORT;
         }

         /* add the newly created instruction to the code segment */
         asm_errorcode = addInstruction(infos, $$);
         if (asm_errorcode != ASM_OK)
         {
               /* an error occurred */
               yyerror(AsmErrorToString(asm_errorcode));
         }
      }
      | NOP {
         int asm_errorcode;
         
         /* initialize a NOP instruction */
         $$ = init_nop();

         /* $$ shouldn't be a NULL pointer */
         if ($$ == NULL)
         {
            /* an out of memory occurred */
            yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
            /* stop the parser */
            YYABORT;
         }

         /* add the newly created instruction to the code segment */
         asm_errorcode = addInstruction(infos, $$);
         if (asm_errorcode != ASM_OK)
         {
               /* an error occurred */
               yyerror(AsmErrorToString(asm_errorcode));
         }
      }
;

data_segm : data_segm data_def   { line_num++; }
            | data_def comment   { line_num++; }
            | data_def           { line_num++; }
;

data_def : label_decl data_value {
                  int asm_errorcode;

                  /* insert data into the data segment */
                  asm_errorcode = addData(infos, $2);
                  if (asm_errorcode != ASM_OK)
                  {
                     /* an error occurred */
                     yyerror(AsmErrorToString(asm_errorcode));
                  }

                  /* assign the label to the current block of data */
                  $1->data = (void *) $2;
         }
         | data_value {
                  int asm_errorcode;

                  /* insert data into the data segment */
                  asm_errorcode = addData(infos, $1);
                  if (asm_errorcode != ASM_OK)
                  {
                     /* an error occurred */
                     yyerror(AsmErrorToString(asm_errorcode));
                  }
         }
;

label_decl : ETI COLON {
            int asm_errorcode;
            
            /* look for the requested label */
            $$ = findLabel(infos, $1, &asm_errorcode);
            if (asm_errorcode != ASM_OK)
            {
               /* an error occurred */
               yyerror(AsmErrorToString(asm_errorcode));
            }
            
            if ($$ == NULL)
            {
               t_asm_label *label;
            
               /* label was never defined before */
               label = allocLabel($1, NULL);

               /* label shouldn't be a NULL pointer */
               if (label == NULL)
               {
                  /* an out of memory occurred */
                  yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
                  /* stop the parser */
                  YYABORT;
               }

               /* insert the new label */
               asm_errorcode = insertLabel(infos, label);

               /* set the $$ value */
               $$ = label;
            }
            else
               free($1);

            /* notify if an error occurred */
            if (asm_errorcode != ASM_OK)
               yyerror(AsmErrorToString(asm_errorcode));
         }
;

data_value : _WORD IMM {
               /* create an instance of `t_asm_data' */
               $$ = allocData(ASM_WORD, $2);

               /* $$ shouldn't be a NULL pointer */
               if ($$ == NULL)
               {
                  /* an out of memory occurred */
                  yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
                  /* stop the parser */
                  YYABORT;
               }
           }
           | _SPACE IMM {
               /* create an instance of `t_asm_data' */
               $$ = allocData(ASM_SPACE, $2);

               /* $$ shouldn't be a NULL pointer */
               if ($$ == NULL)
               {
                  /* an out of memory occurred */
                  yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
                  /* stop the parser */
                  YYABORT;
               }
           }
;
		
register : REG {
               /* alloc memory for a register info. */
               $$ = allocRegister($1, 0);
   
               /* $$ shouldn't be a NULL pointer */
               if ($$ == NULL)
               {
                  /* an out of memory occurred */
                  yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
                  /* stop the parser */
                  YYABORT;
               }
         }
         | LPAR REG RPAR {
               /* alloc memory for a register info. */
               $$ = allocRegister($2, 1);
   
               /* label shouldn't be a NULL pointer */
               if ($$ == NULL)
               {
                  /* an out of memory occurred */
                  yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
                  /* stop the parser */
                  YYABORT;
               }
         }
;

immediate : BEGIN_IMMEDIATE IMM { $$ = $2; }
          | BEGIN_IMMEDIATE MINUS IMM {$$ = - $3; }
;

address  : ETI {
            int asm_errorcode;
            t_asm_label *label_found;
            
            /* look for the requested label */
            label_found = findLabel(infos, $1, &asm_errorcode);
            if (asm_errorcode != ASM_OK)
            {
               /* an error occurred */
               yyerror(AsmErrorToString(asm_errorcode));
            }
            
            if (label_found == NULL)
            {
               /* label was never defined before */
               label_found = allocLabel($1, NULL);

               /* label shouldn't be a NULL pointer */
               if (label_found == NULL)
               {
                  /* an out of memory occurred */
                  yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
                  /* stop the parser */
                  YYABORT;
               }

               /* insert the new label */
               asm_errorcode = insertLabel(infos, label_found);

               /* notify if an error occurred */
               if (asm_errorcode != ASM_OK)
                  yyerror(AsmErrorToString(asm_errorcode));
            }
            else
               free($1);

            /* set the $$ value */
            $$ = allocAddress(0, label_found);
            if ($$ == NULL)
            {
                  /* an out of memory occurred */
                  yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
                  /* stop the parser */
                  YYABORT;
            }
         }
         | IMM {
            $$ = allocAddress($1, NULL);
            if ($$ == NULL)
            {
                  /* an out of memory occurred */
                  yyerror(AsmErrorToString(ASM_OUT_OF_MEMORY));
               
                  /* stop the parser */
                  YYABORT;
            }
         }
;

comment  : BEGIN_COMMENT COMMENT END_COMMENT { $$ = $2; }
;


%%
		 
int yyerror(const char* errmsg)
{
	fprintf(stdout, "line %d , error: %s \n", line_num, errmsg);
	num_error++;
	return 0;
}

int main (int argc, char **argv)
{
   int errorcode;
   char *filename;
   FILE *input_file;
	extern FILE *yyin;

   argc--;
   argv++;
	
   if (argc > 0)
   {
      input_file = fopen(argv[0], "r");
      if (input_file == NULL)
      {
         fprintf( stdout, "File not found : %s.\n", argv[0]);
         exit(-1);
      }
      yyin = input_file;
   }
   else
      yyin = stdin;
	
   if (argc <= 1)
      filename = "output.o";
   else
      filename = argv[1];
	
#ifndef NDEBUG
   fprintf(stdout, "Initializing the assembler data structures.\n");
#endif
   /* initialize the translation infos */
   infos = initStructures(&errorcode);

   /* test if an error occurred */
   if (errorcode != ASM_OK)
   {
         fprintf( stdout, "An error occurred while initializing the assembler.\n"
                  "%s. \n", AsmErrorToString(errorcode));

         /* we can't start the parsing procedure since a critical error occurred */
         abort();
   }

   /* initialize the global variables */
   line_num = 1;
   num_error = 0;
   
#ifndef NDEBUG
   if (argc > 0)
      fprintf(stdout, "Starting the parsing process. "
                        "Input file : %s.\n", argv[0]);
   else
      fprintf(stdout, "Starting the parsing process. "
                        "Input file : standard input.\n");
#endif
   /* start the parsing procedure */
   yyparse();

#ifndef NDEBUG
   fprintf(stdout, "Parsing complete. \n");
#endif
	
   /* test if the parsing job found some errors */
   if (num_error == 0)
   {
#ifndef NDEBUG
      fprintf(stdout, "Output will be written on file : %s. \n", filename);
#endif
      errorcode = asm_writeObjectFile(infos, filename);
      if (errorcode != ASM_OK)
      {
         fprintf( stdout, "An error occurred while writing the object file.\n"
                  "%s. \n", AsmErrorToString(errorcode));
      }
#ifndef NDEBUG
      else
         fprintf(stdout, "Output written on file \"%s\".\n", filename);
#endif
   }
   else {
      fprintf(stdout, "\nInput file contains some error(s). No object file written\n");
      fprintf(stdout, "**%d errors found \n\n", num_error);
   }

#ifndef NDEBUG
   fprintf(stdout, "Finalizing the data structures. \n");
#endif

   /* shutdown the asm engine */
   if (infos != NULL)
   {
         errorcode = finalizeStructures(infos);
         if (errorcode != ASM_OK)
         {
            fprintf(stdout, "SHUTDOWN:: error while finalizing "
                  "the data structures. \n%s. \n", AsmErrorToString(errorcode));
         }
   }

#ifndef NDEBUG
   fprintf(stdout, "Done. \n");
#endif
   
   return 0;
}

char * AsmErrorToString(int errorcode)
{
   switch(errorcode)
   {
      case ASM_NOT_INITIALIZED_INFO : return "global infos not initialized";
      case ASM_UNDEFINED_INSTRUCTION : return "invalid instruction found";
      case ASM_INVALID_LABEL_FOUND : return "invalid label found";
      case ASM_LABEL_ALREADY_PRESENT : return "label already defined";
      case ASM_UNDEFINED_DATA : return "invalid data found";
      case ASM_INVALID_DATA_FORMAT : return "invalid data format";
      case ASM_FOPEN_ERROR : return "fopen failed";
      case ASM_INVALID_INPUT_FILE : return "invalid file pointer";
      case ASM_FWRITE_ERROR : return "fwrite failed";
      case ASM_FCLOSE_ERROR : return "fclose failed";
      case ASM_CODE_NOT_PRESENT : return "code segment is empty";
      case ASM_OUT_OF_MEMORY : return "out of memory";
      case ASM_INVALID_OPCODE : return "invalid opcode found";
      case ASM_UNKNOWN_ERROR : return "unknown error found";
      default : return "<invalid errorcode>";
   }
}
