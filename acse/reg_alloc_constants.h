/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * reg_alloc_constants.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _REG_ALLOC_CONSTANTS_H
#define _REG_ALLOC_CONSTANTS_H

/* minimum number of registers of the target machine */
#define RA_MIN_REG_NUM 3

/* constants */
#define RA_SPILL_REQUIRED -1
#define RA_REGISTER_INVALID 0
#define RA_EXCLUDED_VARIABLE 0

/* errorcodes */
#define RA_OK 0
#define RA_INVALID_ALLOCATOR 1
#define RA_INVALID_INTERVAL 2
#define RA_INTERVAL_ALREADY_INSERTED 3
#define RA_INVALID_NUMBER_OF_REGISTERS 4

#endif
