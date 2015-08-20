/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_cflow_graph.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <assert.h>
#include "axe_labels.h"
#include "axe_cflow_graph.h"
#include "cflow_constants.h"
#include "collections.h"
#include "axe_debug.h"

int cflow_errorcode;


static t_cflow_var * allocVariable (t_cflow_Graph *graph, int identifier);
static int isEndingNode(t_axe_instruction *instr);
static int isStartingNode(t_axe_instruction *instr);
static int isJumpInstruction(t_axe_instruction *instr);
static int isUnconditionalJump(t_axe_instruction *instr);
static int isHaltInstruction(t_axe_instruction *instr);
static int isLoadInstruction(t_axe_instruction *instr);
static void updateFlowGraph(t_cflow_Graph *graph);
static t_basic_block * searchLabel(t_cflow_Graph *graph, t_axe_label *label);
static void setDefUses(t_cflow_Graph *graph, t_cflow_Node *node);
static int performLivenessIteration(t_cflow_Graph *graph);
static int performLivenessOnBlock(t_basic_block *current_block, t_list *out);
static t_list * addVariableToSet(t_list *set
               , t_cflow_var *element, int *modified);
static int compare_CFLOW_Variables (void *a, void *b);
static t_list * computeLiveOutVars(t_cflow_Graph *graph,t_basic_block *block);


void performLivenessAnalysis(t_cflow_Graph *graph)
{
   int modified;

   do
   {
      modified = performLivenessIteration(graph);
   }while(modified);
}

t_list * computeLiveOutVars(t_cflow_Graph *graph, t_basic_block *block)
{
   t_list *current_elem;
   t_basic_block *current_succ;
   t_list *result;
   t_list *liveINVars;

   /* preconditions */
   if (block == NULL)
      return NULL;

   if (graph == NULL) {
      cflow_errorcode = CFLOW_GRAPH_UNDEFINED;
      return NULL;
   }
   
   /* initialize `current_elem' */
   current_elem = block->succ;

   /* initialize `result' */
   result = NULL;
   while(current_elem != NULL)
   {
      current_succ = (t_basic_block *) LDATA(current_elem);
      assert(current_succ != NULL);

      if (current_succ != graph->endingBlock)
      {
         liveINVars = getLiveINVars(current_succ);
         
         /* update the value of `result' */
         result = addListToSet(result
               , liveINVars, NULL, NULL);

         /* free the temporary list of live intervals */
         freeList(liveINVars);
      }
      
      current_elem = LNEXT(current_elem);
   }

   /* postconditions */
   return result;
}

t_list * getLiveOUTVars(t_basic_block *bblock)
{
   t_list *last_Element;
   t_cflow_Node *lastNode;
   
   if (bblock == NULL)
      return NULL;

   if (bblock->nodes == NULL)
      return NULL;

   
   last_Element = getLastElement(bblock->nodes);
   lastNode = (t_cflow_Node *) LDATA(last_Element);
   assert(lastNode != NULL);

   /* return a copy of the list of variables live in
    * input to the current basic block */
   return cloneList(lastNode->out);
}

t_list * getLiveINVars(t_basic_block *bblock)
{
   t_cflow_Node *firstNode;
   
   if (bblock == NULL)
      return NULL;

   if (bblock->nodes == NULL)
      return NULL;

   firstNode = (t_cflow_Node *) LDATA(bblock->nodes);
   assert(firstNode != NULL);

   /* return a copy of the list of variables live in
    * input to the current basic block */
   return cloneList(firstNode->in);
}

t_list * addVariableToSet(t_list *set
               , t_cflow_var *element, int *modified)
{
   /* test the preconditions */
   if (element == NULL)
      return set;

   if (CustomfindElement(set, element
            , compare_CFLOW_Variables) == NULL)
   {
      set = addElement(set, element, -1);
      if (modified != NULL)
         (* modified) = 1;
   }

   /* postconditions */
   return set;
}

t_list * addVariables(t_list *set, t_list *elements, int *modified)
{
   /* test the preconditions */
   if (set == NULL || elements == NULL)
      return set;

   /* update the set of variables */
   set = addListToSet(set, elements
         , compare_CFLOW_Variables, modified);
   
   /* postconditions: return the new list of variables */
   return set;
}

