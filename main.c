#include <stdio.h>
#include <time.h>

#include "parser.h"
#include "huffman_alg.h"

int main(int argc, char **argv)
{
    Parser *parser = newParser(argc, argv);
    if (!parser)
    {
        fprintf(stderr, "Error: Something's wrong with the parser, I'm sorry\n");
        return 1;
    }

    if (!parseArgs(parser))
    {
        if (parser->error == SUCCESS && parser->mode == SHOW_HELP)
        {
            printHelp();
        }
        else
        {
            printError(parser);
        }
        freeParser(parser);
        return parser->error == SUCCESS ? 0 : 1;
    }

    clock_t s = clock();
    int result = 0;

    switch (parser->mode)
    {
    case COMPRESS:
        printf("Compressing %d file(s) into archive '%s'...\n",
               parser->file_count, parser->arch_name);
        compress(parser->input_files, parser->arch_name, parser->file_count);
        printStatistic(parser->arch_name, parser->input_files, parser->file_count);
        break;

    case DECOMPRESS:
        printf("Extracting archive '%s'...\n", parser->arch_name);
        decompress(parser->arch_name);
        break;

    case SHOW_HELP:
        printHelp();
        break;

    default:
        fprintf(stderr, "Hz\n");
        result = 1;
        break;
    }

    clock_t e = clock();
    double time = (double)(e - s) / CLOCKS_PER_SEC;
    printf("It's only been %.2f second(s). Very fast, right?\n", time);

    freeParser(parser);
    return result;
}