#include "cliente_service.h"
#include "../util/log.h"
#include "../repository/cliente_repository.h"
#include "../util/env_util.h"
#include <jansson.h>
#include <unistd.h>

Cliente *cliente_service_find_one(uint32_t id) {
    return cliente_repo_find_one(id);
}

void cliente_service_init() {
    cliente_repo_init();
}

void cliente_service_shared_mem_init() {
    if (is_main_process()) {
        cliente_repo_shared_mem_init();
    }
}

void cliente_inserir_saldos() {
    if (is_main_process()) {
        // sleep(1);
        Cliente *cliente = malloc(sizeof(Cliente));
        int64_t limites[5] = {100000, 80000, 1000000, 10000000, 500000};
        for (int i = 1; i <= 5; i++) {
            Cliente *found = cliente_repo_find_one(i); 
            if (found == NULL) {
                cliente->id = i;
                cliente->saldo = 0;
                cliente->limite = limites[i - 1];
                cliente_repo_update(cliente);
            } else {
                free(found);
            }
        }
        free(cliente);
    }
}