int compare_CFLOW_Variables (void *a, void *b)
{
   t_cflow_var *varA;
   t_cflow_var *varB;

   if (a == NULL)
   {
      if (b == NULL)
         return 1;
      return 0;
   }

   if (b == NULL)
      return 0;

   varA = (t_cflow_var *) a;
   varB = (t_cflow_var *) b;

   return (varA->ID == varB->ID);
}

int performLivenessOnBlock(t_basic_block *bblock, t_list *out)
{
   t_list *current_element;
   t_list *cloned_list;
   t_cflow_Node *next_node;
   t_cflow_Node *current_node;
   int modified;

   /* initialize the local variables */
   modified = 0;

   if (bblock == NULL || bblock->nodes == NULL) {
      cflow_errorcode = CFLOW_INVALID_BBLOCK;
      return modified;
   }

   current_element = getLastElement(bblock->nodes);
   current_node = (t_cflow_Node *) LDATA(current_element);
   assert(current_node != NULL);

   /* update the out set */
   current_node->out = addListToSet
         (current_node->out, out, NULL, &modified);

   /* update the in list */
   cloned_list = cloneList(current_node->out);
   
#if CFLOW_ALWAYS_LIVEIN_R0 == (1)
   if ((current_node->uses)[0] != NULL && (current_node->uses)[0]->ID != REG_0)
      cloned_list = addVariableToSet
            (cloned_list, (current_node->uses)[0], NULL);
   if ((current_node->uses)[1] != NULL && (current_node->uses)[1]->ID != REG_0)
      cloned_list = addVariableToSet
            (cloned_list, (current_node->uses)[1], NULL);
   if ((current_node->uses)[2] != NULL && (current_node->uses)[2]->ID != REG_0)
      cloned_list = addVariableToSet
            (cloned_list, (current_node->uses)[2], NULL);
#else
   if ((current_node->uses)[0] != NULL)
      cloned_list = addVariableToSet
            (cloned_list, (current_node->uses)[0], NULL);
   if ((current_node->uses)[1] != NULL)
      cloned_list = addVariableToSet
            (cloned_list, (current_node->uses)[1], NULL);
   
   if ((current_node->uses)[2] != NULL)
      cloned_list = addVariableToSet
            (cloned_list, (current_node->uses)[2], NULL);
#endif



#if CFLOW_ALWAYS_LIVEIN_R0 == (1)
   if (current_node->def != NULL && (current_node->def)->ID != REG_0)
#else
   if (current_node->def != NULL)
#endif
   {
      int found = 0;
      int current_use_idx = 0;
      
      do
      {
         if ((current_node->uses)[current_use_idx] != NULL)
         {
            if ((current_node->uses)[current_use_idx]->ID == current_node->def->ID)
               found = 1;
         }
         
         current_use_idx++;
         
      } while((found == 0) && (current_use_idx < 3));
      
      if (!found)
         cloned_list = removeElement(cloned_list, current_node->def);
   }
   
   current_node->in = addListToSet
         (current_node->in, cloned_list, NULL, &modified);

   /* remove the cloned list */
   freeList(cloned_list);
   
   /* set the new value of next_node */
   next_node = current_node;
   current_element = LPREV(current_element);
   while (current_element != NULL)
   {
      /* take a new node */
      current_node = (t_cflow_Node *) LDATA(current_element);
      assert(current_node != NULL);

      /* clone the `in' list of the next_node */
      cloned_list = cloneList(next_node->in);
      
      /* update the out list */
      current_node->out = addListToSet
            (current_node->out, cloned_list, NULL, &modified);
      
      /* remove the cloned list */
      freeList(cloned_list);

      /* clone the `in' list of the next_node */
      cloned_list = cloneList(current_node->out);
      
      /* update the in list */
#if CFLOW_ALWAYS_LIVEIN_R0 == (1)
      if ((current_node->uses)[0] != NULL && (current_node->uses)[0]->ID != REG_0)
         cloned_list = addVariableToSet
               (cloned_list, (current_node->uses)[0], NULL);
      if ((current_node->uses)[1] != NULL && (current_node->uses)[1]->ID != REG_0)
         cloned_list = addVariableToSet
               (cloned_list, (current_node->uses)[1], NULL);
      if ((current_node->uses)[2] != NULL && (current_node->uses)[2]->ID != REG_0)
         cloned_list = addVariableToSet
               (cloned_list, (current_node->uses)[2], NULL);
#else
      if ((current_node->uses)[0] != NULL)
         cloned_list = addVariableToSet
               (cloned_list, (current_node->uses)[0], NULL);
      if ((current_node->uses)[1] != NULL)
         cloned_list = addVariableToSet
               (cloned_list, (current_node->uses)[1], NULL);
      if ((current_node->uses)[2] != NULL)
         cloned_list = addVariableToSet
               (cloned_list, (current_node->uses)[2], NULL);
#endif
      
#if CFLOW_ALWAYS_LIVEIN_R0 == (1)
      if (current_node->def != NULL && (current_node->def)->ID != REG_0)
#else
      if (current_node->def != NULL)
#endif
      {
         int found = 0;
         int current_use_idx = 0;
      
         do
         {
            if ((current_node->uses)[current_use_idx] != NULL)
            {
               if ((current_node->uses)[current_use_idx]->ID
                        == current_node->def->ID)
               {
                  found = 1;
               }
            }
            current_use_idx++;
         
         } while((found == 0) && (current_use_idx < 3));
      
         if (!found)
            cloned_list = removeElement(cloned_list, current_node->def);
      }

      current_node->in = addListToSet
            (current_node->in, cloned_list, NULL, &modified);

      /* remove the cloned list */
      freeList(cloned_list);
      
      /* update the loop control informations */
      current_element = LPREV(current_element);
      next_node = current_node;
   }

   /* return the `modified' value */
   return modified;
}

