#ifndef PROC_COMMANDS_H
#define PROC_COMMANDS_H

#include <stdlib.h>

#include "io_utils.h"

#include "processor.h"
#include "executors.h"

#ifdef compilation
#define IF_COMPILER(...) __VA_ARGS__ // compile_time
#define IF_RUNTIME(...)
#else
#define IF_COMPILER(...)
#define IF_RUNTIME(...) __VA_ARGS__ // run time (only in processor)
#endif // compilation

typedef MY_PROCESSOR_STATUS (* executor_t)(my_spu * proc);

enum instruct_arg_type {
    INSTRUCT_NO_ARG = 0,
    INSTRUCT_ARG_TYPE_NUMBER,
    INSTRUCT_ARG_TYPE_REG,
    INSTRUCT_ARG_TYPE_REG_IN_SQ,
    INSTRUCT_ARG_TYPE_PC,
};

struct proc_instruction_t {
    ssize_t                             byte_code;
    size_t                              byte_len; // Длина всей инструкции (со всеми аргументами) в байтах
    IF_COMPILER(const char * const      name;)
    IF_COMPILER(instruct_arg_type       arg_type;)
    IF_RUNTIME(executor_t               executor;)
};

const int COMMAND_SPACE_MAX = 256;

enum PROC_COMANDS {
    HLT  = 100,
    PUSH = 1,
    ADD, SUB, MUL, DIV,
    SQRT, SIN, COS, MOD, IDIV,
    OUT, IN, DRAW,
    PUSHR, POPR,
    PUSHM, POPM,
    JMP, JB, JBE, JA, JAE, JE, JNE,
    CALL, RET,
};

