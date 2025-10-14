#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define _DEBUG
#define tolerance_lvl 3

#define compilation
#include "proc_commands.h"

#include "io_utils.h"
#include "stringNthong.h"
#include "stack_errno.h"
#include "stack.h"

using namespace mystr;

#undef FREE
#define FREE(ptr) free(ptr); ptr = NULL;

const int MAX_INPUT_FILES  = 16;
const int MAX_LABELS_COUNT = 10;

#define conditional_jmp_body(type)                                                                      \
    else if (strcmp(cmd, PROC_INSTRUCTIONS[type].name) == 0) {                                          \
        if (*arg == ':') {                                                                              \
            bytecode[(*bytecode_size)++] = (size_t)(PROC_INSTRUCTIONS[type].byte_code);                 \
            bytecode[(*bytecode_size)++] = labels[*(arg + 1) - '0'];                                    \
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
            bytecode[(*bytecode_size)++] = (size_t)(PROC_INSTRUCTIONS[type].byte_code);                 \
            bytecode[(*bytecode_size)++] = (size_t)(new_program_counter);                               \
        }                                                                                               \
    printf("[%zu]\t (" #type " %zd) \t\t [%08x %08x]\n",                                                    \
        (*bytecode_size)-2, bytecode[(*bytecode_size)-1], bytecode[(*bytecode_size)-2], bytecode[(*bytecode_size)-1]);                \
}

#define no_args_proc_instruct_body(instruct)                                                    \
    else if (strcmp(cmd, PROC_INSTRUCTIONS[instruct].name) == 0) {                              \
        bytecode[(*bytecode_size)++] = (size_t)(PROC_INSTRUCTIONS[instruct].byte_code);         \
                                                                                                \
        printf("[%zu]\t (" #instruct ") \t\t [%08x]\n",                                           \
            (*bytecode_size) - 1, PROC_INSTRUCTIONS[instruct].byte_code);                       \
    }

int compile(char * buf, size_t buf_len, ssize_t * bytecode, size_t * bytecode_size, ssize_t * labels) {
    char * line = buf;
    while (line < buf + buf_len) {
        char * space = strchr(line, ' ');
        char * cmd = line;
        char * arg = NULL;
        if (space) {
            *space = '\0';
            arg = space + 1;
        }
        if (*cmd == ':') { // label
            labels[*(cmd + 1) - '0'] = *bytecode_size;

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[PUSH].name) == 0) {
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

                printf("[%zu]\t (PUSH %lg) \t\t [%08x %08zx]\n", (*bytecode_size),
                    value, PROC_INSTRUCTIONS[PUSH].byte_code, (ssize_t) value);

                bytecode[(*bytecode_size)++] = (ssize_t)(PROC_INSTRUCTIONS[PUSH].byte_code);
                bytecode[(*bytecode_size)++] = (ssize_t)(value);

                token = endptr;
            }
        } else if (strcmp(cmd, PROC_INSTRUCTIONS[PUSHR].name) == 0) {
            size_t register_number = get_register_by_name(arg);

            if (register_number > REGISTERS_COUNT) {
                ERROR_MSG("Неверный регистр в PUSHR: %zu", register_number);
                break;
            }

            bytecode[(*bytecode_size)++] = (size_t)(PROC_INSTRUCTIONS[PUSHR].byte_code);
            bytecode[(*bytecode_size)++] = (size_t)(register_number);

            printf("[%zu]\t (PUSHR) \t\t [%08x %08x]\n",
                (*bytecode_size) - 2, bytecode[(*bytecode_size) - 2], bytecode[(*bytecode_size) - 1]);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[POPR].name) == 0) {
            size_t register_number = get_register_by_name(arg);
            if (register_number > REGISTERS_COUNT) {
                ERROR_MSG("Неверный регистр в POPR: %zu", register_number);
                break;
            }

            bytecode[(*bytecode_size)++] = (size_t)(PROC_INSTRUCTIONS[POPR].byte_code);
            bytecode[(*bytecode_size)++] = (size_t)(register_number);

            printf("[%zu]\t (POPR) \t\t [%08x %08x]\n",
                *(bytecode_size) - 2, bytecode[(*bytecode_size) - 2], bytecode[(*bytecode_size) - 1]);
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
    return 0;
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

struct {
    char *      input_files[MAX_INPUT_FILES];
    int         input_files_count;
    char *      output_file;

    char *      input_text;
    size_t      input_text_len;

    ssize_t     labels[MAX_LABELS_COUNT];

    size_t      bytecode_capacity;
    size_t      bytecode_size;
    ssize_t *   bytecode;
} typedef compiler_internal_data;

#define init_compiler_internal_data(name)   \
    compiler_internal_data name = {};       \
    for (size_t i = 0; i < 10; ++i) {       \
        name.labels[i] = (ssize_t) -1;      \
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
    data.bytecode_size = 0;
    data.bytecode = (ssize_t *) calloc(data.bytecode_capacity, sizeof(ssize_t));

    if (data.bytecode == NULL) {
        fprintf(stderr, RED("error:") " failed to allocate bytecode buffer\n");
        FREE(data.input_text);
        return -1;
    }

    char * copy_of_input_text = (char *) calloc(data.input_text_len, sizeof(data.input_text[0]));
    memcpy(copy_of_input_text, data.input_text, data.input_text_len * sizeof(data.input_text[0]));

    replace_needle_in_haystack(copy_of_input_text, data.input_text_len, '\n', '\0');
    printf(BOLD(BRIGHT_WHITE("Первый проход:\n")));
    printf(BRIGHT_BLACK("%s=\n"), mult("=+", 40));
    compile(copy_of_input_text, data.input_text_len, data.bytecode, &data.bytecode_size, data.labels);
    printf(BRIGHT_BLACK("%s=\n"), mult("=+", 40));

    printf(BOLD(BRIGHT_WHITE("Labels:\n")));
    for (size_t i = 0; i < 10; ++i) {
        printf("[%zu] is %zd\n", i, data.labels[i]);
    }

    data.bytecode_size = 0;
    FREE(copy_of_input_text);

    replace_needle_in_haystack(data.input_text, data.input_text_len, '\n', '\0');
    printf(BOLD(BRIGHT_WHITE("Второй проход:\n")));
    printf(BRIGHT_BLACK("%s=\n"), mult("=+", 40));
    compile(data.input_text, data.input_text_len, data.bytecode, &data.bytecode_size, data.labels);
    printf(BRIGHT_BLACK("%s=\n"), mult("=+", 40));

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
    FREE(data.input_text);j
    return 0;
}