static int isLoadInstruction(t_axe_instruction *instr)
{
   if (instr == NULL) {
      cflow_errorcode = CFLOW_INVALID_INSTRUCTION;
      return 0;
   }

   return (instr->opcode == LOAD) ? 1 : 0;
}

int isHaltInstruction(t_axe_instruction *instr)
{
   if (instr == NULL) {
      cflow_errorcode = CFLOW_INVALID_INSTRUCTION;
      return 0;
   }

   return (instr->opcode == HALT) ? 1 : 0;
}

int performLivenessIteration(t_cflow_Graph *graph)
{
   int modified;
   t_list *current_element;
   t_basic_block *current_bblock;

   /* initialize the value of the local variable `modified' */
   modified = 0;

   /* test the preconditions */
   if (graph == NULL) {
      cflow_errorcode = CFLOW_GRAPH_UNDEFINED;
      return modified;
   }

   /* test if `graph->endingBlock' is valid */
   if (graph->endingBlock == NULL) {
      cflow_errorcode = CFLOW_INVALID_BBLOCK;
      return modified;
   }

   /* retrieve the last basic block in the list */
   current_element = getLastElement(graph->blocks);
   
   while(current_element != NULL)
   {
      t_list *live_out_vars;
      
      current_bblock = (t_basic_block *) LDATA(current_element);
      assert(current_bblock != NULL);

      /* retrieve the variables that will be live out from this block */
      live_out_vars = computeLiveOutVars(graph, current_bblock);

      /* test if an error occurred */
      if (cflow_errorcode != CFLOW_OK)
         return modified;

      /* retrieve the liveness informations for the current bblock */
      if (performLivenessOnBlock(current_bblock, live_out_vars))
         modified = 1;

      /* remove the list `out' */
      freeList(live_out_vars);

      /* test if an error occurred */
      if (cflow_errorcode != CFLOW_OK) {
         return modified;
      }

      /* retrieve the previous element in the list */
      current_element = LPREV(current_element);
   }

   /* return 1 if another liveness iteration is required */
   return modified;
}

/* alloc a variable identifier */
t_cflow_var * allocVariable (t_cflow_Graph *graph, int identifier)
{
   t_cflow_var * result;
   t_list *elementFound;

   if (graph == NULL)
   {
      cflow_errorcode = CFLOW_GRAPH_UNDEFINED;
      return NULL;
   }

   /* alloc memory for a variable information */
   result = _AXE_ALLOC_FUNCTION(sizeof(t_cflow_var));
   if (result == NULL) {
      cflow_errorcode = CFLOW_OUT_OF_MEMORY;
      return NULL;
   }

   /* update the value of result */
   result->ID = identifier;
   
   /* test if a variable with the same identifier was already present */
   elementFound = CustomfindElement
         (graph->cflow_variables, result, compare_CFLOW_Variables);
   
   if (elementFound == NULL)
   {
      /* update the set of variables */
      graph->cflow_variables = addElement
            (graph->cflow_variables, result, -1);
   }
   else
   {
      _AXE_FREE_FUNCTION(result);
      result = (t_cflow_var *) LDATA(elementFound);
      assert(result != NULL);
      assert(result->ID == identifier);
   }
   
   /* return a new var identifier */
   return result;
}

