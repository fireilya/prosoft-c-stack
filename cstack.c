#include "cstack.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int index_t;
typedef int error_t;

struct {
    index_t* indexes;
    index_t write_ptr;
    index_t read_ptr;
    int count;
} static deleted_stacks_FIFO={NULL, 0, 0, 0};

struct stack_node {
    struct stack_node* prev;
    void* data;
    unsigned int data_size;
    unsigned int count;
};

struct {
    struct stack_node** data;
    unsigned int size;
    unsigned int count;
} static stacks_archive={NULL, 0, 0};

static error_t init_stack_archive(void) {
    stacks_archive.size = 4;
    stacks_archive.data = calloc(stacks_archive.size, sizeof(struct stack_node*));
    deleted_stacks_FIFO.indexes = calloc(stacks_archive.size, sizeof(index_t));
    if (!stacks_archive.data || !deleted_stacks_FIFO.indexes) return 1;
    return 0;
}

static error_t expand_stack_archive(void) {
    struct stack_node** new_archive = realloc(stacks_archive.data, stacks_archive.size * 2 * sizeof(struct stack_node*));
    if (!new_archive) return 1;
    index_t* new_indexes = realloc(deleted_stacks_FIFO.indexes, stacks_archive.size*2*sizeof(index_t));
    if (!new_indexes) { free(new_archive); return 1; }
    stacks_archive.size *= 2;
    stacks_archive.data = new_archive;
    deleted_stacks_FIFO.indexes = new_indexes;
    return 0;
}

static void push_null_index(hstack_t hstack) {
    deleted_stacks_FIFO.indexes[deleted_stacks_FIFO.write_ptr++] = (unsigned)hstack;
    deleted_stacks_FIFO.write_ptr %= stacks_archive.size;
    deleted_stacks_FIFO.count++;
}

static index_t pop_null_index(void) {
    if (!deleted_stacks_FIFO.count) return stacks_archive.count;
    index_t poped_index = deleted_stacks_FIFO.indexes[deleted_stacks_FIFO.read_ptr++];
    deleted_stacks_FIFO.read_ptr %= stacks_archive.size;
    deleted_stacks_FIFO.count--;
    return poped_index;
}

static hstack_t add_stack(struct stack_node* stack_pointer){
    if (!stacks_archive.data)
    {
        if (init_stack_archive()) { free(stack_pointer); return -1; }
    }

    if (stacks_archive.size==stacks_archive.count)
    {
        if (expand_stack_archive()) { free(stack_pointer); return -1; }
    }
    index_t place_index = pop_null_index();
    stacks_archive.data[place_index] = stack_pointer;
    stacks_archive.count++;
    return (int)place_index;
}

hstack_t stack_new(void)
{
    struct stack_node* stack_node = malloc(sizeof(struct stack_node));
    if (!stack_node) return -1;
    stack_node->data = NULL;
    stack_node->data_size = 0;
    stack_node->prev = NULL;
    stack_node->count = 0;
    return add_stack(stack_node);
}

void stack_free(const hstack_t hstack)
{
    if (hstack < 0 || (unsigned)hstack >= stacks_archive.size) return;
    struct stack_node* node = stacks_archive.data[hstack];
    while (node && node->count)
    {
        free(node->data);
        struct stack_node* prev_node = node->prev;
        free(node);
        node = prev_node;
    }
    push_null_index(hstack);
    stacks_archive.data[hstack] = NULL;
}

int stack_valid_handler(const hstack_t hstack) 
{
    return hstack >= 0 && (unsigned)hstack < stacks_archive.size && stacks_archive.data[hstack] ? 0 : 1;
}

unsigned int stack_size(const hstack_t hstack)
{
    if (hstack >= 0 && (unsigned)hstack < stacks_archive.size && stacks_archive.data[hstack])
    {
        return stacks_archive.data[hstack]->count;
    }
    return 0;
}

void stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (data_in && hstack >= 0 && (unsigned)hstack < stacks_archive.size && stacks_archive.data[hstack] && size > 0)
    {
        struct stack_node* node = stacks_archive.data[hstack]->count 
            ? malloc(sizeof(struct stack_node)) 
            : stacks_archive.data[hstack];
        node->count = stacks_archive.data[hstack]->count;
        if (node)
        {
            node->data = malloc(size);

            if (node->data)
            {
                node->prev = node->count ? stacks_archive.data[hstack] : NULL;
                node->data_size = size;
                node->count = node->count ? node->prev->count + 1 : 1;
                memcpy_s(node->data, node->data_size, data_in, size); 
                stacks_archive.data[hstack] = node;
                return;
            }
            free(node);
        }
    }
}

unsigned int stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (hstack < 0 || (unsigned)hstack >= stacks_archive.size || !data_out) return 0;
    if (!stacks_archive.data[hstack]) return 0;
    struct stack_node* node = stacks_archive.data[hstack];
    if ((unsigned int)node->data_size > size || !node->count) return 0;
    unsigned int value_to_return = node->data_size;
    memcpy_s(data_out, size, node->data, value_to_return);
    if (node->prev) { stacks_archive.data[hstack] = node->prev; }
    else { stacks_archive.data[hstack]->count -= 1; }
    free(node->data);
    if (node->count) free(node); 
    return value_to_return;
}

