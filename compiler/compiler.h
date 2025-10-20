#ifndef COMPILER_H
#define COMPILER_H

#include <stddef.h>
#include <sys/types.h>
#include "stack.h"

stack_element_t typedef bytecode_t;

// Константы
static const int MAX_INPUT_FILES  = 16;
static const int MAX_LABELS_COUNT = 10;

// Перечисление ошибок компилятора
enum COMPILER_ERRNO {
    COMPILER_NO_PROBLEM               = 0,
    COMPILER_CANNOT_ALLOCATE_MEMORY   = 1,
    COMPILER_CANNOT_REALLOCATE_MEMORY = 2,
    COMPILER_PROVIDE_NULLPTR          = 3,
    COMPILER_NO_SUCH_LABEL            = 4,
    COMPILER_LABEL_ALREADY_EXIST      = 5,
    COMPILER_INVALID_ARG_TYPE         = 6,
    COMPILER_WRONG_INSTRUCTION_USING  = 7,
    COMPILER_SYNTAX_ERROR             = 8,
};

struct label_t {
    const char const * name;
    size_t program_counter;
};

// Структура для внутренних данных компилятора
typedef struct {
    char *          input_files[MAX_INPUT_FILES];
    int             input_files_count;
    char *          output_file;

    char *          input_text;
    size_t          input_text_len;

    size_t          labels_capacity;
    size_t          labels_size;
    label_t *       labels;

    size_t          bytecode_capacity;
    size_t          bytecode_size;
    bytecode_t *    bytecode;

    COMPILER_ERRNO  compile_status;
} compiler_internal_data;

// Макрос для инициализации структуры данных компилятора
#define INIT_COMPILER_INTERNAL_DATA(NAME)                                   \
    compiler_internal_data NAME = {};                                       \
    NAME.bytecode_capacity = 1024;                                          \
    NAME.bytecode_size = BYTECODE_SIGNATURE_SIZE;                           \
    NAME.bytecode = (bytecode_t *)                                          \
                calloc(NAME.bytecode_capacity, sizeof(NAME.bytecode[0]));   \
    NAME.labels_capacity = 1;                                               \
    NAME.labels_size = 1;                                                   \
    NAME.labels = (label_t *)                                               \
                calloc(data.labels_capacity, sizeof(NAME.labels[0]));       \
    NAME.labels[0] = {.name = "__start", .program_counter = BYTECODE_SIGNATURE_SIZE};

#define dtor_compiler_internal_data(NAME)   \
    FREE(NAME.input_text);                  \
    FREE(NAME.labels);                      \
    FREE(NAME.bytecode);

// Функции компилятора
/**
 * Добавляет команду в байткод
 * @param data - указатель на данные компилятора
 * @param command_bytecode - байткод команды
 * @param argc - количество аргументов
 * @param ... - аргументы команды
 * @return код ошибки
 */
COMPILER_ERRNO bytecode_add_command(compiler_internal_data * data, size_t command_bytecode, size_t argc, ...);

COMPILER_ERRNO add_label(compiler_internal_data * data, char * name, size_t new_pc);

COMPILER_ERRNO get_label(compiler_internal_data * data, char * name, label_t * label);

/**
 * Компилирует исходный код в байткод
 * @param data - указатель на данные компилятора
 * @return код ошибки
 */
COMPILER_ERRNO compile(compiler_internal_data * data);

/**
 * Парсит аргументы командной строки
 * @param argc - количество аргументов
 * @param argv - массив аргументов
 * @param output_file - выходной файл
 * @param input_files - массив входных файлов
 * @return количество входных файлов или -1 при ошибке
 */
int parse_args(int argc, char * argv[], char ** output_file, char * input_files[MAX_INPUT_FILES]);

#endif // COMPILER_H