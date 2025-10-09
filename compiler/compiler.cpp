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

const int MAX_INPUT_FILES = 16;

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
    printf("[%zu]\t (" #type ") \t\t [%x %x]\n", (*bytecode_size)-2, bytecode[(*bytecode_size)-2], bytecode[(*bytecode_size)-1]); \
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

                printf("[%zu]\t (PUSH %lg) \t\t [%x %zx]\n", (*bytecode_size)-2,
                    value, PROC_INSTRUCTIONS[PUSH].byte_code, (ssize_t)value);

                bytecode[(*bytecode_size)++] = (ssize_t)(PROC_INSTRUCTIONS[PUSH].byte_code);
                bytecode[(*bytecode_size)++] = (ssize_t)(value);

                // for (size_t i = 0; i < bytecode_size; ++i) {
                //     printf("%p", bytecode[i]);
                // }
                // printf("\n");
                // printf("\n");

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

            printf("[%zu]\t (PUSHR) \t\t [%x %x]\n",
                (*bytecode_size) - 2, bytecode[(*bytecode_size) - 2], bytecode[(*bytecode_size) - 1]);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[POPR].name) == 0) {
            size_t register_number = get_register_by_name(arg);
            if (register_number > REGISTERS_COUNT) {
                ERROR_MSG("Неверный регистр в POPR: %zu", register_number);
                break;
            }

            bytecode[(*bytecode_size)++] = (size_t)(PROC_INSTRUCTIONS[POPR].byte_code);
            bytecode[(*bytecode_size)++] = (size_t)(register_number);

            printf("[%zu]\t (POPR) \t\t [%x %x]\n",
                *(bytecode_size) - 2, bytecode[(*bytecode_size) - 2], bytecode[(*bytecode_size) - 1]);
        }
        conditional_jmp_body(JMP)
        conditional_jmp_body(JB)
        conditional_jmp_body(JBE)
        conditional_jmp_body(JA)
        conditional_jmp_body(JAE)
        conditional_jmp_body(JE)
        conditional_jmp_body(JNE)
        else if (strcmp(cmd, PROC_INSTRUCTIONS[ADD].name) == 0) {
            bytecode[(*bytecode_size)++] = (size_t)(PROC_INSTRUCTIONS[ADD].byte_code);

            printf("[%zu]\t (ADD) \t\t [%x]\n",
                (*bytecode_size) - 1, PROC_INSTRUCTIONS[ADD].byte_code);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[SUB].name) == 0) {
            bytecode[(*bytecode_size)++] = (size_t)(PROC_INSTRUCTIONS[SUB].byte_code);

            printf("[%zu]\t (SUB) \t\t [%x]\n",
                (*bytecode_size) - 1, PROC_INSTRUCTIONS[SUB].byte_code);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[MUL].name) == 0) {
            bytecode[(*bytecode_size)++] = (size_t)(PROC_INSTRUCTIONS[MUL].byte_code);

            printf("[%zu]\t (MUL) \t\t [%x]\n",
                (*bytecode_size) - 1, PROC_INSTRUCTIONS[MUL].byte_code);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[DIV].name) == 0) {
            bytecode[(*bytecode_size)++] = (size_t)(PROC_INSTRUCTIONS[DIV].byte_code);

            printf("[%zu]\t (DIV) \t\t [%x]\n",
                (*bytecode_size) - 1, PROC_INSTRUCTIONS[DIV].byte_code);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[SQRT].name) == 0) {
            bytecode[(*bytecode_size)++] = (size_t)(PROC_INSTRUCTIONS[SQRT].byte_code);

            printf("[%zu]\t (SQRT) \t\t [%x]\n",
                (*bytecode_size) - 1, PROC_INSTRUCTIONS[SQRT].byte_code);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[OUT].name) == 0) {
            bytecode[(*bytecode_size)++] = (size_t)(PROC_INSTRUCTIONS[OUT].byte_code);

            printf("[%zu]\t (OUT) \t\t [%x]\n",
                (*bytecode_size) - 1, PROC_INSTRUCTIONS[OUT].byte_code);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[IN].name) == 0) {
            bytecode[(*bytecode_size)++] = (size_t)(PROC_INSTRUCTIONS[IN].byte_code);

            printf("[%zu]\t (IN) \t\t [%x]\n",
                (*bytecode_size) - 1, PROC_INSTRUCTIONS[IN].byte_code);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[HLT].name) == 0) {
            bytecode[(*bytecode_size)++] = (size_t)(PROC_INSTRUCTIONS[HLT].byte_code);

            printf("[%zu]\t (HLT) \t\t [%x]\n",
                (*bytecode_size) - 1, PROC_INSTRUCTIONS[HLT].byte_code);
        }

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

int main(int argc, char * argv[]) {
    char * output_file = NULL;
    char * input_files[MAX_INPUT_FILES];
    int input_count = parse_args(argc, argv, &output_file, input_files);
    
    printf("Output file: %s\n", output_file);
    printf("Input files:\n");
    for (int i = 0; i < input_count; i++) {
        printf("  %s\n", input_files[i]);
    }

    size_t input_text_len = 0;
    char * input_text = read_file_to_buf(input_files[0], &input_text_len);
    if (errno != 0) {
        printf("%s", strerror(errno));
        return -1;
    }

    ssize_t labels[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    size_t bytecode_capacity = 1024;
    size_t bytecode_size = 0;
    ssize_t * bytecode = (ssize_t *) calloc(bytecode_capacity, sizeof(ssize_t));
    if (!bytecode) {
        fprintf(stderr, RED("error:") " failed to allocate bytecode buffer\n");
        free(input_text);
        return -1;
    }

    char * copy_of_input_text = (char *) calloc(input_text_len, sizeof(input_text[0]));
    memcpy(copy_of_input_text, input_text, input_text_len * sizeof(input_text[0]));

    replace_needle_in_haystack(copy_of_input_text, input_text_len, '\n', '\0');
    compile(copy_of_input_text, input_text_len, bytecode, &bytecode_size, labels);

    for (size_t i = 0; i < 10; ++i) {
        printf("[%zu] is %zd\n", i, labels[i]);
    }

    bytecode_size = 0;
    FREE(copy_of_input_text);

    replace_needle_in_haystack(input_text, input_text_len, '\n', '\0');
    compile(input_text, input_text_len, bytecode, &bytecode_size, labels);

    FILE *out = fopen(output_file, "wb");
    if (!out) {
        perror("fopen output file");
        free(bytecode);
        free(input_text);
        return -1;
    }

    for (size_t i = 0; i < bytecode_size; ++i) {
        printf("%p", bytecode[i]);
    }
    printf("\n");

    if (fwrite(bytecode, 8, bytecode_size, out) != bytecode_size) {
        fprintf(stderr, RED("error:") " failed to write bytecode to file\n");
        fclose(out);
        free(bytecode);
        free(input_text);
        return -1;
    }

    fclose(out);
    printf("Bytecode written to %s (%zu bytes)\n", output_file, bytecode_size);

    free(bytecode);
    free(input_text);
    return 0;
}