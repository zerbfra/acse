/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_reg_alloc.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "axe_reg_alloc.h"
#include "reg_alloc_constants.h"
#include "axe_debug.h"
#include "axe_errors.h"

extern int errorcode;

static int compareIntervalIDs(void *varA, void *varB);
static int compareStartPoints(void *varA, void *varB);
static int compareEndPoints(void *varA, void *varB);
static t_list * updateListOfIntervals(t_list *result
            , t_cflow_Node *current_node, int counter);
static t_list * allocFreeRegisters(int regNum);
static t_list * addFreeRegister
      (t_list *registers, int regID, int position);
static int assignRegister(t_reg_allocator *RA);
static t_list * expireOldIntervals(t_reg_allocator *RA
            , t_list *active_intervals, t_live_interval *interval);
static t_list * getLiveIntervals(t_cflow_Graph *graph);
static int insertListOfIntervals(t_reg_allocator *RA, t_list *intervals);
static int insertLiveInterval(t_reg_allocator *RA, t_live_interval *interval);
static void finalizeLiveInterval (t_live_interval *interval);
static t_live_interval * allocLiveInterval(int varID, int startPoint, int endPoint);
static t_list * spillAtInterval(t_reg_allocator *RA
      , t_list *active_intervals, t_live_interval *interval);


/*
 * Perform a spill that allows the allocation of the given
 * interval, given the list of active live intervals
 */
t_list * spillAtInterval(t_reg_allocator *RA
      , t_list *active_intervals, t_live_interval *interval)
{
   t_list *last_element;
   t_live_interval *last_interval;
   
   /* get the last element of the list of active intervals */
   /* Precondition: if the list of active intervals is empty
    * we are working on a machine with 0 registers available
    * for the register allocation */
   if (active_intervals == NULL)
   {
      RA->bindings[interval->varID] = RA_SPILL_REQUIRED;
      return active_intervals;
   }
   
   last_element = getLastElement(active_intervals);
   last_interval = (t_live_interval *) LDATA(last_element);

   /* If the current interval ends before the last one, spill
    * the last one, otherwise spill the current interval   */
   if (last_interval->endPoint > interval->endPoint)
   {
      RA->bindings[interval->varID]
            = RA->bindings[last_interval->varID];
      RA->bindings[last_interval->varID] = RA_SPILL_REQUIRED;

      active_intervals = removeElement(active_intervals, last_interval);
      
      active_intervals = addSorted(active_intervals
                  , interval, compareEndPoints);
   }
   else
      RA->bindings[interval->varID] = RA_SPILL_REQUIRED;

   return active_intervals;
}

/*
 * Given two live intervals, compare them by
 * the start point (find whichever starts first)
 */
int compareStartPoints(void *varA, void *varB)
{
   t_live_interval *liA;
   t_live_interval *liB;
   
   if (varA == NULL)
      return 0;

   if (varB == NULL)
      return 0;

   liA = (t_live_interval *) varA;
   liB = (t_live_interval *) varB;

   return (liA->startPoint - liB->startPoint);
}

/*
 * Given two live intervals, compare them
 * by the end point (find whichever ends first)
 */
int compareEndPoints(void *varA, void *varB)
{
   t_live_interval *liA;
   t_live_interval *liB;
   
   if (varA == NULL)
     return 0;

   if (varB == NULL)
      return 0;

   liA = (t_live_interval *) varA;
   liB = (t_live_interval *) varB;

   return (liA->endPoint - liB->endPoint);
}

/*
 * Given two live intervals, check if they
 * refer to the same interval
 */
int compareIntervalIDs(void *varA, void *varB)
{
   t_live_interval *liA;
   t_live_interval *liB;

   if (varA == NULL)
      return 0;

   if (varB == NULL)
      return 0;

   liA = (t_live_interval *) varA;
   liB = (t_live_interval *) varB;

   return (liA->varID == liB->varID);
}

/*
 * Insert a live interval in the register allocator
 */
int insertLiveInterval(t_reg_allocator *RA, t_live_interval *interval)
{
   /* test the preconditions */
   if (RA == NULL)
      return RA_INVALID_ALLOCATOR;

   if (interval == NULL)
      return RA_INVALID_INTERVAL;

   /* test if an interval for the requested variable is already inserted */
   if (CustomfindElement(RA->live_intervals
               , interval, compareIntervalIDs) != NULL)
   {
      return RA_INTERVAL_ALREADY_INSERTED;
   }

   /* add the given interval to the list of intervals,
    * in order of starting point */
    RA->live_intervals = addSorted(RA->live_intervals
            , interval, compareStartPoints);
   
   return RA_OK;
}