/* set the def-use values for the current node */
void setDefUses(t_cflow_Graph *graph, t_cflow_Node *node)
{
   t_axe_instruction *instr;
   t_cflow_var *varDest;
   t_cflow_var *varSource1;
   t_cflow_var *varSource2;

   /* preconditions */
   if (graph == NULL) {
      cflow_errorcode = CFLOW_GRAPH_UNDEFINED;
      return;
   }
   
   if (node == NULL) {
      cflow_errorcode = CFLOW_INVALID_NODE;
      return;
   }

   if (node->instr == NULL) {
      cflow_errorcode = CFLOW_INVALID_INSTRUCTION;
      return;
   }

   if ((node->instr)->opcode == INVALID_OPCODE) {
      cflow_errorcode = CFLOW_INVALID_INSTRUCTION;
      return;
   }
   
   /* update the value of `instr' */
   instr = node->instr;

   /* if the instruction is a Jump instruction then does nothing */
   if ( isJumpInstruction(instr))
      return;

   /* initialize the values of varDest, varSource1 and varSource2 */
   varDest = NULL;
   varSource1 = NULL;
   varSource2 = NULL;

   /* update the values of the variables */
   if (instr->reg_1 != NULL)
      varDest = allocVariable(graph, (instr->reg_1)->ID);
   if (instr->reg_2 != NULL)
      varSource1 = allocVariable(graph, (instr->reg_2)->ID);
   if (instr->reg_3 != NULL)
      varSource2 = allocVariable(graph, (instr->reg_3)->ID);
   
   switch(instr->opcode)
   {
      case LOAD : case AXE_READ : node->def = varDest; break;
      case STORE : case AXE_WRITE : (node->uses)[0] = varDest; break;
      case SGE : case SGT: case SLE : case SLT : case SNE :
      case SEQ : node->def = varDest; break;
      case HALT : case RET : case JSR : case NOP : break;
      case MOVA : node->def = varDest; break;
      case NOTB : case NOTL : case ROTRI : case ROTLI :
      case SHRI : case SHLI : case DIVI : case MULI :
      case EORBI : case ORBI : case ANDBI : case EORLI :
      case ORLI : case ANDLI : case SUBI : case ADDI :
                node->def = varDest;
                (node->uses)[0] = varSource1; break;
      default :
                if ((instr->reg_1)->indirect)
                     (node->uses)[2] = varDest;
                else
                     node->def = varDest;
                (node->uses)[0] = varSource1;
                (node->uses)[1] = varSource2;
   }
}

/* test if the current instruction `instr' is a BT or a BF */
int isUnconditionalJump(t_axe_instruction *instr)
{
   if (isJumpInstruction(instr))
   {
      if ((instr->opcode == BT) || (instr->opcode == BF))
         return 1;
   }

   return 0;
}

/* test if the current instruction `instr' is a branch instruction */
int isJumpInstruction(t_axe_instruction *instr)
{
   if (instr == NULL)
      return 0;

   switch(instr->opcode)
   {
      case BT :
      case BF :
      case BHI :
      case BLS :
      case BCC :
      case BCS :
      case BNE :
      case BEQ :
      case BVC :
      case BVS :
      case BPL :
      case BMI :
      case BGE :
      case BLT :
      case BGT :
      case BLE : return 1;
      default : return 0;
   }
}

/* look up for a label inside the graph */
t_basic_block * searchLabel(t_cflow_Graph *graph, t_axe_label *label)
{
   t_list *current_element;
   t_basic_block *bblock;
   t_cflow_Node *current_node;
   
   /* preconditions: graph should not be a NULL pointer */
   if (graph == NULL){
      cflow_errorcode = CFLOW_GRAPH_UNDEFINED;
      return NULL;
   }

   /* test if we haven't to search for a label */
   if (label == NULL)
      return NULL;
   
   /* initialize `bblock' */
   bblock = NULL;
   
   current_element = graph->blocks;
   while(current_element != NULL)
   {
      bblock = (t_basic_block *) LDATA(current_element);
      assert(bblock != NULL);
      assert(bblock->nodes != NULL);

      /* retrieve the first node of the basic block */
      current_node = (t_cflow_Node *) LDATA(bblock->nodes);
      assert(current_node != NULL);

      /* if the first node holds a label information, we
       * have to verify if we have found the right label */
      if ((current_node->instr)->labelID != NULL)
      {
         if (compareLabels((current_node->instr)->labelID, label))
            /* we found the correct basic block */
            break;
      }

      /* retrieve the next element */
      current_element = LNEXT(current_element);
   }

   return bblock;
}

