#include <llhttp.h>
#include <stdbool.h>

#define HTTP_REQUEST_URL_BUFFER_SIZE 256
#define HTTP_REQUEST_BODY_BUFFER_SIZE 1024

typedef struct HttpRequest {
    char url[HTTP_REQUEST_URL_BUFFER_SIZE + 1];
    char body[HTTP_REQUEST_BODY_BUFFER_SIZE + 1];
    bool complete;
} HttpRequest;

extern llhttp_t *http_parser_init();
extern void http_parser_close(llhttp_t *parser);
extern uint8_t http_parser_get_method(llhttp_t *parser);
extern bool http_parser_parse(llhttp_t *parser, const char *data, size_t len);
extern void http_parser_reset_request(llhttp_t *parser);
extern bool http_parser_request_is_complete(llhttp_t *parser);