/*
 * Allocate and initialize the free registers list,
 * assuming regNum general purpose registers
 */
t_list * allocFreeRegisters(int regNum)
{
   int count;
   t_list *result;

   /* initialize the local variables */
   count = 1;
   result = NULL;
   
   while(count <= regNum)
   {
      /* add a new register to the list of free registers */
      result = addFreeRegister(result, count, -1);

      /* update the `count' variable */
      count ++;
   }

   /* return the list of free registers */
   return result;
}

/*
 * Return register regID to the free list in the given position
 */
t_list * addFreeRegister(t_list *registers, int regID, int position)
{
   int *element;

   /* Allocate memory space for the reg id */
   element = (int *) _AXE_ALLOC_FUNCTION(sizeof(int));
   if (element == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* initialize element */
   (* element) = regID;
   
   /* update the list of registers */
   registers = addElement(registers, element, position);

   /* return the list of free registers */
   return registers;
}

/*
 * Get a new register from the free list
 */
int assignRegister(t_reg_allocator *RA)
{
   int regID;

   /* Check for list not empty*/
   if (RA->freeRegisters == NULL)
      return RA_SPILL_REQUIRED;

   /* Get the name of the first free register */
   regID = (* ((int *) LDATA(RA->freeRegisters)));
   /* Free the space allocated to hold the register id */
   _AXE_FREE_FUNCTION(LDATA(RA->freeRegisters));
   /* Remove the structure from the free registers list */
   RA->freeRegisters = removeFirst(RA->freeRegisters);

   return regID;
}

/*
 * Allocate and initialize the register allocator
 */
t_reg_allocator * initializeRegAlloc(t_cflow_Graph *graph)
{
   t_reg_allocator *result; /* the register allocator */
   t_list *intervals;
   t_list *current_elem;
   t_list *current_cflow_var;
   t_cflow_var *cflow_var;
   int max_var_ID;
   int counter;


   /* Check preconditions: the cfg must exist */
   if (graph == NULL)
      notifyError(AXE_INVALID_CFLOW_GRAPH);

   /* allocate memory for a new instance of `t_reg_allocator' */
   result = (t_reg_allocator *) _AXE_ALLOC_FUNCTION(sizeof(t_reg_allocator));
   if (result == NULL)
      notifyError(AXE_OUT_OF_MEMORY);
   
   /* initialize the register allocator informations */
   /* Reserve a few registers (RA_MIN_REG_NUM) to handle spills */
   result->regNum = NUM_REGISTERS - RA_MIN_REG_NUM;
   max_var_ID = getLength(graph->cflow_variables);

   /* retrieve the max identifier from each live interval */
   current_cflow_var = graph->cflow_variables;
   while (current_cflow_var != NULL)
   {
      /* fetch the data informations about a variable */
      cflow_var = (t_cflow_var *) LDATA(current_cflow_var);
      assert(cflow_var != NULL);
      
      /* update the value of max_var_ID */
      max_var_ID = (max_var_ID < cflow_var->ID)? cflow_var->ID : max_var_ID;

      /* retrieve the next variable */
      current_cflow_var = LNEXT(current_cflow_var);
   }
      
   /* update the value of `result->varNum' with the correct var. identifier */
   result->varNum = max_var_ID;
   
   current_elem = graph->cflow_variables;
   
   /* Assuming there are some variables to associate to regs,
    * allocate space for the binding array, and initialize it */
   
   /*alloc memory for the array of bindings */
   result->bindings = (int *) _AXE_ALLOC_FUNCTION
         (sizeof(int) * (result->varNum + 1) );
   
   /* test if an error occurred */
   if (result->bindings == NULL)
      notifyError(AXE_OUT_OF_MEMORY);
      
   /* initialize the array of bindings */
   for(counter = 0; counter <= result->varNum; counter++)
      result->bindings[counter] = RA_REGISTER_INVALID;

   /* Liveness analysis: compute the list of live intervals */
   result->live_intervals = NULL;
   intervals = getLiveIntervals(graph);

   /* Copy the liveness info into the register allocator */
   if (intervals != NULL)
   {
      if (insertListOfIntervals(result, intervals) != RA_OK)
      {
         finalizeRegAlloc(result);
         notifyError(AXE_REG_ALLOC_ERROR);
      }

      /* deallocate memory used to hold the results of the
       * liveness analysis */
      freeList(intervals);
   }

   /* create a list of freeRegisters */
   if (result->regNum > 0)
      result->freeRegisters = allocFreeRegisters(result->regNum);
   else
      result->freeRegisters = NULL;
   
   /* return the new register allocator */
   return result;
}

/*
 * Deallocate the register allocator data structures
 */
void finalizeRegAlloc(t_reg_allocator *RA)
{
   if (RA == NULL)
      return;

   /* If the list of live intervals is not empty,
    * deallocate its content */
   if (RA->live_intervals != NULL)
   {
      t_list *current_element;
      t_live_interval *current_interval;

      /* finalize the memory blocks associated with all
       * the live intervals */
      for (current_element = RA->live_intervals
            ;  current_element != NULL
            ;  current_element = LNEXT(current_element))
      {
         /* fetch the current interval */
         current_interval = (t_live_interval *) LDATA(current_element);
         if (current_interval != NULL)
         {
            /* finalize the memory block associated with
             * the current interval */
            finalizeLiveInterval(current_interval);
         }
      }

      /* deallocate the list of intervals */
      freeList(RA->live_intervals);
   }

   /* Free memory used for the variable/register bindings */
   if (RA->bindings != NULL)
      _AXE_FREE_FUNCTION(RA->bindings);
   if (RA->freeRegisters != NULL)
   {
      t_list *current_element;

      current_element = RA->freeRegisters;
      while (current_element != NULL)
      {
         _AXE_FREE_FUNCTION(LDATA(current_element));
         current_element = LNEXT(current_element);
      }

      freeList(RA->freeRegisters);
   }

   _AXE_FREE_FUNCTION(RA);
}

/*
 * Allocate and initialize a live interval data structure
 * with a given varID, starting and ending points
 */
t_live_interval * allocLiveInterval
               (int varID, int startPoint, int endPoint)
{
   t_live_interval *result;

   /* create a new instance of `t_live_interval' */
   result = _AXE_ALLOC_FUNCTION(sizeof(t_live_interval));
   if (result == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* initialize the new instance */
   result->varID = varID;
   result->startPoint = startPoint;
   result->endPoint = endPoint;

   /* return the new `t_live_interval' */
   return result;
}

/*
 * Deallocate a live interval
 */
void finalizeLiveInterval (t_live_interval *interval)
{
   if (interval == NULL)
      return;

   /* finalize the current interval */
   _AXE_FREE_FUNCTION(interval);
}

/*
 * Insert all elements of the intervals list into
 * the register allocator data structure
 */
int insertListOfIntervals(t_reg_allocator *RA, t_list *intervals)
{
   t_list *current_element;
   t_live_interval *interval;
   int ra_errorcode;
   
   /* preconditions */
   if (RA == NULL)
      return RA_INVALID_ALLOCATOR;
   if (intervals == NULL)
      return RA_OK;

   for (current_element = intervals
         ;  current_element != NULL
         ;  current_element = LNEXT(current_element) )
   {
      /* Get the current live interval */
      interval = (t_live_interval *) LDATA(current_element);
      
      if (interval == NULL)
         return RA_INVALID_INTERVAL;

      /* insert a new live interval */
      ra_errorcode = insertLiveInterval(RA, interval);

      /* test if an error occurred */
      if (ra_errorcode != RA_OK)
         return ra_errorcode;
   }

   return RA_OK;
}

/*
 * Perform live intervals computation
 */
t_list * getLiveIntervals(t_cflow_Graph *graph)
{
   t_list *current_bb_element;
   t_list *current_nd_element;
   t_basic_block *current_block;
   t_cflow_Node *current_node;
   t_list *result;
   int counter;

   /* preconditions */
   if (graph == NULL)
      return NULL;

   if (graph->blocks == NULL)
      return NULL;

   /* initialize the local variable `result' */
   result = NULL;

   /* intialize the instruction counter */
   counter = 0;
   
   /* fetch the first basic block */
   current_bb_element = graph->blocks;
   while (current_bb_element != NULL)
   {
      current_block = (t_basic_block *) LDATA(current_bb_element);

      /* fetch the first node of the basic block */
      current_nd_element = current_block->nodes;
      while(current_nd_element != NULL)
      {
         current_node = (t_cflow_Node *) LDATA(current_nd_element);

         /* update the live intervals with the liveness informations */
         result = updateListOfIntervals(result, current_node, counter);
         
         /* fetch the next node in the basic block */
         counter++;
         current_nd_element = LNEXT(current_nd_element);
      }

      /* fetch the next element in the list of basic blocks */
      current_bb_element = LNEXT(current_bb_element);
   }

   return result;
}

/*
 * Update the liveness interval for the variable 'id', used or defined
 * at position 'counter'.
 */
t_list *updateVarInterval( int id, int counter, t_list *intervals )
{
    t_list *element_found;
    t_live_interval *interval_found;
    t_live_interval pattern;

    if (id == RA_EXCLUDED_VARIABLE)
        return NULL;
    
    pattern.varID = id;
    /* search for the current live interval */
    element_found = CustomfindElement(intervals, &pattern, compareIntervalIDs);
    if (element_found != NULL)
    {
        interval_found = (t_live_interval *) LDATA(element_found);
        /* update the interval informations */
        if (interval_found->startPoint > counter)
            interval_found->startPoint = counter;
        if (interval_found->endPoint < counter)
            interval_found->endPoint = counter;
    }
    else
    {
        /* we have to add a new live interval */
        interval_found = allocLiveInterval(id, counter, counter);
        if (interval_found == NULL)
            notifyError(AXE_OUT_OF_MEMORY);
        intervals = addElement(intervals, interval_found, -1);
    }
    
    return intervals;
}

/*
 * Use liveness information to update the list of
 * live intervals
 */
t_list * updateListOfIntervals(t_list *result
         , t_cflow_Node *current_node, int counter)
{
   t_list *current_element;
   t_cflow_var *current_var;
   
   if (current_node == NULL)
      return result;

   current_element = current_node->in;
   while (current_element != NULL)
   {
      current_var = (t_cflow_var *) LDATA(current_element);

      result = updateVarInterval(current_var->ID, counter, result);
      
      /* fetch the next element in the list of live variables */
      current_element = LNEXT(current_element);
   }
   
   current_element = current_node->out;
   while (current_element != NULL)
   {
      current_var = (t_cflow_var *) LDATA(current_element);

      result = updateVarInterval(current_var->ID, counter, result);
      
      /* fetch the next element in the list of live variables */
      current_element = LNEXT(current_element);
   }

   if (current_node->def)
       result = updateVarInterval(current_node->def->ID, counter, result);

   return result;
}

/*
 * Remove from active_intervals all the live intervals that end before the
 * beginning of the current live interval
 */
t_list * expireOldIntervals(t_reg_allocator *RA, t_list *active_intervals
               , t_live_interval *interval)
{
   t_list *current_element;
   t_list *next_element;
   t_live_interval *current_interval;

   /* Check for valid register allocator and set of active intervals */
   if (active_intervals == NULL)
      return NULL;
   if (RA == NULL)
      return NULL;
   if (interval == NULL)
      return active_intervals;

   /* Iterate over the set of active intervals */
   current_element = active_intervals;
   while(current_element != NULL)
   {
      /* Get the live interval */
      current_interval = (t_live_interval *) LDATA(current_element);

      /* If the considered interval ends before the beginning of 
       * the current live interval, we don't need to keep track of
       * it anymore; otherwise, this is the first interval we must
       * still take into account when assigning registers */
      if (current_interval->endPoint >= interval->startPoint)
         return active_intervals;

      /* Get the next live interval */
      next_element = LNEXT(current_element);

      /* Remove the current element from the list */
      active_intervals = removeElement(active_intervals, current_interval);

      /* Free all the registers associated with the removed interval */
      RA->freeRegisters = addFreeRegister
            (RA->freeRegisters, RA->bindings[current_interval->varID], 0);

      /* Step to the next interval */
      current_element = next_element;
   }

   /* Return the updated list of active intervals */
   return active_intervals;
}

/*
 * Main register allocation function
 */
int execute_linear_scan(t_reg_allocator *RA)
{
   t_list *current_element;
   t_live_interval *current_interval;
   t_list *active_intervals;
   
   /* test the preconditions */
   if (RA == NULL)   /* Register allocator created? */
      return RA_INVALID_ALLOCATOR;
   if (RA->live_intervals == NULL) /* Liveness analysis ready? */
      return RA_OK;

   /* initialize the list of active intervals */
   active_intervals = NULL;
   
   /* Iterate over the list of live intervals */   
   for( current_element = RA->live_intervals
        ;   current_element != NULL
        ;   current_element = LNEXT(current_element) )
   {
      /* Get the live interval */
      current_interval = (t_live_interval *) LDATA(current_element);

      /* Check which intervals are ended and remove 
       * them from the active set, thus freeing registers */
      active_intervals = expireOldIntervals
               (RA, active_intervals, current_interval);

      /* If all registers are busy, perform a spill */
      if (getLength(active_intervals) == RA->regNum)
      {
         /* perform a spill */
         active_intervals = spillAtInterval
               (RA, active_intervals, current_interval);
      }
      else /* Otherwise, assign a new register to the current live interval */
      {
         RA->bindings[current_interval->varID] = assignRegister(RA);

         /* Add the current interval to the list of active intervals, in
          * order of ending points (to allow easier expire management) */
         active_intervals = addSorted(active_intervals
            , current_interval, compareEndPoints);
      }
   }

   /* free the list of active intervals */
   freeList(active_intervals);
   
   return RA_OK;
}
