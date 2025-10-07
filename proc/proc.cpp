#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define _DEBUG
#define tolerance_lvl 3

#include "proc_commands.h"

#include "io_utils.h"
#include "stringNthong.h"
#include "stack_errno.h"
#include "stack.h"

using namespace mystr;

#undef FREE
#define FREE(ptr) free(ptr); ptr = NULL;

#define OK(cond) !(cond)

#define aborted(code) \
    || ({code; false;})

#define Validate_processor_return_if_err(var_name)      \
    if (my_processor_validator(var_name) != PROC_OK) {  \
        ERROR_MSG("");                                  \
        my_processor_dump(var_name);                    \
    }

enum MY_PROCESSOR_STATUS {
    PROC_OK        = 0,
    PROC_ERR_ALLOC,
    PROC_ERR_STACK,
    PROC_ERR_NULLPTR_PASSED,
    PROC_ERR_CODE_OVERFLOW,
    PROC_ERR_INVALID_BYTECODE,
};

struct my_processor_unit {
    StackHandler stk;
    size_t registers[REGISTERS_COUNT];

    size_t program_counter;
    size_t byte_code_len;
    size_t * byte_code;

    MY_PROCESSOR_STATUS status;
};

MY_PROCESSOR_STATUS my_processor_init(my_processor_unit ** proc) {
    my_processor_unit * new_proc = (my_processor_unit *) calloc(1, sizeof(my_processor_unit));
    if (new_proc == NULL) {
        return PROC_ERR_ALLOC;
    }

    STACK_ERRNO stk_err = SUCCESS;
    StackHandler new_stk = StackCtor(10, &stk_err);
    // if (new_stk == NULL || stk_err != SUCCESS) {
    //     ERROR_MSG("stk_err is %d\n", stk_err);
    //     ERROR_MSG("new_stk at %p\n", new_stk);
    //     return PROC_ERR_STACK;
    // }
    new_proc->stk = new_stk;
    new_proc->status = PROC_OK;

    *proc = new_proc;
    return PROC_OK;
}

char * my_processor_error(MY_PROCESSOR_STATUS status) {
    switch (status) {
        case PROC_OK:                   return "Ok";
        case PROC_ERR_ALLOC:            return "Cannot allocate memory";
        case PROC_ERR_STACK:            return "Stack is invalid";
        case PROC_ERR_NULLPTR_PASSED:   return "Null ptr was passed";
        case PROC_ERR_CODE_OVERFLOW:    return "PC is bigger that code len";
        case PROC_ERR_INVALID_BYTECODE: return "Processor bytecode is invalid";
        default:                        return "Unknown status";
    }
}

