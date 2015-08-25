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

// mirrorArray è metodo che "specchia" un array, a partire da start fino a end
void mirrorArray(t_program_infos *program, t_axe_variable *array , t_axe_expression start, t_axe_expression end) {
    
    int index_from,index_to;
    t_axe_expression index_from_exp,index_to_exp;
    
    t_axe_label *condition_label, *end_label;
    
    condition_label = newLabel(program);
    end_label = newLabel(program);
    
    int result;
    
    // valori temporanei
    int data_from_temp, data_to_temp;
    t_axe_expression data_from_temp_exp,data_to_temp_exp;
    
    /* load starting point */
    if(start.expression_type == IMMEDIATE) {
        index_from = gen_load_immediate(program,start.value);
    } else {
        // devo creare un registro sul quale memorizzare
        index_from = getNewRegister(program);
        index_from = gen_andb_instruction(program,index_from,start.value,start.value,CG_DIRECT_ALL);
    }
    
    index_from_exp = create_expression(index_from,REGISTER);
    
    /* load end point (last element = size -1) */
    
    if(end.expression_type == IMMEDIATE) {
        index_to = gen_load_immediate(program,end.value);
    } else {
        // devo creare un registro sul quale memorizzare
        index_to = getNewRegister(program);
        index_to = gen_andb_instruction(program,index_to,end.value,end.value,CG_DIRECT_ALL);
    }
    
    index_to_exp = create_expression(index_to,REGISTER);
    
    // togli 1 dalla size
    gen_subi_instruction(program,index_to,index_to,1);
    
    
    /** SWAPPING ALGORITHM **/
    assignLabel(program,condition_label);
    
    result = getNewRegister(program);
    gen_sub_instruction(program,result,index_to,index_from,CG_DIRECT_ALL);
    
    // se <= 0 termino
    gen_ble_instruction(program,end_label,0);
    
    /* Swap */
    
    // salvo data from
    data_from_temp = loadArrayElement(program,array->ID,index_from_exp);
    data_from_temp_exp = create_expression(data_from_temp,REGISTER);
    
    // salvo data to
    data_to_temp = loadArrayElement(program,array->ID,index_to_exp);
    data_to_temp_exp = create_expression(data_to_temp,REGISTER);
    
    // scambio i valori nell'array scambiandoli
    storeArrayElement(program,array->ID,index_from_exp,data_to_temp_exp);
    storeArrayElement(program,array->ID,index_to_exp,data_from_temp_exp);
    
    /* Update indices */
    
    gen_addi_instruction(program,index_from,index_from,1);
    gen_subi_instruction(program,index_to,index_to,1);
    
    // salto sempre e comunque alla condizione
    gen_bt_instruction(program,condition_label,0);
    
    // end
    assignLabel(program,end_label);
    
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
