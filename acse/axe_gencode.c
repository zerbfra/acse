/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_gencode.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "axe_gencode.h"
#include "axe_errors.h"


static t_axe_instruction * gen_unary_instruction (t_program_infos *program
         , int opcode, int r_dest, t_axe_label *label, int addr);
static t_axe_instruction * gen_binary_instruction (t_program_infos *program
      , int opcode, int r_dest, int r_source1, int immediate);
static t_axe_instruction * gen_ternary_instruction (t_program_infos *program
      , int opcode, int r_dest, int r_source1, int r_source2, int flags);
static t_axe_instruction * gen_jump_instruction (t_program_infos *program
      , int opcode, t_axe_label *label, int addr);


t_axe_instruction * gen_bt_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BT, label, addr);
}

t_axe_instruction * gen_bf_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BF, label, addr);
}

t_axe_instruction * gen_bhi_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BHI, label, addr);
}      
      
t_axe_instruction * gen_bls_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BLS, label, addr);
}

t_axe_instruction * gen_bcc_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BCC, label, addr);
}

t_axe_instruction * gen_bcs_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BCS, label, addr);
}

t_axe_instruction * gen_bne_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BNE, label, addr);
}

t_axe_instruction * gen_beq_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BEQ, label, addr);
}

t_axe_instruction * gen_bvc_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BVC, label, addr);
}

t_axe_instruction * gen_bvs_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BVS, label, addr);
}

t_axe_instruction * gen_bpl_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BPL, label, addr);
}

t_axe_instruction * gen_bmi_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BMI, label, addr);
}

t_axe_instruction * gen_bge_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BGE, label, addr);
}

t_axe_instruction * gen_blt_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BLT, label, addr);
}

t_axe_instruction * gen_bgt_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BGT, label, addr);
}

t_axe_instruction * gen_ble_instruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return gen_jump_instruction (program, BLE, label, addr);
}

