/*
 * Giovanni Agosta, Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * main.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fetch.h"
#include "machine.h"
#include "decode.h"

int main(int argc, char **argv)
{
	FILE *fp;       		/* pointer to the object file    */
	int len=0;      		/* length of the object file   */
	unsigned int *code=NULL;	/* pointer to the code block */
	int lcode = 0;  		/* length of the code block         */
	int mmgmt=BASIC;		/* memory management */
	int i;
	int breakat=-1; 		/* break execution at instruction # */
	int count=0;    		/* iterations counter    */
	pc=0;           		/* PC register is set ot zero in the beginning */
#ifdef DEBUG
   decoded_instr *current_instr;
#endif
	
	/* Apertura del file oggetto */
	if (argc<2)
	{
		/* L'esecuzione termina poichè non è stato specificato il file
		 * oggetto come argomento */
		fprintf(stdout, "Formal Languages & Compilers Machine, 2007/2008.\n"
			"\n\nSyntax:\n\tmace [options] objectfile\n");
	  	return NOARGS;
	}
	
	/* apre il file oggetto specificato come argomento */
  	fp = fopen(argv[argc-1],"r");
	if (fp == NULL)
	{
		fprintf(stderr,"Object file %s doesn't exist.\n", argv[argc-1]);
		
		/* termina l'esecuzione in quanto è stato specificato un file
		 * oggetto inesistente */
		return NOFILE;
	}
	
	for(i=1; i<argc-1; i++)
	{
		if (strcmp(argv[i],"segmented")==0) 
			mmgmt = SEGMENTED;
		else if (strcmp(argv[i],"break")==0)
		{
			char *error;
			
			/* legge il prossimo argomento come numero in base 10 */
			breakat = strtol(argv[i+1],&error,10);
			
			if (*error != '\0')
			{
				/* errore nella lettura dell'argomento */
				#ifdef DEBUG
				fprintf(stderr,"Error while reading option "
					"break: %d=%c, string=%s "
					"=> %d.\n",*error,*error,argv[i+1],breakat);
				#endif
				return WRONG_ARGS;
      			}
			i++; /* salta l'argomento gia' letto */
		}
	}

	#ifdef DEBUG
	fprintf(stderr,"Running with %s memory \n", (mmgmt == BASIC)? "BASIC" : "SEGMENTED");
	#endif
	
	/* Resetta i registri e la memoria */
	for (i=0; i<NREGS; i++) reg[i] = 0;
	for (i=0; i<MEMSIZE; i++) mem[i] = 0;
	
	/* Calcola lunghezza del file */
	fseek(fp,0,SEEK_END);
	len = ftell(fp);
	fseek(fp,0,SEEK_SET);
	
   #ifdef DEBUG
   fprintf(stderr,"Available memory: %d. "
               "Requested memory: %d \n", MEMSIZE, len);
   #endif
   
	if (mmgmt==BASIC)
	{
		/* Usa una sola area di memoria per codice e dati */
		if (len>MEMSIZE)
		{
			fprintf(stderr,"Out of memory.\n");
	
			return MEM_FAULT;
		}
    		
		code = (unsigned int*)mem;
	} else
	{
		/* Alloca spazio in memoria per il codice */
		code = (unsigned int*) malloc(sizeof(unsigned int)*len);
	}

	/* Carica il codice macchina in memoria   */
	for (i=0; i<len; i=i+2)
	{
		if (i==0)
      {
         unsigned int c1,c2,c3,c4;
         c1 = fgetc(fp);
         c2 = fgetc(fp);
         c3 = fgetc(fp);
         c4 = fgetc(fp);

         if (c1!='L' && c2 != 'F' && c3!='C' && c4!='M')
		   {
			   #ifdef DEBUG
			   fprintf(stderr,"Wrong object file format.\n");
			   #endif
		  
			   return WRONG_FORMAT;
		   }
         else
         {
            int value[4];
            int found;
            int count;

            /* initialize the local variables */
            count = 0;
            found = 0;
            memset(value, 0, 4);

            while (count < 4)
            {
               found += fread(value + count, 4, 1, fp);

               /* verify the header format */
               if (value[count] != 0)
               {
                  #ifdef DEBUG
                  fprintf(stderr,"Wrong object file format.\n");
                  #endif
        
                  return WRONG_FORMAT;
               }
               count ++;
            }
          }
      }
		
		if (!fread(code + lcode, 4, 1, fp))
         break;
		
		lcode++;
	}
   #ifdef DEBUG
   fprintf(stderr,"Starting execution.\n");
   print_regs(stderr);
   print_psw(stderr);
   print_Memory_Dump(stderr, lcode);
   if (pc < lcode) {
      current_instr = decode(code[0]);
      print(stderr, current_instr);
      free(current_instr);
   }
   #endif

   /* decodifica ed esegue il codice */
	for (pc=0; pc<lcode && pc>=0; )
	{
      pc = fetch_execute(code,pc);

#ifdef DEBUG
      print_regs(stderr);
      print_psw(stderr);
      print_Memory_Dump(stderr, lcode);
#endif

		reg[0]=0; /* reset R0 to 0; R0 is wired to 0, so we ignore all writes */

		count++; /* conta le istruzioni eseguite */
		if ((breakat>0) && (breakat<=count))
		{
			#ifdef DEBUG
			fprintf(stderr,"Break after %d instructions.\n",count);
			#endif
			return BREAK;
		}
		
		/* Check the HALT condition */
		if (pc == _HALT)
			return OK;

#ifdef DEBUG
      current_instr = decode(code[pc]);
      fprintf(stderr,"\n\n");
      print(stderr, current_instr);
      free(current_instr);
#endif
	}
	
	#ifdef DEBUG
	fprintf(stderr,"Memory access error.\n");
	#endif
	
	return pc;
}