MY_PROCESSOR_STATUS my_processor_dump(my_processor_unit * proc) {
    if (!proc) {
        fprintf(stderr, RED("ERROR: Processor is NULL\n"));
        return PROC_ERR_NULLPTR_PASSED;
    }

    // Заголовок
    printf(BOLD(ON_BLUE("================== PROCESSOR DUMP ==================")) "\n");

    StackDump(proc->stk, SUCCESS, "dump from processor");

    // === РЕГИСТРЫ ===
    printf(BOLD(YELLOW("REGISTERS:")) "\n");
    const char* reg_names[] = {"RAX", "RBX", "RCX", "RDX", "RTX", "DED", "INSIDE", "CURVA"};
    for (int i = 0; i < REGISTERS_COUNT; ++i) {
        printf("    " BRIGHT_GREEN("%-7s") " = " BRIGHT_YELLOW("%zu") " " BRIGHT_CYAN("[0x%016zx]") "\n",
               reg_names[i],
               proc->registers[i],
               proc->registers[i]);
    }

    // === ПРОГРАММНЫЙ СЧЁТЧИК ===
    printf(BOLD(MAGENTA("PROGRAM COUNTER:")) " " BRIGHT_YELLOW("%zu") " " BRIGHT_CYAN("[0x%016zx]") "\n",
           proc->program_counter,
           proc->program_counter);

    // === ТЕКУЩАЯ КОМАНДА ===
    if (proc->byte_code && proc->program_counter < proc->byte_code_len) {
        uint8_t current_cmd = proc->byte_code[proc->program_counter];
        printf(BOLD(GREEN("CURRENT COMMAND:")) " " BRIGHT_YELLOW("0x%02x") " (at offset " BRIGHT_CYAN("%zu") ")\n",
                current_cmd,
                proc->program_counter);
    } else if (proc->byte_code) {
        printf(BOLD(RED("CURRENT COMMAND:")) " " RED("OUT OF BOUNDS (PC = %zu, code length = %zu)") "\n",
                proc->program_counter, proc->byte_code_len);
    } else {
        printf(BOLD(RED("CURRENT COMMAND:")) " " RED("(byte code is NULL)") "\n");
    }

    // === БАЙТ-КОД ===
    printf(BOLD(CYAN("BYTE CODE:")) " ");
    if (proc->byte_code) {
        printf("length = " BRIGHT_YELLOW("%zu") " bytes, at %p\n",
                proc->byte_code_len, (void*)proc->byte_code);
        printf("    " BRIGHT_CYAN("{"));

        size_t printed = 0;

        while (printed < proc->byte_code_len) {
            // Печатаем до 8 байт в строке
            size_t line_start = printed;
            size_t line_end = (line_start + 8 > proc->byte_code_len) ? proc->byte_code_len : line_start + 8;

            if (printed > 0) {
                printf("\n     "); // отступ для продолжения блока
            }

            for (size_t i = line_start; i < line_end; ++i) {
                printf(" 0x%02x", proc->byte_code[i]);
            }

            // Проверяем, есть ли в этой строке program_counter
            if (proc->program_counter >= line_start && proc->program_counter < line_end) {
                printf("\n       "); // новая строка для указателя
                // Отступ до нужного байта внутри строки
                for (size_t i = line_start; i < proc->program_counter; ++i) {
                    printf("     ");
                }
                printf(BOLD(BRIGHT_RED("^^")));
            }

            printed = line_end;
        }

        if (proc->byte_code_len > 32) {
            printf(" " BRIGHT_BLACK("..."));
        }
        printf(" " BRIGHT_CYAN("}"));
    } else {
        printf(RED("(NULL)"));
    }
    printf("\n");

    if (proc->status == PROC_OK)
        printf(BOLD("STATUS:") GREEN("%s\n"), my_processor_error(proc->status));
    else
        printf(BOLD("STATUS:") RED("%s\n"), my_processor_error(proc->status));

    printf(BOLD(ON_BLUE("================================================")) "\n\n");

    return PROC_OK; // TODO указать текущую команду байткода
}

MY_PROCESSOR_STATUS my_processor_validator(my_processor_unit * proc) {
    if (proc == NULL)                                               {proc->status = PROC_ERR_NULLPTR_PASSED;    return PROC_ERR_NULLPTR_PASSED;}
    // if (proc->stk == NULL || StackValidator(proc->stk) != SUCCESS)  {StackDump(proc->stk, StackValidator(proc->stk), "PIZDEC POLNY"); proc->status = PROC_ERR_STACK;             return PROC_ERR_STACK;}
    if (proc->program_counter>=proc->byte_code_len)                 {proc->status = PROC_ERR_CODE_OVERFLOW;     return PROC_ERR_CODE_OVERFLOW;}
    return PROC_OK;
}

void my_processor_destroy(my_processor_unit ** proc) {
    StackDtor((*proc)->stk);

    (*proc)->program_counter = 0;
    (*proc)->byte_code_len = 0;
    (*proc)->byte_code = NULL;

    for (size_t i = 0; i < REGISTERS_COUNT; ++i) {
        (*proc)->registers[i] = 0;
    }

    free((*proc));
    *proc = NULL;
}