/* test if the current instruction `instr' is a labelled instruction */
int isStartingNode(t_axe_instruction *instr)
{
   /* preconditions */
   if ((instr == NULL) || (instr->opcode == INVALID_OPCODE))
   {
      cflow_errorcode = CFLOW_INVALID_INSTRUCTION;
      return 0;
   }

   /* test if the instruction holds a label identifier */
   if (instr->labelID != NULL)
   {
      if ((instr->labelID)->labelID == LABEL_UNSPECIFIED)
      {
         cflow_errorcode = CFLOW_INVALID_INSTRUCTION;
         return 0;
      }

      return 1;
   }
   
   return 0;
}

/* test if the current instruction will end a basic block */
int isEndingNode(t_axe_instruction *instr)
{
   /* preconditions */
   if ((instr == NULL) || (instr->opcode == INVALID_OPCODE))
   {
      cflow_errorcode = CFLOW_INVALID_INSTRUCTION;
      return 0;
   }

   switch (instr->opcode)
   {
      case JSR :
      case RET :
      case HALT:
      case BT :
      case BF :
      case BHI :
      case BLS :
      case BCC :
      case BCS :
      case BNE :
      case BEQ :
      case BVC :
      case BVS :
      case BPL :
      case BMI :
      case BGE :
      case BLT :
      case BGT :
      case BLE : return 1;
      default : return 0;
   }
}

/* allocate memory for a control flow graph */
t_cflow_Graph * allocGraph()
{
   t_cflow_Graph *result;

   result = _AXE_ALLOC_FUNCTION(sizeof(t_cflow_Graph));
   if (result == NULL) {
      cflow_errorcode = CFLOW_OUT_OF_MEMORY;
      return NULL;
   }

   /* initialize `result' */
   result->startingBlock = NULL;
   result->blocks = NULL;
   result->cflow_variables = NULL;
   result->endingBlock = allocBasicBlock();

   /* test if an error occurred */
   if (result->endingBlock == NULL) {
      cflow_errorcode = CFLOW_OUT_OF_MEMORY;
      _AXE_FREE_FUNCTION(result);
      return NULL;
   }

   /* return the just created cflow graph */
   return result;
}

/* finalize the memory associated with the given control flow graph */
void finalizeGraph(t_cflow_Graph *graph)
{
   t_list *current_element;
   t_basic_block *current_block;

   if (graph == NULL)
      return;

   current_element = graph->blocks;
   while (current_element != NULL)
   {
      /* retrieve the current node */
      current_block = (t_basic_block *) LDATA(current_element);
      assert(current_block != NULL);

      finalizeBasicBlock(current_block);

      current_element = LNEXT(current_element);
   }

   if (graph->blocks != NULL)
      freeList(graph->blocks);
   if (graph->endingBlock != NULL)
      finalizeBasicBlock(graph->endingBlock);
   if (graph->cflow_variables != NULL)
   {
      t_list *current_element;
      t_cflow_var *current_variable;

      current_element = graph->cflow_variables;
      while (current_element != NULL)
      {
         current_variable = (t_cflow_var *) LDATA(current_element);

         if (current_variable != NULL)
            _AXE_FREE_FUNCTION(current_variable);

         /* retrieve the next variable in the list */
         current_element = LNEXT(current_element);
      }

      freeList(graph->cflow_variables);
   }

   _AXE_FREE_FUNCTION(graph);
}

/* allocate memory for a basic block */
t_basic_block * allocBasicBlock()
{
   t_basic_block *result;
   
   result = _AXE_ALLOC_FUNCTION(sizeof(t_basic_block));
   if (result == NULL)
   {
      cflow_errorcode = CFLOW_OUT_OF_MEMORY;
      return NULL;
   }

   /* initialize result */
   result->pred = NULL;
   result->succ = NULL;
   result->nodes = NULL;

   return result;
}

