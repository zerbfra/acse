/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * asm_engine.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _ASM_TRANSLATION
#define _ASM_TRANSLATION

#include "collections.h"
#include "asm_struct.h"

typedef struct
{
	t_list *code; /* the instruction+data segment */
	t_list *labels; /* a set of asm_labels */
	int codesize; /* the size of the instruction segment */
}t_translation_infos;

/* create an instance of `t_translation_info' initializing the internal data
 * of every field of the structure */
extern t_translation_infos * initStructures(int *errorcode);

/* Insert an instruction inside the `code' list of `infos' */
extern int addInstruction(t_translation_infos *infos
		, t_asm_instruction *instruction);

/* Insert a new label. The label must be initialized externally */
extern int insertLabel(t_translation_infos *infos, t_asm_label *label);

/* find a label with a given `ID' */
extern t_asm_label * findLabel(t_translation_infos *infos, char *ID, int *asm_errorcode);

/* remove a label */
extern int removeLabel(t_translation_infos *infos, char *ID);

/* add a block of data into the data segment */
extern int addData(t_translation_infos *infos, t_asm_data *data);

/* finalization of the `infos' structure */
extern int finalizeStructures(t_translation_infos *infos);

/* begin the translation process */
extern int asm_writeObjectFile(t_translation_infos *infos, char *output_file);

#endif
