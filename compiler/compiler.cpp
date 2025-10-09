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

int main(int argc, char *argv[]) {
    if (argc == 1) {
        fprintf(stderr, RED("error:") " no input files\n");
        return -1;
    }

    char *output_file = NULL;

    const int MAX_INPUT_FILES = 16;
    char *input_files[MAX_INPUT_FILES]; // ограничим макс. число входных файлов
    int input_count = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, RED("error:") " option -o requires an argument\n");
                return -1;
            }
            if (output_file != NULL) {
                fprintf(stderr, RED("error:") " option -o specified more than once\n");
                return -1;
            }
            output_file = argv[i + 1];
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

    if (output_file == NULL) {
        output_file = "a.bin";
        // fprintf(stderr, RED("error:") " option -o is required\n");
        // return -1;
    }

    if (input_count == 0) {
        fprintf(stderr, RED("error:") " no input files\n");
        return -1;
    }

    // Всё готово!
    printf("Output file: %s\n", output_file);
    printf("Input files:\n");
    for (int i = 0; i < input_count; i++) {
        printf("  %s\n", input_files[i]);
    }

    size_t buf_len = 0;
    char * buf = read_file_to_buf(input_files[0], &buf_len);
    if (errno != 0) {
        printf("%s", strerror(errno));
        return -1;
    }

    size_t bytecode_capacity = 1024;
    size_t bytecode_size = 0;
    size_t * bytecode = (size_t *) calloc(bytecode_capacity, sizeof(size_t));
    if (!bytecode) {
        fprintf(stderr, RED("error:") " failed to allocate bytecode buffer\n");
        free(buf);
        return -1;
    }

    #define PUSH_BYTES(ptr, n)                       \
        for (size_t i = 0; i < (n); i++) {           \
            bytecode[bytecode_size++] = (((size_t *)(ptr))[i]);  \
        }

    replace_needle_in_haystack(buf, buf_len, '\n', '\0');

    char * line = buf;
    while (line < buf + buf_len) {
        char * space = strchr(line, ' ');
        char * cmd = line;
        char * arg = NULL;
        if (space) {
            *space = '\0';
            arg = space + 1;
        }

        if (strcmp(cmd, PROC_INSTRUCTIONS[PUSH].name) == 0) {
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

                printf("bytecode of (PUSH %lg) is [%x %zx]\n",
                    value, PROC_INSTRUCTIONS[PUSH].byte_code, (size_t) value);

                bytecode[bytecode_size++] = (size_t)(PROC_INSTRUCTIONS[PUSH].byte_code);

                // Добавляем double в бинарном виде
                bytecode[bytecode_size++] = (size_t)(value);

                for (size_t i = 0; i < bytecode_size; ++i) {
                    printf("%p", bytecode[i]);
                }
                printf("\n");
                printf("\n");

                token = endptr;
            }
        } else if (strcmp(cmd, PROC_INSTRUCTIONS[PUSHR].name) == 0) {
            size_t register_number = get_register_by_name(arg);

            printf("REG NAME is %s, REG is %x\n", arg, register_number);

            if (register_number > REGISTERS_COUNT) {
                ERROR_MSG("Неверный регистр в PUSHR: %zu", register_number);
                break;
            }

            bytecode[bytecode_size++] = (size_t)(PROC_INSTRUCTIONS[PUSHR].byte_code);
            bytecode[bytecode_size++] = (size_t)(register_number);

            printf("bytecode of (PUSHR) is [%x %x]\n", bytecode[bytecode_size-2], bytecode[bytecode_size-1]);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[POPR].name) == 0) {
            size_t register_number = get_register_by_name(arg);
            if (register_number > REGISTERS_COUNT) {
                ERROR_MSG("Неверный регистр в POPR: %zu", register_number);
                break;
            }

            bytecode[bytecode_size++] = (size_t)(PROC_INSTRUCTIONS[POPR].byte_code);
            bytecode[bytecode_size++] = (size_t)(register_number);

            printf("bytecode of (POPR) is [%x %x]\n", bytecode[bytecode_size-2], bytecode[bytecode_size-1]);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[ADD].name) == 0) {
            bytecode[bytecode_size++] = (size_t)(PROC_INSTRUCTIONS[ADD].byte_code);

            printf("bytecode of (ADD) is [%x]\n",
                PROC_INSTRUCTIONS[ADD].byte_code);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[SUB].name) == 0) {
            bytecode[bytecode_size++] = (size_t)(PROC_INSTRUCTIONS[SUB].byte_code);

            printf("bytecode of (SUB) is [%x]\n",
                PROC_INSTRUCTIONS[SUB].byte_code);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[MUL].name) == 0) {
            bytecode[bytecode_size++] = (size_t)(PROC_INSTRUCTIONS[MUL].byte_code);

            printf("bytecode of (MUL) is [%x]\n",
                PROC_INSTRUCTIONS[MUL].byte_code);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[DIV].name) == 0) {
            bytecode[bytecode_size++] = (size_t)(PROC_INSTRUCTIONS[DIV].byte_code);

            printf("bytecode of (DIV) is [%x]\n",
                PROC_INSTRUCTIONS[DIV].byte_code);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[OUT].name) == 0) {
            bytecode[bytecode_size++] = (size_t)(PROC_INSTRUCTIONS[OUT].byte_code);

            printf("bytecode of (OUT) is [%x]\n",
                PROC_INSTRUCTIONS[OUT].byte_code);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[IN].name) == 0) {
            bytecode[bytecode_size++] = (size_t)(PROC_INSTRUCTIONS[IN].byte_code);

            printf("bytecode of (IN) is [%x]\n",
                PROC_INSTRUCTIONS[IN].byte_code);

        } else if (strcmp(cmd, PROC_INSTRUCTIONS[HLT].name) == 0) {
            bytecode[bytecode_size++] = (size_t)(PROC_INSTRUCTIONS[HLT].byte_code);

            printf("bytecode of (HLT) is [%x]\n",
                PROC_INSTRUCTIONS[HLT].byte_code);
            break;
        }

        line += (strlen(line) + 1);
    }

    FILE *out = fopen(output_file, "wb");
    if (!out) {
        perror("fopen output file");
        free(bytecode);
        free(buf);
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
        free(buf);
        return -1;
    }

    fclose(out);
    printf("Bytecode written to %s (%zu bytes)\n", output_file, bytecode_size);

    free(bytecode);
    free(buf);
    return 0;
}