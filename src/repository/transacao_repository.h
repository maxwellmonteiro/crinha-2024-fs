#include "../entity/transacao.h"
#include "../entity/cliente.h"
#include <stdbool.h>
#include <inttypes.h>

#define TRANSACAO_FILE_NAME_TEMPLATE "/tmp/data/TRANSACAO_%d.dat"

extern TransacaoList transacao_repo_find_last_10(uint32_t cliente_id);
extern TransacaoList transacao_repo_find_all(uint32_t id_cliente);
extern bool transacao_repo_insert(Transacao *transacao, Cliente *cliente);
extern void transacao_repo_free_list(TransacaoList transacoes);
extern void transacao_repo_init();