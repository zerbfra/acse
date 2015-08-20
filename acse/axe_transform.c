/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_transform.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "axe_transform.h"
#include "symbol_table.h"
#include "cflow_constants.h"
#include "reg_alloc_constants.h"
#include "axe_errors.h"
#include "axe_debug.h"

extern int errorcode;
extern int cflow_errorcode;

typedef struct t_tempLabel
{
   t_axe_label *labelID;
   int regID;
} t_tempLabel;


static t_axe_instruction * _createUnary (t_program_infos *program
            , int reg, t_axe_label *label, int opcode);
static t_axe_instruction * createUnaryInstruction
               (t_program_infos *program, int reg, int opcode);
static t_tempLabel * allocTempLabel(t_axe_label *labelID, int regID);
static void freeTempLabel(t_tempLabel *tempLabel);
static void finalizeListOfTempLabels(t_list *tempLabels);
static int compareTempLabels(void *valA, void *valB);

/* create new locations into the data segment in order to manage correctly
 * spilled variables */
static void updateTheDataSegment
            (t_program_infos *program, t_list *tempLabels);
static void updateTheCodeSegment
            (t_program_infos *program, t_cflow_Graph *graph);
static t_list * _insertLoad(t_program_infos *program, t_cflow_Graph *graph
      , t_basic_block *block, t_cflow_Node *current_node
            , t_cflow_var *var, t_list *usedVars);
static t_list * _insertStore(t_program_infos *program, t_cflow_Graph *graph
      , t_basic_block *block, t_cflow_Node *current_node
            , t_cflow_var *var, t_list *usedVars);
            
/* create a load instruction without assigning it to program */
static t_axe_instruction * createLoadInstruction
                  (t_program_infos *program, int reg);
/* create a store instruction without assigning it to program */
static t_axe_instruction * createStoreInstruction
                  (t_program_infos *program, int reg);

/* update the control flow informations by unsing the result
 * of the register allocation process and a list of bindings
 * between new assembly labels and spilled variables */
static void updatCflowInfos(t_program_infos *program, t_cflow_Graph *graph
            , t_reg_allocator *RA, t_list *label_bindings);

/* this function returns a list of t_templabel containing binding
 * informations between spilled variables and labels that will point
 * to a memory block in the data segment */
static t_list * retrieveLabelBindings(t_program_infos *program, t_reg_allocator *RA);
      
int _insertLoadSpill(t_program_infos *program, int temp_register, int selected_register
            , t_cflow_Graph *graph, t_basic_block *current_block
            , t_cflow_Node *current_node, t_list *labelBindings, int before);
            
int _insertStoreSpill(t_program_infos *program, int temp_register, int selected_register
            , t_cflow_Graph *graph, t_basic_block *current_block
            , t_cflow_Node *current_node, t_list *labelBindings, int before);

      
static t_list * _insertStore(t_program_infos *program, t_cflow_Graph *graph
      , t_basic_block *current_block, t_cflow_Node *current_node
            , t_cflow_var *var, t_list *usedVars)
{
   /* we have to insert a store instruction into the code */
   t_axe_instruction *storeInstr;
   t_cflow_Node *storeNode;

   /* create a load instruction */
   storeInstr = createStoreInstruction
            (program, var->ID);

   /* test if an error occurred */
   if (errorcode != AXE_OK) {
      free_Instruction(storeInstr);
      notifyError(errorcode);
   }
               
   if (storeInstr != NULL)
   {
      /* create a node for the store instruction */
      storeNode = allocNode (graph, storeInstr);
      if (cflow_errorcode != CFLOW_OK) {
         finalizeNode(storeNode);
         free_Instruction(storeInstr);
         return usedVars;
      }

      /* insert the node `loadNode' before `current_node' */
      insertNodeAfter(current_block, current_node, storeNode);
   
      /* update the list of usedVars */
      usedVars = removeElement(usedVars, storeNode->uses[0]);
   }

   return usedVars;
}

