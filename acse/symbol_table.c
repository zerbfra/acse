/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * symbol_table.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <stdlib.h>
#include <assert.h>
#include "symbol_table.h"
#include "axe_debug.h"
#include "collections.h"
#include "axe_labels.h"

/* symbol table */

struct t_symbol_table
{
   t_list *symbols;
};

static int symCompare (void *symA, void *symB);
static int regCompare (void *symA, void *symB);
static t_symbol * getSymFromID(t_symbol_table *table, char *ID);
static t_symbol * getSymFromLocation(t_symbol_table *table, int location);


char * getIDfromLocation(t_symbol_table *table, int location, int *errorcode)
{
   t_symbol *sym;

   /* preconditions */
   if (table == NULL) {
      if (errorcode != NULL)
         (* errorcode) =  SY_TABLE_NOT_INITIALIZED;
      return NULL;
   }

   if (location == REG_INVALID) {
      if (errorcode != NULL)
         (* errorcode) =  SY_INVALID_REQUEST;
      return NULL;
   }

   /* retrieve the symbol from the location info. */
   sym = getSymFromLocation(table, location);
   if (sym == NULL)
      return NULL;

   /* postconditions */
   assert(sym->ID != NULL);

   return sym->ID;
}

int getTypeFromID(t_symbol_table *table, char *ID, int type)
{
   t_symbol *found;
   
   /* test the preconditions */
   if (ID == NULL)
      return SY_LOCATION_UNSPECIFIED;

   /* search for the symbol with the given ID */
   found = getSymFromID(table, ID);

   /* test the postconditions */
   if (found == NULL)
      return SY_LOCATION_UNSPECIFIED;

   /* return the value of the `reg_location' field for the found symbol */
   return found->reg_location;
}

int getLocation(t_symbol_table *table, char *ID, int *errorcode)
{
   t_symbol *found;
   
   /* test the preconditions */
   if (ID == NULL)
   {
      if (errorcode != NULL)
      {
         (* errorcode) = SY_INVALID_REQUEST;
         return SY_LOCATION_UNSPECIFIED;
      }
   }

   /* search for the symbol with the given ID */
   found = getSymFromID(table, ID);

   /* test the postconditions */
   if (found == NULL)
   {
      if (errorcode != NULL)
      {
         (* errorcode) = SY_UNDEFINED;
         return SY_LOCATION_UNSPECIFIED;
      }
   }

   if (errorcode != NULL)
      (* errorcode) = SY_TABLE_OK;
   
   /* return the value of the `reg_location' field for the found symbol */
   return found->reg_location;
}

int setLocation(t_symbol_table *table, char *ID, int reg)
{
   t_symbol *found;
   
   /* test the preconditions */
   if (ID == NULL)
      return SY_INVALID_REQUEST;

   /* search for the symbol with the given ID */
   found = getSymFromID(table, ID);

   /* test the postconditions */
   if (found == NULL)
      return SY_UNDEFINED;

   /* set the `reg_location' field for the found symbol */
   found->reg_location = reg;

   return SY_TABLE_OK;
}

/* initialize the symbol table */
t_symbol_table * initialize_sy_table()
{
   t_symbol_table *result;

   /* create an instance of `t_symbol_tabel' */
   result = (t_symbol_table *) malloc(sizeof(t_symbol_table));
   if (result == NULL)
      return NULL;

   /* initialize the internal data associated with the symbol table */
   result->symbols = NULL;

   /* return the symbol table */
   return result;
}

int finalize_sy_table(t_symbol_table *table)
{
   t_list   *current_symbol;
   
   if (table == NULL)
      return SY_TABLE_NOT_INITIALIZED;

   /* initialize the value of current_symbol */
   current_symbol = table->symbols;

   while (current_symbol != NULL)
   {
      /* free the symbol */
      free(LDATA(current_symbol));

      /* select the new symbol */
      current_symbol = LNEXT(current_symbol);
   }
   
   /* deallocate memory for the sy_table */
   freeList(table->symbols);

   /* update the global variable sy_table */
   table->symbols = NULL;

   /* free the memory slot associated with the symbol table */
   free(table);
   
   return SY_TABLE_OK;
}

static int regCompare (void *symA, void *symB)
{
   t_symbol *sA;
   t_symbol *sB;
         
   /* preconditions */
   if (symA == NULL)
   {
      if (symB == NULL)
         return 1;
      return 0;
   }

   if (symB == NULL)
      return 0;

   sA = (t_symbol *) symA;
   sB = (t_symbol *) symB;

   return (sA->reg_location == sB->reg_location);
}

