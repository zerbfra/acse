%{
/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * Acse.y
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

/*************************************************************************

                   Compiler for the language LANCE

***************************************************************************/

#include <stdio.h>       
#include <stdlib.h>
#include <assert.h>
#include <malloc/malloc.h>
#include "axe_struct.h"
#include "axe_engine.h"
#include "symbol_table.h"
#include "axe_errors.h"
#include "collections.h"
#include "axe_expressions.h"
#include "axe_gencode.h"
#include "axe_utils.h"
#include "axe_array.h"
#include "axe_cflow_graph.h"
#include "cflow_constants.h"
#include "axe_transform.h"
#include "axe_reg_alloc.h"
#include "reg_alloc_constants.h"
#include "axe_io_manager.h"
#ifndef NDEBUG
#  include "axe_debug.h"
#endif



/* global variables */
int line_num;        /* this variable will keep track of the
                      * source code line number. Every time that a newline
                      * is encountered while parsing the input file, this
                      * value is increased by 1. This value is then used
                      * for error tracking: if the parser returns an error
                      * or a warning, this value is used in order to notify
                      * in which line of code the error has been found */
int num_error;       /* the number of errors found in the code. This value
                      * is increased by 1 every time a new error is found
                      * in the code. */
int num_warning;     /* As for the `num_error' global variable, this one
                      * keeps track of all the warning messages displayed */

/* errorcode is defined inside "axe_engine.c" */
extern int errorcode;   /* this variable is used to test if an error is found
                         * while parsing the input file. It also is set
                         * to notify if the compiler internal state is invalid.
                         * When the parsing process is started, the value
                         * of `errorcode' is set to the value of the macro
                         * `AXE_OK' defined in "axe_constants.h".
                         * As long as everything (the parsed source code and
                         * the internal state of the compiler) is correct,
                         * the value of `errorcode' is set to `AXE_OK'.
                         * When an error occurs (because the input file contains
                         * one or more syntax errors or because something went
                         * wrong in the machine internal state), the errorcode
                         * is set to a value that is different from `AXE_OK'. */
                         

extern int cflow_errorcode;   /* As for `errorcode' this value is used to
                        * test if an error occurs during the creation process of
                        * a control flow graph. More informations can be found
                        * analyzing the file `axe_cflow_graph.h'. */
                     
/* program informations */
t_program_infos *program;  /* The singleton instance of `program'.
                            * An instance of `t_program_infos' holds in its
                            * internal structure, all the useful informations
                            * about a program. For example: the assembly
                            * (code and directives); the symbol table;
                            * the label manager (see axe_labels.h) etc. */
t_cflow_Graph *graph;      /* An instance of a control flow graph. This instance
                            * will be generated starting from `program' and will
                            * be used during the register allocation process */

t_reg_allocator *RA;       /* Register allocator. It implements the "Linear scan"
                            * algorythm */

t_io_infos *file_infos;    /* input and output files used by the compiler */

%}

%expect 1

/*=========================================================================
                          SEMANTIC RECORDS
=========================================================================*/

%union {            
   int intval;
   char *svalue;
   t_axe_expression expr;
   t_axe_declaration *decl;
   t_list *list;
   t_axe_label *label;
   t_while_statement while_stmt;
   t_unless_statement unless_stmt;
   t_foreach_statement foreach_stmt;
} 
/*=========================================================================
                               TOKENS 
 Nei tokens, alcuni sono preceduti dalla struttura che devono chiamare.
 Ad esempio, il WHILE ha in fronte <while_stmt> che va ad istanziare la relativa
 struttura. Ci si può referenziare a questa usando $1 nella grammatica, andando
 a recuperare le variabili della struttura stessa ($1 perchè è il primo parametro).
 
 Altre, ad esempio il DO, non hanno bisogno di una struttura complessa ma di una
 sola label, perciò si utilizza semplicemente <label> e ci si riferenzia ad essa
 come $1. Si può infatti creare $1 = newLabel(program);
 
 Altre ancora, non hanno nessuna "entità" in fronte: questo perchè non necessitano
 di label per fare dei jump o simili (esempio: RETURN, WRITE...)
 
=========================================================================*/
%start program

%token LBRACE RBRACE LPAR RPAR LSQUARE RSQUARE
%token SEMI COLON PLUS MINUS MUL_OP DIV_OP MOD_OP
%token AND_OP OR_OP NOT_OP
%token ASSIGN LT GT SHL_OP SHR_OP EQ NOTEQ LTEQ GTEQ
%token ANDAND OROR
%token COMMA
%token RETURN
%token READ
%token WRITE