t_list * _insertLoad(t_program_infos *program, t_cflow_Graph *graph
      , t_basic_block *current_block, t_cflow_Node *current_node
            , t_cflow_var *var, t_list *usedVars)
{
   /* we have to insert a load instruction into the code */
   t_axe_instruction *current_instr;
   t_axe_instruction *loadInstr;
   t_cflow_Node *loadNode;

   /* retrieve the current instruction from the current node */
   current_instr = current_node->instr;

   /* create a load instruction */
   loadInstr = createLoadInstruction
         (program, var->ID);

   /* test if an error occurred */
   if (errorcode != AXE_OK) {
      free_Instruction(loadInstr);
      notifyError(errorcode);
   }

   if (loadInstr != NULL)
   {
      /* create a node for the load instruction */
      loadNode = allocNode (graph, loadInstr);

      /* test if an error occurred */
      if (cflow_errorcode != CFLOW_OK) {
         finalizeNode(loadNode);
         free_Instruction(loadInstr);
         return usedVars;
      }

      /* update the label informations */
      if (current_instr->labelID != NULL) {
         loadInstr->labelID = current_instr->labelID;
         current_instr->labelID = NULL;
      }
   
      /* insert the node `loadNode' before `current_node' */
      insertNodeBefore(current_block, current_node, loadNode);

      /* update the list of usedVars */
      usedVars = addElement(usedVars, loadNode->def, -1);
   }

   return usedVars;
}

void updateTheCodeSegment(t_program_infos *program, t_cflow_Graph *graph)
{
   t_list *current_bb_element;
   t_list *current_nd_element;
   t_basic_block *bblock;
   t_cflow_Node *node;
   
   /* preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   if (graph == NULL)
      notifyError(AXE_INVALID_CFLOW_GRAPH);

   current_bb_element = graph->blocks;
   while(current_bb_element != NULL)
   {
      bblock = (t_basic_block *) LDATA(current_bb_element);

      current_nd_element = bblock->nodes;
      while(current_nd_element != NULL)
      {
         node = (t_cflow_Node *) LDATA(current_nd_element);

         program->instructions =
               addElement(program->instructions, node->instr, -1);
         
         current_nd_element = LNEXT(current_nd_element);
      }

      current_bb_element = LNEXT(current_bb_element);
   }
}

void updateTheDataSegment(t_program_infos *program, t_list *labelBindings)
{
   t_list *current_element;
   t_tempLabel *current_TL;
   t_axe_data *new_data_info;

   /* preconditions */
   if (program == NULL) {
      finalizeListOfTempLabels(labelBindings);
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   }

   /* initialize the value of `current_element' */
   current_element = labelBindings;
   while(current_element != NULL)
   {
      current_TL = (t_tempLabel *) LDATA(current_element);

      new_data_info = alloc_data (DIR_WORD, 0, current_TL->labelID);
         
      if (new_data_info == NULL){
         finalizeListOfTempLabels(labelBindings);
         notifyError(AXE_OUT_OF_MEMORY);
      }

      /* update the list of directives */
      program->data = addElement(program->data, new_data_info, -1);
      
      current_element = LNEXT(current_element);
   }
}

t_axe_instruction * _createUnary (t_program_infos *program
            , int reg, t_axe_label *label, int opcode)
{
   t_axe_instruction *result;
   
   /* preconditions */
   if (program == NULL) {
      errorcode = AXE_PROGRAM_NOT_INITIALIZED;
      return NULL;
   }
   
   if (label == NULL) {
      errorcode = AXE_INVALID_LABEL;
      return NULL;
   }

   /* create an instance of `t_axe_instruction' */
   result = alloc_instruction(opcode);
   if (result == NULL) {
      errorcode = AXE_OUT_OF_MEMORY;
      return NULL;
   }

   result->reg_1 = alloc_register(reg, 0);
   if (result->reg_1 == NULL) {
      errorcode = AXE_OUT_OF_MEMORY;
      free_Instruction(result);
      return NULL;
   }

   /* initialize an address info */
   result->address = alloc_address(LABEL_TYPE, 0, label);
   if (result->address == NULL) {
      errorcode = AXE_OUT_OF_MEMORY;
      free_Instruction(result);
      return NULL;
   }

   return result;
}

