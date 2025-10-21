#undef _DEBUG

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "proc_commands.h"

#include "io_utils.h"
#include "stringNthong.h"
#include "stack_errno.h"
#include "stack.h"

#include "processor.h"
#include "executors.h"

using namespace mystr;

#undef FREE
#define FREE(ptr) free(ptr); ptr = NULL;

#define OK(cond) !(cond)

#define verified(code) \
    || ({code; false;})

#define Validate_processor_return_if_err(var_name)      \
    if (my_processor_validator(var_name) != PROC_OK) {  \
        ERROR_MSG("");                                  \
        /*my_processor_dump(var_name);*/                    \
    }

#define Add_command_executor(NAME) \
    [NAME] = execute_##NAME

const executor_t EXECUTORS[COMMAND_SPACE_MAX] = {
    Add_command_executor(HLT),
    Add_command_executor(PUSH),
    Add_command_executor(ADD),
    Add_command_executor(SUB),
    Add_command_executor(MUL),
    Add_command_executor(DIV),
    Add_command_executor(SQRT),
    Add_command_executor(SIN),
    Add_command_executor(COS),
    Add_command_executor(MOD),
    Add_command_executor(IDIV),
    Add_command_executor(OUT),
    Add_command_executor(IN),
    Add_command_executor(DRAW),
    Add_command_executor(PUSHR),
    Add_command_executor(POPR),
    Add_command_executor(PUSHM),
    Add_command_executor(POPM),
    Add_command_executor(JMP),
    Add_command_executor(JB),
    Add_command_executor(JBE),
    Add_command_executor(JA),
    Add_command_executor(JAE),
    Add_command_executor(JE),
    Add_command_executor(JNE),
    Add_command_executor(CALL),
    Add_command_executor(RET),
};

