#define _GNU_SOURCE

#include "cliente_repository.h"
#include "../util/fs_connection.h"
#include "../util/string_util.h"
#include "../util/log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

Cliente *cliente_repo_find_one(uint32_t id) {
    char file_name[MAX_FILE_NAME];
    sprintf(file_name, CLIENTE_FILE_NAME_TEMPLATE, id);

    // FILE *file = fopen(file_name, FS_READ);
    File *file = fs_get_file(file_name);
    fs_return_on_open_fail(file, file_name, NULL);

    fs_return_on_open_fail(file->file, file_name, NULL);

    Cliente *cliente = malloc(sizeof(Cliente));
    fseek(file->file, 0, SEEK_SET);
    size_t n_read = fread(cliente, sizeof(Cliente), 1, file->file);
    // fclose(file);

    if (n_read <= 0) {
        free(cliente);
        return NULL;
    }

    return cliente;
}

void cliente_repo_update(Cliente *cliente) {
    char file_name[MAX_FILE_NAME];

    sprintf(file_name, CLIENTE_FILE_NAME_TEMPLATE, cliente->id);

    // FILE *file = fopen(file_name, FS_READ_WRITE);
    FILE *file = fs_get_file(file_name)->file;

    fs_return_void_on_open_fail(file, file_name);

    fseek(file, 0, SEEK_SET);
    fwrite(cliente, sizeof(Cliente), 1, file);
    fflush(file);
    // fclose(file);
}

void cliente_repo_init() {
    char file_name[MAX_FILE_NAME];

    for (int i = 1; i <= 5; i++) {
        sprintf(file_name, CLIENTE_FILE_NAME_TEMPLATE, i);

        // FILE *file = fopen(file_name, FS_READ);
        // if (file == NULL) {
        //     file = fopen(file_name, FS_CREATE_READ_WRITE);
        // }
        File *file = fs_file_new(file_name, FS_CREATE_READ_WRITE);
        fs_pool_add(file);
        // fclose(file);
    }
}