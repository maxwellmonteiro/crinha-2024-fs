#include "fs_connection.h"
#include "../util/log.h"
#include "../util/env_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

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

static bool fs_was_reallocated(File *file) {
    return file->mfile->max_size > file->max_size;
}

static void fs_reallocate(File *file) {
    posix_fallocate(file->fd, 0, file->mfile->max_size);
}

static void fs_map_file_to_memory(File *file, size_t size) {
    posix_fallocate(file->fd, 0, size);
    file->mfile = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, file->fd, 0);

    if (file->mfile == MAP_FAILED) {
        log_fatal("Falha ao alocar memória compartilhada (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    file->file = ((char *) file->mfile) + sizeof(MFile);
    file->cursor = 0;
}

File *fs_file_new(char *name, int flags) {
    int fd = open(name, flags);
    if (fd <= 0) { perror("erro"); return NULL; }

    if (is_main_process()) {
        fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    }

    File *file = malloc(sizeof(File));
    file->name = malloc(sizeof(char) * strlen(name) + 1);
    strcpy(file->name, name);
    file->fd = fd;
    file->flags = flags;

    return file;
}

char *get_sem_name(char *name) {
    int idx_last_slash = 0;
    int len = strlen(name);
    for (int i = 0; i < len; i++) {
        if (name[i] == '/') {
            idx_last_slash = i;
        }
    }
    return &name[idx_last_slash];
}

File *fs_mfile_new(char *name, int flags, size_t size) {
    File *file = fs_file_new(name, flags);

    if (file != NULL) {         
        fs_map_file_to_memory(file, size); 
    }
    return file;
}

void fs_shared_mem_init(File *file, size_t size) {
    if (file != NULL && is_main_process()) {        
        file->mfile->max_size = size - sizeof(MFile);
        file->mfile->size = 0;
        file->mutex = sem_open(get_sem_name(file->name), O_CREAT, 0644, 1);
        if (file->mutex == NULL) {
            log_fatal("Falha ao obter semaforo (%s)", strerror(errno));
            exit(EXIT_FAILURE);
        }
        // Containers sao isolados, logo nao enxergam o semaforo criado pelo outro container, por isso essa copia do semaforo para a memoria compartilhada
        // Em um ambiante não container, não precisaria disso pois o semaforo é a nível de Sistema Operacional
        memcpy(&file->mfile->mutex_copy, file->mutex, sizeof(sem_t));
    }
}

void fs_file_destroy(File *file) {
    if (file->mfile != NULL) {
        munmap(file->mfile, sizeof(MFile) + file->mfile->max_size);
    }
    if (is_main_process() && file->mutex != NULL) {
        sem_close(file->mutex);
        sem_unlink(get_sem_name(file->name));
    }
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
    free(pool->values);
    free(pool);
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

size_t fs_read(File *file, void *buffer, size_t size) {
    if (file->cursor >= file->mfile->size) return 0;

    memcpy(buffer, file->file + file->cursor, size);
    file->cursor += size;
    return size;
}

size_t fs_write(File *file, void *buffer, size_t size) {
    if (file->flags & O_APPEND) {
        memcpy(file->file + file->mfile->size, buffer, size);
        file->mfile->size += size;
    } else {
        memcpy(file->file + file->cursor, buffer, size);
        if (file->cursor + size > file->mfile->size) {
            file->mfile->size = file->cursor + size;
        }
    }
    file->cursor += size;
    return size;
}

size_t fs_seek_set(File *file, size_t offset) {
    file->cursor = offset;
    return file->cursor;
}

size_t fs_seek_end(File *file) {
    file->cursor = file->mfile->size;
    return file->cursor;
}

int fs_flush(File *file) {
    // fsync(file->fd);
    return 0;
}

void fs_lock(File *file) {
    if (sem_wait(&file->mfile->mutex_copy) < 0) {
        log_error("erro semaforo %s (%s)", file->name, strerror(errno));
    }
}

void fs_unlock(File *file) {
    if (sem_post(&file->mfile->mutex_copy) < 0) {
        log_error("erro semaforo %s (%s)", file->name, strerror(errno));
    }
}