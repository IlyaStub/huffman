// huffman_builder.h
#ifndef HUFFMAN_BUILDER_H
#define HUFFMAN_BUILDER_H

#include <stdint.h>
#include <stddef.h>
#include "huffman_tree.h"
#include "binary_writer.h"
#include "binary_reader.h"

#define ALF_SIZE 256

typedef struct {
    uint64_t bits;
    uint8_t length;
} huffman_code_t;

void buildFreqTable(const char* filename, size_t* freq_table);
tree_t* buildHuffmanTree(const size_t* freq_table);
void genCodes(tree_t* root, huffman_code_t* table);
void serTree(tree_t* root, file_writer* writer);
tree_t* deserTree(file_reader* reader);

#endif