MY_PROCESSOR_STATUS my_processor_init(my_spu ** proc) {
    my_spu * new_proc = (my_spu *) calloc(1, sizeof(my_spu));
    if (new_proc == NULL) {
        return PROC_ERR_ALLOC;
    }

    STACK_ERRNO stk_err = SUCCESS;
    StackHandler new_stk = StackCtor(8, &stk_err);
    new_proc->stk = new_stk;

    stk_err = SUCCESS;
    StackHandler new_call_stk = StackCtor(4, &stk_err);
    new_proc->call_stack = new_call_stk;

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

MY_PROCESSOR_STATUS my_processor_dump(my_spu * proc) {
    if (!proc) {
        fprintf(stderr, RED("ERROR: Processor is NULL\n"));
        return PROC_ERR_NULLPTR_PASSED;
    }

    // Заголовок
    printf(BOLD(ON_BLUE("================== PROCESSOR DUMP ==================")) "\n");

    StackDump(proc->stk, SUCCESS, "stack dump from processor");
    StackDump(proc->call_stack, SUCCESS, "call_stack dump from processor");

    // === РЕГИСТРЫ ===
    printf(BOLD(YELLOW("REGISTERS:")) "\n");
    const char* reg_names[] = {"SHADOW", "RAX", "RBX", "RCX", "RDX", "RTX", "DED", "INSIDE", "CURVA"};
    for (int i = 0; i < REGISTERS_COUNT; ++i) {
        printf("    " BRIGHT_GREEN("%-7s") " = " BRIGHT_YELLOW("%lg") " " BRIGHT_CYAN("[0x%016zx]") "\n",
               reg_names[i],
               proc->registers[i],
               proc->registers[i]);
    }

    // === ПРОГРАММНЫЙ СЧЁТЧИК ===
    printf(BOLD(MAGENTA("PROGRAM COUNTER:")) " " BRIGHT_YELLOW("%zu") " " BRIGHT_CYAN("[0x%016zx]") "\n",
           proc->program_counter,
           proc->program_counter);

    // === ТЕКУЩАЯ КОМАНДА ===
    if (proc->bytecode && proc->program_counter < proc->bytecode_len) {
        size_t current_cmd = proc->bytecode[proc->program_counter].cmd;
        printf(BOLD(GREEN("CURRENT COMMAND:")) " " BRIGHT_YELLOW("0x%02x") " (at offset " BRIGHT_CYAN("%zu") ")\n",
                current_cmd,
                proc->program_counter);
    } else if (proc->bytecode) {
        printf(BOLD(RED("CURRENT COMMAND:")) " " RED("OUT OF BOUNDS (PC = %zu, code length = %zu)") "\n",
                proc->program_counter, proc->bytecode_len);
    } else {
        printf(BOLD(RED("CURRENT COMMAND:")) " " RED("(byte code is NULL)") "\n");
    }

    // === БАЙТ-КОД ===
    printf(BOLD(CYAN("BYTE CODE:")) " ");
    if (proc->bytecode) {
        printf("length = " BRIGHT_YELLOW("%zu") " bytes, at %p\n",
                proc->bytecode_len, (void*)proc->bytecode);
        printf("    " BRIGHT_CYAN("{"));

        size_t printed = 0;

        while (printed < proc->bytecode_len) {
            // Печатаем до 8 байт в строке
            size_t line_start = printed;
            size_t line_end = (line_start + 8 > proc->bytecode_len) ? proc->bytecode_len : line_start + 8;

            if (printed > 0) {
                printf("\n     "); // отступ для продолжения блока
            }

            for (size_t i = line_start; i < line_end; ++i) {
                if (i == 0)
                    printf(" SIGN");
                else
                    printf(" 0x%02x", proc->bytecode[i]);
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

        if (proc->bytecode_len > 32) {
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

    return PROC_OK;
}

MY_PROCESSOR_STATUS my_processor_validator(my_spu * proc) {
    if (proc == NULL)                                               {proc->status = PROC_ERR_NULLPTR_PASSED;    return PROC_ERR_NULLPTR_PASSED;}
    if (proc->program_counter>=proc->bytecode_len)                  {proc->status = PROC_ERR_CODE_OVERFLOW;     return PROC_ERR_CODE_OVERFLOW;}
    return PROC_OK;
}

void my_processor_destroy(my_spu ** proc) {
    StackDtor((*proc)->stk);
    StackDtor((*proc)->call_stack);

    (*proc)->program_counter = 0;
    (*proc)->bytecode_len = 0;
    (*proc)->bytecode = NULL;

    for (size_t i = 0; i < REGISTERS_COUNT; ++i) {
        (*proc)->registers[i] = 0;
    }

    free((*proc));
    *proc = NULL;
}

MY_PROCESSOR_STATUS execute_bytecode(my_spu * proc) {
    Validate_processor_return_if_err(proc);

    size_t command = 0;

    while (proc->program_counter < proc->bytecode_len && command != HLT) {
        // my_processor_dump(proc);
        // getchar();
        command = proc->bytecode[proc->program_counter].cmd;

        const executor_t executor = EXECUTORS[command];

        if (executor == NULL) {
            ERROR_MSG("Unknown command %zu\n", command);
            proc->status = PROC_ERR_INVALID_BYTECODE;
            break;
        }

        executor(proc);
    }

    Validate_processor_return_if_err(proc);
    return PROC_OK;
}

int main(int argc, char * argv[]) {
    // TERMINAL_ENTER_ALT_SCREEN();
    // TERMINAL_CLEAR_SCREEN();

    OK(verify_proc_instructions()) verified(return -1);

    if (argc == 1) {
        ERROR_MSG("U must provide bytecode for execution");
        return -1;
    }
    size_t buf_len = 0;
    bytecode_t * buf = (bytecode_t *) read_file_to_size_t_buf(argv[1], &buf_len);
    if (errno != 0) {
        printf("%s", strerror(errno));
        return -1;
    }

    my_spu * proc = NULL;
    my_processor_init(&proc);
    // printf("status is %d\n", my_processor_init(&proc));
    // printf("%p\n", proc);

    if (buf[0].cmd != PROC_SIGNATURE) {
        ERROR_MSG("the file is not a processor executable");
        return 1;
    }

    if (buf[1].cmd != PROC_COMANDS_VERSION)
    {
        ERROR_MSG("the file is an executable file of a different version of the processor");
        return 2;
    }

    if (buf[2].cmd != buf_len) {
        ERROR_MSG("the executable file signature is corrupted");
        return 3;
    }

    proc->bytecode = buf;
    proc->bytecode_len = buf_len;
    proc->program_counter = BYTECODE_SIGNATURE_SIZE;

    // my_processor_dump(proc);

    execute_bytecode(proc);

    my_processor_destroy(&proc);
    free(buf);
    PAUSE_ANY_KEY(BRIGHT_BLACK("Нажмите любую клавишу для выхода ...\n"));
    // TERMINAL_EXIT_ALT_SCREEN();
    return 0;
}