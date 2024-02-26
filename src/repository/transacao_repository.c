#define _GNU_SOURCE

#include "transacao_repository.h"
#include "cliente_repository.h"
#include "../util/fs_connection.h"
#include "../util/string_util.h"
#include "../util/log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#define LAST_10 10

void transacao_repo_reverse(TransacaoList transacoes) {
    Transacao tmp;
    for (int i = 0; i < transacoes.size / 2; i++) {
        tmp = transacoes.values[i];
        transacoes.values[i] = transacoes.values[transacoes.size - 1 - i];
        transacoes.values[transacoes.size - 1 - i] = tmp;
    }
}

void transacao_repo_clear_if_none(size_t n_read, TransacaoList *transacoes) {
    if (n_read <= 0) {
        transacoes->size = 0;
        if (transacoes->values != NULL) {
            free(transacoes->values);
            transacoes->values = NULL;
        }
    }
}

TransacaoList transacao_repo_find_last_10(uint32_t id_cliente) {
    TransacaoList transacoes = { 0, NULL };
    char file_name[MAX_FILE_NAME];

    sprintf(file_name, TRANSACAO_FILE_NAME_TEMPLATE, id_cliente);

    // FILE *file = fopen(file_name, FS_READ);
    File *file = fs_get_file(file_name);

    fs_return_on_open_fail(file, file_name, transacoes);

    fseek(file->file, 0, SEEK_END);
    size_t size = ftell(file->file);
    long start = size - sizeof(Transacao) * LAST_10;
    start = start < 0 ? 0 : start;

    int actual_count = (size - start) / sizeof(Transacao);
    transacoes.size = actual_count;

    if (actual_count >  0) {
        transacoes.values = malloc(sizeof(Transacao) * actual_count);
        transacoes.size = actual_count;

        fseek(file->file, start, SEEK_SET);
        size_t n_read = fread(transacoes.values, sizeof(Transacao), actual_count, file->file);

        transacao_repo_clear_if_none(n_read, &transacoes);
        if (n_read > 0) {
            transacao_repo_reverse(transacoes);
        }
    }
    // fclose(file);
    return transacoes;
}

TransacaoList transacao_repo_find_all(uint32_t id_cliente) {
    TransacaoList transacoes = { 0, NULL };
    char file_name[MAX_FILE_NAME];

    sprintf(file_name, TRANSACAO_FILE_NAME_TEMPLATE, id_cliente);

    // FILE *file = fopen(file_name, FS_READ);
    FILE *file = fs_get_file(file_name)->file;

    fs_return_on_open_fail(file, file_name, transacoes);

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);    

    int actual_count = size / sizeof(Transacao);

    if (actual_count >  0) {
        transacoes.values = malloc(sizeof(Transacao) * actual_count);
        transacoes.size = actual_count;

        fseek(file, 0, SEEK_SET);
        size_t n_read = fread(transacoes.values, sizeof(Transacao), actual_count, file);
        transacao_repo_clear_if_none(n_read, &transacoes);
    }
    // fclose(file);
    return transacoes;
}

bool transacao_repo_insert(Transacao *transacao, Cliente *cliente) {
    char file_name[MAX_FILE_NAME];
    sprintf(file_name, TRANSACAO_FILE_NAME_TEMPLATE, cliente->id);

    if (transacao->tipo[0] == 'c') {
        cliente->saldo += transacao->valor;
    } else {
        cliente->saldo -= transacao->valor;
    }

    cliente_repo_update(cliente);

    // FILE *file = fopen(file_name, FS_APPEND);
    FILE *file = fs_get_file(file_name)->file;

    fs_return_on_open_fail(file, file_name, false);

    fwrite(transacao, sizeof(Transacao), 1, file);
    fflush(file);
    // fclose(file);

    return true;
}

void transacao_repo_free_list(TransacaoList transacoes) {
    if (transacoes.values != NULL) {
        free(transacoes.values);
    }
}

void transacao_repo_init() {
    char file_name[MAX_FILE_NAME];

    for (int i = 1; i <= 5; i++) {
        sprintf(file_name, TRANSACAO_FILE_NAME_TEMPLATE, i);

        // FILE *file = fopen(file_name, FS_READ);
        // if (file == NULL) {
        //     file = fopen(file_name, FS_CREATE_READ_WRITE);
        // }
        // fclose(file);
        File *file = fs_file_new(file_name, FS_READ_APPEND);
        fs_pool_add(file);
    }
}