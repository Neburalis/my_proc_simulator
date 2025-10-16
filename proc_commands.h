#ifndef PROC_COMMANDS_H
#define PROC_COMMANDS_H

#include <stdlib.h>

#ifdef compilation
#define ct(...) __VA_ARGS__ // compile_time
#else
#define ct(...)
#endif // compilation

const size_t  BYTECODE_SIGNATURE_SIZE = 3; // PROC_SIGNATURE, PROC_COMANDS_VERSION, COUNT OF BYTES IN BYTECODE
const ssize_t PROC_SIGNATURE = 0x314D4953434F5250; // PROCSIM1
const size_t  PROC_COMANDS_VERSION = 30;

struct proc_instruction_t {
    ssize_t                   byte_code;
    ct(const char * const     name;)
    size_t                    byte_len; // Длина всей инструкции (со всеми аргументами) в байтах
};

enum PROC_COMANDS {
    HLT  = 100,
    PUSH = 1,
    ADD, SUB, MUL, DIV,
    OUT, IN,
    PUSHR, POPR,
    JMP, JB, JBE, JA, JAE, JE, JNE,
    SQRT, SIN, COS, MOD, IDIV,
    CALL, RET,
};

const int COMMAND_SPACE_MAX = 256;

const proc_instruction_t PROC_INSTRUCTIONS[COMMAND_SPACE_MAX] = {
    [HLT]   = {.byte_code = HLT ct(, .name = "HLT"), .byte_len = 1},

    [PUSH]  = {.byte_code = PUSH ct(, .name = "PUSH"), .byte_len = 2},

    [ADD]   = {.byte_code = ADD ct(, .name = "ADD"), .byte_len = 1},
    [SUB]   = {.byte_code = SUB ct(, .name = "SUB"), .byte_len = 1},
    [MUL]   = {.byte_code = MUL ct(, .name = "MUL"), .byte_len = 1},
    [DIV]   = {.byte_code = DIV ct(, .name = "DIV"), .byte_len = 1},

    [OUT]   = {.byte_code = OUT ct(, .name = "OUT"), .byte_len = 1},
    [IN]    = {.byte_code = IN  ct(, .name = "IN"),  .byte_len = 1},

    [PUSHR] = {.byte_code = PUSHR  ct(, .name = "PUSHR"), .byte_len = 2},
    [POPR]  = {.byte_code = POPR   ct(, .name = "POPR"),  .byte_len = 2},

    [JMP]   = {.byte_code = JMP  ct(, .name = "JMP"), .byte_len = 2},

    [JB]    = {.byte_code = JB   ct(, .name = "JB"),  .byte_len = 2},
    [JBE]   = {.byte_code = JBE  ct(, .name = "JBE"), .byte_len = 2},
    [JA]    = {.byte_code = JA   ct(, .name = "JA"),  .byte_len = 2},
    [JAE]   = {.byte_code = JAE  ct(, .name = "JAE"), .byte_len = 2},
    [JE]    = {.byte_code = JE   ct(, .name = "JE"),  .byte_len = 2},
    [JNE]   = {.byte_code = JNE  ct(, .name = "JNE"), .byte_len = 2},

    [SQRT]  = {.byte_code = SQRT ct(, .name = "SQRT"), .byte_len = 1},
    [SIN]   = {.byte_code = SIN  ct(, .name = "SIN"),  .byte_len = 1},
    [COS]   = {.byte_code = COS  ct(, .name = "COS"),  .byte_len = 1},
    [MOD]   = {.byte_code = MOD  ct(, .name = "MOD"),  .byte_len = 1},
    [IDIV]  = {.byte_code = IDIV ct(, .name = "IDIV"), .byte_len = 1},

    [CALL]  = {.byte_code = CALL ct(, .name = "CALL"), .byte_len = 2},
    [RET]   = {.byte_code = RET  ct(, .name = "RET"),  .byte_len = 1},
};

int verify_proc_instructions();

const int REGISTERS_COUNT = 9;

enum REGISTERS_NAMES {
    REG_RAX = 1, REG_RBX, REG_RCX, REG_RDX, REG_RTX, REG_DED, REG_INSIDE, REG_CURVA,
};

int get_register_by_name(const char * const name);

#endif // PROC_COMMANDS_H