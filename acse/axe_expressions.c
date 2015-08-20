/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_expressions.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "axe_expressions.h"
#include "axe_gencode.h"
#include "axe_errors.h"


static t_axe_expression handle_bin_numeric_op_Imm
         (int val1, int val2, int binop);
         
static t_axe_expression handle_bin_comparison_Imm
         (int val1, int val2, int condition);


t_axe_expression handle_bin_numeric_op (t_program_infos *program
         , t_axe_expression exp1, t_axe_expression exp2, int binop)
{
   int output_register;
   t_axe_expression result;

   /* initialize result */
   result.expression_type = INVALID_EXPRESSION;

   /* we have to test if one (or both) of
   * the two operands is an immediate value */
   if (  (exp2.expression_type == IMMEDIATE)
         && (exp1.expression_type == IMMEDIATE) )
   {
      return handle_bin_numeric_op_Imm(exp1.value, exp2.value, binop);
   }
   
   /* at first we have to ask for a free register
   * where to store the result of the operation. */
   output_register = getNewRegister(program);

   if (exp2.expression_type == IMMEDIATE)
   {
      /* we have to produce an instruction */
      switch(binop)
      {
         case ADD : gen_addi_instruction (program, output_register
                             , exp1.value, exp2.value); break;
         case ANDB : gen_andbi_instruction (program, output_register
                              , exp1.value, exp2.value); break;
         case ORB  : gen_orbi_instruction (program, output_register
                             , exp1.value, exp2.value); break;
         case SUB : gen_subi_instruction (program, output_register
                             , exp1.value, exp2.value); break;
         case MUL : gen_muli_instruction (program, output_register
                             , exp1.value, exp2.value); break;
         case SHL : gen_shli_instruction (program, output_register
                             , exp1.value, exp2.value); break;
         case SHR : gen_shri_instruction (program, output_register
                             , exp1.value, exp2.value); break;
         case DIV :
               if (exp2.value == 0){
                  printWarningMessage(WARN_DIVISION_BY_ZERO);
               }
               gen_divi_instruction (program, output_register
                        , exp1.value, exp2.value);
               break;
         default :
               notifyError(AXE_INVALID_EXPRESSION);
      }
   }
   else if (exp1.expression_type == IMMEDIATE)
   {
      int other_reg;

      /* we have to produce an instruction */
      switch(binop)
      {
         case ADD :  gen_addi_instruction (program, output_register
                              , exp2.value, exp1.value); break;
         case ANDB :  gen_andbi_instruction (program, output_register
                              , exp2.value, exp1.value); break;
         case ORB  :  gen_orbi_instruction (program, output_register
                              , exp2.value, exp1.value); break;
         case SUB :
                  gen_subi_instruction (program, output_register
                           , exp2.value, exp1.value);

                  /* we have to produce a NEG instruction */
                  gen_neg_instruction (program, output_register
                           , output_register, CG_DIRECT_ALL);
                  break;
         case MUL :  gen_muli_instruction (program, output_register
                              , exp2.value, exp1.value); break;
         case DIV :
                  /* we have to load into a register the immediate value */
                  other_reg = getNewRegister(program);

                  /* In order to load the immediate inside a new
                   * register we have to insert an ADDI instr. */
                  gen_addi_instruction (program, other_reg
                           , REG_0, exp1.value);

                  /* we have to produce a DIV instruction */
                  gen_div_instruction (program, output_register
                           , other_reg, exp2.value, CG_DIRECT_ALL);
                  break;
         case SHL :
                  /* we have to load into a register the immediate value */
                  other_reg = getNewRegister(program);

                  /* In order to load the immediate inside a new
                   * register we have to insert an ADDI instr. */
                  gen_addi_instruction (program, other_reg
                           , REG_0, exp1.value);

                  /* we have to produce a SHL instruction */
                  gen_shl_instruction (program, output_register
                           , other_reg, exp2.value, CG_DIRECT_ALL);
                  break;
         case SHR :
                  /* we have to load into a register the immediate value */
                  other_reg = getNewRegister(program);

                  /* In order to load the immediate inside a new
                   * register we have to insert an ADDI instr. */
                  gen_addi_instruction (program, other_reg
                           , REG_0, exp1.value);

                  /* we have to produce a SHR instruction */
                  gen_shr_instruction (program, output_register
                           , other_reg, exp2.value, CG_DIRECT_ALL);
                  break;
         default :
                  notifyError(AXE_INVALID_EXPRESSION);
      }
   }
   else
   {
      /* we have to produce an instruction */
      switch(binop)
      {
         case ADD :  gen_add_instruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case ANDB :  gen_andb_instruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case ORB :  gen_orb_instruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case SUB :  gen_sub_instruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case MUL :  gen_mul_instruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case DIV :  gen_div_instruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case SHL :  gen_shl_instruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case SHR :  gen_shl_instruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         default :
                     notifyError(AXE_INVALID_EXPRESSION);
      }
   }
         
   /* assign a value to result */
   return create_expression (output_register, REGISTER);
}

