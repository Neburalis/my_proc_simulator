#ifndef PROC_COMMANDS_H
#define PROC_COMMANDS_H

#include <stdlib.h>

#ifdef compilation
#define IF_COMPILER(...) __VA_ARGS__ // compile_time
#else
#define IF_COMPILER(...)
#endif // compilation

const size_t  BYTECODE_SIGNATURE_SIZE = 4; // PROC_SIGNATURE, PROC_COMANDS_VERSION, COUNT OF BYTES IN BYTECODE, assembly date
const ssize_t PROC_SIGNATURE = 0x314D4953434F5250; // PROCSIM1
const size_t  PROC_COMANDS_VERSION = 44;

struct proc_instruction_t {
    ssize_t                   byte_code;
    IF_COMPILER(const char * const     name;)
    size_t                    byte_len; // Длина всей инструкции (со всеми аргументами) в байтах
};

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

const int COMMAND_SPACE_MAX = 256;

const proc_instruction_t PROC_INSTRUCTIONS[COMMAND_SPACE_MAX] = {
    [HLT]   = {.byte_code = HLT     IF_COMPILER(, .name = "HLT"),    .byte_len = 1},

    [PUSH]  = {.byte_code = PUSH    IF_COMPILER(, .name = "PUSH"),   .byte_len = 2},

    [ADD]   = {.byte_code = ADD     IF_COMPILER(, .name = "ADD"),    .byte_len = 1},
    [SUB]   = {.byte_code = SUB     IF_COMPILER(, .name = "SUB"),    .byte_len = 1},
    [MUL]   = {.byte_code = MUL     IF_COMPILER(, .name = "MUL"),    .byte_len = 1},
    [DIV]   = {.byte_code = DIV     IF_COMPILER(, .name = "DIV"),    .byte_len = 1},

    [SQRT]  = {.byte_code = SQRT    IF_COMPILER(, .name = "SQRT"),   .byte_len = 1},
    [SIN]   = {.byte_code = SIN     IF_COMPILER(, .name = "SIN"),    .byte_len = 1},
    [COS]   = {.byte_code = COS     IF_COMPILER(, .name = "COS"),    .byte_len = 1},
    [MOD]   = {.byte_code = MOD     IF_COMPILER(, .name = "MOD"),    .byte_len = 1},
    [IDIV]  = {.byte_code = IDIV    IF_COMPILER(, .name = "IDIV"),   .byte_len = 1},

    [OUT]   = {.byte_code = OUT     IF_COMPILER(, .name = "OUT"),    .byte_len = 1},
    [IN]    = {.byte_code = IN      IF_COMPILER(, .name = "IN"),     .byte_len = 1},
    [DRAW]  = {.byte_code = DRAW    IF_COMPILER(, .name = "DRAW"),   .byte_len = 2},

    [PUSHR] = {.byte_code = PUSHR   IF_COMPILER(, .name = "PUSHR"),  .byte_len = 2},
    [POPR]  = {.byte_code = POPR    IF_COMPILER(, .name = "POPR"),   .byte_len = 2},
    [PUSHM] = {.byte_code = PUSHM   IF_COMPILER(, .name = "PUSHM"),  .byte_len = 2},
    [POPM]  = {.byte_code = POPM    IF_COMPILER(, .name = "POPM"),   .byte_len = 2},

    [JMP]   = {.byte_code = JMP     IF_COMPILER(, .name = "JMP"),    .byte_len = 2},

    [JB]    = {.byte_code = JB      IF_COMPILER(, .name = "JB"),     .byte_len = 2},
    [JBE]   = {.byte_code = JBE     IF_COMPILER(, .name = "JBE"),    .byte_len = 2},
    [JA]    = {.byte_code = JA      IF_COMPILER(, .name = "JA"),     .byte_len = 2},
    [JAE]   = {.byte_code = JAE     IF_COMPILER(, .name = "JAE"),    .byte_len = 2},
    [JE]    = {.byte_code = JE      IF_COMPILER(, .name = "JE"),     .byte_len = 2},
    [JNE]   = {.byte_code = JNE     IF_COMPILER(, .name = "JNE"),    .byte_len = 2},

    [CALL]  = {.byte_code = CALL    IF_COMPILER(, .name = "CALL"),   .byte_len = 2},
    [RET]   = {.byte_code = RET     IF_COMPILER(, .name = "RET"),    .byte_len = 1},
};

int verify_proc_instructions();

const int REGISTERS_COUNT = 9;

enum REGISTERS_NAMES {
    REG_RAX = 1, REG_RBX, REG_RCX, REG_RDX, REG_RTX, REG_DED, REG_INSIDE, REG_CURVA,
};

int get_register_by_name(const char * const name);

#endif // PROC_COMMANDS_H