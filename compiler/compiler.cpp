#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define _DEBUG
#define tolerance_lvl 3

#define compilation
#include "proc_commands.h"
#include "compiler.h"

#include "io_utils.h"
#include "stringNthong.h"
#include "stack_errno.h"
#include "stack.h"

using namespace mystr;

#undef FREE
#define FREE(ptr) free(ptr); ptr = NULL;

#define conditional_jmp_body(type)                                                                      \
    else if (comp_to(cmd, PROC_INSTRUCTIONS[type].name, ' ') == 0) {                                    \
        if (*arg == ':') {                                                                              \
            bytecode_add_and_log_1(data, type, data->labels[*(arg + 1) - '0']);                         \
        }                                                                                               \
        else {                                                                                          \
            char * endptr = NULL;                                                                       \
            errno = 0;                                                                                  \
                                                                                                        \
            size_t new_program_counter = strtoull(arg, &endptr, 10);                                    \
                                                                                                        \
            if (errno == ERANGE || endptr == arg) {                                                     \
                ERROR_MSG("Не удалось получить новый PC из аргументов (%s)", arg);                      \
                break;                                                                                  \
            }                                                                                           \
                                                                                                        \
            bytecode_add_and_log_1(data, type, new_program_counter);                                    \
        }                                                                                               \
    }

#define no_args_proc_instruct_body(instruct)                                                            \
    else if (comp_to(cmd, PROC_INSTRUCTIONS[instruct].name, ' ') == 0) {                                \
        bytecode_add_and_log_0(data, instruct);                                                         \
    }

#define bytecode_add_and_log_0(data, CMD) do {                                                          \
    size_t pc = (data)->bytecode_size;                                                                  \
    bytecode_add_command((data), PROC_INSTRUCTIONS[CMD].byte_code, 0);                                  \
    printf(BRIGHT_BLACK("[%4zu]") "  " YELLOW("%-30s") "  " CYAN("%08zx") "\n",                         \
           pc, #CMD, (size_t)PROC_INSTRUCTIONS[CMD].byte_code);                                         \
} while (0)

#define bytecode_add_and_log_1(data, CMD, arg) do {                                                     \
    size_t pc = (data)->bytecode_size;                                                                  \
    bytecode_add_command((data), PROC_INSTRUCTIONS[CMD].byte_code, 1, (ssize_t)(arg));                  \
    char cmd_repr[64];                                                                                  \
    snprintf(cmd_repr, sizeof(cmd_repr), "%s %zd", #CMD, (ssize_t)(arg));                               \
    printf(BRIGHT_BLACK("[%4zu]") "  " YELLOW("%-30s") "  " CYAN("%08zx") " " BLUE("%016llx") "\n",     \
           pc, cmd_repr,                                                                                \
           (size_t)PROC_INSTRUCTIONS[CMD].byte_code, (int64_t)(arg));                                   \
} while(0)

COMPILER_ERRNO bytecode_add_command(compiler_internal_data * data, ssize_t command_bytecode, size_t argc, ...) {
    assert(data);
    assert(data->bytecode);

    if (data == NULL || data->bytecode == NULL) {
        return COMPILER_ERRNO::COMPILER_PROVIDE_NULLPTR;
    }

    size_t required_space = 1 + argc;
    if (data->bytecode_size + required_space > data->bytecode_capacity) {
        size_t new_capacity = data->bytecode_capacity * 2;
        ssize_t * new_bytecode = (ssize_t *) realloc(data->bytecode, new_capacity * sizeof(data->bytecode[0]));

        if (new_bytecode == NULL) {
            return COMPILER_ERRNO::COMPILER_CANNOT_REALLOCATE_MEMORY;
        }
        data->bytecode = new_bytecode;
        data->bytecode_capacity = new_capacity;
    }

    data->bytecode[data->bytecode_size++] = command_bytecode;

    if (argc > 0) {
        va_list args;
        va_start(args, argc);
        for (size_t i = 0; i < argc; ++i) {
            data->bytecode[data->bytecode_size++] = va_arg(args, ssize_t);
        }
        va_end(args);
    }

    return COMPILER_ERRNO::COMPILER_NO_PROBLEM;
}

COMPILER_ERRNO compile(compiler_internal_data * data) {
    char * line = data->input_text;
    while (line < data->input_text + data->input_text_len) {
        char * space = strchr(line, ' ');
        char * cmd = line;
        char * arg = NULL;
        if (space) {
            // *space = '\0';
            arg = space + 1;
        }
        if (*cmd == ':') { // label
            data->labels[*(cmd + 1) - '0'] = data->bytecode_size;

        } else if (comp_to(cmd, PROC_INSTRUCTIONS[PUSH].name, ' ') == 0) {
            if (arg == NULL) {
                ERROR_MSG("Команде PUSH было передано аргументов 0, а должно быть хотя бы 1");
                break;
            }

            char * token = arg;
            char * endptr = NULL;

            while (*token != '\0') {
                while (*token == ' ') token++;
                if (*token == '\0') break;

                errno = 0;
                double value = strtod(token, &endptr);

                if (errno != 0 || token == endptr) {
                    ERROR_MSG("Неверный числовой аргумент в PUSH: %s", token);
                    break;
                }

                bytecode_add_and_log_1(data, PUSH, value);

                token = endptr;
            }
        } else if (comp_to(cmd, PROC_INSTRUCTIONS[PUSHR].name, ' ') == 0) {
            size_t register_number = get_register_by_name(arg);

            if (register_number > REGISTERS_COUNT) {
                ERROR_MSG("Неверный регистр в PUSHR: %zu", register_number);
                break;
            }

            bytecode_add_and_log_1(data, PUSHR, register_number);

        } else if (comp_to(cmd, PROC_INSTRUCTIONS[POPR].name, ' ') == 0) {
            size_t register_number = get_register_by_name(arg);
            if (register_number > REGISTERS_COUNT) {
                ERROR_MSG("Неверный регистр в POPR: %zu", register_number);
                break;
            }

            bytecode_add_and_log_1(data, POPR, register_number);
        }
        conditional_jmp_body(JMP)
        conditional_jmp_body(JB)
        conditional_jmp_body(JBE)
        conditional_jmp_body(JA)
        conditional_jmp_body(JAE)
        conditional_jmp_body(JE)
        conditional_jmp_body(JNE)
        no_args_proc_instruct_body(ADD)
        no_args_proc_instruct_body(SUB)
        no_args_proc_instruct_body(MUL)
        no_args_proc_instruct_body(DIV)
        no_args_proc_instruct_body(SQRT)
        no_args_proc_instruct_body(OUT)
        no_args_proc_instruct_body(IN)
        no_args_proc_instruct_body(HLT)

        line += (strlen(line) + 1);
    }
    return COMPILER_ERRNO::COMPILER_NO_PROBLEM;
}