%token <label> DO
%token <while_stmt> WHILE
%token <label> IF
%token <label> ELSE
%token <intval> TYPE
%token <svalue> IDENTIFIER
%token <intval> NUMBER
%token <unless_stmt> EVAL
%token <label> UNLESS
%token <foreach_stmt> FOR

%type <expr> exp
%type <expr> assign_statement
%type <decl> declaration
%type <list> declaration_list
%type <label> if_stmt
%type <label> unless_statement

/*=========================================================================
                          OPERATOR PRECEDENCES
 =========================================================================*/

%left COMMA
%left ASSIGN
%left OROR
%left ANDAND
%left OR_OP
%left AND_OP
%left EQ NOTEQ
%left LT GT LTEQ GTEQ
%left SHL_OP SHR_OP
%left MINUS PLUS
%left MUL_OP DIV_OP MOD_OP
%right NOT

/*=========================================================================
                         BISON GRAMMAR
=========================================================================*/
%% 

/* `program' is the starting non-terminal of the grammar.
 * A program is composed by:
      1. declarations (zero or more);
      2. A list of instructions. (at least one instruction!).
 * When the rule associated with the non-terminal `program' is executed,
 * the parser notify it to the `program' singleton instance. */
program  : var_declarations statements
         {
            /* Notify the end of the program. Once called
             * the function `set_end_Program' - if necessary -
             * introduces a `HALT' instruction into the
             * list of instructions. */
            set_end_Program(program);

            /* return from yyparse() */
            YYACCEPT;
         }
;

var_declarations : var_declarations var_declaration   { /* does nothing */ }
                 | /* empty */                        { /* does nothing */ }
;

var_declaration   : TYPE declaration_list SEMI
                  {
                     /* update the program infos by adding new variables */
                     set_new_variables(program, $1, $2);
                  }
;

declaration_list  : declaration_list COMMA declaration
                  {  /* add the new declaration to the list of declarations */
                     $$ = addElement($1, $3, -1);
                  }
                  | declaration
                  {
                     /* add the new declaration to the list of declarations */
                     $$ = addElement(NULL, $1, -1);
                  }
;

declaration : IDENTIFIER ASSIGN NUMBER
            {
               /* create a new instance of t_axe_declaration */
               $$ = alloc_declaration($1, 0, 0, $3);

               /* test if an `out of memory' occurred */
               if ($$ == NULL)
                  notifyError(AXE_OUT_OF_MEMORY);
            }
            | IDENTIFIER LSQUARE NUMBER RSQUARE
            {
               /* create a new instance of t_axe_declaration */
               $$ = alloc_declaration($1, 1, $3, 0);

                  /* test if an `out of memory' occurred */
               if ($$ == NULL)
                  notifyError(AXE_OUT_OF_MEMORY);
            }
            | IDENTIFIER
            {
               /* create a new instance of t_axe_declaration */
               $$ = alloc_declaration($1, 0, 0, 0);
               
               /* test if an `out of memory' occurred */
               if ($$ == NULL)
                  notifyError(AXE_OUT_OF_MEMORY);
            }
;

/* A block of code can be either a single statement or
 * a set of statements enclosed between braces */
code_block  : statement                  { /* does nothing */ }
            | LBRACE statements RBRACE   { /* does nothing */ }
;

/* One or more code statements */
statements  : statements statement       { /* does nothing */ }
            | statement                  { /* does nothing */ }
;

/* A statement can be either an assignment statement or a control statement
 * or a read/write statement or a semicolon */
statement   : assign_statement SEMI      { /* does nothing */ }
            | control_statement          { /* does nothing */ }
            | read_write_statement SEMI  { /* does nothing */ }
            | SEMI            { gen_nop_instruction(program); }
;

control_statement : if_statement         { /* does nothing */ }
			| unless_statement			 { /* does nothing */ }
            | while_statement            { /* does nothing */ }
            | do_while_statement SEMI    { /* does nothing */ }
			| foreach_statement			 { /* does nothing */ }
            | return_statement SEMI      { /* does nothing */ }
;

read_write_statement : read_statement  { /* does nothing */ }
                     | write_statement { /* does nothing */ }
;