int compareTempLabels(void *valA, void *valB)
{
   t_tempLabel *tlA;
   t_tempLabel *tlB;
   
   if (valA == NULL)
      return 0;
   if (valB == NULL)
      return 0;

   tlA = (t_tempLabel *) valA;
   tlB = (t_tempLabel *) valB;

   return (tlA->regID == tlB->regID);
}

t_tempLabel * allocTempLabel(t_axe_label *labelID, int regID)
{
   t_tempLabel *result;

   /* preconditions */
   if (labelID == NULL) {
      errorcode = AXE_INVALID_LABEL;
      return NULL;
   }

   /* create a new temp-label */
   result = _AXE_ALLOC_FUNCTION(sizeof(t_tempLabel));
   if (result == NULL) {
      errorcode = AXE_OUT_OF_MEMORY;
      return NULL;
   }
   
   /* initialize the temp label */
   result->labelID = labelID;
   result->regID = regID;

   return result;
}

void freeTempLabel(t_tempLabel *tempLabel)
{
   if (tempLabel != NULL)
      _AXE_FREE_FUNCTION(tempLabel);
}

void finalizeListOfTempLabels(t_list *tempLabels)
{
   t_list *current_element;
   t_tempLabel *tempLabel;

   /* test the preconditions */
   if (tempLabels == NULL)
      return;

   /* assign to current_element the address of the first
    * element of the list `tempLabels' */
   current_element = tempLabels;
   
   while(current_element != NULL)
   {
      tempLabel = (t_tempLabel *) LDATA(current_element);
      if (tempLabel != NULL)
         freeTempLabel(tempLabel);
      
      current_element = LNEXT(current_element);
   }

   /* finalize the list of elements of type `t_tempLabel' */
   freeList(tempLabels);
}

void updateProgramInfos(t_program_infos *program
         , t_cflow_Graph *graph, t_reg_allocator *RA)
{
   t_list *label_bindings;

   /* retrieve a list of t_templabels for the given RA infos.*/
   label_bindings = retrieveLabelBindings(program, RA);

   /* update the content of the data segment */
   updateTheDataSegment(program, label_bindings);
   
   /* update the control flow graph with the reg-alloc infos. */
   updatCflowInfos(program, graph, RA, label_bindings);

   /* erase the old code segment */
   freeList(program->instructions);
   program->instructions = NULL;

   /* update the code segment informations */
   updateTheCodeSegment(program, graph);

   /* finalize the list of tempLabels */
   finalizeListOfTempLabels(label_bindings);
}

t_list * retrieveLabelBindings(t_program_infos *program, t_reg_allocator *RA)
{
   int counter;
   t_list *result;
   t_tempLabel *tlabel;
   t_axe_label *axe_label;
   
   /* preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   if (RA == NULL)
      notifyError(AXE_INVALID_REG_ALLOC);

   /* initialize the local variable `result' */
   result = NULL;
   tlabel = NULL;

   for (counter = 0; counter <= RA->varNum; counter++)
   {
      if (RA->bindings[counter] == RA_SPILL_REQUIRED)
      {
         /* retrieve a new label */
         axe_label = newLabel(program);
         if (axe_label == NULL)
            notifyError(AXE_INVALID_LABEL);

         /* create a new tempLabel */
         tlabel = allocTempLabel(axe_label, counter);

         /* add the current tlabel to the list of labelbindings */
         result = addElement(result, tlabel, -1);
      }
   }
   
   /* postcondition: return the list of bindings */
   return result;
}

