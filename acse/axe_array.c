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


void rotateArray(t_program_infos *program, t_axe_variable *array, t_axe_expression displacement, int dir) {
    
    int disp,index;
    
    t_axe_label *condition_loop = newLabel(program);
    t_axe_label *end_loop = newLabel(program);
    
    
    // lunghezza array
    t_axe_expression lenght = create_expression(array->arraySize,REGISTER);
    // tolgo 1 alla lunghezza (mi da ultimo elemento)
    gen_subi_instruction(program,lenght.value,lenght.value,1);
    
    // carico disp
    if(displacement.expression_type == IMMEDIATE) {
        disp = gen_load_immediate(program,displacement.value);
    } else {
        disp = getNewRegister(program);
        disp = gen_andb_instruction(program,disp,displacement.value,displacement.value,CG_DIRECT_ALL);
    }
    t_axe_expression disp_exp = create_expression(disp,REGISTER);


    
    // per conteggiare i cicli che portano il displacement a 0
    assignLabel(program,condition_loop);
    
    // se il disp è 0<= finisco
    gen_ble_instruction(program,end_loop,0);
    
    
    

    
        // carico il primo elemento dell'array, me lo salvo altrimenti andrebbe perso!
        t_axe_expression index_0 = create_expression(0,IMMEDIATE);
        int first_element = loadArrayElement(program,array->ID,index_0);
        t_axe_expression first_element_exp = create_expression(first_element,REGISTER);
    
        // fisso index a 1 (elemento 0 è già stato memorizzato)
        index = gen_load_immediate(program,1);
    
        ///// INIZIO LOOP INTERNO
        t_axe_label *condition = assignNewLabel(program);
    
        t_axe_expression index_expr = create_expression(index,REGISTER);
    
        int element = loadArrayElement(program,array->ID,index_expr); // posizione corrente (index)
        t_axe_expression element_exp = create_expression(element,REGISTER);
    
        gen_subi_instruction(program,index,index,1);            // posizione precedente (index-1)
        storeArrayElement(program,array->ID,index_expr,element_exp); // salvo in index-1 quello che è a index

        gen_addi_instruction(program,index,index,2); // proseguo (ero tornato indietro di 1 quindi +2)

        // se lenght-index <= 0 devo finire (ho finito array)
    
        t_axe_label *end = newLabel(program);
        gen_bge_instruction(program,end,lenght.value);
        // altrimenti torno alla condizione
        gen_bt_instruction(program,condition,0);
    
        /// FINE LOOP INTERNO
    
        assignLabel(program,end);
    
        // metto quello che era il primo alla fine
        storeArrayElement(program,array->ID,lenght,first_element_exp);

    

    
    // tolgo 1 al displacement, perchè un ciclo di spostamenti è fatto
    gen_subi_instruction(program,disp,disp,1);
    // torno alla condizione
    gen_bt_instruction(program,condition_loop,0);
    
    assignLabel(program,end_loop);
    
    
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
