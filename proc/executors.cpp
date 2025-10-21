#undef _DEBUG

#include "io_utils.h"
#include <math.h>

#include "processor.h"

#include "executors.h"


MY_PROCESSOR_STATUS execute_HLT(my_spu * proc) {
    DEBUG_PRINT(BRIGHT_BLACK("Command is HLT\n"));

    return MY_PROCESSOR_STATUS::PROC_OK;
}

MY_PROCESSOR_STATUS execute_PUSH(my_spu * proc) {
    DEBUG_PRINT(BRIGHT_BLACK("Command is PUSH, "));
    if ((proc)->program_counter + 1 + 1 > (proc)->bytecode_len) {
        (proc)->status = MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
        return MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
    }
    bytecode_t value = {.arg = NAN};
    value = (proc)->bytecode[(proc)->program_counter + 1];
    DEBUG_PRINT(BRIGHT_BLACK("arg is %lg\n"), value.arg);
    StackPush(proc->stk, value);

    proc->program_counter += PROC_INSTRUCTIONS[PUSH].byte_len;
    return MY_PROCESSOR_STATUS::PROC_OK;
}

#define EXECUTE_MATH_OPR_impl(name, opr)                                                                \
    MY_PROCESSOR_STATUS execute_##name(my_spu * proc) {                                                 \
        DEBUG_PRINT(BRIGHT_BLACK("Command is " #name "\n"));                                            \
        bytecode_t var1 = {.arg = NAN}, var2 = {.arg = NAN}, var = {.arg = NAN};                        \
        StackPop(proc->stk, &var1);                                                                     \
        StackPop(proc->stk, &var2);                                                                     \
        var.arg = var2.arg opr var1.arg;                                                                \
        StackPush(proc->stk, var);                                                                      \
        proc->program_counter += PROC_INSTRUCTIONS[name].byte_len;                                      \
        return MY_PROCESSOR_STATUS::PROC_OK;                                                                                 \
    }

#define EXECUTE_MATH_H_FUNC_impl(name, func)                                                            \
    MY_PROCESSOR_STATUS execute_##name(my_spu * proc) {                                                 \
        DEBUG_PRINT(BRIGHT_BLACK("Command is " #name "\n"));                                            \
        bytecode_t var = {.arg = NAN}, new_var = {.arg = NAN};                                          \
        StackPop(proc->stk, &var);                                                                      \
        new_var.arg = func(var.arg);                                                                    \
        StackPush(proc->stk, new_var);                                                                  \
        proc->program_counter += PROC_INSTRUCTIONS[name].byte_len;                                      \
        return MY_PROCESSOR_STATUS::PROC_OK;                                                                                 \
    }

EXECUTE_MATH_OPR_impl(ADD, +)
EXECUTE_MATH_OPR_impl(SUB, -)
EXECUTE_MATH_OPR_impl(MUL, *)
EXECUTE_MATH_OPR_impl(DIV, /)

EXECUTE_MATH_H_FUNC_impl(SQRT, sqrt)
EXECUTE_MATH_H_FUNC_impl(SIN, sin)
EXECUTE_MATH_H_FUNC_impl(COS, cos)

MY_PROCESSOR_STATUS execute_MOD(my_spu * proc) {
    DEBUG_PRINT(BRIGHT_BLACK("Command is MOD\n"));
    bytecode_t var1 = {.arg = NAN}, var2 = {.arg = NAN}, var = {.arg = NAN};
    StackPop(proc->stk, &var1);
    StackPop(proc->stk, &var2);
    var.arg = ((int64_t) (var2.arg)) % ((int64_t) (var1.arg));
    StackPush(proc->stk, var);
    proc->program_counter += PROC_INSTRUCTIONS[MOD].byte_len;
    return MY_PROCESSOR_STATUS::PROC_OK;
}

MY_PROCESSOR_STATUS execute_IDIV(my_spu * proc) {
    DEBUG_PRINT(BRIGHT_BLACK("Command is INT_DIV\n"));
    bytecode_t var1 = {.arg = NAN}, var2 = {.arg = NAN}, var = {.arg = NAN};
    StackPop(proc->stk, &var1);
    StackPop(proc->stk, &var2);
    var.arg = (int64_t) ((var2.arg) / (var1.arg));
    StackPush(proc->stk, var);
    proc->program_counter += PROC_INSTRUCTIONS[IDIV].byte_len;
    return MY_PROCESSOR_STATUS::PROC_OK;
}

MY_PROCESSOR_STATUS execute_DRAW(my_spu * proc) {
    TERMINAL_CLEAR_SCREEN();
    DEBUG_PRINT(BRIGHT_BLACK("Command is DRAW"));

    char * ram = proc->ram;

    size_t sleep_time = proc->bytecode[proc->program_counter + 1].cmd;

    for (size_t i = 0; i < RAM_SIZE; ++i) {
        if (i % DRAW_LINE_WIDTH == 0)
            printf("\n");
        if (ram[i] == '\0')
            printf("%c ", ' ');
        else
            printf("%c ", ram[i]);
    }
    printf("\n");

    nsleep(sleep_time);

    proc->program_counter += PROC_INSTRUCTIONS[DRAW].byte_len;
    return MY_PROCESSOR_STATUS::PROC_OK;
}

MY_PROCESSOR_STATUS execute_OUT(my_spu * proc) {
    DEBUG_PRINT(BRIGHT_BLACK("Command is OUT\n"));
    bytecode_t value = {.arg = NAN};
    StackPop(proc->stk, &value);
    printf("%lg\n", value.arg);

    proc->program_counter += PROC_INSTRUCTIONS[OUT].byte_len;
    return MY_PROCESSOR_STATUS::PROC_OK;
}

MY_PROCESSOR_STATUS execute_IN(my_spu * proc) {
    DEBUG_PRINT(BRIGHT_BLACK("Command is IN\n"));
    bytecode_t value = {.arg = NAN};
    printf("Введите число: ");
    fflush(stdout);
    scanf("%lg", &(value.arg));
    printf("\n");
    getchar();
    // printf("Parsed value is [%lg]\n", value);
    StackPush(proc->stk, value);

    proc->program_counter += PROC_INSTRUCTIONS[IN].byte_len;
    return MY_PROCESSOR_STATUS::PROC_OK;
}

MY_PROCESSOR_STATUS execute_PUSHR(my_spu * proc) {
    DEBUG_PRINT(BRIGHT_BLACK("Command is PUSHR, "));
    if (proc->program_counter + 1 + 1 > proc->bytecode_len) {
        proc->status = MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
        return MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
    }

    bytecode_t value = {.arg = NAN};

    size_t register_number = proc->bytecode[proc->program_counter + 1].cmd;
    DEBUG_PRINT(BRIGHT_BLACK("register is %zu, "), register_number);

    if (register_number > REGISTERS_COUNT) {
        proc->status = MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
        return MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
    }

    value.arg = proc->registers[register_number];
    DEBUG_PRINT(BRIGHT_BLACK("value is %lg\n"), value.arg);

    StackPush(proc->stk, value);

    proc->program_counter += PROC_INSTRUCTIONS[PUSHR].byte_len;

    return MY_PROCESSOR_STATUS::PROC_OK;
}

MY_PROCESSOR_STATUS execute_POPR(my_spu * proc) {
    DEBUG_PRINT(BRIGHT_BLACK("Command is POPR, "));
    if (proc->program_counter + 1 + 1 > proc->bytecode_len) {
        proc->status = MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
        return MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
    }

    bytecode_t value = {.arg = NAN};

    StackPop(proc->stk, &value);

    size_t register_number = proc->bytecode[proc->program_counter + 1].cmd;
    DEBUG_PRINT(BRIGHT_BLACK("register is %zu, "), register_number);

    DEBUG_PRINT(BRIGHT_BLACK("arg is %lg, "), value);
    if (register_number > REGISTERS_COUNT) {
        proc->status = MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
        return MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
    }

    proc->registers[register_number] = value.arg;
    DEBUG_PRINT(BRIGHT_BLACK("value is %lg\n"), value);

    proc->program_counter += PROC_INSTRUCTIONS[POPR].byte_len;

    return MY_PROCESSOR_STATUS::PROC_OK;
}

MY_PROCESSOR_STATUS execute_PUSHM(my_spu * proc) {
    DEBUG_PRINT(BRIGHT_BLACK("Command is PUSHM, "));
    if (proc->program_counter + 1 + 1 > proc->bytecode_len) {
        proc->status = MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
        return MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
    }
    size_t register_number = proc->bytecode[proc->program_counter + 1].cmd;

    DEBUG_PRINT(BRIGHT_BLACK("register is %zu, "), register_number);

    if (register_number > REGISTERS_COUNT) {
        proc->status = MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
        return MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
    }
    bytecode_t index = {.arg = NAN};
    index.cmd = proc->registers[register_number];

    DEBUG_PRINT(BRIGHT_BLACK("index is %zu, "), index.cmd);

    if (index.cmd >= RAM_SIZE) {
        proc->status = MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
        return MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
    }

    bytecode_t value = {.arg = NAN};
    value.arg = proc->ram[index.cmd];

    DEBUG_PRINT(BRIGHT_BLACK("value is %c\n"), proc->ram[index.cmd]);

    StackPush(proc->stk, value);
    proc->program_counter += PROC_INSTRUCTIONS[PUSHM].byte_len;
    return MY_PROCESSOR_STATUS::PROC_OK;
}

MY_PROCESSOR_STATUS execute_POPM(my_spu * proc) {
    DEBUG_PRINT(BRIGHT_BLACK("Command is POPM, "));
    if (proc->program_counter + 1 + 1 > proc->bytecode_len) {
        proc->status = MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
        return MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
    }
    size_t register_number = proc->bytecode[proc->program_counter + 1].cmd;

    DEBUG_PRINT(BRIGHT_BLACK("register is %zu, "), register_number);

    if (register_number > REGISTERS_COUNT) {
        proc->status = MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
        return MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
    }
    bytecode_t index = {.arg = NAN};
    index.cmd = proc->registers[register_number];

    DEBUG_PRINT(BRIGHT_BLACK("index is %zu, "), index.cmd);

    if (index.cmd >= RAM_SIZE) {
        proc->status = MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
        return MY_PROCESSOR_STATUS::PROC_ERR_INVALID_BYTECODE;
    }

    bytecode_t value = {.arg = NAN};
    StackPop(proc->stk, &value);

    DEBUG_PRINT(BRIGHT_BLACK("value is (%c) [%zu]\n"), (char) value.arg, (char) value.arg);

    proc->ram[index.cmd] = (char) value.arg;
    proc->program_counter += PROC_INSTRUCTIONS[POPM].byte_len;
    return MY_PROCESSOR_STATUS::PROC_OK;
}

MY_PROCESSOR_STATUS execute_JMP(my_spu * proc) {
    DEBUG_PRINT(BRIGHT_BLACK("Command is JMP, new PC is %zu\n"), proc->bytecode[proc->program_counter + 1]);
    proc->program_counter = proc->bytecode[proc->program_counter + 1].cmd;
    return MY_PROCESSOR_STATUS::PROC_OK;
}

#define EXECUTE_COND_JMP_impl(name, opt)                                                                \
    MY_PROCESSOR_STATUS execute_J##name(my_spu * proc) {                                                \
        bytecode_t var1 = {.arg = NAN}, var2 = {.arg = NAN};                                            \
        StackPop(proc->stk, &var1);                                                                     \
        StackPop(proc->stk, &var2);                                                                     \
        DEBUG_PRINT(BRIGHT_BLACK("Command is J" #name ", var2 is %lg, var1 is %lg, new PC is %zu\n"),   \
                    var2.arg, var1.arg, proc->bytecode[proc->program_counter + 1].cmd);                 \
        if (var2.arg opt var1.arg) {                                                                    \
            proc->program_counter = proc->bytecode[proc->program_counter + 1].cmd;                      \
            return MY_PROCESSOR_STATUS::PROC_OK;                                                                             \
        }                                                                                               \
        proc->program_counter += PROC_INSTRUCTIONS[J##name].byte_len;                                   \
        return MY_PROCESSOR_STATUS::PROC_OK;                                                                                 \
    }

EXECUTE_COND_JMP_impl(B, <)
EXECUTE_COND_JMP_impl(BE, <=)
EXECUTE_COND_JMP_impl(A, >)
EXECUTE_COND_JMP_impl(AE, >=)
EXECUTE_COND_JMP_impl(E, ==)
EXECUTE_COND_JMP_impl(NE, !=)

MY_PROCESSOR_STATUS execute_CALL(my_spu * proc) {
    DEBUG_PRINT(BRIGHT_BLACK("Command is CALL, now PC is %zu, new PC is %zu\n"),
        proc->program_counter, proc->bytecode[proc->program_counter + 1]);

    bytecode_t return_pc = {.cmd = proc->program_counter + PROC_INSTRUCTIONS[CALL].byte_len};
    StackPush(proc->call_stack, return_pc);

    proc->program_counter = proc->bytecode[proc->program_counter + 1].cmd;
    return MY_PROCESSOR_STATUS::PROC_OK;
}

MY_PROCESSOR_STATUS execute_RET(my_spu * proc) {
    bytecode_t return_pc = {};
    StackPop(proc->call_stack, &return_pc);

    DEBUG_PRINT(BRIGHT_BLACK("Command is RET, now PC is %zu, jump PC is %zu\n"),
        proc->program_counter, return_pc.cmd);

    proc->program_counter = return_pc.cmd;
    return MY_PROCESSOR_STATUS::PROC_OK;
}