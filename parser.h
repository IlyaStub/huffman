#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

typedef enum mode {
    COMPRESS,
    DECOMPRESS,
    SHOW_HELP,
    INVALID_OPTION
} Option;

typedef enum parsing_error {
    SUCCESS,
    INVALID_OPTION_ERROR,
    NO_ARGUMENTS_ERROR,
    TOO_FEW_ARGUMENTS_ERROR,
    TARGET_NOT_SPECIFIED_ERROR,
    TOO_MANY_FILES_ERROR,
    MEMORY_ERROR
} ParsingError;

typedef struct parser {
    int argc;
    char** argv;
    ParsingError error;
    Option mode;
    char* arch_name;
    const char** input_files;
    int file_count;
} Parser;

Parser* newParser(int argc, char** argv);

void freeParser(Parser* parser);

int parseArgs(Parser* parser);

void printHelp();

void printError(Parser* parser);

void printStatistic(const char* arch_name, const char** input_files, int file_count);

#endif