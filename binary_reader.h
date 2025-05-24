#ifndef BINARY_READER_H
#define BINARY_READER_H

#include <stdio.h>
#include <stdint.h>

typedef struct BinaryReader
{
    FILE *file;
    size_t file_size;

    uint8_t *buffer;
    size_t buffer_size;
    size_t buffer_pos;

    uint8_t cur_byte;
    uint8_t bit_pos;
    int eof;

    uint8_t (*read_byte)(struct BinaryReader *self);
    uint8_t (*read_bit)(struct BinaryReader *self);
    void (*reset)(struct BinaryReader *self);
    float (*progress)(struct BinaryReader *self);
} file_reader;

file_reader *newFileReader(const char *filename, size_t buffer_size);
void freeFileReader(file_reader *self);

#endif