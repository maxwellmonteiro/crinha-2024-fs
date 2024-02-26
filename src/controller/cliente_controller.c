#include "cliente_controller.h"
#include "../service/cliente_service.h"
#include "../service/transacao_service.h"
#include "../server/router.h"
#include "../server/router.h"
#include "../util/string_util.h"
#include "../util/log.h"
#include "../util/env_util.h"
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <jansson.h>


#define MAX_BUFFER_URL_PARAM 128
#define MAX_BUFFER_FIND_BY_ID 2048
#define MAX_BUFFER_TRANSACAO_SAVE 256

#define MATCH_ONE_URL_PARAM(STR, STR2) ("^\\"STR"\\/([a-zA-Z0-9\\-]*)\\"STR2"$")
#define EXTRACT_ONE_URL_PARAM "\\/([0-9]*)\\/"

static Route route_find_by_id = { HTTP_GET, MATCH_ONE_URL_PARAM("/clientes", "/extrato"), cliente_controller_find_by_id, NULL };
static Route route_transacao_save = { HTTP_POST, MATCH_ONE_URL_PARAM("/clientes", "/transacoes"), NULL, cliente_controller_save_transacao };
static Route route_transacoes_saldo = { HTTP_GET, MATCH_ONE_URL_PARAM("/clientes", "/saldo"), cliente_controller_transacoes_saldo, NULL };

static char buffer_date_time_iso_str[MAX_BUFFER_DATE_TIME_ISO_STR];
static char buffer_url_param[MAX_BUFFER_URL_PARAM];
static char buffer_response_find_by_id[MAX_BUFFER_FIND_BY_ID];
static char buffer_response_trasacao_save[MAX_BUFFER_TRANSACAO_SAVE];

char *extract_param(char *buffer, const char *url, char delimiter, size_t size);
char *extract_url_param(const char *url);
json_t *build_json_saldo(Cliente *cliente);
json_t *build_json_saldo_data(Cliente *cliente);
json_t *build_json_transacoes(TransacaoList transacoes);
json_t *build_json_transacao(Transacao *transacao);
json_t *build_json_extrato(json_t *json_saldo, json_t *json_transacoes);
void update_saldo_cliente(Cliente *cliente, Transacao *transacao);
long calcular_saldo(TransacaoList transacoes);


bool is_id_cliente_valido(const char *id);
bool is_descricao_transacao_valida(const char *descricao);
bool is_tipo_transacao_valido(const char *tipo);
bool is_valor_transacao_valido(long long valor);
bool is_limite_excedido(Cliente *cliente, Transacao *transacao);
bool digits_only(const char *s);

void cliente_controller_init() {  
    router_add_route(&route_find_by_id);
    router_add_route(&route_transacao_save);
    router_add_route(&route_transacoes_saldo);
}

char template_response_http_200_json[] = "HTTP/1.1 200 OK""\r\n"
                                "Content-Type: application/json; charset=utf-8""\r\n"                            
                                "Content-Length: %d""\r\n"
                                "\r\n"
                                "%s";

