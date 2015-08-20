/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * asm_debug.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "asm_debug.h"

/* convert the integer code of `dataType' into a string */
char * dataType_toString(int dataType)
{
   if (dataType == ASM_WORD)
      return ".WORD";
   if (dataType == ASM_SPACE)
      return ".SPACE";
   return "<UNKNOWN>";
}

char * dataFormat_toString(int dataFormat)
{
   if (dataFormat == ASM_FORMAT_TER)
      return "TERNARY";
   if (dataFormat == ASM_FORMAT_BIN)
      return "BINARY";
   if (dataFormat == ASM_FORMAT_UNR)
      return "UNARY";
   if (dataFormat == ASM_FORMAT_JMP)
      return "JUMP";
   if (dataFormat == ASM_FORMAT_NULL)
      return "NULL";
   return "<UNKNOWN_FORMAT>";
}

char * opcode_toString(int opcode)
{
   switch(opcode)
   {
      case ADD_OP : return "ADD";
      case SUB_OP : return "SUB";
      case ANDL_OP : return "ANDL";
      case ORL_OP : return "ORL";
      case XORL_OP : return "XORL";
      case ANDB_OP : return "ANDB";
      case ORB_OP : return "ORB";
      case XORB_OP : return "XORB";
      case MUL_OP : return "MUL";
      case DIV_OP : return "DIV";
      case SHL_OP : return "SHL";
      case SHR_OP : return "SHR";
      case ROTL_OP : return "ROTL";
      case ROTR_OP : return "ROTR";
      case NEG_OP : return "NEG";
      case SPCL_OP : return "SPCL";
      case ADDI_OP : return "ADDI";
      case SUBI_OP : return "SUBI";
      case ANDLI_OP : return "ANDLI";
      case ORLI_OP : return "ORLI";
      case XORLI_OP : return "XORLI";
      case ANDBI_OP : return "ANDBI";
      case ORBI_OP : return "ORBI";
      case XORBI_OP : return "XORBI";
      case MULI_OP : return "MULI";
      case DIVI_OP : return "DIVI";
      case SHLI_OP : return "SHLI";
      case SHRI_OP : return "SHRI";
      case ROTLI_OP : return "ROTLI";
      case ROTRI_OP : return "ROTRI";
      case NOTL_OP : return "NOTL";
      case NOTB_OP : return "NOTB";
      case NOP_OP : return "NOP";
      case MOVA_OP : return "MOVA";
      case JSR_OP : return "JSR";
      case RET_OP : return "RET";
      case HALT_OP : return "HALT";
      case BT_OP : return "BT";
      case BF_OP : return "BF";
      case BHI_OP : return "BHI";
      case BLS_OP : return "BLS";
      case BCC_OP : return "BCC";
      case BCS_OP : return "BCS";
      case BNE_OP : return "BNE";
      case BEQ_OP : return "BEQ";
      case BVC_OP : return "BVC";
      case BVS_OP : return "BVS";
      case BPL_OP : return "BPL";
      case BMI_OP : return "BMI";
      case BGE_OP : return "BGE";
      case BLT_OP : return "BLT";
      case BGT_OP : return "BGT";
      case BLE_OP : return "BLE";
      case SEQ_OP : return "SEQ";
      case SGE_OP : return "SGE";
      case SGT_OP : return "SGT";
      case SLE_OP : return "SLE";
      case SLT_OP : return "SLT";
      case SNE_OP : return "SNE";
      case LOAD_OP : return "LOAD";
      case STORE_OP : return "STORE";
      case READ_OP : return "READ";
      case WRITE_OP : return "WRITE";
      default : return "<INVALID_OPCODE>";
      
   }
}
