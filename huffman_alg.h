#ifndef HUFFMAN_ALG_H
#define HUFFMAN_ALG_H

#include <stdio.h>

void compress(const char **in_filenames, const char *arch_name, int count);

typedef struct {
    char name[256];
    size_t size;
} file_meta;

void decompress(const char *arch_name);

#endif