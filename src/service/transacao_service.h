#include "../entity/transacao.h"
#include "../entity/cliente.h"
#include <stdbool.h>
#include <inttypes.h>

extern TransacaoList transacao_service_find_last_10(uint32_t cliente_id);
extern TransacaoList transacao_service_find_all(uint32_t id_cliente);
extern bool transacao_service_save(Transacao *transacao, Cliente *cliente);
extern void transacao_service_free_list(TransacaoList transacoes);
extern void transacao_service_init();