#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "huffman_alg.h"
#include "binary_writer.h"
#include "binary_reader.h"
#include "huffman_builder.h"
#include "huffman_tree.h"

void compress(const char **in_filenames, const char *arch_name, int count)
{
    file_writer *writer = newFileWriter(arch_name, 4096);
    if (!writer)
    {
        fprintf(stderr, "Error: could not create archive '%s'. Sorry, my mistake.\n", arch_name);
        return;
    }

    //записываем количество файлов
    writer->write_byte(writer, (uint8_t)count);

    for (int i = 0; i < count; i++)
    {
        const char *path = in_filenames[i];

        const char *filename = strrchr(path, '/');
        filename = filename ? filename + 1 : path;

    

        //записывем длину имени файла
        size_t name_len = strlen(filename);
        writer->write_byte(writer, (uint8_t)name_len);

        //записываем имя файла
        for (size_t j = 0; j < name_len; j++)
        {
            writer->write_byte(writer, (uint8_t)filename[j]);
        }

        //записываем размер файла
        struct stat file_size;
        stat(path, &file_size);
        // printf("file size: %lu\n", file_size.st_size);
        for (int j = 7; j >= 0; j--)
        {
            writer->write_byte(writer, (uint8_t)((file_size.st_size >> (j * 8)) & 0xFF));
        }
    }

    for (int i = 0; i < count; i++)
    {
        const char *path = in_filenames[i];

        const char *filename = strrchr(path, '/');
        filename = filename ? filename + 1 : path;

        printf("Compressing %s...\n", filename);

        size_t freq_table[ALF_SIZE] = {0};
        buildFreqTable(path, freq_table);

        // printf("freq table:\n");
        // for (int i = 0; i < ALF_SIZE; i++) {
        //     if (freq_table[i] > 0) {
        //         printf("'%c' (%d): %zu\n", (char)i, i, freq_table[i]);
        //     }
        // }

        tree_t *tree = buildHuffmanTree(freq_table);
        // print_tree(tree);
        huffman_code_t codes[ALF_SIZE];
        genCodes(tree, codes);

        // printf("codes:\n");
        // for (int i = 0; i < ALF_SIZE; i++) {
        //     if (codes[i].length > 0) {
        //         printf("'%c' (%d): ", (char)i, i);
        //         for (int b = codes[i].length - 1; b >= 0; b--) {
        //             printf("%ld", (codes[i].bits >> b) & 1);
        //         }
        //         printf(" (length=%d)\n", codes[i].length);
        //     }
        // }

        serTree(tree, writer);
        // printf("Serialized Huffman Tree structure:\n");
        // debug_serialize_tree(tree);
        
        file_reader *reader = newFileReader(path, 4096);
        if (!reader)
        {
            fprintf(stderr, "Error: cannot open file '%s' for reading\n", filename);
            freeTree(tree);
            continue;
        }

        size_t c = 0;
        while (!reader->eof)
        {
            uint8_t byte = reader->read_byte(reader);
            if (!reader->eof)
            {
                //printf("bits: ");
                for (int b = codes[byte].length - 1; b >= 0; b--)
                {
                    uint8_t bit = (codes[byte].bits >> b) & 1;
                    //printf("%d", bit);
                    writer->write_bit(writer, bit);
                }
                //printf("\n");
                c++;
                if (c % 1000 == 0)
                {
                    float p = reader->progress(reader);
                    printf("\rProgress: %.0f%%", p * 100);
                    fflush(stdout);
                }
            }
        }

        printf("\rProgress: 100%%\n");

        freeFileReader(reader);
        freeTree(tree);
    }

    writer->reset(writer);
    freeFileWriter(writer);
    printf("Compression complete\n");
}

void decompress(const char *arch_name)
{
    file_reader *reader = newFileReader(arch_name, 4096);
    if (!reader)
    {
        fprintf(stderr, "Error: could not open archive file '%s'. Are you sure it's there?\n", arch_name);
        return;
    }

    if (reader->eof)
    {
        fprintf(stderr, "Error: archive is empty\n");
        freeFileReader(reader);
        return;
    }

    uint8_t count = reader->read_byte(reader);
    // printf("count files: %d\n", count);

    file_meta *files = malloc(sizeof(file_meta) * count);
    if (!files)
    {
        fprintf(stderr, "Error: out of memory\n");
        freeFileReader(reader);
        return;
    }

    for (int i = 0; i < count; i++)
    {
        if (reader->eof)
        {
            fprintf(stderr, "Error: something's wrong with the archive\n");
            free(files);
            freeFileReader(reader);
            return;
        }

        uint8_t name_len = reader->read_byte(reader);
        // printf("len_name: %d\n", name_len);

        for (int j = 0; j < name_len; j++)
        {
            if (reader->eof)
            {
                fprintf(stderr, "Error: something's wrong with the archive\n");
                free(files);
                freeFileReader(reader);
                return;
            }
            files[i].name[j] = (char)reader->read_byte(reader);
            // printf(" %c", files[i].name[j]);
        }
        files[i].name[name_len] = '\0';
        // printf("\n");

        size_t size = 0;
        for (int j = 0; j < 8; j++)
        {
            if (reader->eof)
            {
                fprintf(stderr, "Error: something's wrong with the archive\n");
                free(files);
                freeFileReader(reader);
                return;
            }
            size = (size << 8) | reader->read_byte(reader);
        }
        files[i].size = size;
        // printf("size: %ld\n", size);
    }

    for (int i = 0; i < count; i++)
    {
        tree_t *tree = deserTree(reader);
        //print_tree(tree);

        if (!tree)
        {
            fprintf(stderr, "Error: can not deserialize tree for file '%s'\n", files[i].name);
            continue;
        }

        file_writer *writer = newFileWriter(files[i].name, 4096);
        if (!writer) {
            fprintf(stderr, "Error: can not create file '%s'\n", files[i].name);
            freeTree(tree);
            continue;
        }

        printf("Decompressing %s...\n", files[i].name);
        tree_t *curr = tree;
        size_t decoded = 0;

        // printf("bits: ");

        while (decoded < files[i].size && !reader->eof)
        {
            uint8_t bit = reader->read_bit(reader);
            // printf("%d", bit);
            curr = bit ? curr->rightNode : curr->leftNode;
            
            if (!curr->leftNode && !curr->rightNode)
            {
                writer->write_byte(writer, curr->symbol);
                curr = tree;
                decoded++;
            }
        
        }

        writer->reset(writer);
        freeFileWriter(writer);
        freeTree(tree);
        printf("Finished %s (%zu bytes)\n", files[i].name, files[i].size);
    }

    free(files);
    freeFileReader(reader);
}