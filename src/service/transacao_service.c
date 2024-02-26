
#include "transacao_service.h"
#include "../repository/transacao_repository.h"
#include "../entity/cliente.h"
#include "../util/uuid_util.h"
#include "../util/string_util.h"
#include <stdlib.h>
#include <string.h>

TransacaoList transacao_service_find_last_10(uint32_t cliente_id) {
    return transacao_repo_find_last_10(cliente_id);
}

TransacaoList transacao_service_find_all(uint32_t id_cliente) {
    return transacao_repo_find_all(id_cliente);
}

bool transacao_service_save(Transacao *transacao, Cliente *cliente) {
    return transacao_repo_insert(transacao, cliente);
}

void transacao_service_free_list(TransacaoList transacoes) {
    transacao_repo_free_list(transacoes);
}

void transacao_service_init() {
    transacao_repo_init();
}