#ifndef COMPILER_H
#define COMPILER_H

#include <stddef.h>
#include <sys/types.h>

// Константы
static const int MAX_INPUT_FILES  = 16;
static const int MAX_LABELS_COUNT = 10;

// Перечисление ошибок компилятора
enum COMPILER_ERRNO {
    COMPILER_NO_PROBLEM               = 0,
    COMPILER_CANNOT_ALLOCATE_MEMORY   = 1,
    COMPILER_CANNOT_REALLOCATE_MEMORY = 2,
    COMPILER_PROVIDE_NULLPTR          = 3,
};

// Структура для внутренних данных компилятора
typedef struct {
    char *      input_files[MAX_INPUT_FILES];
    int         input_files_count;
    char *      output_file;

    char *      input_text;
    size_t      input_text_len;

    ssize_t     labels[MAX_LABELS_COUNT];

    size_t      bytecode_capacity;
    size_t      bytecode_size;
    ssize_t *   bytecode;
} compiler_internal_data;

// Макрос для инициализации структуры данных компилятора
#define init_compiler_internal_data(name)   \
    compiler_internal_data name = {};       \
    for (size_t i = 0; i < 10; ++i) {       \
        name.labels[i] = (ssize_t) -1;      \
    }

// Функции компилятора
/**
 * Добавляет команду в байткод
 * @param data - указатель на данные компилятора
 * @param command_bytecode - байткод команды
 * @param argc - количество аргументов
 * @param ... - аргументы команды
 * @return код ошибки
 */
COMPILER_ERRNO bytecode_add_command(compiler_internal_data * data, ssize_t command_bytecode, size_t argc, ...);

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