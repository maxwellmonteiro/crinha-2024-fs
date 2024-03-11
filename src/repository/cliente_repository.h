#include "../entity/cliente.h"
#include <inttypes.h>

#define CLIENTE_FILE_NAME_TEMPLATE "/usr/local/rinha/data/CLIENTE_%d.dat"

extern Cliente *cliente_repo_find_one(uint32_t id);
extern void cliente_repo_update(Cliente *cliente);
extern void cliente_repo_init();
extern void cliente_repo_shared_mem_init();
