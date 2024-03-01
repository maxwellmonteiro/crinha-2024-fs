#include "../entity/transacao.h"
#include "../entity/cliente.h"
#include <stdbool.h>
#include <inttypes.h>

extern TransacaoList transacao_service_find_last_10(uint32_t cliente_id);
extern TransacaoList transacao_service_find_all(uint32_t id_cliente);
extern Cliente *transacao_service_save(Transacao *transacao);
extern void transacao_service_free_list(TransacaoList transacoes);
extern void transacao_service_init();
extern void transacao_service_shared_mem_init();