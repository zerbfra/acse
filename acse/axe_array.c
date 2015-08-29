/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_array.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "axe_array.h"
#include "axe_gencode.h"
#include "symbol_table.h"
#include "axe_utils.h"
#include "axe_errors.h"

void shift_array(t_program_infos *program,char *ID, t_axe_expression shift, int direction) {
    
    t_axe_variable *id = getVariable(program,ID);
    
    if(!id->isArray) exit(-1);
    
    // inizializzo
    int sha_reg = getNewRegister(program);
    t_axe_expression zero_exp = create_expression(0,IMMEDIATE);
    
    int size_reg = gen_load_immediate(program,id->arraySize);
    
    // carico il valore dello shift
    if(shift.expression_type == IMMEDIATE) gen_addi_instruction(program,sha_reg,REG_0,shift.value);
    else gen_add_instruction(program,sha_reg,REG_0,shift.value,CG_DIRECT_ALL);
    
    int temp_reg;
    int tr = getNewRegister(program);
    int src_index;
    int dest_index;
    
    t_axe_expression src_exp, dest_exp;
    
    src_exp = create_expression(src_index,REGISTER);
    dest_exp = create_expression(dest_index,REGISTER);
    
    if(direction == LEFTSHIFT) {
        // source parte dal displacement
        // dest dall'inizio
        gen_add_instruction(program,src_index,REG_0,sha_reg,CG_DIRECT_ALL);
        gen_add_instruction(program,dest_index,REG_0,REG_0,CG_DIRECT_ALL);
    } else {
        // dest_index = ultimo elemento
        // src_index = dest_index - displacement
        gen_subi_instruction(program,dest_index,size_reg,1);
        gen_sub_instruction(program,src_index,dest_index,sha_reg,CG_DIRECT_ALL);
    }
    
    t_axe_label *stop_shifting = newLabel(program);
    t_axe_label *continue_shifting = assignNewLabel(program);
    
    // inizio shift
    
    // valuto se fermarmi o meno (se size = src | se src_index == 0)
    if(direction == LEFTSHIFT) {
        gen_sub_instruction(program,tr,size_reg,src_index,CG_DIRECT_ALL);
        gen_beq_instruction(program,stop_shifting,0);
    } else {
        gen_andb_instruction(program,src_index,src_index,src_index,CG_DIRECT_ALL);
        gen_blt_instruction(program,stop_shifting,0);
    }
    
    // scambio gli elementi
    temp_reg = loadArrayElement(program,ID,src_exp);
    t_axe_expression temp_exp = create_expression(temp_reg,REGISTER);
    storeArrayElement(program,ID,dest_exp,temp_exp);
    
    if(direction == LEFTSHIFT) {
        // aumento i due indici
        gen_addi_instruction(program,src_index,src_index,1);
        gen_addi_instruction(program,dest_index,dest_index,1);
    } else {
        // diminuisco i due indici
        gen_subi_instruction(program,src_index,src_index,1);
        gen_subi_instruction(program,dest_index,dest_index,1);
    }
    
    gen_bt_instruction(program,continue_shifting,0);
    
    assignLabel(program,stop_shifting);
    
    /* ho finito gli spostamenti, ora riempio di 0 */
 
    
    t_axe_label *stop_filling = newLabel(program);
    t_axe_label *continue_filling = assignNewLabel(program);
    
    if(direction == LEFTSHIFT) {
        gen_sub_instruction(program,tr,size_reg,dest_index,CG_DIRECT_ALL);
        gen_beq_instruction(program,stop_filling,0);
        storeArrayElement(program,ID,dest_exp,zero_exp);
        gen_addi_instruction(program,dest_index,dest_index,1);
    } else {
        gen_andb_instruction(program,dest_index,dest_index,dest_index,CG_DIRECT_ALL);
        gen_blt_instruction(program,stop_filling,0);
        storeArrayElement(program,ID,dest_exp,zero_exp);
        gen_subi_instruction(program,dest_index,dest_index,1);
    }
    
    gen_bt_instruction(program,continue_filling,0);
    assignLabel(program,stop_filling);
    
    
}

void storeArrayElement(t_program_infos *program, char *ID
            , t_axe_expression index, t_axe_expression data)
{
   int address;
   
   address =  loadArrayAddress(program, ID, index);

   if (data.expression_type == REGISTER)
   {
      /* load the value indirectly into `mova_register' */
      gen_add_instruction(program, address, REG_0
               , data.value, CG_INDIRECT_DEST);
   }
   else
   {
      int imm_register;

      imm_register = gen_load_immediate(program, data.value);

      /* load the value indirectly into `load_register' */
      gen_add_instruction(program, address, REG_0
               ,imm_register, CG_INDIRECT_DEST);
   }
}

int loadArrayElement(t_program_infos *program
               , char *ID, t_axe_expression index)
{
   int load_register;
   int address;

   /* retrieve the address of the array slot */
   address = loadArrayAddress(program, ID, index);

   /* get a new register */
   load_register = getNewRegister(program);

   /* load the value into `load_register' */
   gen_add_instruction(program, load_register, REG_0
            , address, CG_INDIRECT_SOURCE);

   /* return the register ID that holds the required data */
   return load_register;
}

int loadArrayAddress(t_program_infos *program
            , char *ID, t_axe_expression index)
{
   int mova_register;
   t_axe_label *label;

   /* preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   if (ID == NULL)
      notifyError(AXE_VARIABLE_ID_UNSPECIFIED);
   
   /* retrieve the label associated with the given
   * identifier */
   label = getLabelFromVariableID(program, ID);
                     
   /* test if an error occurred */
   if (label == NULL)
      return REG_INVALID;

   /* get a new register */
   mova_register = getNewRegister(program);

   /* generate the MOVA instruction */
   gen_mova_instruction(program, mova_register, label, 0);

   if (index.expression_type == IMMEDIATE)
   {
      if (index.value != 0)
      {
         gen_addi_instruction (program, mova_register
                  , mova_register, index.value);
      }
   }
   else
   {
      assert(index.expression_type == REGISTER);

      /* We are making the following assumption:
      * the type can only be an INTEGER_TYPE */
      gen_add_instruction(program, mova_register, mova_register
               , index.value, CG_DIRECT_ALL);
   }

   /* return the identifier of the register that contains
    * the value of the array slot */
   return mova_register;
}
