#include "fs_connection.h"
#include "../util/log.h"
#include "../util/env_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#define MAX_POOL_SIZE 10

static FileArrayList *pool = NULL;

static void push(FileArrayList *self, File *value);
static File *pop(FileArrayList *self);
static void destroy(FileArrayList *self);
static bool is_full(FileArrayList *self);

FileArrayList *fs_array_list_new(size_t size) {
    FileArrayList *self = malloc(sizeof(FileArrayList));
    self->values = malloc(sizeof(File *) * size);
    self->size = 0;
    self->max_size = size;
    self->push = push;
    self->pop = pop;
    self->is_full = is_full;
    self->destroy = destroy;
    return self;
}

File *fs_file_new(char *name, int flags) {
    int fd = open(name, flags);
    if (fd <= 0) return NULL;
    File *file = malloc(sizeof(File));
    file->name = malloc(sizeof(char) * strlen(name) + 1);
    strcpy(file->name, name);
    file->fd = fd;

    return file;
}

void fs_file_destroy(File *file) {
    free(file->name);
    close(file->fd);
    free(file);
}

void push(FileArrayList *self, File *value) {
    if (self->size >= self->max_size) {
        log_fatal("Tamanho lista excedido");
        exit(EXIT_FAILURE);
    }
    self->values[self->size] = value;
    self->size++;
}

File *pop(FileArrayList *self) {
    File *val = self->values[self->size - 1];
    self->size--;
    return val;
}

void destroy(FileArrayList *self) {
    while(self->size > 0) {
        fs_file_destroy(self->pop(self));
    }
    free(self->values);
    self->values = NULL;
    free(self);
}

bool is_full(FileArrayList *self) {
    return self->size >= self->max_size;
}

void fs_init_pool() {
    pool = fs_array_list_new(MAX_POOL_SIZE);
}

void fs_close_pool() {
    while (pool->size > 0) {
        File *file = pool->pop(pool);
        fs_file_destroy(file);
    }
}

void fs_pool_add(File *file) {
    pool->push(pool, file);
}

File *fs_get_file(char *name) {
    for (int i = 0; i < pool->size; i++) {
        if (strcmp(pool->values[i]->name, name) == 0) {
            return pool->values[i];
        }
    }
    return NULL;
}