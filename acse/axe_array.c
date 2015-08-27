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
#include "axe_constants.h"


void rotateArray(t_program_infos *program, t_axe_variable *array, t_axe_expression displacement, int dir) {
    
    /* variabili ciclo esterno */
    t_axe_label *condition_loop = newLabel(program);
    t_axe_label *end_loop = newLabel(program);
    int index_last; // posizione dell'ultimo elemento
    int disp;
    int size;
    
    /* variabili ciclo interno */
    int index;
    int data;
    t_axe_expression index_exp,data_exp;
    
    /************* LOGICA *****************/
    
    // carico la lunghezza dell'array come immediato
    size = gen_load_immediate(program,array->arraySize);
    
    // carico la lunghezza, in index_last, per poi togliere 1 ed ottenere indice ultima cella
    index_last = gen_load_immediate(program,array->arraySize);
    gen_subi_instruction(program,index_last,index_last,1);
    
    // last_exp ora contiene l'indice dell'ultima cella dell'array
    t_axe_expression last_exp = create_expression(index_last,REGISTER);
    
    // carico displacement (di quanto spostare tutti gli elementi)
    if(displacement.expression_type == IMMEDIATE) {
        disp = gen_load_immediate(program,displacement.value);
    } else {
        disp = getNewRegister(program);
        disp = gen_andb_instruction(program,disp,displacement.value,displacement.value,CG_DIRECT_ALL);
    }
    // genero exp per il displacement
    t_axe_expression disp_exp = create_expression(disp,REGISTER);


    /***** INZIO CICLO ESTERNO ***/
    
        // inizio ciclo esterno (conta displacement da valore attuale fino a 0, decrementando)
        assignLabel(program,condition_loop);
        
        // se il disp è 0<= FINISCO! e mando su end_loop
        gen_ble_instruction(program,end_loop,0);
        
        
        // SHIFT LEFT:  salvo array[0], altrimenti andrebbe perso!
        // SHIFT RIGHT: salvo array[index_last]
        t_axe_expression index_0 = create_expression(0,IMMEDIATE);
        int savedEl;
    
        if(dir == VECSL) savedEl = loadArrayElement(program,array->ID,index_0);
        else savedEl = loadArrayElement(program,array->ID,last_exp);
    
        t_axe_expression savedEl_exp = create_expression(savedEl,REGISTER);

        // SHIFT LEFT:  fisso index a 1 (elemento 0 è già stato memorizzato)
        // SHIFT RIGHT: fisso a index-last in quanto vado dalla fine all'inizio
        if(dir == VECSL) index = gen_load_immediate(program,1);
        else index = gen_load_immediate(program,index_last-1);
    
        index_exp = create_expression(index,REGISTER);

        /***** CICLO INTERNO *******/
        
            t_axe_label *end = newLabel(program);
            
            // assegno la label per ritornare qua
            t_axe_label *condition = assignNewLabel(program);
            
            // carico il dato dall'index corrente
            data = loadArrayElement(program,array->ID,index_exp);
            data_exp = create_expression(data,REGISTER);
            
            // SHIFT LEFT:  per copiare il dato array[index] ad array[index-1] (scalano tutti verso sx)
            // SHIFT RIGHT: aumento di uno copiare da [index] a [index+1] (li scalo tutti verso dx)
            if(dir == VECSL) gen_subi_instruction(program,index,index,1);
            else gen_addi_instruction(program,index,index,1);
            
            // salvo data in array[index-1]
            // oppure in array[index+1]
            storeArrayElement(program,array->ID,index_exp,data_exp);
            
            // SHIFT LEFT:  aggiungo +2: +1 per avanzamento indice, +1 perchè ho fatto la subi per ottere index-1
            // SHIFT RIGHT: devo fare -2: -1 per la addi, -1 per indietreggiare
            if(dir == VECSL) gen_addi_instruction(program,index,index,2);
            else gen_subi_instruction(program,index,index,2);
    

            // SHIFT LEFT:  valuto se index è arrivato alla dimensione dell'array, sottraggo:
            // SHIFT RIGHT: se sto indietreggiando devo valutare solo se index = 0 quindi non lo faccio
            int result = getNewRegister(program);
            if(dir==VECSL) gen_sub_instruction(program,result,index,size,CG_DIRECT_ALL);
            
            // SHIFT LEFT:  se result = size-index = 0, allora vado alla fine (index=size, ho finito!)
            // SHIFT RIGHT: devo ripetere ancora una volta per l'elemento in posizione 0,
            //              quindi faccio un altro ciclo (mi fermerò al prossimo che da index = -1
            if(dir == VECSL) gen_beq_instruction(program,end,0);
            else gen_blt_instruction(program,end,0);
            
            // altrimenti, torno alla condizione
            gen_bt_instruction(program,condition,0);

            assignLabel(program,end);
        
        /***** FINE CICLO INTERNO *******/
        

        // SHIFT LEFT: metto quello che era il primo alla fine
        // SHIFT RIGHT: devo mettere il finale nella prima posizione
        if(dir == VECSL) storeArrayElement(program,array->ID,last_exp,savedEl_exp);
        else storeArrayElement(program,array->ID,index_0,savedEl_exp);

        
        // tolgo 1 al displacement, perchè un ciclo di spostamenti è fatto
        gen_subi_instruction(program,disp,disp,1);
        
        // torno alla condizione
        gen_bt_instruction(program,condition_loop,0);
    
    /***** FINE CICLO ESTERNO ***/
    
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