char *cliente_controller_save_transacao(const char *url, const char *body) {
    char *resp = HTTP_BAD_REQUEST;
    char *param = extract_url_param(url);

    if (body == NULL) {
        log_error("Request sem body");
        return HTTP_BAD_REQUEST;
    }
    if (!is_id_cliente_valido(param)) {
        return HTTP_UNPROCESSABLE_ENTITY;
    }
    
    int32_t id_cliente = atoi(param);
    json_t *json_transacao = json_loads(body, 0, NULL);

    json_t *json_valor = json_object_get(json_transacao, "valor");
    json_t *json_tipo = json_object_get(json_transacao, "tipo");
    json_t *json_descricao = json_object_get(json_transacao, "descricao");

    if (is_descricao_transacao_valida(json_string_value(json_descricao)) && 
        is_tipo_transacao_valido(json_string_value(json_tipo)) &&
        json_is_integer(json_valor) && is_valor_transacao_valido(json_integer_value(json_valor))
        ) {

        Transacao *transacao = malloc(sizeof(Transacao));
        transacao->id_cliente = id_cliente;
        transacao->valor = json_integer_value(json_valor);
        strzcpy(transacao->tipo, json_string_value(json_tipo), TRANSACAO_TIPO_LEN);
        strzcpy(transacao->descricao, json_string_value(json_descricao), UTF8_TRANSACAO_DESCRICAO_LEN);
        get_current_time_as_iso_str(buffer_date_time_iso_str);
        strzcpy(transacao->realizada_em, buffer_date_time_iso_str, TRANSACAO_REALIZADA_EM_LEN);

        Cliente *cliente = cliente_service_find_one(id_cliente);
        if (cliente != NULL) {
            if (!is_limite_excedido(cliente, transacao)) {
                bool ret = transacao_service_save(transacao, cliente);
                if (ret) {         
                    json_t *json_saldo = build_json_saldo(cliente);
                    char *buffer_saldo = json_dumps(json_saldo, JSON_COMPACT);
                    sprintf(buffer_response_trasacao_save, template_response_http_200_json, strlen(buffer_saldo), buffer_saldo);
                    resp = buffer_response_trasacao_save;
                    json_decref(json_saldo);
                    free(buffer_saldo);
                } else {
                    resp = HTTP_UNPROCESSABLE_ENTITY;
                }
            } else {
                resp = HTTP_UNPROCESSABLE_ENTITY;
            }
            free(cliente);
        } else {
            resp = HTTP_NOT_FOUND;
        }
        free(transacao);
    } else {
        resp = HTTP_UNPROCESSABLE_ENTITY;
    }
    json_decref(json_transacao);
    return resp;
}

char *cliente_controller_find_by_id(const char *url) {
    char *resp = HTTP_BAD_REQUEST;

    char *param = extract_url_param(url);

    if (is_id_cliente_valido(param)) {
        int id_cliente = atoi(param);
        Cliente *cliente = cliente_service_find_one(id_cliente);
        if (cliente != NULL) {
            TransacaoList transacoes = transacao_service_find_last_10(id_cliente);
            json_t *json_saldo = build_json_saldo_data(cliente);
            json_t *json_transacoes = build_json_transacoes(transacoes);
            json_t *json_extrato = build_json_extrato(json_saldo, json_transacoes);
            char *buffer_extrato = json_dumps(json_extrato, JSON_COMPACT);
            sprintf(buffer_response_find_by_id, template_response_http_200_json, strlen(buffer_extrato), buffer_extrato);
            free(buffer_extrato);
            json_decref(json_extrato);
            transacao_service_free_list(transacoes);
            free(cliente);
            resp = buffer_response_find_by_id;
        } else {
            resp = HTTP_NOT_FOUND;
        }
    }
    return resp;
}

char *cliente_controller_transacoes_saldo(const char *url) {
    char *resp = HTTP_BAD_REQUEST;

    char *param = extract_url_param(url);

    if (is_id_cliente_valido(param)) {
        int id_cliente = atoi(param);
        Cliente *cliente = cliente_service_find_one(id_cliente);
        if (cliente != NULL) {
            TransacaoList transacoes = transacao_service_find_all(id_cliente);
            cliente->saldo = calcular_saldo(transacoes);
            json_t *json_saldo = build_json_saldo(cliente);
            char *buffer_saldo = json_dumps(json_saldo, JSON_COMPACT);
            sprintf(buffer_response_find_by_id, template_response_http_200_json, strlen(buffer_saldo), buffer_saldo);
            free(buffer_saldo);
            json_decref(json_saldo);
            transacao_service_free_list(transacoes);
            free(cliente);
            resp = buffer_response_find_by_id;
        } else {
            resp = HTTP_NOT_FOUND;
        }
    }
    return resp;
}

//////////////////////////////////////////////////////////////////////////////////////

bool is_id_cliente_valido(const char *id) {
    return strlen(id) > 0 && strlen(id) < 10 && digits_only(id);
}

