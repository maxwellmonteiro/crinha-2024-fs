
#include "transacao_service.h"
#include "../repository/transacao_repository.h"
#include "../entity/cliente.h"
#include "../util/uuid_util.h"
#include "../util/string_util.h"
#include "../util/env_util.h"
#include "../util/log.h"
#include <stdlib.h>
#include <string.h>

TransacaoList transacao_service_find_last_10(uint32_t cliente_id) {
    return transacao_repo_find_last_10(cliente_id);
}

TransacaoList transacao_service_find_all(uint32_t id_cliente) {
    return transacao_repo_find_all(id_cliente);
}

Cliente *transacao_service_save(Transacao *transacao) {
    return transacao_repo_insert(transacao);
}

void transacao_service_free_list(TransacaoList transacoes) {
    transacao_repo_free_list(transacoes);
}

void transacao_service_init() {
    transacao_repo_init();
}

void transacao_service_shared_mem_init() {
    if (is_main_process()) {
        transacao_repo_shared_mem_init();
    }
}