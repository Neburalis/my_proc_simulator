#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "stack_errno.h"
#include "stack.h"

#include <stddef.h>

#include "proc_commands.h"

const size_t  BYTECODE_SIGNATURE_SIZE = 4; // PROC_SIGNATURE, PROC_COMANDS_VERSION, COUNT OF BYTES IN BYTECODE, assembly date
const ssize_t PROC_SIGNATURE = 0x314D4953434F5250; // PROCSIM1
const size_t  PROC_COMANDS_VERSION = 44;

enum MY_PROCESSOR_STATUS {
    PROC_OK        = 0,
    PROC_ERR_ALLOC,
    PROC_ERR_STACK,
    PROC_ERR_NULLPTR_PASSED,
    PROC_ERR_CODE_OVERFLOW,
    PROC_ERR_INVALID_BYTECODE,
};

stack_element_t typedef bytecode_t;

const size_t RAM_SIZE        = 1024;
const size_t DRAW_LINE_WIDTH = 32;

struct my_spu {
    StackHandler stk;
    double registers[REGISTERS_COUNT];

    size_t program_counter;
    size_t bytecode_len;
    bytecode_t * bytecode;

    StackHandler call_stack;

    char ram[RAM_SIZE];

    MY_PROCESSOR_STATUS status;
};

typedef MY_PROCESSOR_STATUS (*executor_t)(my_spu *proc);

#endif // PROCESSOR_H