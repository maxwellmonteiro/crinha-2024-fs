#include "array_list.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>

static void push(IntArrayList *self, int value);
static int pop(IntArrayList *self);
static void destroy(IntArrayList *self);
static bool is_full(IntArrayList *self);

IntArrayList *int_array_list_new(size_t size) {
    IntArrayList *self = malloc(sizeof(IntArrayList));
    self->values = malloc(sizeof(int) * size);
    self->size = 0;
    self->push = push;
    self->pop = pop;
    self->is_full = is_full;
    self->destroy = destroy;
    return self;
}

void push(IntArrayList *self, int value) {
    if (self->size >= self->max_size) {
        log_fatal("Tamanho lista excedido");
        exit(EXIT_FAILURE);
    }
    self->values[self->size] = value;
    self->size++;
}

int pop(IntArrayList *self) {
    int val = self->values[0];
    self->size--;
    memcpy(&self->values[0], &self->values[1], sizeof(IntArrayList) * self->size);
    return val;
}

void destroy(IntArrayList *self) {
    free(self->values);
    self->values = NULL;
    free(self);
}

bool is_full(IntArrayList *self) {
    return self->size >= self->max_size;
}