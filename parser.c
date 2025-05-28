#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "parser.h"

Parser *newParser(int argc, char **argv)
{
    Parser *parser = malloc(sizeof(Parser));
    if (!parser)
        return NULL;

    parser->argc = argc;
    parser->argv = argv;
    parser->error = SUCCESS;
    parser->mode = INVALID_OPTION;
    parser->arch_name = NULL;
    parser->input_files = NULL;
    parser->file_count = 0;

    return parser;
}

void freeParser(Parser *parser)
{
    if (parser)
    {
        if (parser->input_files)
        {
            free(parser->input_files);
        }
        if (parser->arch_name)
        {
            free(parser->arch_name);
        }
        free(parser);
    }
}

void addExtension(char *name)
{
    if (strlen(name) > 5 || strcmp(name + strlen(name) - 5, ".huff") != 0)
    {
        strcat(name, ".huff");
    }
}

int parseArgs(Parser *parser)
{
    if (parser->argc < 2)
    {
        parser->error = NO_ARGUMENTS_ERROR;
        return 0;
    }

    if (strcmp(parser->argv[1], "-h") == 0)
    {
        parser->mode = SHOW_HELP;
        return 1;
    }

    if (strcmp(parser->argv[1], "-c") == 0)
    {
        parser->mode = COMPRESS;
    }
    else if (strcmp(parser->argv[1], "-d") == 0)
    {
        parser->mode = DECOMPRESS;
    }
    else
    {
        parser->error = INVALID_OPTION_ERROR;
        return 0;
    }

    if (parser->mode == COMPRESS)
    {
        if (parser->argc < 4)
        {
            parser->error = TOO_FEW_ARGUMENTS_ERROR;
            return 0;
        }

        char with_extension[256];
        strncpy(with_extension, parser->argv[2], 250);
        with_extension[250] = '\0';
        addExtension(with_extension);
        parser->arch_name = strdup(with_extension);

        parser->file_count = parser->argc - 3;
        if (parser->file_count > 255)
        {
            parser->error = TOO_MANY_FILES_ERROR;
            return 0;
        }
        parser->input_files = malloc(sizeof(char *) * parser->file_count);
        if (!parser->input_files)
        {
            parser->error = MEMORY_ERROR;
            return 0;
        }

        for (int i = 0; i < parser->file_count; i++)
    {
        const char *filepath = parser->argv[3 + i];

        struct stat st;
        if (stat(filepath, &st) != 0)
        {
            fprintf(stderr, "Error: The file %s does not exist\n", filepath);
            return 0;
        }

        const char *basename = strrchr(filepath, '/');
        basename = basename ? basename + 1 : filepath;

        if (strlen(basename) > 255)
        {
            fprintf(stderr, "Error: The filename '%s' is too long (max 255 chars).\n", basename);
            return 0;
        }

        parser->input_files[i] = filepath;
    }
    }
    else if (parser->mode == DECOMPRESS)
    {
        if (parser->argc < 3)
        {
            parser->error = TARGET_NOT_SPECIFIED_ERROR;
            return 0;
        }
        char with_extension[256];
        strncpy(with_extension, parser->argv[2], 250);
        with_extension[250] = '\0';
        addExtension(with_extension);
        parser->arch_name = strdup(with_extension);

        struct stat st;
        if (stat(parser->arch_name, &st) != 0)
        {
            fprintf(stderr, "Error: The archive %s does not exist\n", parser->arch_name);
            return 0;
        }
    }

    return 1;
}

void printHelp()
{
    printf("Usage:\n");
    printf("   ./huffman -c archive_name file1 [file2...]    Compress files into archive\n");
    printf("   ./huffman -d archive_name                     Decompress archive\n");
    printf("   ./huffman -h                                  Show this help\n");
    printf("PS:\n");
    printf("   - .huff extension is automatically added if not specified\n");
    printf("   - Maximum 255 files can be compressed at once\n");
    printf("   - Maximum filename length is 255 characters\n");
    printf("   - You can enter the path to the file or just the name of the file if it is in the same directory.\n");
}

void printError(Parser *parser)
{
    switch (parser->error)
    {
    case NO_ARGUMENTS_ERROR:
        fprintf(stderr, "Error: Provided some arguments, please.\n");
        break;
    case INVALID_OPTION_ERROR:
        fprintf(stderr, "Error: Invalid option. There are only -c, -d or -h.\n");
        break;
    case TOO_FEW_ARGUMENTS_ERROR:
        fprintf(stderr, "Error: Not enough arguments.\n");
        break;
    case TARGET_NOT_SPECIFIED_ERROR:
        fprintf(stderr, "Error: You forgot to specify the name of the archive\n");
        break;
    case TOO_MANY_FILES_ERROR:
        fprintf(stderr, "Error: 255 files is maximum in one archive. To archive more, you need to drop 300 rubles at 89134848577 on t-bank\n");
        break;
    case MEMORY_ERROR:
        fprintf(stderr, "Error: The memory has not been allocated.\n");
        break;
    case SUCCESS:
        break;
    }
    printHelp();
}

void printStatistic(const char *arch_name, const char **input_files, int file_count)
{
    struct stat file_info;
    long orig_size = 0;
    for (int i = 0; i < file_count; i++)
    {
        stat(input_files[i], &file_info);
        orig_size += file_info.st_size;
    }

    long arch_size = 0;
    stat(arch_name, &file_info);
    arch_size = file_info.st_size;

    printf("\nCompression results:\n");
    printf("   Original size: %ld bytes\n", orig_size);
    printf("   Compressed size: %ld bytes\n", arch_size);
    if (orig_size > 0)
    {
        double ratio = (double)arch_size / orig_size * 100;
        printf("   Compression ratio: %.4f\n", ratio / 100);
        if (ratio > 100)
        {
            printf("   -Vibe: Archive is larger than original data\n");
        }
    }
}