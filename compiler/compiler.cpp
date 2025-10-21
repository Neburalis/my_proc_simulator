#undef _DEBUG

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#define compilation
#include "proc_commands.h"
#include "compiler.h"

#include "io_utils.h"
#include "stringNthong.h"

using namespace mystr;

#undef FREE
#define FREE(ptr) do {free((ptr)); (ptr) = NULL;} while(0)

#define OK(cond) !(cond)

#define verified(code) \
    || ({code; false;})

#define CONDITIONAL_JMP_BODY_RETURN_IF_ERR(data, TYPE)                                                  \
    else if (comp_to(cmd, PROC_INSTRUCTIONS[TYPE].name, ' ') == 0) {                                    \
        if (*arg == ':') {                                                                              \
            label_t label = {};                                                                         \
            COMPILER_ERRNO status = get_label(data, arg + 1, &label);                                   \
            ssize_t new_pc = -1;                                                                        \
                                                                                                        \
            if (status == COMPILER_ERRNO::COMPILER_NO_PROBLEM)                                          \
                new_pc = label.program_counter;                                                         \
            else if (status != COMPILER_ERRNO::COMPILER_NO_SUCH_LABEL){                                 \
                data->compile_status = status;                                                           \
                return status;                                                                          \
            }                                                                                           \
                                                                                                        \
            BYTECODE_ADD_AND_LOG_1_INT(data, TYPE, new_pc);                                             \
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
            BYTECODE_ADD_AND_LOG_1_INT(data, TYPE, new_program_counter);                                \
        }                                                                                               \
    }

#define MEMORY_ACCESS_PROC_INSTRUCT_RETURN_IF_ERR(data, TYPE)                                           \
    else if (comp_to(cmd, PROC_INSTRUCTIONS[TYPE].name, ' ') == 0) {                                    \
        if (*arg != '[') {                                                                              \
            ERROR_MSG("Неверно передан аргумент в " #TYPE ": %s\n"                                      \
                "Ожидалась [, а было получено %c\n", arg, *arg);                                        \
            data->compile_status = COMPILER_WRONG_INSTRUCTION_USING;                                     \
            return COMPILER_WRONG_INSTRUCTION_USING;                                                    \
        }                                                                                               \
                                                                                                        \
        char * copy_of_arg = strdup(arg);                                                               \
        char * end = copy_of_arg + strlen(copy_of_arg) - 1;                                             \
        move_ptr_to_first_alpha_symbol(&end, true);                                                     \
        end++;                                                                                          \
                                                                                                        \
        if (*end != ']') {                                                                              \
            ERROR_MSG("Неверно передан аргумент в " #TYPE ": %s\n"                                      \
                "Ожидалась ], а было получено (%c) %d\n", arg, *end, *end);                             \
            FREE(copy_of_arg);                                                                          \
            data->compile_status = COMPILER_WRONG_INSTRUCTION_USING;                                     \
            return COMPILER_WRONG_INSTRUCTION_USING;                                                    \
        }                                                                                               \
        *end = '\0';                                                                                    \
                                                                                                        \
        size_t register_number = get_register_by_name(copy_of_arg + 1);                                 \
        FREE(copy_of_arg);                                                                              \
                                                                                                        \
        BYTECODE_ADD_AND_LOG_1_INT(data, TYPE, register_number);                                        \
    }

#define PUSH_INSTRUCT_RETURN_IF_ERR(data)                                                               \
    else if (comp_to(cmd, PROC_INSTRUCTIONS[PUSH].name, ' ') == 0) {                                    \
        if (arg == NULL) {                                                                              \
            ERROR_MSG("Команде PUSH было передано аргументов 0, а должно быть хотя бы 1");              \
            data->compile_status = COMPILER_WRONG_INSTRUCTION_USING;                                    \
            return COMPILER_WRONG_INSTRUCTION_USING;                                                    \
        }                                                                                               \
                                                                                                        \
        char * token = arg;                                                                             \
        char * endptr = NULL;                                                                           \
                                                                                                        \
        while (*token != '\0') {                                                                        \
            while (*token == ' ') token++;                                                              \
            if (*token == '\0') break;                                                                  \
                                                                                                        \
            errno = 0;                                                                                  \
            double value = strtod(token, &endptr);                                                      \
                                                                                                        \
            if (errno != 0 || token == endptr) {                                                        \
                ERROR_MSG("Неверный числовой аргумент в PUSH: %s", token);                              \
                data->compile_status = COMPILER_WRONG_INSTRUCTION_USING;                                \
                return COMPILER_WRONG_INSTRUCTION_USING;                                                \
            }                                                                                           \
                                                                                                        \
            BYTECODE_ADD_AND_LOG_1_FRAC(data, PUSH, value);                                             \
                                                                                                        \
            token = endptr;                                                                             \
        }                                                                                               \
    }

