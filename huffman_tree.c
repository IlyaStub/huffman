#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "huffman_tree.h"

// tree
tree_t *newNode(tree_t *left, tree_t *right, size_t weight, unsigned char symbol)
{
    tree_t *node = (tree_t *)malloc(sizeof(tree_t));
    node->leftNode = left;
    node->rightNode = right;
    node->weight = weight;
    node->symbol = symbol;
    return node;
}

static void print_huffman_tree(tree_t* tree, int level) {
    if (tree == NULL) {
        return;
    }

    for (int i = 0; i < level; i++) {
        printf("  ");
    }
    
    if (tree->leftNode == NULL && tree->rightNode == NULL) {
        printf("'%c' (weight: %zu)\n", tree->symbol, tree->weight);
    } else {
        printf("Node (weight: %zu)\n", tree->weight);
    }

    print_huffman_tree(tree->leftNode, level + 1);
    print_huffman_tree(tree->rightNode, level + 1);
}

void print_tree(tree_t* root) {
    printf("Huffman Tree:\n");
    print_huffman_tree(root, 0);
}

void freeTree(tree_t *tree)
{
    if (tree == NULL)
        return;
    freeTree(tree->leftNode);
    freeTree(tree->rightNode);
    free(tree);
}

// PRIORITY QUEUE
static void swap(tree_t **first, tree_t **second)
{
    tree_t *temp = *first;
    *first = *second;
    *second = temp;
}

static void sift_up(queue_t *queue, int ind)
{
    while (ind > 0 && queue->data[ind]->weight < queue->data[(ind - 1) / 2]->weight)
    {
        swap(&queue->data[ind], &queue->data[(ind - 1) / 2]);
        ind = (ind - 1) / 2;
    }
}

static void sift_down(queue_t *queue, size_t ind)
{
    size_t left = 2 * ind + 1;
    size_t right = 2 * ind + 2;
    size_t min_el = ind;
    if (left < queue->size && queue->data[left]->weight < queue->data[min_el]->weight)
    {
        min_el = left;
    }
    if (right < queue->size && queue->data[right]->weight < queue->data[min_el]->weight)
    {
        min_el = right;
    }
    if (min_el != ind)
    {
        swap(&queue->data[ind], &queue->data[min_el]);
        sift_down(queue, min_el);
    }
}

queue_t *createQueue(size_t capacity)
{
    queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
    queue->capacity = capacity;
    queue->size = 0;
    queue->data = malloc(sizeof(tree_t *) * capacity);
    return queue;
}

void enqueue(queue_t* queue, tree_t* node){
    if (queue->size >= queue->capacity){
        size_t new_capacity = queue->capacity * 2;
        tree_t** new_data = realloc(queue->data, sizeof(tree_t*) * new_capacity);

        queue->data = new_data;
        queue->capacity = new_capacity;
    }
    queue->data[queue->size] = node;
    queue->size++;
    sift_up(queue, queue->size-1);
}

tree_t *dequeue(queue_t *queue)
{
    if (queue->size == 0) return NULL;

    tree_t *min = queue->data[0];
    queue->size -= 1;

    if (queue->size > 0)
    {
        queue->data[0] = queue->data[queue->size];
        sift_down(queue, 0);
    }

    return min;
}

void freeQueue(queue_t* queue){
    free(queue->data);
    free(queue);
}