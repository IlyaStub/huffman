#include "binary_writer.h"
#include <stdlib.h>

static void write_byte(file_writer *self, uint8_t byte)
{
    if (self->bit_pos != 0)
    {
        reset(self);
    }

    if (self->buffer_pos >= self->buffer_size)
    {
        fwrite(self->buffer, 1, self->buffer_pos, self->file);
        self->buffer_pos = 0;
    }
    self->buffer[self->buffer_pos++] = byte;
}

static void write_bit(file_writer *self, uint8_t bit)
{
    bit = bit & 1;
    self->cur_byte = (self->cur_byte << 1) | bit;
    self->bit_pos++;

    if (self->bit_pos == 8) {
        self->write_byte(self, self->cur_byte);
        self->bit_pos = 0;
        self->cur_byte = 0;
    }
}

static void reset(file_writer *self)
{
    if (self->bit_pos > 0) {
        self->cur_byte <<= (8 - self->bit_pos);
        self->write_byte(self, self->cur_byte);
        self->bit_pos = 0;
        self->cur_byte = 0;
    }

    if (self->buffer_pos > 0) {
        fwrite(self->buffer, 1, self->buffer_pos, self->file);
        self->buffer_pos = 0;
    }
}
file_writer* new_file_writer(const char *filename, size_t buffer_size)
{
    file_writer *self = calloc(1, sizeof(file_writer));
    if (self == NULL) {
        return NULL;
    }

    self->file = fopen(filename, "wb");
    if (self->file == NULL) {
        free(self);
        return NULL;
    }

    self->buffer = malloc(buffer_size);
    if (self->buffer == NULL) {
        fclose(self->file);
        free(self);
        return NULL;
    }

    self->buffer_size = buffer_size;
    self->buffer_pos = 0;
    self->cur_byte = 0;
    self->bit_pos = 0;

    self->write_byte = write_byte;
    self->write_bit = write_bit;
    self->reset = reset;

    return self;
}

void free_file_writer(file_writer *self)
{
    if (self == NULL) return;

    self->reset(self);
    fclose(self->file);

    if (self->buffer != NULL) {
        free(self->buffer);
    }

    free(self);
}