/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_errors.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _AXE_ERRORS_H
#define _AXE_ERRORS_H

#include "axe_engine.h"
#include "axe_constants.h"

extern void printWarningMessage(int warningcode);
extern void printMessage(const char *msg);
extern void notifyError(int axe_errorcode);
extern void checkConsistency();

#endif