t_axe_expression handle_bin_numeric_op_Imm
         (int val1, int val2, int binop)
{
   switch(binop)
   {
      case ADD : return create_expression ((val1 + val2), IMMEDIATE);
      case ANDB : return create_expression ((val1 & val2), IMMEDIATE);
      case ORB  : return create_expression ((val1 | val2), IMMEDIATE);
      case SUB : return create_expression ((val1 - val2), IMMEDIATE);
      case MUL : return create_expression ((val1 * val2), IMMEDIATE);
      case SHL : return create_expression ((val1 << val2), IMMEDIATE);
      case SHR : return create_expression ((val1 >> val2), IMMEDIATE);
      case DIV :
         if (val2 == 0){
            printWarningMessage(WARN_DIVISION_BY_ZERO);
         }
         return create_expression ((val1 / val2), IMMEDIATE);
      default :
         notifyError(AXE_INVALID_EXPRESSION);
   }
   
   return create_expression (0, INVALID_EXPRESSION);
}

t_axe_expression handle_bin_comparison_Imm
         (int val1, int val2, int condition)
{
   switch(condition)
   {
      case _LT_ : return create_expression ((val1 < val2), IMMEDIATE);
      case _GT_ : return create_expression ((val1 > val2), IMMEDIATE);
      case _EQ_  : return create_expression ((val1 == val2), IMMEDIATE);
      case _NOTEQ_ : return create_expression ((val1 != val2), IMMEDIATE);
      case _LTEQ_ : return create_expression ((val1 <= val2), IMMEDIATE);
      case _GTEQ_ : return create_expression ((val1 >= val2), IMMEDIATE);
      default :
         notifyError(AXE_INVALID_EXPRESSION);
   }

   return create_expression (0, INVALID_EXPRESSION);
}

t_axe_expression handle_binary_comparison (t_program_infos *program
         , t_axe_expression exp1, t_axe_expression exp2, int condition)
{
   t_axe_expression result;
   int output_register;
   
   /* initialize result */
   result.expression_type = INVALID_EXPRESSION;

   /* we have to test if one (or both) of
   * the two operands is an immediate value */
   if (  (exp2.expression_type == IMMEDIATE)
         && (exp1.expression_type == IMMEDIATE) )
   {
      return handle_bin_comparison_Imm
                  (exp1.value, exp2.value, condition);
   }
                     
   /* at first we have to ask for a free register
   * where to store the result of the comparison. */
   output_register = getNewRegister(program);

   if (exp2.expression_type == IMMEDIATE)
   {
      /* we have to produce a SUBI instruction */
      gen_subi_instruction (program, output_register
               , exp1.value, exp2.value);
   }
   else if (exp1.expression_type == IMMEDIATE)
   {
      gen_subi_instruction (program, output_register
               , exp2.value, exp1.value);

      /* we have to produce a NEG instruction */
      gen_neg_instruction (program, output_register
               , output_register, CG_DIRECT_ALL);
   }
   else
   {
      /* we have to produce a SUB instruction */
      gen_sub_instruction (program, output_register
               , exp1.value, exp2.value, CG_DIRECT_ALL);
   }

   /* generate a set instruction */
   switch(condition)
   {
      case _LT_ : gen_slt_instruction (program, output_register); break;
      case _GT_ : gen_sgt_instruction (program, output_register); break;
      case _EQ_  : gen_seq_instruction (program, output_register); break;
      case _NOTEQ_ : gen_sne_instruction (program, output_register); break;
      case _LTEQ_ : gen_sle_instruction (program, output_register); break;
      case _GTEQ_ : gen_sge_instruction (program, output_register); break;
      default :
         notifyError(AXE_INVALID_EXPRESSION);
   }

   /* return the new expression */
   return create_expression (output_register, REGISTER);
}