bool is_descricao_transacao_valida(const char *descricao) {
    if (descricao != NULL) {
        size_t len = string_util_utf8_strlen(descricao);
        return len >= 1 && len <= TRANSACAO_DESCRICAO_LEN;
    } 
    return false;
}

bool is_tipo_transacao_valido(const char *tipo) {
    return strlen(tipo) == 1 && (tipo[0] == 'c' || tipo[0] == 'd');
}

bool is_valor_transacao_valido(long long valor) {
    return valor > 0;
}

bool is_limite_excedido(Cliente *cliente, Transacao *transacao) {
    if (transacao->tipo[0] == 'c') {
        return false;
    } else if ((cliente->saldo - transacao->valor) < -cliente->limite) {
        return true;
    }
    return false;
}

bool digits_only(const char *s) {
    while (*s) {
        if (isdigit(*s++) == 0) {
            return false;
        }
    }
    return true;
}

size_t extract_param_regex(char *buffer, const char *pattern, const char *str) {
    int status;
    regex_t regex;
    regmatch_t regmatch;

    status = regcomp(&regex, pattern, REG_EXTENDED);
    if (status) {
        log_error("Falha ao compilar regex (%s)", strerror(errno));
        return false;
    }
    
    status = regexec(&regex, str, 1, &regmatch, 0);
    regfree(&regex);    
    if (status == 0) {
        uint32_t len = regmatch.rm_eo - regmatch.rm_so;
        sprintf(buffer, "%.*s", len - 2, &str[regmatch.rm_so + 1]);
        return len;
    }
    return 0;
}

char *extract_param(char *buffer, const char *url, char delimiter, size_t size) {
    size_t len = extract_param_regex(buffer, EXTRACT_ONE_URL_PARAM, url);
    if (len == 0) {
        buffer[0] = 0;
    }
    return buffer;
}

char *extract_url_param(const char *url) {
    return extract_param(buffer_url_param, url, '/', MAX_BUFFER_URL_PARAM);
}

json_t *build_json_saldo(Cliente *cliente) {
    return json_pack("{s:i, s:i}",        
        "limite", cliente->limite,
        "saldo", cliente->saldo
        );
}

json_t *build_json_saldo_data(Cliente *cliente) {
    get_current_time_as_iso_str(buffer_date_time_iso_str);
    return json_pack("{s:i, s:s?, s:i}",
        "total", cliente->saldo,
        "data_extrato", buffer_date_time_iso_str,
        "limite", cliente->limite
        );
}

json_t *build_json_transacoes(TransacaoList transacoes) {
    json_t *json_transacoes = json_array();
    json_t *json_transacao;
    size_t size = transacoes.size;
    for (int i = 0; i < size; i++) {
        json_transacao = build_json_transacao(&transacoes.values[i]);
        json_array_append_new(json_transacoes, json_transacao);
    }
    return json_transacoes;
}

json_t *build_json_transacao(Transacao *transacao) {
    return json_pack("{s:i, s:s?, s:s?, s:s?}",
        "valor", transacao->valor,
        "tipo", transacao->tipo,
        "descricao", transacao->descricao,
        "realizada_em", transacao->realizada_em
        );
}

json_t *build_json_extrato(json_t *json_saldo, json_t *json_transacoes) {
    return json_pack("{s:o?, s:o?}",
        "saldo", json_saldo,
        "ultimas_transacoes", json_transacoes
        );
}

void update_saldo_cliente(Cliente *cliente, Transacao *transacao) {
    if (transacao->tipo[0] == 'c') {
        cliente->saldo += transacao->valor;
    } else {
        cliente->saldo -= transacao->valor;
    }
}

long calcular_saldo(TransacaoList transacoes) {
    long total = 0;
    for (int i = 0; i < transacoes.size; i++) {
        if (transacoes.values[i].tipo[0] == 'c') {
            total += transacoes.values[i].valor;
        } else {
            total -= transacoes.values[i].valor;
        }
    }
    return total;
}