t_axe_instruction * createUnaryInstruction
               (t_program_infos *program, int reg, int opcode)
{
   char *varID;
   int sy_errorcode;
   t_axe_variable *axe_var;

   /* test the preconditions */
   if ((reg == REG_INVALID) || (reg == REG_0)) {
      errorcode = AXE_INVALID_REGISTER_INFO;
      return NULL;
   }

   if (program == NULL || program->sy_table == NULL) {
      errorcode = AXE_PROGRAM_NOT_INITIALIZED;
      return NULL;
   }

   /* initialize the value of errorcode */
   sy_errorcode = SY_TABLE_OK;
   varID = getIDfromLocation(program->sy_table, reg, &sy_errorcode);
   if (varID == NULL)
      return NULL;

   /* retrieve the variable associated with `varID' */
   axe_var = getVariable (program, varID);

   return _createUnary (program, reg, axe_var->labelID, opcode);
}

t_axe_instruction * createStoreInstruction
                  (t_program_infos *program, int reg)
{
   return createUnaryInstruction(program, reg, STORE);
}

t_axe_instruction * createLoadInstruction
                  (t_program_infos *program, int reg)
{
   return createUnaryInstruction(program, reg, LOAD);
}

t_cflow_Graph * insertLoadAndStoreInstr
         (t_program_infos *program, t_cflow_Graph *graph)
{
   t_list *current_bb_element;
   t_basic_block *current_block;
   t_list *current_nd_element;
   t_cflow_Node *current_node;
   t_list *usedVars;

   /* assertions */
   assert(program != NULL);
   assert(graph != NULL);
   
   /* initialize the list of variables used by this basic block */
   usedVars = NULL;
   current_bb_element = graph->blocks;
   while (current_bb_element != NULL)
   {
      current_block = (t_basic_block *) LDATA(current_bb_element);

      /* retrieve the list of nodes for the current basic block */
      current_nd_element = current_block->nodes;
      while(current_nd_element != NULL)
      {
         current_node = (t_cflow_Node *) LDATA(current_nd_element);

         /* test if we have to insert a store */
         if (  (current_node->def != NULL)
               && ((current_node->def)->ID != REG_0) )
         {
            if (findElement(usedVars, current_node->def) == NULL)
            {
               usedVars = addElement(usedVars, current_node->def, -1);
            }
         }

         /* test if we have to insert a load */
         if (  (current_node->uses[0] != NULL)
               && ((current_node->uses[0])->ID != REG_0) )
         {
            if (findElement(usedVars, current_node->uses[0]) == NULL)
            {
               usedVars = _insertLoad(program, graph, current_block
                     , current_node, current_node->uses[0], usedVars);
            }
         }
         if (  (current_node->uses[1] != NULL)
               && ((current_node->uses[1])->ID != REG_0) )
         {
            if (findElement(usedVars, current_node->uses[1]) == NULL)
            {
               usedVars = _insertLoad(program, graph, current_block
                     , current_node, current_node->uses[1], usedVars);
            }
         }
         if (  (current_node->uses[2] != NULL)
               && ((current_node->uses[2])->ID != REG_0) )
         {
            if (findElement(usedVars, current_node->uses[2]) == NULL)
            {
               usedVars = _insertLoad(program, graph, current_block
                     , current_node, current_node->uses[2], usedVars);
            }
         }

         /* retrieve the next element */
         current_nd_element = LNEXT(current_nd_element);
      }

      current_nd_element = getLastElement(current_block->nodes);
      while((current_nd_element != NULL) && (usedVars != NULL))
      {
         current_node = (t_cflow_Node *) LDATA(current_nd_element);

         /* test if we have to insert a store */
         if (  (current_node->uses[0] != NULL)
               && ((current_node->uses[0])->ID != REG_0) )
         {
            if (findElement(usedVars, current_node->uses[0]) != NULL)
            {
               usedVars = _insertStore(program, graph, current_block
                     , current_node, current_node->uses[0], usedVars);
            }
         }
         /* test if we have to insert a store */
         if (  (current_node->uses[1] != NULL)
               && ((current_node->uses[1])->ID != REG_0) )
         {
            if (findElement(usedVars, current_node->uses[1]) != NULL)
            {
               usedVars = _insertStore(program, graph, current_block
                     , current_node, current_node->uses[1], usedVars);
            }
         }
         /* test if we have to insert a store */
         if (  (current_node->uses[2] != NULL)
               && ((current_node->uses[2])->ID != REG_0) )
         {
            if (findElement(usedVars, current_node->uses[2]) != NULL)
            {
               usedVars = _insertStore(program, graph, current_block
                     , current_node, current_node->uses[2], usedVars);
            }
         }
         /* test if we have to insert a store */
         if (  (current_node->def != NULL)
               && ((current_node->def)->ID != REG_0) )
         {
            if (findElement(usedVars, current_node->def) != NULL)
            {
               usedVars = _insertStore(program, graph, current_block
                     , current_node, current_node->def, usedVars);
            }
         }
         
         /* retrieve the previous element */
         current_nd_element = LPREV(current_nd_element);
      }

      current_bb_element = LNEXT(current_bb_element);
   }

   /* free the list `usedVars' */
   freeList(usedVars);
   
   return graph;
}