const proc_instruction_t PROC_INSTRUCTIONS[COMMAND_SPACE_MAX] = {
    [HLT]   = {.byte_code = HLT     IF_COMPILER(, .name = "HLT",    .arg_type = INSTRUCT_NO_ARG),               .byte_len = 1, IF_RUNTIME(.executor = execute_HLT)},

    [PUSH]  = {.byte_code = PUSH    IF_COMPILER(, .name = "PUSH",   .arg_type = INSTRUCT_ARG_TYPE_NUMBER),      .byte_len = 2, IF_RUNTIME(.executor = execute_PUSH)},

    [ADD]   = {.byte_code = ADD     IF_COMPILER(, .name = "ADD",    .arg_type = INSTRUCT_NO_ARG),               .byte_len = 1, IF_RUNTIME(.executor = execute_ADD)},
    [SUB]   = {.byte_code = SUB     IF_COMPILER(, .name = "SUB",    .arg_type = INSTRUCT_NO_ARG),               .byte_len = 1, IF_RUNTIME(.executor = execute_SUB)},
    [MUL]   = {.byte_code = MUL     IF_COMPILER(, .name = "MUL",    .arg_type = INSTRUCT_NO_ARG),               .byte_len = 1, IF_RUNTIME(.executor = execute_MUL)},
    [DIV]   = {.byte_code = DIV     IF_COMPILER(, .name = "DIV",    .arg_type = INSTRUCT_NO_ARG),               .byte_len = 1, IF_RUNTIME(.executor = execute_DIV)},

    [SQRT]  = {.byte_code = SQRT    IF_COMPILER(, .name = "SQRT",   .arg_type = INSTRUCT_NO_ARG),               .byte_len = 1, IF_RUNTIME(.executor = execute_SQRT)},
    [SIN]   = {.byte_code = SIN     IF_COMPILER(, .name = "SIN",    .arg_type = INSTRUCT_NO_ARG),               .byte_len = 1, IF_RUNTIME(.executor = execute_SIN)},
    [COS]   = {.byte_code = COS     IF_COMPILER(, .name = "COS",    .arg_type = INSTRUCT_NO_ARG),               .byte_len = 1, IF_RUNTIME(.executor = execute_COS)},
    [MOD]   = {.byte_code = MOD     IF_COMPILER(, .name = "MOD",    .arg_type = INSTRUCT_NO_ARG),               .byte_len = 1, IF_RUNTIME(.executor = execute_MOD)},
    [IDIV]  = {.byte_code = IDIV    IF_COMPILER(, .name = "IDIV",   .arg_type = INSTRUCT_NO_ARG),               .byte_len = 1, IF_RUNTIME(.executor = execute_IDIV)},

    [OUT]   = {.byte_code = OUT     IF_COMPILER(, .name = "OUT",    .arg_type = INSTRUCT_NO_ARG),               .byte_len = 1, IF_RUNTIME(.executor = execute_OUT)},
    [IN]    = {.byte_code = IN      IF_COMPILER(, .name = "IN",     .arg_type = INSTRUCT_NO_ARG),               .byte_len = 1, IF_RUNTIME(.executor = execute_IN)},
    [DRAW]  = {.byte_code = DRAW    IF_COMPILER(, .name = "DRAW",   .arg_type = INSTRUCT_ARG_TYPE_NUMBER),      .byte_len = 2, IF_RUNTIME(.executor = execute_DRAW)},

    [PUSHR] = {.byte_code = PUSHR   IF_COMPILER(, .name = "PUSHR",  .arg_type = INSTRUCT_ARG_TYPE_REG),         .byte_len = 2, IF_RUNTIME(.executor = execute_PUSHR)},
    [POPR]  = {.byte_code = POPR    IF_COMPILER(, .name = "POPR",   .arg_type = INSTRUCT_ARG_TYPE_REG),         .byte_len = 2, IF_RUNTIME(.executor = execute_POPR)},
    [PUSHM] = {.byte_code = PUSHM   IF_COMPILER(, .name = "PUSHM",  .arg_type = INSTRUCT_ARG_TYPE_REG_IN_SQ),   .byte_len = 2, IF_RUNTIME(.executor = execute_PUSHM)},
    [POPM]  = {.byte_code = POPM    IF_COMPILER(, .name = "POPM",   .arg_type = INSTRUCT_ARG_TYPE_REG_IN_SQ),   .byte_len = 2, IF_RUNTIME(.executor = execute_POPM)},

    [JMP]   = {.byte_code = JMP     IF_COMPILER(, .name = "JMP",    .arg_type = INSTRUCT_ARG_TYPE_PC),          .byte_len = 2, IF_RUNTIME(.executor = execute_JMP)},

    [JB]    = {.byte_code = JB      IF_COMPILER(, .name = "JB",     .arg_type = INSTRUCT_ARG_TYPE_PC),          .byte_len = 2, IF_RUNTIME(.executor = execute_JB)},
    [JBE]   = {.byte_code = JBE     IF_COMPILER(, .name = "JBE",    .arg_type = INSTRUCT_ARG_TYPE_PC),          .byte_len = 2, IF_RUNTIME(.executor = execute_JBE)},
    [JA]    = {.byte_code = JA      IF_COMPILER(, .name = "JA",     .arg_type = INSTRUCT_ARG_TYPE_PC),          .byte_len = 2, IF_RUNTIME(.executor = execute_JA)},
    [JAE]   = {.byte_code = JAE     IF_COMPILER(, .name = "JAE",    .arg_type = INSTRUCT_ARG_TYPE_PC),          .byte_len = 2, IF_RUNTIME(.executor = execute_JAE)},
    [JE]    = {.byte_code = JE      IF_COMPILER(, .name = "JE",     .arg_type = INSTRUCT_ARG_TYPE_PC),          .byte_len = 2, IF_RUNTIME(.executor = execute_JE)},
    [JNE]   = {.byte_code = JNE     IF_COMPILER(, .name = "JNE",    .arg_type = INSTRUCT_ARG_TYPE_PC),          .byte_len = 2, IF_RUNTIME(.executor = execute_JNE)},

    [CALL]  = {.byte_code = CALL    IF_COMPILER(, .name = "CALL",   .arg_type = INSTRUCT_ARG_TYPE_PC),          .byte_len = 2, IF_RUNTIME(.executor = execute_CALL)},
    [RET]   = {.byte_code = RET     IF_COMPILER(, .name = "RET",    .arg_type = INSTRUCT_NO_ARG),               .byte_len = 1, IF_RUNTIME(.executor = execute_RET)},
};

#endif // PROC_COMMANDS_H