#include "http_parser.h"
#include "../util/log.h"
#include "../util/string_util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LLHTTP_PARSER_STATE_CLOSED 0X1

int handle_on_url(llhttp_t *parser, const char *at, size_t length) {    
    HttpRequest *request = (HttpRequest *) parser->data;
    int current_url_len = strlen(request->url); // if already read some bytes
    if ((length + current_url_len) >= HTTP_REQUEST_URL_BUFFER_SIZE) {
        log_error("Falha ao copiar url da requisicao, tamanho do buffer excedido");
        return 1;
    }
    strzcpy(&request->url[current_url_len], at, length); // append if already have some byte
    request->url[length + current_url_len] = 0;

	return HPE_OK;
}

int handle_on_body(llhttp_t *parser, const char *at, size_t length) { 
    HttpRequest *request = (HttpRequest *) parser->data;
    int current_body_len = strlen(request->body);   
    if ((length + current_body_len) > HTTP_REQUEST_BODY_BUFFER_SIZE) {
        log_error("Falha ao copiar body da requisicao, tamanho do buffer excedido");
        return 1;
    }
    strzcpy(&request->body[current_body_len], at, length);
    request->body[length + current_body_len] = 0;

	return HPE_OK;
}

int handle_on_message_complete(llhttp_t *parser) {
    HttpRequest *request = (HttpRequest *) parser->data;
    request->complete = true;
    return HPE_OK;
}

llhttp_t *http_parser_init() {
    llhttp_t *parser;
	llhttp_settings_t *settings;

    parser = malloc(sizeof(llhttp_t));
    settings = malloc(sizeof(llhttp_settings_t));

    llhttp_settings_init(settings);

    settings->on_url = handle_on_url;
    settings->on_body = handle_on_body;
    settings->on_message_complete = handle_on_message_complete;

    llhttp_init(parser, HTTP_REQUEST, settings);

    parser->data = malloc(sizeof(HttpRequest));
    http_parser_reset_request(parser);

    return parser;
}

void http_parser_close(llhttp_t *parser) {
    free(parser->settings);
    parser->settings = NULL;
    free(parser->data);
    parser->data = NULL;
    free(parser);
}

uint8_t http_parser_get_method(llhttp_t *parser) {
    return llhttp_get_method(parser);
}

bool http_parser_parse(llhttp_t *parser, const char *data, size_t len) {
    llhttp_errno_t status = llhttp_get_errno(parser);
    if (status == HPE_PAUSED || status == HPE_PAUSED_UPGRADE) {
        llhttp_resume(parser);
        llhttp_resume_after_upgrade(parser);
    }
    status = llhttp_execute(parser, data, len);
    if (status != HPE_OK && status != HPE_PAUSED && status != HPE_PAUSED_UPGRADE) {
        log_error("Falha no parse da requisição (%d)", status);
        log_trace("Payload: %s", data);
        llhttp_finish(parser);
        return false;
    }
    return true;
}

void http_parser_reset_request(llhttp_t *parser) {
    HttpRequest *request = (HttpRequest *) parser->data;
    request->url[0] = 0;
    request->body[0] = 0;
    request->complete = false;
    if ((intptr_t) parser->_current == LLHTTP_PARSER_STATE_CLOSED) {
        llhttp_reset(parser);
    }
}

bool http_parser_request_is_complete(llhttp_t *parser) {
    HttpRequest *request = (HttpRequest *) parser->data;
    return request->complete;
}