void updatCflowInfos(t_program_infos *program, t_cflow_Graph *graph
            , t_reg_allocator *RA, t_list *label_bindings)
{
   t_list *current_bb_element;
   t_basic_block *current_block;
   t_axe_instruction *current_instr;
   t_list *current_nd_element;
   t_cflow_Node *current_node;
   int current_row;
   int found;
      
   /* preconditions */
   assert(program != NULL);
   assert(graph != NULL);
   assert(RA != NULL);
   
   /* initialize local variables */
   current_row = 0;
   found = 0;
   
   current_bb_element = graph->blocks;
   while (current_bb_element != NULL)
   {
      int counter;
      int assignedRegisters[RA_MIN_REG_NUM][3];

      current_block = (t_basic_block *) LDATA(current_bb_element);
      assert(current_block != NULL);

      /* initialize used_Registers */
      for (counter = 0; counter < RA_MIN_REG_NUM; counter ++)
      {
         assignedRegisters[counter][0] = REG_INVALID; /* invalid register */
         assignedRegisters[counter][1] = 0;           /* need write back */
         assignedRegisters[counter][2] = 0;           /* currently used */
      }

      /* retrieve the list of nodes for the current basic block */
      current_nd_element = current_block->nodes;
      while(current_nd_element != NULL)
      {
         int used_Registers[3] = {-1, -1, -1};

         /* retrieve the data associated with the current node of the block */
         current_node = (t_cflow_Node *) LDATA(current_nd_element);

         /* fetch the current instruction */
         current_instr = current_node->instr;

         /* Test if a requested variable is already loaded into a register */
         if (current_instr->reg_1  != NULL)
         {
            if (RA->bindings[(current_instr->reg_1)->ID]
                  == RA_SPILL_REQUIRED)
            {
               int current_row = 0;
               int found = 0;
               
               while ((current_row < RA_MIN_REG_NUM) && !found)
               {
                  if (assignedRegisters[current_row][0]
                           == (current_instr->reg_1)->ID)
                  {
                     /* update the value of used_Register */
                     used_Registers[0] = current_row;

                     /* update the value of `assignedRegisters` */
                     /* set currently used flag */
                     assignedRegisters[current_row][2] = 1;

                     /* test if a write back is needed */
                     if (! (current_instr->reg_1)->indirect)
                     {
                        /* if the current instruction is a STORE we don't need
                         * to write back the value of reg_1 again in memory. */
                        /* Also if the current instruction is a WRITE instruction
                         * we don't have to set the flag "dirty" for the
                         * register assignedRegisters[current_row], since reg_1
                         * is "used" but not "defined" */
                        if (  (current_instr->opcode != STORE)
                              && (current_instr->opcode != AXE_WRITE) )
                        {
                           assignedRegisters[current_row][1] = 1;
                        }
                        /* if the current instruction is a STORE we have to
                         * notify that the write back is happened by
                         * resetting the flag "dirty" */
                        if (current_instr->opcode == STORE)
                           assignedRegisters[current_row][1] = 0;
                     }

                     /* notify that the value was found */
                     found = 1;
                  }

                  current_row ++;
               }
            }
         }
         if (current_instr->reg_2  != NULL)
         {
            if (RA->bindings[(current_instr->reg_2)->ID]
                  == RA_SPILL_REQUIRED)
            {
               int current_row = 0;
               int found = 0;
               
               while ((current_row < RA_MIN_REG_NUM) && !found)
               {
                  if (assignedRegisters[current_row][0]
                           == (current_instr->reg_2)->ID)
                  {
                     /* update the value of used_Register */
                     used_Registers[1] = current_row;

                     /* update the value of `assignedRegisters` */
                     /* set currently used flag */
                     assignedRegisters[current_row][2] = 1;

                     /* notify that the value was found */
                     found = 1;
                  }
                  
                  current_row ++;
               }
            }
         }
         if (current_instr->reg_3  != NULL)
         {
            if (RA->bindings[(current_instr->reg_3)->ID]
                  == RA_SPILL_REQUIRED)
            {
               int current_row = 0;
               int found = 0;
               
               while ((current_row < RA_MIN_REG_NUM) && !found)
               {
                  if (assignedRegisters[current_row][0]
                           == (current_instr->reg_3)->ID)
                  {
                     /* update the value of used_Register */
                     used_Registers[2] = current_row;

                     /* update the value of `assignedRegisters` */
                     /* set currently used flag */
                     assignedRegisters[current_row][2] = 1;

                     /* notify that the value was found */
                     found = 1;
                  }
                  
                  current_row ++;
               }
            }
         }
         /* phase two */
         if (current_instr->reg_2  != NULL)
         {
            if ((RA->bindings[(current_instr->reg_2)->ID]
                  == RA_SPILL_REQUIRED) && (used_Registers[1] == -1))
            {
               int current_row = 0;
               int found = 0;
               
               while ((current_row < RA_MIN_REG_NUM) && !found)
               {
                  if (assignedRegisters[current_row][2] == 0)
                  {
                     int register_found = current_row + NUM_REGISTERS
                           + 1 - RA_MIN_REG_NUM;
                     
                     if (assignedRegisters[current_row][1] == 1)
                     {
                        /* NEED WRITE BACK */
                        _insertStoreSpill(program, assignedRegisters[current_row][0]
                              , register_found, graph, current_block
                                    , current_node, label_bindings, 1);
                     }

                     _insertLoadSpill(program, (current_instr->reg_2)->ID
                           , register_found, graph, current_block
                                , current_node, label_bindings, 1);

                     /* update the control informations */
                     assignedRegisters[current_row][0] = (current_instr->reg_2)->ID;
                     assignedRegisters[current_row][1] = 0;
                     assignedRegisters[current_row][2] = 1;
                     used_Registers[1] = current_row;

                     found = 1;
                  }
                  
                  current_row ++;
               }
               assert(found != 0);
            }
         }
         if (current_instr->reg_3  != NULL)
         {
            if ((RA->bindings[(current_instr->reg_3)->ID]
                  == RA_SPILL_REQUIRED) && (used_Registers[2] == -1))
            {
               int current_row = 0;
               int found = 0;

               if ((current_instr->reg_3)->ID == (current_instr->reg_2)->ID)
               {
                  used_Registers[2] = used_Registers[1];
                  found = 1;
               }

               while ((current_row < RA_MIN_REG_NUM) && !found)
               {
                  if (assignedRegisters[current_row][2] == 0)
                  {
                     int register_found = current_row + NUM_REGISTERS
                           + 1 - RA_MIN_REG_NUM;
                     
                     if (assignedRegisters[current_row][1] == 1)
                     {
                        /* NEED WRITE BACK */
                        _insertStoreSpill(program, assignedRegisters[current_row][0]
                              , register_found, graph, current_block
                                    , current_node, label_bindings, 1);
                     }

                     _insertLoadSpill(program, (current_instr->reg_3)->ID
                           , register_found, graph, current_block
                                , current_node, label_bindings, 1);

                     /* update the control informations */
                     assignedRegisters[current_row][0] = (current_instr->reg_3)->ID;
                     assignedRegisters[current_row][1] = 0;
                     assignedRegisters[current_row][2] = 1;
                     used_Registers[2] = current_row;
                     
                     found = 1;
                  }
                  
                  current_row ++;
               }
            }
         }
         if (current_instr->reg_1  != NULL)
         {
            if ((RA->bindings[(current_instr->reg_1)->ID]
                  == RA_SPILL_REQUIRED) && (used_Registers[0] == -1))
            {
               int current_row = 0;
               int found = 0;
               
               while ((current_row < RA_MIN_REG_NUM) && !found)
               {
                  if (assignedRegisters[current_row][2] == 0)
                  {
                     int register_found = current_row + NUM_REGISTERS
                           + 1 - RA_MIN_REG_NUM;
                     
                     if (assignedRegisters[current_row][1] == 1)
                     {
                        /* NEED WRITE BACK */
                        _insertStoreSpill(program, assignedRegisters[current_row][0]
                              , register_found, graph, current_block
                                    , current_node, label_bindings, 1);
                     }

                     /* test if we need to load the value from register */
                     if (  (current_instr->reg_1)->indirect
                           || (current_instr->opcode == AXE_WRITE))
                     {
                        _insertLoadSpill(program, (current_instr->reg_1)->ID
                              , register_found, graph, current_block
                                    , current_node, label_bindings, 1);
                     }

                     /* update the control informations */
                     assignedRegisters[current_row][0] = (current_instr->reg_1)->ID;
                     
                     if (! (current_instr->reg_1)->indirect)
                     {
                        if (  (current_instr->opcode != STORE)
                              && (current_instr->opcode != AXE_WRITE) )
                        {
                           assignedRegisters[current_row][1] = 1;
                        }
                        if (current_instr->opcode == STORE)
                           assignedRegisters[current_row][1] = 0;
                     }
                     assignedRegisters[current_row][2] = 1;

                     used_Registers[0] = current_row;
                     
                     found = 1;
                  }
                  
                  current_row ++;
               }
               assert(found != 0);
            }
         }
         
         /* update the instruction informations */
         if (current_instr->reg_1 != NULL)
         {
            if (used_Registers[0] != -1)
            {
               current_instr->reg_1->ID = used_Registers[0] + NUM_REGISTERS
                           + 1 - RA_MIN_REG_NUM;

               assignedRegisters[used_Registers[0]][2] = 0;
            }
            else
               current_instr->reg_1->ID =
                     RA->bindings[(current_instr->reg_1)->ID];
         }
         if (current_instr->reg_2 != NULL)
         {
            if (used_Registers[1] != -1)
            {
               current_instr->reg_2->ID = used_Registers[1] + NUM_REGISTERS
                           + 1 - RA_MIN_REG_NUM;
               assignedRegisters[used_Registers[1]][2] = 0;
            }
            else
               current_instr->reg_2->ID =
                     RA->bindings[(current_instr->reg_2)->ID];
         }
         if (current_instr->reg_3 != NULL)
         {
            if (used_Registers[2] != -1)
            {
               current_instr->reg_3->ID = used_Registers[2] + NUM_REGISTERS
                           + 1 - RA_MIN_REG_NUM;
               assignedRegisters[used_Registers[2]][2] = 0;
            }
            else
               current_instr->reg_3->ID =
                     RA->bindings[(current_instr->reg_3)->ID];
         }


         /* retrieve the previous element */
         current_nd_element = LNEXT(current_nd_element);
      }

      /* initialize used_Registers */
      for (counter = 0; counter < RA_MIN_REG_NUM; counter ++)
      {
         if (assignedRegisters[counter][1] == 1)
         {
            /* NEED WRITE BACK */
            _insertStoreSpill(program, assignedRegisters[counter][0]
                     , (counter + NUM_REGISTERS + 1 - RA_MIN_REG_NUM)
                           , graph, current_block
                              , current_node, label_bindings, 1);
         }
      }
      
      /* retrieve the next basic block element */
      current_bb_element = LNEXT(current_bb_element);
   }
}

