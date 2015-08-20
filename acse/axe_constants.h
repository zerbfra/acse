/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_constants.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _AXE_CONSTANTS_H
#define _AXE_CONSTANTS_H

/* registers */

#define REG_INVALID -1
#define REG_0 0
#define NUM_REGISTERS 31

/* opcodes pseudo-M68000 */
#define ADD 0
#define SUB 1
#define ANDL 2
#define ORL 3
#define EORL 4
#define ANDB 5
#define ORB 6
#define EORB 7
#define MUL 8
#define DIV 9
#define SHL 10
#define SHR 11
#define ROTL 12
#define ROTR 13
#define NEG 14
#define SPCL 15
#define ADDI 16
#define SUBI 17
#define ANDLI 18
#define ORLI 19
#define EORLI 20
#define ANDBI 21
#define ORBI 22
#define EORBI 23
#define MULI 24
#define DIVI 25
#define SHLI 26
#define SHRI 27
#define ROTLI 28
#define ROTRI 29
#define NOTL 30
#define NOTB 31
#define NOP 32
#define MOVA 33
#define JSR 34
#define RET 35
#define HALT 36
#define SEQ 37
#define SGE 38
#define SGT 39
#define SLE 40
#define SLT 41
#define SNE 42
#define BT 43
#define BF 44
#define BHI 45
#define BLS 46
#define BCC 47
#define BCS 48
#define BNE 49
#define BEQ 50
#define BVC 51
#define BVS 52
#define BPL 53
#define BMI 54
#define BGE 55
#define BLT 56
#define BGT 57
#define BLE 58
#define LOAD 59
#define STORE 60
#define AXE_READ 61
#define AXE_WRITE 62
#define INVALID_OPCODE -1

/* data types */
#define INTEGER_TYPE 0
#define UNKNOWN_TYPE -1

/* label special values */
#define LABEL_UNSPECIFIED -1

/* WARNINGS */
#define WARN_DIVISION_BY_ZERO 1

/* SIM errorcodes */
#define AXE_OK 0
#define AXE_OUT_OF_MEMORY 1
#define AXE_PROGRAM_NOT_INITIALIZED 2
#define AXE_INVALID_INSTRUCTION 3
#define AXE_VARIABLE_ID_UNSPECIFIED 4
#define AXE_VARIABLE_ALREADY_DECLARED 5
#define AXE_INVALID_TYPE 6
#define AXE_FOPEN_ERROR 7
#define AXE_FCLOSE_ERROR 8
#define AXE_INVALID_INPUT_FILE 9
#define AXE_FWRITE_ERROR 10
#define AXE_INVALID_DATA_FORMAT 11
#define AXE_INVALID_OPCODE 12
#define AXE_INVALID_REGISTER_INFO 13
#define AXE_INVALID_LABEL 14
#define AXE_INVALID_ARRAY_SIZE 15
#define AXE_INVALID_VARIABLE 16
#define AXE_INVALID_ADDRESS 17
#define AXE_INVALID_EXPRESSION 18
#define AXE_UNKNOWN_VARIABLE 19
#define AXE_LABEL_ALREADY_ASSIGNED 20
#define AXE_INVALID_LABEL_MANAGER 21
#define AXE_SY_TABLE_ERROR 22
#define AXE_NULL_DECLARATION 23
#define AXE_INVALID_CFLOW_GRAPH 24
#define AXE_INVALID_REG_ALLOC 25
#define AXE_REG_ALLOC_ERROR 26
#define AXE_TRANSFORM_ERROR 27
#define AXE_SYNTAX_ERROR 28
#define AXE_UNKNOWN_ERROR 29

/* DIRECTIVE TYPES */
#define DIR_WORD 0
#define DIR_SPACE 1
#define DIR_INVALID -1

/* ADDRESS TYPES */
#define ADDRESS_TYPE 0
#define LABEL_TYPE 1

/* CODEGEN FLAGS */
#define CG_DIRECT_ALL 0
#define CG_INDIRECT_ALL 3
#define CG_INDIRECT_DEST 1
#define CG_INDIRECT_SOURCE 2

/* EXPRESSION TYPES */
#define IMMEDIATE 0
#define REGISTER 1
#define INVALID_EXPRESSION -1

/* binary comparison constants */
#define _LT_ 0
#define _GT_ 1
#define _EQ_ 2
#define _NOTEQ_ 3
#define _LTEQ_ 4
#define _GTEQ_ 5

#endif
