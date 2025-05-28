#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include <stddef.h>
#include <stdio.h>

typedef struct Huf_tree{
    struct Huf_tree* leftNode;
    struct Huf_tree* rightNode;
    size_t weight;
    unsigned char symbol; // только для листьев будет
} tree_t;

tree_t* newNode(tree_t* left, tree_t* right, size_t weight, unsigned char symbol);

void print_tree(tree_t* root);

void freeTree(tree_t* tree);


typedef struct PriorityQueue{
    tree_t** data;
    size_t size;
    size_t capacity;
} queue_t;

queue_t* createQueue(size_t capacity);

void enqueue(queue_t* queue, tree_t* node);

tree_t* dequeue(queue_t* queue);

void freeQueue(queue_t* queue);

#endif