#define NO_ARGS_PROC_INSTRUCT_BODY(data, INSTRUCT)                                                      \
    else if (comp_to(cmd, PROC_INSTRUCTIONS[INSTRUCT].name, ' ') == 0) {                                \
        BYTECODE_ADD_AND_LOG_0(data, INSTRUCT);                                                         \
    }

#ifdef _DEBUG
    #define BYTECODE_ADD_AND_LOG_0(data, CMD) do {                                                          \
        size_t pc = (data)->bytecode_size;                                                                  \
        bytecode_add_command((data), PROC_INSTRUCTIONS[CMD].byte_code, 0);                                  \
        printf(BRIGHT_BLACK("[%4zu]") "  " YELLOW("%-30s") "  " CYAN("%08zx") "\n",                         \
            pc, #CMD, (size_t)PROC_INSTRUCTIONS[CMD].byte_code);                                            \
    } while (0)
#else
    #define BYTECODE_ADD_AND_LOG_0(data, CMD) do {                                                          \
        bytecode_add_command((data), PROC_INSTRUCTIONS[CMD].byte_code, 0);                                  \
    } while (0)
#endif

#ifdef _DEBUG
    #define BYTECODE_ADD_AND_LOG_1_INT(data, CMD, arg) do {                                                 \
        bytecode_add_command((data), PROC_INSTRUCTIONS[CMD].byte_code, 1, ARG_INT, (arg));                  \
        size_t pc = (data)->bytecode_size;                                                                  \
        char cmd_repr[64] = "";                                                                             \
        snprintf(cmd_repr, sizeof(cmd_repr), "%s %zd", #CMD, (arg));                                        \
        printf(BRIGHT_BLACK("[%4zu]") "  " YELLOW("%-30s") "  " CYAN("%08zx") " " BLUE("%016llx") "\n",     \
            pc, cmd_repr,                                                                                   \
            (size_t)PROC_INSTRUCTIONS[CMD].byte_code, (int64_t)(arg));                                      \
    } while(0)
#else
    #define BYTECODE_ADD_AND_LOG_1_INT(data, CMD, arg) do {                                                 \
        bytecode_add_command((data), PROC_INSTRUCTIONS[CMD].byte_code, 1, ARG_INT, (arg));                  \
    } while (0)
#endif

#ifdef _DEBUG
    #define BYTECODE_ADD_AND_LOG_1_FRAC(data, CMD, arg) do {                                                \
        size_t pc = (data)->bytecode_size;                                                                  \
        bytecode_add_command((data), PROC_INSTRUCTIONS[CMD].byte_code, 1, ARG_FRAC, (arg));                 \
        char cmd_repr[64] = "";                                                                             \
        unsigned char bytes[sizeof(double)] = "";                                                           \
        memcpy(bytes, &arg, sizeof(double));                                                                \
        snprintf(cmd_repr, sizeof(cmd_repr), "%s %lg", #CMD, (arg));                                        \
        printf(BRIGHT_BLACK("[%4zu]") "  " YELLOW("%-30s") "  " CYAN("%08zx") " " BLUE("%x") "\n",          \
            pc, cmd_repr,                                                                                   \
            (size_t)PROC_INSTRUCTIONS[CMD].byte_code, bytes);                                               \
    } while(0)
#else
    #define BYTECODE_ADD_AND_LOG_1_FRAC(data, CMD, arg) do {                                                \
        bytecode_add_command((data), PROC_INSTRUCTIONS[CMD].byte_code, 1, ARG_FRAC, (arg));                 \
    } while(0)
#endif

enum arg_type_t {
    ARG_INT,
    ARG_FRAC,
};