int parse_args(int argc, char * argv[], char ** output_file, char * input_files[MAX_INPUT_FILES]) {
    assert(output_file != NULL);
    assert(input_files != NULL);
    if (argc == 1) {
        fprintf(stderr, RED("error:") " no input files\n");
        return -1;
    }

    int input_count = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, RED("error:") " option -o requires an argument\n");
                return -1;
            }
            if (*output_file != NULL) {
                fprintf(stderr, RED("error:") " option -o specified more than once\n");
                return -1;
            }
            *output_file = argv[i + 1];
            i++; // пропускаем аргумент -o
        } else {
            // Это входной файл
            if (input_count >= MAX_INPUT_FILES) {
                fprintf(stderr, RED("error:") " too many input files\n");
                return -1;
            }
            input_files[input_count++] = argv[i];
        }
    }

    if (*output_file == NULL) {
        *output_file = "a.bin";
    }

    if (input_count == 0) {
        fprintf(stderr, RED("error:") " no input files\n");
        return -1;
    }

    return input_count;
}

int main(int argc, char * argv[]) {
    init_compiler_internal_data(data);

    data.input_files_count = parse_args(argc, argv, &data.output_file, data.input_files);

    printf("Output file: %s\n", data.output_file);
    printf("Input files:\n");
    for (int i = 0; i < data.input_files_count; i++) {
        printf("  %s\n", data.input_files[i]);
    }

    data.input_text = read_file_to_buf(data.input_files[0], &data.input_text_len);
    if (errno != 0) {
        printf("%s", strerror(errno));
        return -1;
    }

    data.bytecode_capacity = 1024;
    data.bytecode_size = BYTECODE_SIGNATURE_SIZE;
    data.bytecode = (ssize_t *) calloc(data.bytecode_capacity, sizeof(ssize_t));

    if (data.bytecode == NULL) {
        fprintf(stderr, RED("error:") " failed to allocate bytecode buffer\n");
        FREE(data.input_text);
        return -1;
    }

    replace_needle_in_haystack(data.input_text, data.input_text_len, '\n', '\0');
    printf(BOLD(BRIGHT_WHITE("Первый проход:\n")));
    printf(BRIGHT_BLACK("%s=\n"), mult("=+", 40));

    printf(BRIGHT_BLACK("[  PC]") "  " BOLD(BRIGHT_WHITE("%-30s")) "  "
           BOLD(BRIGHT_CYAN("%s")) " " BOLD(BLUE("  ARGS")) "\n",
       "ASSEMBLY",
       "BYTECODE");

    compile(&data); // Первый проход

    printf(BRIGHT_BLACK("%s=\n"), mult("=+", 40));

    bool is_second_pass_necessary = false;

    for (size_t i = 0; i < 10; ++i) {
        if (data.labels[i] != -1) is_second_pass_necessary = true;
    }

    if (is_second_pass_necessary) {
        printf(BOLD(BRIGHT_WHITE("Labels:\n")));
        for (size_t i = 0; i < 10; ++i) {
            printf("[%zu] is %zd\n", i, data.labels[i]);
        }

        printf(BOLD(BRIGHT_WHITE("Второй проход:\n")));
        printf(BRIGHT_BLACK("%s=\n"), mult("=+", 40));

        data.bytecode_size = 0;

        printf(BRIGHT_BLACK("[  PC]") "  " BOLD(BRIGHT_WHITE("%-30s")) "  "
            BOLD(BRIGHT_CYAN("%s")) " " BOLD(BLUE("  ARGS")) "\n",
        "ASSEMBLY",
        "BYTECODE");

        compile(&data); // Второй проход

        printf(BRIGHT_BLACK("%s=\n"), mult("=+", 40));
    }
    FILE *out = fopen(data.output_file, "wb");
    if (!out) {
        perror("fopen output file");
        FREE(data.bytecode);
        FREE(data.input_text);
        return -1;
    }

    if (fwrite(data.bytecode, sizeof(data.bytecode[0]), data.bytecode_size, out) != data.bytecode_size) {
        fprintf(stderr, RED("error:") " failed to write bytecode to file\n");
        fclose(out);
        FREE(data.bytecode);
        FREE(data.input_text);
        return -1;
    }

    fclose(out);
    printf("Bytecode written to " MAGENTA("%s") " (%zu bytes)\n", data.output_file, data.bytecode_size);

    FREE(data.bytecode);
    FREE(data.input_text);
    return 0;
}