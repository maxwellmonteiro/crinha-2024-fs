#include <inttypes.h>

#ifndef CLIENTE_H
#define CLIENTE_H

typedef struct Cliente {
    uint32_t id;
    int32_t limite;
    int64_t saldo;
} Cliente;

typedef struct ClienteList {
    uint32_t size;
    Cliente *values;    
} ClienteList;

#endif