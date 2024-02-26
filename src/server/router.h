#include <stdbool.h>
#include <llhttp.h>
#include <regex.h>

#ifndef ROUTER_H
#define ROUTER_H

#define HTTP_OK "HTTP/1.1 200 OK\r\n\r\n"
#define HTTP_NOT_FOUND "HTTP/1.1 404 Not Found\r\n\r\n"
#define HTTP_BAD_REQUEST "HTTP/1.1 400 Bad Request\r\n\r\n"
#define HTTP_UNPROCESSABLE_ENTITY "HTTP/1.1 422 Unprocessable Entity/Content\r\n\r\n"
#define HTTP_INTERNAL_SERVER_ERROR "HTTP/1.1 500 Internal Server Error\r\n\r\n"

typedef char *(*controller_t)(const char *url);
typedef char *(*controller_body_t)(const char *url, const char *body);

typedef struct Route {
    llhttp_method_t method;
    char *url;
    controller_t controller;
    controller_body_t controller_body;
    // regex_t *matcher;
} Route;

#endif

extern bool router_add_route(Route *route);
// extern bool router_compile_matcher(Route *route);
extern char *router_route(uint8_t method, const char *url, const char *body);