assign_statement : IDENTIFIER LSQUARE exp RSQUARE ASSIGN exp
            {
               storeArrayElement(program, $1, $3, $6);
			   $$ = create_expression($6.value, IMMEDIATE);
               free($1);
            }
            | IDENTIFIER ASSIGN exp
            {
               int location;
               location = get_symbol_location(program, $1, 0);

               if ($3.expression_type == IMMEDIATE) {
					gen_addi_instruction(program, location, REG_0, $3.value);
					$$ = create_expression($3.value, IMMEDIATE);
			   }
               else {
			        gen_add_instruction(program, location, REG_0, $3.value, CG_DIRECT_ALL);
					$$ = create_expression($3.value, REGISTER);
			   }
			   free($1);
            }
;
            
if_statement   : if_stmt
               {
                  /* fix the `label_else' */
                  assignLabel(program, $1);
               }
               | if_stmt ELSE
               {
                  /* reserve a new label that points to the address where to jump if
                   * `exp' is verified */
                  $2 = newLabel(program);
   
                  /* exit from the if-else */
                  gen_bt_instruction (program, $2, 0);
   
                  /* fix the `label_else' */
                  assignLabel(program, $1);
               }
               code_block
               {
                  /* fix the `label_else' */
                  assignLabel(program, $2);
               }
;
            
if_stmt  :  IF
               {
                  /* the label that points to the address where to jump if
                   * `exp' is not verified */
                  $1 = newLabel(program);
               }
               LPAR exp RPAR
               {
                     if ($4.expression_type == IMMEDIATE)
                         gen_load_immediate(program, $4.value);
                     else
                         gen_andb_instruction(program, $4.value,
                             $4.value, $4.value, CG_DIRECT_ALL);

                     /* if `exp' returns FALSE, jump to the label $1 */
                     gen_beq_instruction (program, $1, 0);
               }
               code_block { $$ = $1; }
;

unless_statement : EVAL {
				$1 = create_unless_statement();
				$1.condition = newLabel(program);
				$1.code_block = newLabel(program);
				$1.end = newLabel(program);
				gen_bt_instruction(program, $1.condition, 0);
				assignLabel(program, $1.code_block);
			} 
			code_block {
				gen_bt_instruction(program, $1.end, 0);
				assignLabel(program, $1.condition);
			}
			UNLESS exp {
				if ($6.expression_type == IMMEDIATE) {
					gen_load_immediate(program, $6.value);
				}
				else {
					gen_andb_instruction(program, $6.value, $6.value, $6.value, CG_DIRECT_ALL);
				}
				gen_beq_instruction(program, $1.code_block, 0);
				assignLabel(program, $1.end);
			}
;

while_statement  : WHILE
                  {
                     /* initialize the value of the non-terminal */
                     $1 = create_while_statement();

                     /* reserve and fix a new label */
                     $1.label_condition
                           = assignNewLabel(program);
                  }
                  LPAR exp RPAR
                  {
                     if ($4.expression_type == IMMEDIATE)
                        gen_load_immediate(program, $4.value);
                     else
                         gen_andb_instruction(program, $4.value,
                             $4.value, $4.value, CG_DIRECT_ALL);

                     /* reserve a new label. This new label will point
                      * to the first instruction after the while code
                      * block */
                     $1.label_end = newLabel(program);

                     /* if `exp' returns FALSE, jump to the label $1.label_end */
                     gen_beq_instruction (program, $1.label_end, 0);
                  }
                  code_block
                  {
                     /* jump to the beginning of the loop */
                     gen_bt_instruction
                           (program, $1.label_condition, 0);

                     /* fix the label `label_end' */
                     assignLabel(program, $1.label_end);
                  }
;
                  
do_while_statement  : DO
                     {
                        /* the label that points to the address where to jump if
                         * `exp' is not verified */
                        $1 = newLabel(program);
                        
                        /* fix the label */
                        assignLabel(program, $1);
                     }
                     code_block WHILE LPAR exp RPAR
                     {
                           if ($6.expression_type == IMMEDIATE)
                               gen_load_immediate(program, $6.value);
                           else
                               gen_andb_instruction(program, $6.value,
                                   $6.value, $6.value, CG_DIRECT_ALL);

                           /* if `exp' returns TRUE, jump to the label $1 */
                           gen_bne_instruction (program, $1, 0);
                     }
;