t_axe_instruction * gen_add_instruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return gen_ternary_instruction
         (program, ADD, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * gen_sub_instruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return gen_ternary_instruction
         (program, SUB, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * gen_andl_instruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return gen_ternary_instruction
         (program, ANDL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * gen_orl_instruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return gen_ternary_instruction
         (program, ORL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * gen_eorl_instruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return gen_ternary_instruction
         (program, EORL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * gen_andb_instruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return gen_ternary_instruction
         (program, ANDB, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * gen_orb_instruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return gen_ternary_instruction
         (program, ORB, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * gen_eorb_instruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return gen_ternary_instruction
         (program, EORB, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * gen_mul_instruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return gen_ternary_instruction
         (program, MUL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * gen_div_instruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return gen_ternary_instruction
         (program, DIV, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * gen_shl_instruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return gen_ternary_instruction
         (program, SHL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * gen_shr_instruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return gen_ternary_instruction
         (program, SHL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * gen_neg_instruction (t_program_infos *program
      , int r_dest, int r_source, int flags)
{
   return gen_ternary_instruction
         (program, SUB, r_dest, REG_0, r_source, flags);
}

t_axe_instruction * gen_spcl_instruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return gen_ternary_instruction
         (program, SPCL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * gen_addi_instruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return gen_binary_instruction(program, ADDI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * gen_subi_instruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return gen_binary_instruction(program, SUBI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * gen_andli_instruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return gen_binary_instruction(program, ANDLI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * gen_orli_instruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return gen_binary_instruction(program, ORLI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * gen_eorli_instruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return gen_binary_instruction(program, EORLI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * gen_andbi_instruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return gen_binary_instruction(program, ANDBI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * gen_muli_instruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return gen_binary_instruction(program, MULI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * gen_orbi_instruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return gen_binary_instruction(program, ORBI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * gen_eorbi_instruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return gen_binary_instruction(program, EORBI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * gen_divi_instruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return gen_binary_instruction(program, DIVI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * gen_shli_instruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return gen_binary_instruction(program, SHLI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * gen_shri_instruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return gen_binary_instruction(program, SHRI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * gen_notl_instruction
      (t_program_infos *program, int r_dest, int r_source1)
{
   return gen_binary_instruction(program, NOTL
         , r_dest, r_source1, 0);
}

t_axe_instruction * gen_notb_instruction
      (t_program_infos *program, int r_dest, int r_source1)
{
   return gen_binary_instruction(program, NOTB
         , r_dest, r_source1, 0);
}

t_axe_instruction * gen_read_instruction
               (t_program_infos *program, int r_dest)
{
   return gen_unary_instruction(program, AXE_READ, r_dest, NULL, 0);
}

t_axe_instruction * gen_write_instruction
               (t_program_infos *program, int r_dest)
{
   return gen_unary_instruction(program, AXE_WRITE, r_dest, NULL, 0);
}

t_axe_instruction * gen_load_instruction
      (t_program_infos *program, int r_dest, t_axe_label *label, int address)
{
   return gen_unary_instruction(program, LOAD, r_dest, label, address);
}

t_axe_instruction * gen_store_instruction
      (t_program_infos *program, int r_dest, t_axe_label *label, int address)
{
   return gen_unary_instruction(program, STORE, r_dest, label, address);
}

t_axe_instruction * gen_mova_instruction
      (t_program_infos *program, int r_dest, t_axe_label *label, int address)
{
   return gen_unary_instruction(program, MOVA, r_dest, label, address);
}

t_axe_instruction * gen_sge_instruction
                  (t_program_infos *program, int r_dest)
{
   return gen_unary_instruction(program, SGE, r_dest, NULL, 0);
}
   
t_axe_instruction * gen_seq_instruction
                  (t_program_infos *program, int r_dest)
{
   return gen_unary_instruction(program, SEQ, r_dest, NULL, 0);
}

t_axe_instruction * gen_sgt_instruction
                  (t_program_infos *program, int r_dest)
{
   return gen_unary_instruction(program, SGT, r_dest, NULL, 0);
}

t_axe_instruction * gen_sle_instruction
                  (t_program_infos *program, int r_dest)
{
   return gen_unary_instruction(program, SLE, r_dest, NULL, 0);
}

t_axe_instruction * gen_slt_instruction
                  (t_program_infos *program, int r_dest)
{
   return gen_unary_instruction(program, SLT, r_dest, NULL, 0);
}

t_axe_instruction * gen_sne_instruction
                  (t_program_infos *program, int r_dest)
{
   return gen_unary_instruction(program, SNE, r_dest, NULL, 0);
}

t_axe_instruction * gen_halt_instruction
      (t_program_infos *program)
{
   t_axe_instruction *instr;

   /* test if program is initialized */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
      
   /* create an instance of `t_axe_instruction' */
   instr = alloc_instruction(HALT);

   if (instr == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* add the newly created instruction to the current program */
   addInstruction(program, instr);

   /* return the load instruction */
   return instr;
}

t_axe_instruction * gen_nop_instruction(t_program_infos *program)
{
   t_axe_instruction *instr;

   /* test if program is initialized */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   
   /* create an instance of `t_axe_instruction' */
   instr = alloc_instruction(NOP);

   if (instr == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* add the newly created instruction to the current program */
   addInstruction(program, instr);

   /* return the load instruction */
   return instr;
}

t_axe_instruction * gen_unary_instruction (t_program_infos *program
         , int opcode, int r_dest, t_axe_label *label, int addr)
{
   t_axe_instruction *instr;
   t_axe_register *reg;
   t_axe_address *address;
   int addressType;

   /* test if program is initialized */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   if (r_dest == REG_INVALID)
      notifyError(AXE_INVALID_REGISTER_INFO);
   
   /* test if value is correctly initialized */
   if (label != NULL)
   {
      if (label->labelID == LABEL_UNSPECIFIED)
      {
         notifyError(AXE_INVALID_LABEL);
      }

      /* address type is a label type */
      addressType = LABEL_TYPE;
   }
   else
   {
      if (addr < 0)
         notifyError(AXE_INVALID_ADDRESS);

      /* address type is a label type */
      addressType = ADDRESS_TYPE;

   }

   /* test if the opcode is a valid opcode */
   if (opcode == INVALID_OPCODE)
      notifyError(AXE_INVALID_OPCODE);

   /* create an instance of `t_axe_instruction' */
   instr = alloc_instruction(opcode);

   if (instr == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* initialize a register info */
   reg = alloc_register(r_dest, 0);

   if (reg == NULL)
   {
      free_Instruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }
   
   /* update the reg_1 info */
   instr->reg_1 = reg;

   /* initialize an address info */
   address = alloc_address(addressType, addr, label);
   
   if (address == NULL)
   {
      free_Instruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   /* update the instruction address info */
   instr->address = address;

   /* add the newly created instruction to the current program */
   addInstruction(program, instr);

   /* return the load instruction */
   return instr;
}

t_axe_instruction * gen_binary_instruction (t_program_infos *program
      , int opcode, int r_dest, int r_source1, int immediate)
{
   t_axe_instruction *instr;
   t_axe_register *reg_dest;
   t_axe_register *reg_source1;

   /* test if program is initialized */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   /* test if value is correctly initialized */
   if (  (r_dest == REG_INVALID)
         || (r_source1 == REG_INVALID) )
   {
      notifyError(AXE_INVALID_REGISTER_INFO);
   }

   /* create an instance of `t_axe_instruction' */
   instr = alloc_instruction(opcode);

   if (instr == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* initialize a register info */
   reg_dest = alloc_register(r_dest, 0);
   if (reg_dest == NULL)
   {
      free_Instruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   /* update the reg_1 info */
   instr->reg_1 = reg_dest;

   reg_source1 = alloc_register(r_source1, 0);
   if (reg_source1 == NULL)
   {
      free_Instruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
      return NULL;
   }

   /* update the reg_1 info */
   instr->reg_2 = reg_source1;

   /* assign an immediate value */
   instr->immediate = immediate;

   /* add the newly created instruction to the current program */
   addInstruction(program, instr);

   /* return the load instruction */
   return instr;
}

t_axe_instruction * gen_ternary_instruction (t_program_infos *program
      , int opcode, int r_dest, int r_source1, int r_source2, int flags)
{
   t_axe_instruction *instr;
   t_axe_register *reg_dest;
   t_axe_register *reg_source1;
   t_axe_register *reg_source2;
   
   /* test if program is initialized */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   /* test if value is correctly initialized */
   if (  (r_dest == REG_INVALID)
         || (r_source1 == REG_INVALID)
         || (r_source2 == REG_INVALID))
   {
      notifyError(AXE_INVALID_REGISTER_INFO);
   }

   /* create an instance of `t_axe_instruction' */
   instr = alloc_instruction(opcode);

   if (instr == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* initialize a register info */
   reg_dest = alloc_register(r_dest, ((flags & CG_INDIRECT_DEST)? 1 : 0) );
   if (reg_dest == NULL)
   {
      free_Instruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   /* update the reg_1 info */
   instr->reg_1 = reg_dest;

   reg_source1 = alloc_register(r_source1, 0);
   if (reg_source1 == NULL)
   {
      free_Instruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   /* update the reg_2 info */
   instr->reg_2 = reg_source1;

   reg_source2 = alloc_register(r_source2, ((flags & CG_INDIRECT_SOURCE)? 1 : 0));
   if (reg_source1 == NULL)
   {
      free_Instruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   /* update the reg_3 info */
   instr->reg_3 = reg_source2;

   /* add the newly created instruction to the current program */
   addInstruction(program, instr);

   /* return the load instruction */
   return instr;
}

t_axe_instruction * gen_jump_instruction (t_program_infos *program
      , int opcode, t_axe_label *label, int addr)
{
   t_axe_instruction *instr;
   t_axe_address * address;
   int addressType;

   /* test if program is initialized */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   /* test if value is correctly initialized */
   if (label != NULL)
   {
      if (label->labelID == LABEL_UNSPECIFIED)
         notifyError(AXE_INVALID_LABEL);

      addressType = LABEL_TYPE;
   }
   else
   {
      if (addr < 0)
         notifyError(AXE_INVALID_ADDRESS);

      addressType = ADDRESS_TYPE;
   }

   /* test if the opcode is a valid opcode */
   if (opcode == INVALID_OPCODE)
      notifyError(AXE_INVALID_OPCODE);

   /* create an instance of `t_axe_instruction' */
   instr = alloc_instruction(opcode);

   if (instr == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* initialize an address info */
   address = alloc_address(addressType, addr, label);
   
   if (address == NULL)
   {
      free_Instruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   /* update the instruction address info */
   instr->address = address;

   /* add the newly created instruction to the current program */
   addInstruction(program, instr);

   /* return the load instruction */
   return instr;
}
