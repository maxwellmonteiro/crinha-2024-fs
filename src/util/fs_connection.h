#include <stdio.h>
#include <stdbool.h>

#ifndef FS_CONNECTION_H
#define FS_CONNECTION_H

#define FS_READ "r"
#define FS_APPEND "a"
#define FS_READ_APPEND "a+"
#define FS_READ_WRITE "r+"
#define FS_CREATE_READ_WRITE "w+"

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


typedef struct File {
    char *name;
    FILE *file;
    int fd;
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
extern File *fs_file_new(char *name, const char *mode);
extern void fs_file_destroy(File *file);
extern void fs_init_pool();
extern void fs_close_pool();
extern void fs_pool_add(File *file);
extern File *fs_get_file(char *name);

#endif
