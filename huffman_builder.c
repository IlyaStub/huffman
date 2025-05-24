#include <stdio.h>
#include <stdlib.h>

#include "huffman_builder.h"

void buildFreqTable(const char *filename, size_t *freq_table)
{
    file_reader *reader = newFileReader(filename, 4096);
    if (!reader)
    {
        fprintf(stderr, "Error: cannot open file '%s'\n", filename);
        return;
    }

    for (int i = 0; i < ALF_SIZE; i++)
        freq_table[i] = 0;

    size_t counter = 0;
    while (!reader->eof)
    {
        uint8_t byte = reader->read_byte(reader);
        if (!reader->eof)
        {
            freq_table[byte]++;
            counter++;

            if (counter % 1000000 == 0)
            {
                float p = reader->progress(reader);
                printf("\rReading: %.1f%%", p * 100);
                fflush(stdout);
            }
        }
    }

    printf("\rReading: 100.0%%\n");
    freeFileReader(reader);
}

tree_t *buildHuffmanTree(const size_t *freq_table)
{
    queue_t *queue = createQueue(1000);
    for (int i = 0; i < ALF_SIZE; i++)
    {
        if (freq_table[i] > 0)
        {
            tree_t *node = newNode(NULL, NULL, freq_table[i], (unsigned char)i);
            enqueue(queue, node);
        }
    }

    while (queue->size > 1)
    {
        tree_t *l = dequeue(queue);
        tree_t *r = dequeue(queue);
        tree_t *parent = newNode(l, r, l->weight + r->weight, 0);
        enqueue(queue, parent);
    }

    tree_t *root = dequeue(queue);
    freeQueue(queue);
    return root;
}

static void genCodRec(tree_t *node, huffman_code_t *table, uint64_t code, uint8_t deep)
{
    if (!node)
        return;

    if (!node->leftNode && !node->rightNode)
    {
        table[node->symbol].bits = code;
        table[node->symbol].length = deep;
        return;
    }

    genCodRec(node->leftNode, table, (code << 1), deep + 1);
    genCodRec(node->rightNode, table, (code << 1) | 1, deep + 1);
}

void genCodes(tree_t *root, huffman_code_t *table)
{
    for (int i = 0; i < ALF_SIZE; i++)
    {
        table[i].length = 0;
    }
    genCodRec(root, table, 0, 0);
}

void serTree(tree_t *root, file_writer *writer)
{
    if (root == NULL) return;

    if (!root->leftNode && !root->rightNode)
    {
        writer->write_bit(writer, 1);
        writer->write_byte(writer, root->symbol);
    }
    else
    {
        serTree(root->leftNode, writer);
        serTree(root->rightNode, writer);
        writer->write_bit(writer, 0);
    }
}

static tree_t *deserTreeRec(file_reader *reader)
{
    uint8_t bit = reader->read_bit(reader);

    if (bit == 1)
    {
        uint8_t symbol = reader->read_byte(reader);
        return newNode(NULL, NULL, 0, symbol);
    }
    else
    {
        tree_t *l = deserTreeRec(reader);
        tree_t *r = deserTreeRec(reader);
        return newNode(l, r, 0, 0);
    }
}

tree_t *deserTree(file_reader *reader)
{
    return deserTreeRec(reader);
}