/* Function used when a compare is needed between two labels */
int symCompare (void *symA, void *symB)
{
   t_symbol *sA;
   t_symbol *sB;
         
   /* preconditions */
   if (symA == NULL)
   {
      if (symB == NULL)
         return 1;
      return 0;
   }

   if (symB == NULL)
      return 0;

   sA = (t_symbol *) symA;
   sB = (t_symbol *) symB;

   assert(sA->ID != NULL);
   assert(sB->ID != NULL);

   return (!strcmp(sA->ID, sB->ID));
}

/* put a symbol into the symbol table */
int putSym(t_symbol_table *table, char *ID, int type)
{
   t_symbol pattern;
   t_symbol *new_symbol;
   
   if (table == NULL)
      return SY_TABLE_NOT_INITIALIZED;

   if (table->symbols == NULL)
   {
      /* initialize pattern */
      pattern.ID = ID;

      /* verify if the symbol is valid */
      if (  CustomfindElement(table->symbols
                  , &pattern, symCompare) != NULL)
      {
         /* symbol already defined */
         return SY_ALREADY_DEFINED;
      }
   }
   
   /* add the new symbol to the symbol table */
   new_symbol = (t_symbol *) malloc(sizeof(t_symbol));

   /* verify if new_symbol is a valid pointer */
   if (new_symbol == NULL)
   {
      /* out of memory error */
      return SY_MEMALLOC_ERROR;
   }
   
   /* initialize the new symbol */
   new_symbol->ID = ID;
   new_symbol->type = type;
   new_symbol->reg_location = SY_LOCATION_UNSPECIFIED;

   /* add the new symbol to the symbol table */
   table->symbols = addElement(table->symbols, new_symbol, -1);

   return SY_TABLE_OK;
}

t_symbol * getSymFromLocation(t_symbol_table *table, int location)
{
   t_symbol pattern;
   t_symbol *symbol_found;
   t_list   *l_element;
   
   /* preconditions */
   if (table == NULL)
      return NULL;

   /* initialize pattern */
   pattern.reg_location = location;

   /* search for a symbol with the given ID */
   l_element = CustomfindElement(table->symbols, &pattern, regCompare);

   /* postconditions */
   if (l_element == NULL)
      return NULL;

   /* retrieve the symbol information */
   symbol_found = (t_symbol *) LDATA(l_element);

   /* return the symbol */
   return symbol_found;
}

/* retrieve informations about a symbol */
t_symbol * getSymFromID(t_symbol_table *table, char *ID)
{
   t_symbol pattern;
   t_symbol *symbol_found;
   t_list   *l_element;
   
   /* preconditions */
   if (table == NULL)
      return NULL;

   /* initialize pattern */
   pattern.ID = ID;

   /* search for a symbol with the given ID */
   l_element = CustomfindElement(table->symbols, &pattern, symCompare);

   /* postconditions */
   if (l_element == NULL)
      return NULL;

   /* retrieve the symbol information */
   symbol_found = (t_symbol *) LDATA(l_element);

   /* return the symbol */
   return symbol_found;
}

#ifndef NDEBUG
/* This function print out to the file `fout' the content of the
 * symbol table given as input. The resulting text is formatted in
 * the following way: <ID> -- <TYPE> -- <REGISTER> */
void printSymbolTable(t_symbol_table *table, FILE *fout)
{
   t_list *current_element;
   t_symbol *current_symbol;
   
   /* preconditions */
   if (table == NULL)
      return;

   if (fout == NULL)
      fout = stdin;

   fprintf(fout, "--------------------------------\n");
   fprintf(fout, "          SYMBOL TABLE\n");
   fprintf(fout, "--------------------------------\n");
   fprintf(fout, "NUMBER OF SYMBOLS : %d \n"
            , getLength(table->symbols));
   fprintf(fout, "--------------------------------\n\n");

   /* initialize the value of current_symbol */
   current_element = table->symbols;

   while (current_element != NULL)
   {
      current_symbol = (t_symbol *) LDATA(current_element);

      fprintf(fout, "ID : %s\t;; TYPE : %s\t;;", current_symbol->ID
         , dataTypeToString(current_symbol->type) );
      if (current_symbol->reg_location == SY_LOCATION_UNSPECIFIED)
         fprintf(fout, " LOCATION : [UNSPECIFIED] \n");
      else
         fprintf(fout, " LOCATION : R%d \n", current_symbol->reg_location);
      
      /* select the new symbol */
      current_element = LNEXT(current_element);
   }
}
#endif
