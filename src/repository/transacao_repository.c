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

bool is_limite_excedido(Cliente *cliente, Transacao *transacao) {
    if (transacao->tipo[0] == 'c') {
        return false;
    } else if ((cliente->saldo - transacao->valor) < -cliente->limite) {
        return true;
    }
    return false;
}

TransacaoList transacao_repo_find_last_10(uint32_t id_cliente) {
    TransacaoList transacoes = { 0, NULL };
    char file_name[MAX_FILE_NAME];

    sprintf(file_name, TRANSACAO_FILE_NAME_TEMPLATE, id_cliente);

    File *file = fs_get_file(file_name);

    fs_return_on_open_fail(file, file_name, transacoes);

    size_t size = fs_seek_end(file);
    long start = size - sizeof(Transacao) * LAST_10;
    start = start < 0 ? 0 : start;

    int actual_count = (size - start) / sizeof(Transacao);
    transacoes.size = actual_count;

    if (actual_count >  0) {
        transacoes.values = malloc(sizeof(Transacao) * actual_count);
        transacoes.size = actual_count;

        fs_seek_set(file, start);
        size_t n_read = fs_read(file, transacoes.values, sizeof(Transacao) * actual_count);

        transacao_repo_clear_if_none(n_read, &transacoes);
        if (n_read > 0) {
            transacao_repo_reverse(transacoes);
        }
    }
    return transacoes;
}

TransacaoList transacao_repo_find_all(uint32_t id_cliente) {
    TransacaoList transacoes = { 0, NULL };
    char file_name[MAX_FILE_NAME];

    sprintf(file_name, TRANSACAO_FILE_NAME_TEMPLATE, id_cliente);

    File *file = fs_get_file(file_name);

    fs_return_on_open_fail(file, file_name, transacoes);
    
    size_t size = fs_seek_end(file);

    int actual_count = size / sizeof(Transacao);

    if (actual_count >  0) {
        transacoes.values = malloc(sizeof(Transacao) * actual_count);
        transacoes.size = actual_count;

        fs_seek_set(file, 0);
        size_t n_read = fs_read(file, transacoes.values, sizeof(Transacao) * actual_count);
        transacao_repo_clear_if_none(n_read, &transacoes);
    }
    return transacoes;
}

Cliente *transacao_repo_insert(Transacao *transacao) {
    char file_name[MAX_FILE_NAME];
    sprintf(file_name, TRANSACAO_FILE_NAME_TEMPLATE, transacao->id_cliente);

    File *file = fs_get_file(file_name);

    fs_return_on_open_fail(file, file_name, false);
    
    fs_lock(file);

    Cliente *cliente = cliente_repo_find_one(transacao->id_cliente);

    if (!is_limite_excedido(cliente, transacao)) {
        if (transacao->tipo[0] == 'c') {
            cliente->saldo += transacao->valor;
        } else {
            cliente->saldo -= transacao->valor;
        }

        cliente_repo_update(cliente);    

        fs_write(file, transacao, sizeof(Transacao));
        fs_flush(file);
    } else {
        free(cliente);
        cliente = NULL;
    }

    fs_unlock(file);

    return cliente;
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

        File *file = fs_mfile_new(file_name, FS_CREATE_RW_APPEND, TRANSACAO_MAX_PAGES * getpagesize());
        if (file == NULL) {
            log_fatal("Falha ao iniciar o repositório de transações");
            exit(EXIT_FAILURE);
        }
        fs_pool_add(file);
    }
}

void transacao_repo_shared_mem_init() {
    char file_name[MAX_FILE_NAME];

    for (int i = 1; i <= 5; i++) {
        sprintf(file_name, TRANSACAO_FILE_NAME_TEMPLATE, i);

        File *file = fs_get_file(file_name);
        fs_shared_mem_init(file, TRANSACAO_MAX_PAGES * getpagesize());
    }
}