COMPILER_ERRNO bytecode_add_command(compiler_internal_data * data, size_t command_bytecode, size_t argc, ...) {
    assert(data);
    assert(data->bytecode);

    if (data == NULL || data->bytecode == NULL) {
        data->compile_status = COMPILER_PROVIDE_NULLPTR;
        return COMPILER_ERRNO::COMPILER_PROVIDE_NULLPTR;
    }

    size_t required_space = 1 + argc;
    if (data->bytecode_size + required_space > data->bytecode_capacity) {
        size_t new_capacity = data->bytecode_capacity * 2;
        bytecode_t * new_bytecode = (bytecode_t *) realloc(data->bytecode, new_capacity * sizeof(data->bytecode[0]));

        if (new_bytecode == NULL) {
            data->compile_status = COMPILER_CANNOT_REALLOCATE_MEMORY;
            return COMPILER_ERRNO::COMPILER_CANNOT_REALLOCATE_MEMORY;
        }
        data->bytecode = new_bytecode;
        data->bytecode_capacity = new_capacity;
    }

    data->bytecode[data->bytecode_size++].cmd = command_bytecode;

    if (argc > 0) {
        va_list args;
        va_start(args, argc);

        for (size_t i = 0; i < argc; ++i) {
            arg_type_t type = va_arg(args, arg_type_t);

            if (type == ARG_INT) {
                data->bytecode[data->bytecode_size++].cmd = va_arg(args, ssize_t);
            } else if (type == ARG_FRAC) {
                data->bytecode[data->bytecode_size++].arg = va_arg(args, double);
            } else {
                va_end(args);
                data->compile_status = COMPILER_INVALID_ARG_TYPE;
                return COMPILER_ERRNO::COMPILER_INVALID_ARG_TYPE;
            }
        }
        va_end(args);
    }

    data->compile_status = COMPILER_NO_PROBLEM;
    return COMPILER_ERRNO::COMPILER_NO_PROBLEM;
}