/* free the memory associated with a given basic block */
void finalizeBasicBlock(t_basic_block *block)
{
   t_list *current_element;
   t_cflow_Node *current_node;

   if (block == NULL)
      return;

   if (block->pred != NULL)
      freeList(block->pred);
   if (block->succ != NULL)
      freeList(block->succ);

   /* initialize current_element */
   current_element = block->nodes;
   
   while (current_element != NULL)
   {
      /* retrieve the current node */
      current_node = (t_cflow_Node *) LDATA(current_element);

      /* free the memory associated with the current node */
      finalizeNode(current_node);

      /* retrieve the next node in the list */
      current_element = LNEXT(current_element);
   }

   freeList(block->nodes);
   
   /* free the memory associated with this basic block */
   _AXE_FREE_FUNCTION(block);
}

/* free the memory associated with a node of the graph */
void finalizeNode(t_cflow_Node *node)
{
   if (node == NULL)
      return;

   /* free the two lists `in' and `out' */
   if (node->in != NULL)
      freeList(node->in);
   if (node->out != NULL)
      freeList(node->out);

   /* free the current node */
   _AXE_FREE_FUNCTION(node);
}

t_cflow_Node * allocNode
      (t_cflow_Graph *graph, t_axe_instruction *instr)
{
   t_cflow_Node *result;

   /* test the preconditions */
   if (graph == NULL) {
      cflow_errorcode = CFLOW_GRAPH_UNDEFINED;
      return NULL;
   }
   
   if (instr == NULL)
   {
      cflow_errorcode = CFLOW_INVALID_INSTRUCTION;
      return NULL;
   }

   /* create a new instance of type `t_cflow_node' */
   result = _AXE_ALLOC_FUNCTION(sizeof(t_cflow_Node));

   /* test if an error occurred */
   if (result == NULL) {
      cflow_errorcode = CFLOW_OUT_OF_MEMORY;
      return NULL;
   }

   /* initialize result */
   result->def = NULL;
   result->uses[0] = NULL;
   result->uses[1] = NULL;
   result->uses[2] = NULL;
   result->instr = instr;

   /* set the def-uses for the current node */
   setDefUses(graph, result);

   /* test if an error occurred */
   if (cflow_errorcode != CFLOW_OK) {
      _AXE_FREE_FUNCTION(result);
      return NULL;
   }

   /* set the list of variables that are live in
    * and live out from the current node */
   result->in = NULL;
   result->out = NULL;
   
   /* return the node */
   return result;
}

void setPred(t_basic_block *block, t_basic_block *pred)
{
   /* preconditions */
   if (block == NULL) {
      cflow_errorcode = CFLOW_BBLOCK_UNDEFINED;
      return;
   }

   if (pred == NULL) {
      cflow_errorcode = CFLOW_INVALID_BBLOCK;
      return;
   }

   /* test if the block is already inserted in the list of predecessors */
   if (findElement(block->pred, pred) == NULL)
   {
      block->pred = addElement(block->pred, pred, -1);
      pred->succ = addElement(pred->succ, block, -1);
   }
}

void setSucc(t_basic_block *block, t_basic_block *succ)
{
   t_list *element_found;
   
   /* preconditions */
   if (block == NULL) {
      cflow_errorcode = CFLOW_BBLOCK_UNDEFINED;
      return;
   }

   if (succ == NULL) {
      cflow_errorcode = CFLOW_INVALID_BBLOCK;
      return;
   }

   element_found = findElement(block->succ, succ);
   
   /* test if the node is already inserted in the list of successors */
   if (element_found == NULL)
   {
      block->succ = addElement(block->succ, succ, -1);
      succ->pred = addElement(succ->pred, block, -1);
   }
}

void insertBlock(t_cflow_Graph *graph, t_basic_block *block)
{
   /* preconditions */
   if (graph == NULL)
   {
      cflow_errorcode = CFLOW_GRAPH_UNDEFINED;
      return;
   }
   
   if (block == NULL)
   {
      cflow_errorcode = CFLOW_INVALID_BBLOCK;
      return;
   }

   if (findElement(graph->blocks, block) != NULL)
   {
      cflow_errorcode = CFLOW_BBLOCK_ALREADY_INSERTED;
      return;
   }

   /* add the current node to the basic block */
   graph->blocks = addElement(graph->blocks, block, -1);

   /* test if this is the first basic block for the program */
   if (graph->startingBlock == NULL)
      graph->startingBlock = block;
}

