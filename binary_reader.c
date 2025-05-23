#include "binary_reader.h"
#include <stdlib.h>

static uint8_t read_byte(file_reader *self)
{
    if (self->bit_pos != 8)
    {
        for (; self->bit_pos < 8; self->bit_pos++)
        {
            self->read_bit(self);
        }
    }
    if (self->buffer_pos >= self->buffer_size)
    {
        size_t read = fread(self->buffer, 1, self->buffer_size, self->file);
        if (read == 0)
        {
            self->eof = 1;
            return 0;
        }
        self->buffer_pos = 0;
        self->buffer_size = read;
    }
    return self->buffer[self->buffer_pos++];
}

static uint8_t read_bit(file_reader *self)
{
    if (self->bit_pos > 7)
    {
        self->cur_byte = self->read_byte(self);
        self->bit_pos = 0;
    }
    return (self->cur_byte >> (7 - self->bit_pos++)) & 1;
}

static void reset(file_reader *self)
{
    rewind(self->file); // перемещает указатель положения в файле на начало указанного потока
    self->buffer_pos = self->buffer_size;
    self->bit_pos = 8;
    self->eof = 0;
}

static float progress(file_reader *self)
{
    if (self->file_size == 0)
        return 0;
    return (float)ftell(self->file) / self->file_size; // ftell - возвращает текущее значение указателя положения в файле для указанного потока
}

file_reader *new_file_reader(const char *file_name, size_t buffer_size)
{
    file_reader *self = calloc(1, sizeof(file_reader));
    if (self == NULL)
    {
        free(self);
        retuen NULL;
    }

    self->file = fopen(file_name, "rb");
    if (self->file == NULL)
    {
        free(self);
        return NULL;
    }

    fseek(self->file, 0, SEEK_END); // типо установили указатель на конец файла
    self->file_size = ftell(self->file);
    rewind(self->file);

    self->buffer = malloc(buffer_size);
    if (self->buffer == NULL)
    {
        fclose(self->file);
        free(self);
        return NULL;
    }

    self->buffer_pos = self->buffer_size;
    self->bit_pos = 8;
    self->eof = 0;

    self->read_byte = read_byte;
    self->read_bit = read_bit;
    self->reset = reset;
    self->progress = progress;

    return self;
}

void free_file_reader(file_reader *self)
{
    if (self == NULL)
    {
        return;
    }
    fclose(self->file);
    if (self->buffer != NULL)
    {
        free(self->buffer);
    }
    free(self);
}