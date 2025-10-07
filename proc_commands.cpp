#include <limits.h>
#include <string.h>

#include "io_utils.h"
#include "proc_commands.h"

int verify_proc_instructions() {
    for (size_t i = 0; i < COMMAND_SPACE_MAX; ++i) {
        if (PROC_INSTRUCTIONS[i].byte_code == i || PROC_INSTRUCTIONS[i].byte_len == 0)
            continue;
        else {
            ERROR_MSG("PROC_INSTRUCTIONS[%zu].byte_code is %zu must be %zu", i, PROC_INSTRUCTIONS[i].byte_code, i);
            return -1;
        }
    }
    return 0;
}

int get_register_by_name(const char * const name) {
    if (strcmp(name, "RAX") == 0)        return REG_RAX;
    if (strcmp(name, "RBX") == 0)        return REG_RBX;
    if (strcmp(name, "RCX") == 0)        return REG_RCX;
    if (strcmp(name, "RDX") == 0)        return REG_RDX;
    if (strcmp(name, "RTX") == 0)        return REG_RTX;
    if (strcmp(name, "DED") == 0)        return REG_DED;
    if (strcmp(name, "INSIDE") == 0)     return REG_INSIDE;
    if (strcmp(name, "CURVA") == 0)      return REG_CURVA;
    return -1;
}