foreach_statement : FOR  
			{
				$1.counter = getNewRegister(program);
				gen_addi_instruction(program, $1.counter, REG_0, 0);
				$1.iteration = newLabel(program);
				$1.end = newLabel(program);
				assignLabel(program, $1.iteration);
			}
			LPAR IDENTIFIER COLON IDENTIFIER RPAR 
			{
				int var_location = get_symbol_location(program, $4, 0);
				t_axe_variable * array_declaration = getVariable(program, $6);
				
				int temp_register = getNewRegister(program);
				gen_subi_instruction(program, temp_register, $1.counter, array_declaration->arraySize);
				gen_beq_instruction(program, $1.end, 0);
					
				int new_value = loadArrayElement(program, $6, create_expression($1.counter, REGISTER));
				gen_add_instruction(program, var_location, REG_0, new_value, CG_DIRECT_ALL);
				free($4);
				free($6);
			}
			code_block 
			{
				gen_addi_instruction(program, $1.counter, $1.counter, 1);
				gen_bt_instruction(program, $1.iteration, 0);
				assignLabel(program, $1.end);
			}
;


return_statement : RETURN
            {
               /* insert an HALT instruction */
               gen_halt_instruction(program);
            }
;

read_statement : READ LPAR IDENTIFIER RPAR 
            {
               int location;
               
               /* read from standard input an integer value and assign
                * it to a variable associated with the given identifier */
               /* get the location of the symbol with the given ID */
               
               /* lookup the symbol table and fetch the register location
                * associated with the IDENTIFIER $3. */
               location = get_symbol_location(program, $3, 0);

               /* insert a read instruction */
               gen_read_instruction (program, location);

               /* free the memory associated with the IDENTIFIER */
               free($3);
            }
;
            
write_statement : WRITE LPAR exp RPAR 
            {
   
               int location;

               if ($3.expression_type == IMMEDIATE)
               {
                  /* load `immediate' into a new register. Returns the new register
                   * identifier or REG_INVALID if an error occurs */
                  location = gen_load_immediate(program, $3.value);
               }
               else
                  location = $3.value;

               /* write to standard output an integer value */
               gen_write_instruction (program, location);
            }
;

exp: NUMBER      { $$ = create_expression ($1, IMMEDIATE); }
   | IDENTIFIER  {
                     int location;
   
                     /* get the location of the symbol with the given ID */
                     location = get_symbol_location(program, $1, 0);
                     
                     /* return the register location of IDENTIFIER as
                      * a value for `exp' */
                     $$ = create_expression (location, REGISTER);

                     /* free the memory associated with the IDENTIFIER */
                     free($1);
   }
   | IDENTIFIER LSQUARE exp RSQUARE {
                     int reg;
                     
                     /* load the value IDENTIFIER[exp]
                      * into `arrayElement' */
                     reg = loadArrayElement(program, $1, $3);

                     /* create a new expression */
                     $$ = create_expression (reg, REGISTER);

                     /* free the memory associated with the IDENTIFIER */
                     free($1);
   }
   | NOT_OP NUMBER   {  if ($2 == 0)
                           $$ = create_expression (1, IMMEDIATE);
                        else
                           $$ = create_expression (0, IMMEDIATE);
   }
   | NOT_OP IDENTIFIER  {
                           int identifier_location;
                           int output_register;
   
                           /* get the location of the symbol with the given ID */
                           identifier_location =
                                 get_symbol_location(program, $2, 0);

                           /* generate a NOT instruction. In order to do this,
                            * at first we have to ask for a free register where
                            * to store the result of the NOT instruction. */
                           output_register = getNewRegister(program);

                           /* Now we are able to generate a NOT instruction */
                           gen_notl_instruction (program, output_register
                                 , identifier_location);

                           $$ = create_expression (output_register, REGISTER);

                           /* free the memory associated with the IDENTIFIER */
                           free($2);
   }
   | assign_statement {}
   | exp AND_OP exp     {
                           $$ = handle_bin_numeric_op(program, $1, $3, ANDB);
   }
   | exp OR_OP exp      {
                           $$ = handle_bin_numeric_op(program, $1, $3, ORB);
   }
   | exp PLUS exp       {
                           $$ = handle_bin_numeric_op(program, $1, $3, ADD);
   }
   | exp MINUS exp      {
                           $$ = handle_bin_numeric_op(program, $1, $3, SUB);
   }
   | exp MUL_OP exp     {
                           $$ = handle_bin_numeric_op(program, $1, $3, MUL);
   }
   | exp DIV_OP exp     {
                           $$ = handle_bin_numeric_op(program, $1, $3, DIV);
   }

   | exp MOD_OP exp  {
       if($1.expression_type == IMMEDIATE && $3.expression_type == IMMEDIATE) {
           $$ = create_expression($1.value % $3.value,IMMEDIATE);
       } else {
           t_axe_expression div = handle_bin_numeric_op(program,$1,$3, DIV);
           t_axe_expression mul = handle_bin_numeric_op(program,div,$3, MUL);
           
           $$ = handle_bin_numeric_op(program,$1,mul,SUB);
           
           
       }
    
    }

   | exp LT exp      {
                        $$ = handle_binary_comparison (program, $1, $3, _LT_);
   }
   | exp GT exp      {
                        $$ = handle_binary_comparison (program, $1, $3, _GT_);
   }
   | exp EQ exp      {
                        $$ = handle_binary_comparison (program, $1, $3, _EQ_);
   }
   | exp NOTEQ exp   {
                        $$ = handle_binary_comparison (program, $1, $3, _NOTEQ_);
   }
   | exp LTEQ exp    {
                        $$ = handle_binary_comparison (program, $1, $3, _LTEQ_);
   }
   | exp GTEQ exp    {
                        $$ = handle_binary_comparison (program, $1, $3, _GTEQ_);
   }
   | exp SHL_OP exp  {  $$ = handle_bin_numeric_op(program, $1, $3, SHL); }
   | exp SHR_OP exp  {  $$ = handle_bin_numeric_op(program, $1, $3, SHR); }
   | exp ANDAND exp  {  $$ = handle_bin_numeric_op(program, $1, $3, ANDL); }
   | exp OROR exp    {  $$ = handle_bin_numeric_op(program, $1, $3, ORL); }
   | LPAR exp RPAR   { $$ = $2; }
   | MINUS exp       {
                        if ($2.expression_type == IMMEDIATE)
                        {
                           $$ = $2;
                           $$.value = - ($$.value);
                        }
                        else
                        {
                           t_axe_expression exp_r0;

                           /* create an expression for regisrer REG_0 */
                           exp_r0.value = REG_0;
                           exp_r0.expression_type = REGISTER;
                           
                           $$ = handle_bin_numeric_op
                                 (program, exp_r0, $2, SUB);
                        }
                     }