/* insert a new node without updating the dataflow informations */
void insertNodeBefore(t_basic_block *block
      , t_cflow_Node *before_node, t_cflow_Node *new_node)
{
   int before_node_posn;
   t_list *before_node_elem;
   
   /* preconditions */
   if (block == NULL)
   {
      cflow_errorcode = CFLOW_INVALID_BBLOCK;
      return;
   }
   
   if (  (new_node == NULL)
         || (new_node->instr == NULL)
         || (before_node == NULL) )
   {
      cflow_errorcode = CFLOW_INVALID_NODE;
      return;
   }

   before_node_elem = findElement(block->nodes, before_node);
   if (before_node_elem == NULL)
   {
      cflow_errorcode = CFLOW_INVALID_NODE;
      return;
   }
   
   if (findElement(block->nodes, new_node) != NULL)
   {
      cflow_errorcode = CFLOW_NODE_ALREADY_INSERTED;
      return;
   }

   /* get the position of the before node */
   before_node_posn = getPosition(block->nodes, before_node_elem);
   assert(before_node_posn != -1);

   /* add the current node to the basic block */
   block->nodes = addElement(block->nodes, new_node, before_node_posn);
}

/* insert a new node without updating the dataflow informations */
void insertNodeAfter(t_basic_block *block
      , t_cflow_Node *after_node, t_cflow_Node *new_node)
{
   int after_node_posn;
   t_list *after_node_elem;
   
   /* preconditions */
   if (block == NULL)
   {
      cflow_errorcode = CFLOW_INVALID_BBLOCK;
      return;
   }
   
   if (  (new_node == NULL)
         || (new_node->instr == NULL)
         || (after_node == NULL) )
   {
      cflow_errorcode = CFLOW_INVALID_NODE;
      return;
   }

   after_node_elem = findElement(block->nodes, after_node);
   if (after_node_elem == NULL)
   {
      cflow_errorcode = CFLOW_INVALID_NODE;
      return;
   }
   
   if (findElement(block->nodes, new_node) != NULL)
   {
      cflow_errorcode = CFLOW_NODE_ALREADY_INSERTED;
      return;
   }

   /* get the position of the after node */
   after_node_posn = getPosition(block->nodes, after_node_elem);
   assert(after_node_posn != -1);

   /* add the current node to the basic block */
   block->nodes = addElement(block->nodes, new_node, (after_node_posn + 1));
}

void insertNode(t_basic_block *block, t_cflow_Node *node)
{
   /* preconditions */
   if (block == NULL)
   {
      cflow_errorcode = CFLOW_INVALID_BBLOCK;
      return;
   }
   
   if (node == NULL || node->instr == NULL)
   {
      cflow_errorcode = CFLOW_INVALID_NODE;
      return;
   }

   if (findElement(block->nodes, node) != NULL)
   {
      cflow_errorcode = CFLOW_NODE_ALREADY_INSERTED;
      return;
   }

   /* add the current node to the basic block */
   block->nodes = addElement(block->nodes, node, -1);
}

