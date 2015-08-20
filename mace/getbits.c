/*
 * Giovanni Agosta, Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * getbits.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include "getbits.h"

unsigned int getbits(unsigned int data, int from, int to){
   unsigned int res;
   unsigned int mask;

   mask = (0xFFFFFFFF >> (31 - from));
   res = data & mask;
   mask = (0xFFFFFFFF << to);
   res = res & mask;
   res = res >> to;
  return res;
}