#define PRINT_LABELS(data) \
    printf(BRIGHT_BLACK("Running from %s:%d in func %s\n"), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
    print_labels_impl(data);

void print_labels_impl(compiler_internal_data * data) {
    printf("labels_size is \t%zu\n", data->labels_size);
    printf("labels_capacity is \t%zu\n", data->labels_capacity);
    for (size_t i = 0; i < data->labels_size; ++i) {
        printf("[%zu] {.name = [%p]\"%s\", .program_counter = %zu}\n", i, data->labels[i].name, data->labels[i].name, data->labels[i].program_counter);
    }
}

COMPILER_ERRNO get_label(compiler_internal_data * data, char * name, label_t * label) {
    for (size_t i = 0; i < data->labels_size; ++i) {
        if (strcmp(name, data->labels[i].name) == 0) {
            if (label != NULL)
                *label = data->labels[i];
            data->compile_status = COMPILER_NO_PROBLEM;
            return COMPILER_ERRNO::COMPILER_NO_PROBLEM;
        }
    }
    data->compile_status = COMPILER_NO_SUCH_LABEL;
    return COMPILER_ERRNO::COMPILER_NO_SUCH_LABEL;
}

COMPILER_ERRNO add_label(compiler_internal_data * data, char * name, size_t new_pc) {
    if (get_label(data, name, NULL) != COMPILER_ERRNO::COMPILER_NO_SUCH_LABEL) {
        data->compile_status = COMPILER_LABEL_ALREADY_EXIST;
        return COMPILER_ERRNO::COMPILER_LABEL_ALREADY_EXIST;
    }

    if (data->labels_size >= data->labels_capacity) {
        size_t new_labels_capacity = data->labels_capacity * 2;
        label_t * new_labels = (label_t *) realloc(data->labels, new_labels_capacity * sizeof(data->labels[0]));
        if (new_labels == NULL) {
            data->compile_status = COMPILER_CANNOT_REALLOCATE_MEMORY;
            return COMPILER_ERRNO::COMPILER_CANNOT_REALLOCATE_MEMORY;
        }
        data->labels_capacity = new_labels_capacity;
        data->labels = new_labels;
    }

    data->labels[data->labels_size++] = {.name = name, .program_counter = new_pc};
    data->compile_status = COMPILER_NO_PROBLEM;
    return COMPILER_ERRNO::COMPILER_NO_PROBLEM;
}

COMPILER_ERRNO compile(compiler_internal_data * data) {
    char * line = data->input_text;
    while (line < data->input_text + data->input_text_len) {
        move_ptr_to_first_not_space_symbol(&line, false);
        // printf("\tline is [%s]\n", line);
        char * space = strchr(line, ' ');
        char * cmd = line;
        char * arg = NULL;
        if (space) {
            // *space = '\0';
            arg = space + 1;
        }
        if (*cmd == ':') { // label
            add_label(data, (cmd + 1), data->bytecode_size);
        } else if (comp_to(cmd, PROC_INSTRUCTIONS[PUSHR].name, ' ') == 0) {
            size_t register_number = get_register_by_name(arg);

            if (register_number > REGISTERS_COUNT) {
                ERROR_MSG("Неверный регистр в PUSHR: %zu", register_number);
                data->compile_status = COMPILER_WRONG_INSTRUCTION_USING;
                return COMPILER_WRONG_INSTRUCTION_USING;
            }

            BYTECODE_ADD_AND_LOG_1_INT(data, PUSHR, register_number);

        } else if (comp_to(cmd, PROC_INSTRUCTIONS[POPR].name, ' ') == 0) {
            size_t register_number = get_register_by_name(arg);
            if (register_number > REGISTERS_COUNT) {
                ERROR_MSG("Неверный регистр в POPR: %zu", register_number);
                data->compile_status = COMPILER_WRONG_INSTRUCTION_USING;
                return COMPILER_WRONG_INSTRUCTION_USING;
            }

            BYTECODE_ADD_AND_LOG_1_INT(data, POPR, register_number);
        } else if (comp_to(cmd, PROC_INSTRUCTIONS[CALL].name, ' ') == 0) {
            if (*arg == ':') {
                label_t label = {};
                COMPILER_ERRNO status = get_label(data, arg + 1, &label);
                ssize_t new_pc = -1;

                if (status == COMPILER_ERRNO::COMPILER_NO_PROBLEM)
                    new_pc = label.program_counter;
                else if (status != COMPILER_ERRNO::COMPILER_NO_SUCH_LABEL) {
                    data->compile_status = status;
                    return status;
                }

                BYTECODE_ADD_AND_LOG_1_INT(data, CALL, new_pc);
            }
            else {
                char * endptr = NULL;
                errno = 0;

                size_t new_program_counter = strtoull(arg, &endptr, 10);

                if (errno == ERANGE || endptr == arg) {
                    ERROR_MSG("Не удалось получить новый PC из аргументов (%s)", arg);
                    data->compile_status = COMPILER_WRONG_INSTRUCTION_USING;
                    return COMPILER_WRONG_INSTRUCTION_USING;
                }

                BYTECODE_ADD_AND_LOG_1_INT(data, CALL, new_program_counter);
            }
        } else if (comp_to(cmd, PROC_INSTRUCTIONS[DRAW].name, ' ') == 0) {
            char * endptr = NULL;
            errno = 0;

            size_t sleep_time = strtoull(arg, &endptr, 10);

            if (errno == ERANGE || endptr == arg) {
                ERROR_MSG("Не удалось получить время сна из аргументов (%s)", arg);
                data->compile_status = COMPILER_WRONG_INSTRUCTION_USING;
                return COMPILER_WRONG_INSTRUCTION_USING;
            }

           BYTECODE_ADD_AND_LOG_1_INT(data, DRAW, sleep_time);
        }
        PUSH_INSTRUCT_RETURN_IF_ERR(data)
        MEMORY_ACCESS_PROC_INSTRUCT_RETURN_IF_ERR(data, PUSHM)
        MEMORY_ACCESS_PROC_INSTRUCT_RETURN_IF_ERR(data, POPM)
        CONDITIONAL_JMP_BODY_RETURN_IF_ERR(data, JMP)
        CONDITIONAL_JMP_BODY_RETURN_IF_ERR(data, JB)
        CONDITIONAL_JMP_BODY_RETURN_IF_ERR(data, JBE)
        CONDITIONAL_JMP_BODY_RETURN_IF_ERR(data, JA)
        CONDITIONAL_JMP_BODY_RETURN_IF_ERR(data, JAE)
        CONDITIONAL_JMP_BODY_RETURN_IF_ERR(data, JE)
        CONDITIONAL_JMP_BODY_RETURN_IF_ERR(data, JNE)
        NO_ARGS_PROC_INSTRUCT_BODY(data, ADD)
        NO_ARGS_PROC_INSTRUCT_BODY(data, SUB)
        NO_ARGS_PROC_INSTRUCT_BODY(data, MUL)
        NO_ARGS_PROC_INSTRUCT_BODY(data, DIV)
        NO_ARGS_PROC_INSTRUCT_BODY(data, SQRT)
        NO_ARGS_PROC_INSTRUCT_BODY(data, SIN)
        NO_ARGS_PROC_INSTRUCT_BODY(data, COS)
        NO_ARGS_PROC_INSTRUCT_BODY(data, MOD)
        NO_ARGS_PROC_INSTRUCT_BODY(data, IDIV)
        NO_ARGS_PROC_INSTRUCT_BODY(data, OUT)
        NO_ARGS_PROC_INSTRUCT_BODY(data, IN)
        NO_ARGS_PROC_INSTRUCT_BODY(data, RET)
        NO_ARGS_PROC_INSTRUCT_BODY(data, HLT)
        else {
            if (*cmd == ';' || strlen(line) == 0) { // comment or empty
                line += (strlen(line) + 1);
                continue;
            }

            ERROR_MSG("Syntax error in (%s) len is %d\n", line, strlen(line));

            data->compile_status = COMPILER_SYNTAX_ERROR;
            return COMPILER_ERRNO::COMPILER_SYNTAX_ERROR;
        }

        line += (strlen(line) + 1);
    }
    data->compile_status = COMPILER_NO_PROBLEM;
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
    INIT_COMPILER_INTERNAL_DATA(data);

    data.input_files_count = parse_args(argc, argv, &data.output_file, data.input_files);

    printf("Output file: %s\n", data.output_file);
    printf("Input files:\n");
    for (int i = 0; i < data.input_files_count; i++) {
        printf("  %s\n", data.input_files[i]);
    }

    data.input_text = read_file_to_buf(data.input_files[0], &data.input_text_len);
    if (errno != 0) {
        printf("%s", strerror(errno));
        dtor_compiler_internal_data(data);
        return -1;
    }

    if (data.bytecode == NULL) {
        fprintf(stderr, RED("error:") " failed to allocate bytecode buffer\n");
        dtor_compiler_internal_data(data);
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

    if (data.compile_status != COMPILER_ERRNO::COMPILER_NO_PROBLEM) {
        ERROR_MSG("Error in compilation: %d\n", data.compile_status);

        dtor_compiler_internal_data(data);
        return -1;
    }

    printf(BRIGHT_BLACK("%s=\n"), mult("=+", 40));

    if (data.labels_size > 1) {
        printf(BOLD(BRIGHT_WHITE("Labels:\n")));
        printf(BRIGHT_BLACK("\t%-30s new_pc\n"), "name");
        for (size_t i = 0; i < data.labels_size; ++i) {
            printf("[%zu]\t%-30s %zd\n", i, data.labels[i].name, data.labels[i].program_counter);
        }

        printf(BOLD(BRIGHT_WHITE("Второй проход:\n")));
        printf(BRIGHT_BLACK("%s=\n"), mult("=+", 40));

        data.bytecode_size = BYTECODE_SIGNATURE_SIZE;

        printf(BRIGHT_BLACK("[  PC]") "  " BOLD(BRIGHT_WHITE("%-30s")) "  "
            BOLD(BRIGHT_CYAN("%s")) " " BOLD(BLUE("  ARGS")) "\n",
        "ASSEMBLY",
        "BYTECODE");

        compile(&data); // Второй проход

        if (data.compile_status != COMPILER_ERRNO::COMPILER_NO_PROBLEM) {
            ERROR_MSG("Error in compilation: %d", data.compile_status);

            dtor_compiler_internal_data(data);
            return -1;
        }

        printf(BRIGHT_BLACK("%s=\n"), mult("=+", 40));
    }

    data.bytecode[0].cmd = PROC_SIGNATURE;
    data.bytecode[1].cmd = PROC_COMANDS_VERSION;
    data.bytecode[2].cmd = data.bytecode_size;
    data.bytecode[3].cmd = (uint64_t) time(NULL);

    FILE *out = fopen(data.output_file, "wb");
    if (!out) {
        perror("fopen output file");
        dtor_compiler_internal_data(data);
        return -1;
    }

    if (fwrite(data.bytecode, sizeof(data.bytecode[0]), data.bytecode_size, out) != data.bytecode_size) {
        fprintf(stderr, RED("error:") " failed to write bytecode to file\n");
        fclose(out);
        dtor_compiler_internal_data(data);
        return -1;
    }

    fclose(out);
    printf("Bytecode written to " MAGENTA("%s") " (%zu bytes)\n", data.output_file, data.bytecode_size);

    dtor_compiler_internal_data(data);
    return 0;
}