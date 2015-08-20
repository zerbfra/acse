/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * asm_constants.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _ASM_CONSTANTS
#define _ASM_CONSTANTS

/* the size of an instruction (in bytes) */
#define ASM_INSTRUCTION_SIZE 4

/* the size of a word (in bytes) */
#define ASM_WORD_SIZE 4

/* alignment information */
#define ASM_ALIGMENT_SIZE 4

/* invalid memory offset */
#define ASM_INVALID_MEMORY_OFFSET -1

/* opcodes pseudo-M68000 */
#define ADD_OP	0
#define SUB_OP 1
#define ANDL_OP 2	
#define ORL_OP 3	
#define XORL_OP 4
#define ANDB_OP 5
#define ORB_OP 6
#define XORB_OP 7
#define MUL_OP 8
#define DIV_OP 9
#define SHL_OP 10
#define SHR_OP 11
#define ROTL_OP 12
#define ROTR_OP 13
#define NEG_OP 14
#define SPCL_OP 15
#define ADDI_OP 16
#define SUBI_OP 17
#define ANDLI_OP 18
#define ORLI_OP 19
#define XORLI_OP 20
#define ANDBI_OP 21
#define ORBI_OP 22
#define XORBI_OP 23
#define MULI_OP 24
#define DIVI_OP 25
#define SHLI_OP 26
#define SHRI_OP 27
#define ROTLI_OP 28
#define ROTRI_OP 29
#define NOTL_OP 30
#define NOTB_OP 31
#define NOP_OP 32
#define MOVA_OP 33
#define JSR_OP 34
#define RET_OP 35
#define HALT_OP 36
#define BT_OP 37
#define BF_OP 38
#define BHI_OP 39
#define BLS_OP 40
#define BCC_OP 41
#define BCS_OP 42
#define BNE_OP 43
#define BEQ_OP 44
#define BVC_OP 45
#define BVS_OP 46
#define BPL_OP 47
#define BMI_OP 48
#define BGE_OP 49
#define BLT_OP 50
#define BGT_OP 51
#define BLE_OP 52
#define SEQ_OP 53
#define SGE_OP 54
#define SGT_OP 55
#define SLE_OP 56
#define SLT_OP 57
#define SNE_OP 58
#define LOAD_OP 59
#define STORE_OP 60
#define READ_OP 61
#define WRITE_OP 62
#define INVALID_OPCODE -1

/* error codes */
#define ASM_OK 0
#define ASM_NOT_INITIALIZED_INFO 1
#define ASM_UNDEFINED_INSTRUCTION 2
#define ASM_INVALID_LABEL_FOUND 3
#define ASM_LABEL_ALREADY_PRESENT 4
#define ASM_UNDEFINED_DATA 5
#define ASM_INVALID_DATA_FORMAT 6
#define ASM_FOPEN_ERROR 7
#define ASM_INVALID_INPUT_FILE 8
#define ASM_FWRITE_ERROR 9
#define ASM_FCLOSE_ERROR 10
#define ASM_CODE_NOT_PRESENT 11
#define ASM_OUT_OF_MEMORY 12
#define ASM_INVALID_OPCODE 13
#define ASM_UNKNOWN_ERROR 14

/* instruction formats */
#define ASM_FORMAT_TER	0
#define ASM_FORMAT_BIN	1
#define ASM_FORMAT_UNR	2
#define ASM_FORMAT_JMP	3
#define ASM_FORMAT_NULL	4

/* data formats */
#define ASM_WORD	0
#define ASM_SPACE 1
#endif