MY_PROCESSOR_STATUS execute_bytecode(my_processor_unit * proc) {
    Validate_processor_return_if_err(proc);

    size_t command = 0;

    while (proc->program_counter < proc->byte_code_len && command != HLT) {
        my_processor_dump(proc);
        command = proc->byte_code[proc->program_counter];

        StackHandler stk = proc->stk;

        switch(command) {
            case PUSH: {
                DEBUG_PRINT(BRIGHT_BLACK("Command is PUSH,"));
                if (proc->program_counter + 1 + 1 > proc->byte_code_len) {
                    proc->status = PROC_ERR_INVALID_BYTECODE;
                    return PROC_ERR_INVALID_BYTECODE;
                }

                double value = NAN;

                value = (double) proc->byte_code[proc->program_counter + 1];
                DEBUG_PRINT(BRIGHT_BLACK("arg is %lg"), value);

                StackPush(stk, value);

                break;
            }
            case PUSHR: {
                DEBUG_PRINT(BRIGHT_BLACK("Command is PUSHR,"));
                if (proc->program_counter + 1 + 1 > proc->byte_code_len) {
                    proc->status = PROC_ERR_INVALID_BYTECODE;
                    return PROC_ERR_INVALID_BYTECODE;
                }

                double value = NAN;

                size_t register_number = proc->byte_code[proc->program_counter + 1];
                DEBUG_PRINT(BRIGHT_BLACK("register is %zu, "), register_number);

                if (register_number > REGISTERS_COUNT) {
                    proc->status = PROC_ERR_INVALID_BYTECODE;
                    return PROC_ERR_INVALID_BYTECODE;
                }

                value = (double) proc->registers[register_number];
                DEBUG_PRINT(BRIGHT_BLACK("value is %lg\n"), value);

                StackPush(stk, value);

                break;
            }
            case POPR: {
                DEBUG_PRINT(BRIGHT_BLACK("Command is POPR,"));
                if (proc->program_counter + 1 + 1 > proc->byte_code_len) {
                    proc->status = PROC_ERR_INVALID_BYTECODE;
                    return PROC_ERR_INVALID_BYTECODE;
                }

                double value = NAN;

                StackPop(stk, &value);

                size_t register_number = proc->byte_code[proc->program_counter + 1];
                DEBUG_PRINT(BRIGHT_BLACK("register is %zu, "), register_number);

                DEBUG_PRINT(BRIGHT_BLACK("arg is %lg"), value);
                if (register_number > REGISTERS_COUNT) {
                    proc->status = PROC_ERR_INVALID_BYTECODE;
                    return PROC_ERR_INVALID_BYTECODE;
                }

                proc->registers[register_number] = (size_t) value;
                DEBUG_PRINT(BRIGHT_BLACK("value is %lg\n"), value);

                break;
            }
            case ADD: {
                DEBUG_PRINT(BRIGHT_BLACK("Command is ADD\n"));
                double var1 = NAN, var2 = NAN;
                StackPop(stk, &var1);
                StackPop(stk, &var2);
                StackPush(stk, var2 + var1);
                break;
            }
            case SUB: {
                DEBUG_PRINT(BRIGHT_BLACK("Command is SUB\n"));
                double var1 = NAN, var2 = NAN;
                StackPop(stk, &var1);
                StackPop(stk, &var2);
                StackPush(stk, var2 - var1);
                break;
            }
            case MUL: {
                DEBUG_PRINT(BRIGHT_BLACK("Command is MUL\n"));
                double var1 = NAN, var2 = NAN;
                StackPop(stk, &var1);
                StackPop(stk, &var2);
                StackPush(stk, var2 * var1);
                break;
            }
            case DIV: {
                DEBUG_PRINT(BRIGHT_BLACK("Command is DIV\n"));
                double var1 = NAN, var2 = NAN;
                StackPop(stk, &var1);
                StackPop(stk, &var2);
                StackPush(stk, var2 / var1);
                break;
            }
            case OUT: {
                DEBUG_PRINT(BRIGHT_BLACK("Command is OUT\n"));
                double value = NAN;
                StackPop(stk, &value);
                printf("%lg\n", value);
                break;
            }
            case IN: {
                DEBUG_PRINT(BRIGHT_BLACK("Command is IN\n"));
                double value = NAN;
                scanf("%lg", &value);
                StackPush(stk, value);
                break;
            }
            default:
            case HLT:
                DEBUG_PRINT(BRIGHT_BLACK("Command is HLT\n"));
                continue;
        }

        proc->program_counter += PROC_INSTRUCTIONS[command].byte_len;
    }

    Validate_processor_return_if_err(proc);
    return PROC_OK;
}

int main(int argc, char * argv[]) {

    OK(verify_proc_instructions()) aborted(return -1);

    if (argc == 1) {
        fprintf(stderr, "U must provide bytecode for execution");
        return -1;
    }
    size_t buf_len = 0;
    size_t * buf = read_file_to_size_t_buf(argv[1], &buf_len);
    if (errno != 0) {
        printf("%s", strerror(errno));
        return -1;
    }

    my_processor_unit * proc = NULL;
    printf("status is %d\n", my_processor_init(&proc));
    printf("%p\n", proc);

    proc->byte_code = buf;
    proc->byte_code_len = buf_len;

    my_processor_dump(proc);

    execute_bytecode(proc);

    my_processor_destroy(&proc);
    free(buf);
    return 0;
}