CC = gcc
CFLAGS = -Wall -Wextra #-g

LDFLAGS = 

TARGET = huffman

SOURCES = main.c parser.c huffman_alg.c huffman_builder.c huffman_tree.c \
          binary_reader.c binary_writer.c

OBJECTS = $(SOURCES:.c=.o)

HEADERS = parser.h huffman_alg.h huffman_builder.h huffman_tree.h binary_reader.h binary_writer.h

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)