t_cflow_Graph * createFlowGraph(t_list *instructions)
{
   t_cflow_Graph *result;
   t_basic_block *bblock;
   t_list *current_element;
   t_cflow_Node *current_node;
   t_axe_instruction *current_instr;
   int startingNode;
   int endingNode;

   /* initialize the global variable `cflow_errorcode' */
   cflow_errorcode = CFLOW_OK;
   
   /* preconditions */
   if (instructions == NULL){
      cflow_errorcode = CFLOW_INVALID_PROGRAM_INFO;
      return NULL;
   }
   
   /* alloc memory for a new control flow graph */
   result = allocGraph();
   if (result == NULL)
      return NULL;

   /* set the starting basic block */
   bblock = NULL;

   /* initialize the current element */
   current_element = instructions;
   while(current_element != NULL)
   {
      /* retrieve the current instruction */
      current_instr = (t_axe_instruction *) LDATA(current_element);
      assert(current_instr != NULL);

      if (isLoadInstruction(current_instr))
      {
         current_element = LNEXT(current_element);
         continue;
      }
         
      /* create a new node for the current basic block */
      current_node = allocNode(result, current_instr);
      if (current_node == NULL){
         finalizeGraph(result);
         return NULL;
      }

      /* test if the current instruction will start or end a block */
      startingNode = isStartingNode(current_instr);
      endingNode = isEndingNode(current_instr);

      if (startingNode || bblock == NULL)
      {
         /* alloc a new basic block */
         bblock = allocBasicBlock();
         if (bblock == NULL) {
            finalizeGraph(result);
            finalizeNode(current_node);
            return NULL;
         }

         /* add the current instruction to the newly created
          * basic block */
         insertNode(bblock, current_node);
         if (cflow_errorcode != CFLOW_OK) {
            finalizeGraph(result);
            finalizeNode(current_node);
            finalizeBasicBlock(bblock);
            return NULL;
         }

         /* add the new basic block to the control flow graph */
         insertBlock(result, bblock);
         if (cflow_errorcode != CFLOW_OK) {
            finalizeGraph(result);
            finalizeNode(current_node);
            finalizeBasicBlock(bblock);
            return NULL;
         }
      }
      else
      {
         /* add the current instruction to the current
          * basic block */
         insertNode(bblock, current_node);
         if (cflow_errorcode != CFLOW_OK) {
            finalizeGraph(result);
            finalizeNode(current_node);
            return NULL;
         }
      }

      if (endingNode)
         bblock = NULL;

      /* retrieve the next element */
      current_element = LNEXT(current_element);
   }

   /* update the basic blocks chain */
   updateFlowGraph(result);
   if (cflow_errorcode != CFLOW_OK) {
      finalizeGraph(result);
      return NULL;
   }

   /*return the graph */
   return result;
}

void updateFlowGraph(t_cflow_Graph *graph)
{
   t_list *current_element;
   t_basic_block *current_block;
   t_basic_block *successor;
   
   /* preconditions: graph should not be a NULL pointer */
   if (graph == NULL){
      cflow_errorcode = CFLOW_GRAPH_UNDEFINED;
      return;
   }

   successor = NULL;
   current_element = graph->blocks;
   while(current_element != NULL)
   {
      t_list *last_element;
      t_cflow_Node *last_node;
      t_axe_instruction *last_instruction;
      t_basic_block *jumpBlock;
      
      /* retrieve the current block */
      current_block = (t_basic_block *) LDATA(current_element);
      assert(current_block != NULL);
      assert(current_block->nodes != NULL);

      /* get the last node of the basic block */
      last_element = getLastElement(current_block->nodes);
      assert(last_element != NULL);
      
      last_node = (t_cflow_Node *) LDATA(last_element);
      assert(last_node != NULL);

      last_instruction = last_node->instr;
      assert(last_instruction != NULL);

      if (isHaltInstruction(last_instruction))
      {
         setSucc(current_block, graph->endingBlock);
         setPred(graph->endingBlock, current_block);
      }
      else
      {
         if (isJumpInstruction(last_instruction))
         {
            if (  (last_instruction->address == NULL)
                  || ((last_instruction->address)->labelID == NULL) )
            {
               cflow_errorcode = CFLOW_INVALID_LABEL_FOUND;
               return;
            }
         
            jumpBlock = searchLabel(graph
                  , (last_instruction->address)->labelID);
            if (jumpBlock == NULL) {
               cflow_errorcode = CFLOW_INVALID_LABEL_FOUND;
               return;
            }

            /* add the jumpBlock to the list of successors of current_block */
            /* add also current_block to the list of predecessors of jumpBlock */
            setPred(jumpBlock, current_block);
            if (cflow_errorcode != CFLOW_OK)
               return;
            setSucc(current_block, jumpBlock);
            if (cflow_errorcode != CFLOW_OK)
               return;
         }

         if (!isUnconditionalJump(last_instruction))
         {
            t_basic_block *nextBlock;
            t_list *next_element;
            
            next_element = LNEXT(current_element);
            if (next_element != NULL)
            {
               nextBlock = LDATA(next_element);
               assert(nextBlock != NULL);
               
               setSucc(current_block, nextBlock);
               setPred(nextBlock, current_block);
            }
            else
            {
               setSucc(current_block, graph->endingBlock);
               setPred(graph->endingBlock, current_block);
            }
         
            if (cflow_errorcode != CFLOW_OK)
               return;
         }
      }

      /* update the value of `current_element' */
      current_element = LNEXT(current_element);
   }
}
