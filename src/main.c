#include "util/fs_connection.h"
#include "server/server.h"
#include "controller/cliente_controller.h"
#include "util/fs_connection.h"
#include "service/cliente_service.h"
#include "service/transacao_service.h"
#include "util/log.h"
#include "util/env_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENV_PORT "PORT"

int main(int argc, char **argv) {

    log_set_level(LOG_INFO);

    fs_init_pool();
    
    cliente_service_init();
    transacao_service_init();
    cliente_controller_init();

    server_init(atoi(env_util_get(ENV_PORT)));

    fs_close_pool();

    return 0;
}