;

%%
/*=========================================================================
                                  MAIN
=========================================================================*/
int main (int argc, char **argv)
{
   /* initialize all the compiler data structures and global variables */
   init_compiler(argc, argv);
   
   /* start the parsing procedure */
   yyparse();
   
#ifndef NDEBUG
   fprintf(stdout, "Parsing process completed. \n");
#endif

   /* test if the parsing process completed succesfully */
   checkConsistency();
   
#ifndef NDEBUG
   fprintf(stdout, "Creating a control flow graph. \n");
#endif

   /* create the control flow graph */
   graph = createFlowGraph(program->instructions);
   checkConsistency();

#ifndef NDEBUG
   assert(program != NULL);
   assert(program->sy_table != NULL);
   assert(file_infos != NULL);
   assert(file_infos->syTable_output != NULL);
   printSymbolTable(program->sy_table, file_infos->syTable_output);
   printGraphInfos(graph, file_infos->cfg_1, 0);
      
   fprintf(stdout, "Updating the basic blocks. \n");
#endif
      
   /* update the control flow graph by inserting load and stores inside
   * every basic block */
   graph = insertLoadAndStoreInstr(program, graph);

#ifndef NDEBUG
   fprintf(stdout, "Executing a liveness analysis on the intermediate code \n");
#endif
   performLivenessAnalysis(graph);
   checkConsistency();

#ifndef NDEBUG
   printGraphInfos(graph, file_infos->cfg_2, 1);
#endif
      
#ifndef NDEBUG
   fprintf(stdout, "Starting the register allocation process. \n");
#endif
   /* initialize the register allocator by using the control flow
    * informations stored into the control flow graph */
   RA = initializeRegAlloc(graph);
      
   /* execute the linear scan algorythm */
   execute_linear_scan(RA);
      
#ifndef NDEBUG
   printRegAllocInfos(RA, file_infos->reg_alloc_output);
#endif

#ifndef NDEBUG
   fprintf(stdout, "Updating the control flow informations. \n");
#endif
   /* apply changes to the program informations by using the informations
   * of the register allocation process */
   updateProgramInfos(program, graph, RA);

#ifndef NDEBUG
   fprintf(stdout, "Writing the assembly file... \n");
#endif
   writeAssembly(program, file_infos->output_file_name);
      
#ifndef NDEBUG
   fprintf(stdout, "Assembly written on file \"%s\".\n", file_infos->output_file_name);
#endif
   
   /* shutdown the compiler */
   shutdownCompiler(0);

   return 0;
}

/*=========================================================================
                                 YYERROR
=========================================================================*/
int yyerror(const char* errmsg)
{
   errorcode = AXE_SYNTAX_ERROR;
   
   return 0;
}
