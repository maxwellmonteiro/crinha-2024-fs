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

    File *file = fs_get_file(file_name);
    fs_return_on_open_fail(file, file_name, NULL);

    Cliente *cliente = malloc(sizeof(Cliente));
    fs_seek_set(file, 0);
    size_t n_read = fs_read(file, cliente, sizeof(Cliente));

    if (n_read <= 0) {
        free(cliente);
        return NULL;
    }

    return cliente;
}

void cliente_repo_update(Cliente *cliente) {
    char file_name[MAX_FILE_NAME];

    sprintf(file_name, CLIENTE_FILE_NAME_TEMPLATE, cliente->id);

    File *file = fs_get_file(file_name);

    fs_return_void_on_open_fail(file, file_name);

    fs_seek_set(file, 0);
    fs_write(file, cliente, sizeof(Cliente));
    fs_flush(file);

}

void cliente_repo_init() {
    char file_name[MAX_FILE_NAME];

    for (int i = 1; i <= 5; i++) {
        sprintf(file_name, CLIENTE_FILE_NAME_TEMPLATE, i);

        File *file = fs_mfile_new(file_name, FS_CREATE_RW, getpagesize());
        if (file == NULL) {
            log_fatal("Falha ao iniciar o repositório de clientes");
            exit(EXIT_FAILURE);
        }
        fs_pool_add(file);
    }
}

void cliente_repo_shared_mem_init() {
    char file_name[MAX_FILE_NAME];

    for (int i = 1; i <= 5; i++) {
        sprintf(file_name, CLIENTE_FILE_NAME_TEMPLATE, i);

        File *file = fs_get_file(file_name);
        fs_shared_mem_init(file, getpagesize());
    }
}