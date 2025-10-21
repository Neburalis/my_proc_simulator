#ifndef EXECUTORS_H
#define EXECUTORS_H

#include "processor.h"

MY_PROCESSOR_STATUS execute_HLT(my_spu * proc);
MY_PROCESSOR_STATUS execute_PUSH(my_spu * proc);

MY_PROCESSOR_STATUS execute_ADD(my_spu * proc);
MY_PROCESSOR_STATUS execute_SUB(my_spu * proc);
MY_PROCESSOR_STATUS execute_MUL(my_spu * proc);
MY_PROCESSOR_STATUS execute_DIV(my_spu * proc);

MY_PROCESSOR_STATUS execute_SQRT(my_spu * proc);
MY_PROCESSOR_STATUS execute_SIN(my_spu * proc);
MY_PROCESSOR_STATUS execute_COS(my_spu * proc);
MY_PROCESSOR_STATUS execute_MOD(my_spu * proc);
MY_PROCESSOR_STATUS execute_IDIV(my_spu * proc);

MY_PROCESSOR_STATUS execute_IN(my_spu * proc);
MY_PROCESSOR_STATUS execute_OUT(my_spu * proc);
MY_PROCESSOR_STATUS execute_DRAW(my_spu * proc);

MY_PROCESSOR_STATUS execute_PUSHR(my_spu * proc);
MY_PROCESSOR_STATUS execute_POPR(my_spu * proc);

MY_PROCESSOR_STATUS execute_PUSHM(my_spu * proc);
MY_PROCESSOR_STATUS execute_POPM(my_spu * proc);

MY_PROCESSOR_STATUS execute_JMP(my_spu * proc);
MY_PROCESSOR_STATUS execute_JE(my_spu * proc);
MY_PROCESSOR_STATUS execute_JNE(my_spu * proc);
MY_PROCESSOR_STATUS execute_JB(my_spu * proc);
MY_PROCESSOR_STATUS execute_JBE(my_spu * proc);
MY_PROCESSOR_STATUS execute_JA(my_spu * proc);
MY_PROCESSOR_STATUS execute_JAE(my_spu * proc);

MY_PROCESSOR_STATUS execute_CALL(my_spu * proc);
MY_PROCESSOR_STATUS execute_RET(my_spu * proc);

#endif // EXECUTORS_H