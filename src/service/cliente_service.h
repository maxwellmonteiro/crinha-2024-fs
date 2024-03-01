#include "../entity/cliente.h"
#include <inttypes.h>

extern Cliente *cliente_service_find_one(uint32_t id);
extern void cliente_service_init();
extern void cliente_inserir_saldos();
extern void cliente_service_shared_mem_init();