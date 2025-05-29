#ifndef BINARY_WRITER_H
#define BINARY_WRITER_H

#include <stdio.h>
#include <stdint.h>

typedef struct BinaryWriter
{
    FILE *file;

    uint8_t *buffer;
    size_t buffer_size;
    size_t buffer_pos;

    uint8_t cur_byte;
    uint8_t bit_pos;
    
    void (*write_byte)(struct BinaryWriter *self, uint8_t byte);
    void (*write_bit)(struct BinaryWriter *self, uint8_t bit);
    void (*reset)(struct BinaryWriter *self);
} file_writer;

file_writer *newFileWriter(const char *filename, size_t buffer_size);
void freeFileWriter(file_writer *self);

#endif