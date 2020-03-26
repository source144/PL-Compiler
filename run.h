#ifndef __PL0RUN__

#include <stdio.h> 
#include "pvm.h"

instruction_t **compile(char *sourcecodePath, int lexFlag, int asmblyFlag);
void run(instruction_t **code, int runFlag);

#endif