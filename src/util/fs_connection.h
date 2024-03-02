 #define _GNU_SOURCE  

#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <semaphore.h>

#ifndef FS_CONNECTION_H
#define FS_CONNECTION_H

#define FS_CREATE_RW (O_CREAT | O_TRUNC | O_RDWR)
#define FS_CREATE_RW_APPEND (O_CREAT | O_TRUNC | O_RDWR | O_APPEND)

#define MAX_FILE_NAME 64

#define fs_return_on_open_fail(FD, FILE_NAME, RETURN) \
    if (FD == NULL) { \
        log_error("Falha ao abrir arquivo %s", FILE_NAME); \
        return RETURN; \
    }

#define fs_return_void_on_open_fail(FD, FILE_NAME) \
    if (FD == NULL) { \
        log_error("Falha ao abrir arquivo %s", FILE_NAME); \
        return; \
    }

typedef struct MFile {
    size_t size;
    size_t max_size;
    sem_t mutex_copy;
} MFile;

typedef struct File {
    char *name;
    void *file;
    MFile *mfile;
    int fd;
    int flags;
    size_t max_size;
    size_t cursor;
    sem_t *mutex;
} File;

typedef struct FileArrayList {
    int max_size;
    int size;
    File **values;
    void (*push) (struct FileArrayList *, File *);
    File *(*pop) (struct FileArrayList *);
    bool (*is_full) (struct FileArrayList *);
    void (*destroy) (struct FileArrayList *);
} FileArrayList;

extern FileArrayList *fs_array_list_new();
extern File *fs_file_new(char *name, int flags);
extern File *fs_mfile_new(char *name, int flags, size_t size);
extern void fs_file_destroy(File *file);
extern void fs_init_pool();
extern void fs_close_pool();
extern void fs_pool_add(File *file);
extern File *fs_get_file(char *name);

extern size_t fs_read(File *file, void *buffer, size_t size);
extern size_t fs_write(File *file, void *buffer, size_t size);
extern size_t fs_seek_set(File *file, size_t offset);
extern size_t fs_seek_end(File *file);
extern int fs_flush(File *file);
extern void fs_lock(File *file);
extern void fs_unlock(File *file);
extern void fs_shared_mem_init(File *file, size_t size);
extern void fs_pool_sync();

#endif
