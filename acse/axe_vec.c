/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_array.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "axe_vec.h"


void handle_vec_op(t_program_infos *program, char *destID, char *sourceID1, char *sourceID2, int vec_op) {
    
    t_axe_variable *dest_var = getVariable(program,destID);
    t_axe_variable *source_var1 = getVariable(program,sourceID1);
    t_axe_variable *source_var2 = getVariable(program,sourceID2);
    
    // controllo che siano tutti array
    if(!dest_var->isArray || !source_var1->isArray || !source_var2->isArray) {
        notifyError(AXE_INVALID_TYPE);
    }
    
    if(dest_var->arraySize != source_var1->arraySize || dest_var->arraySize != source_var2->arraySize) {
        notifyError(AXE_INVALID_ARRAY_SIZE);
    }
    
    int index_register, dest_register;
    
    t_axe_expression index,dest;
    
    t_axe_label *start, *end;
    
    // index: vado dall'ultimo elemento al primo
    index_register = gen_load_immediate(program,dest_var->arraySize);
    index = create_expression(index_register,REGISTER);

    
    dest_register = getNewRegister(program);
    dest = create_expression(dest_register,REGISTER);
    
    start = assignNewLabel(program);
    end = newLabel(program);
    
    // se index è a 0 salto alla fine
    gen_andb_instruction(program,index_register,index_register,index_register,CG_DIRECT_ALL);
    gen_beq_instruction(program,end,0);
    
    // compute index
    gen_subi_instruction(program,index_register,index_register,1);
    
    // carico i due sorgenti
    int src1_register = loadArrayElement(program,source_var1->ID,index);
    int src2_register = loadArrayElement(program,source_var2->ID,index);
    
    // eseguo l'operazione adatta a seconda di vec_op
    switch (vec_op) {
        case VADD:
            gen_add_instruction(program,dest_register,src1_register,src2_register,CG_DIRECT_ALL);
            break;
        case VSUB:
            gen_sub_instruction(program,dest_register,src1_register,src2_register,CG_DIRECT_ALL);
            break;
        default:
            notifyError(AXE_INVALID_OPCODE);
            break;
    }
    
    storeArrayElement(program,dest_var->ID,index,dest);
    
    gen_bt_instruction(program,start,0);
    
    assignLabel(program,end);
    
    
}