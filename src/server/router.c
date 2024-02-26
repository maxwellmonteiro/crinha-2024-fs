#include "router.h"
#include "../util/log.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define MAX_ROUTES 10

static Route *routes[MAX_ROUTES] = { NULL };

int router_routes_len() {
    int i;
    for (i = 0; routes[i] != NULL && i < MAX_ROUTES; i++);
    return i;
}

bool router_add_route(Route *route) {
    int len = router_routes_len();
    if (len < MAX_ROUTES) {
        routes[len] = route;
        return true;
    }
    return false;
}

// bool router_compile_matcher(Route *route) {
//     int status = regcomp(route->matcher, route->url, REG_EXTENDED);
//     if (status) {
//         log_error("Falha ao compilar regex (%s)", strerror(errno));
//         return false;
//     }
//     return true;
// }

bool router_match_url(const char *url, const char *request_url) {
    int status;
    regex_t regex;

    status = regcomp(&regex, url, REG_EXTENDED);
    if (status) {
        log_error("Falha ao compilar regex (%s)", strerror(errno));
        return false;
    }
    status = regexec(&regex, request_url, 0, NULL, 0);
    regfree(&regex);
    return !status ? true : false;
}

char *router_route(uint8_t method, const char *url, const char *body) {
    for (int i = 0; routes[i] != NULL && i < MAX_ROUTES; i++) {
        if (routes[i]->method == method && router_match_url(routes[i]->url, url)) {
            if (method == HTTP_POST) {
                return routes[i]->controller_body(url, body);
            } else {
                return routes[i]->controller(url);
            }
        }
    }
    return HTTP_BAD_REQUEST;
}