int _insertStoreSpill(t_program_infos *program, int temp_register, int selected_register
            , t_cflow_Graph *graph, t_basic_block *current_block
            , t_cflow_Node *current_node, t_list *labelBindings, int before)
{
   t_axe_instruction *storeInstr;
   t_axe_instruction *current_instr;
   t_cflow_Node *storeNode;
   t_list *elementFound;
   t_tempLabel pattern;
   t_tempLabel *tlabel;

   /* initialize current_instr */
   current_instr = current_node->instr;

   pattern.regID = temp_register;
   elementFound = CustomfindElement (labelBindings
               , &pattern, compareTempLabels);

   if (elementFound == NULL) {
      finalizeNode(storeNode);
      errorcode = AXE_TRANSFORM_ERROR;
      return -1;
   }

   tlabel = (t_tempLabel *) LDATA(elementFound);
   assert(tlabel != NULL);

   /* create a store instruction */
   storeInstr = _createUnary (program
            , selected_register, tlabel->labelID, STORE);

   /* test if an error occurred */
   if (errorcode != AXE_OK) {
      free_Instruction(storeInstr);
      return -1;
   }

   /* create a node for the load instruction */
   storeNode = allocNode (graph, storeInstr);
   if (cflow_errorcode != CFLOW_OK) {
      finalizeNode(storeNode);
      free_Instruction(storeInstr);
      errorcode = AXE_TRANSFORM_ERROR;
      return -1;
   }

   /* test if we have to insert the node `storeNode' before `current_node'
    * inside the basic block */
   if (before == 0)
      insertNodeAfter(current_block, current_node, storeNode);
   else
      insertNodeBefore(current_block, current_node, storeNode);
   
   return 0;
}

int _insertLoadSpill(t_program_infos *program, int temp_register, int selected_register
            , t_cflow_Graph *graph, t_basic_block *block
            , t_cflow_Node *current_node, t_list *labelBindings, int before)
{
   t_axe_instruction *loadInstr;
   t_axe_instruction *current_instr;
   t_cflow_Node *loadNode;
   t_list *elementFound;
   t_tempLabel pattern;
   t_tempLabel *tlabel;

   /* initialize current_instr */
   current_instr = current_node->instr;

   pattern.regID = temp_register;
   elementFound = CustomfindElement (labelBindings
               , &pattern, compareTempLabels);

   if (elementFound == NULL) {
      finalizeNode(loadNode);
      errorcode = AXE_TRANSFORM_ERROR;
      return -1;
   }

   tlabel = (t_tempLabel *) LDATA(elementFound);
   assert(tlabel != NULL);
      
   /* create a load instruction */
   loadInstr = _createUnary (program
            , selected_register, tlabel->labelID, LOAD);

   /* test if an error occurred */
   if (errorcode != AXE_OK) {
      free_Instruction(loadInstr);
      finalizeNode(loadNode);
      return -1;
   }

   /* update the value of storeInstr and instr */
   (loadInstr->reg_1)->ID = selected_register;

   /* create a node for the load instruction */
   loadNode = allocNode (graph, loadInstr);

   /* test if an error occurred */
   if (cflow_errorcode != CFLOW_OK) {
      finalizeNode(loadNode);
      free_Instruction(loadInstr);
      errorcode = AXE_TRANSFORM_ERROR;
      return -1;
   }

   if ((current_node->instr)->labelID != NULL)
   {
      /* modify the label informations */
      loadInstr->labelID = (current_node->instr)->labelID;
      (current_node->instr)->labelID = NULL;
   }
                  
   if (before == 1)
      /* insert the node `loadNode' before `current_node' */
      insertNodeBefore(block, current_node, loadNode);
   else
      insertNodeAfter(block, current_node, loadNode);

   return 0;
}
