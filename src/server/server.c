#include "server.h"
#include "http_parser.h"
#include "router.h"
#include "../util/log.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>


#define SOCKET_MAX_READ_BUFFER 1024
#define SOCKET_MAX_POLL 16
#define SOCKET_MAX_QUEUED 96

#define IS_SERVER_SCK(I) (I == 0)
#define IS_POLL_FULL(S) (S >= SOCKET_MAX_POLL)

volatile sig_atomic_t shoud_terminate = 0;

void handle_signal(int signum) {   
   shoud_terminate = 1;
}

void server_set_sock_opt(int sock_handle, int opt_name, int *opt_val, socklen_t opt_len) {
    if (setsockopt(sock_handle, SOL_SOCKET, opt_name, opt_val, opt_len)) {
        log_fatal("Falha ao definir socket options (%s)", strerror(errno));
        shutdown(sock_handle, SHUT_RDWR);
        exit(EXIT_FAILURE);
    }
}

int socket_create() {
    int sock_handle = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_handle < 0) {
        log_fatal("Falha ao criar o socket (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }
    server_set_sock_opt(sock_handle, SO_REUSEADDR, &(int){1}, sizeof(int));
    server_set_sock_opt(sock_handle, SO_REUSEPORT, &(int){1}, sizeof(int));
    return sock_handle;
}

void socket_listen(int sock_handle, struct sockaddr_in *address) {
    if (bind(sock_handle, (struct sockaddr *) address, sizeof(*address)) < 0) {
        log_fatal("Falha ao fazer bind (%s)", strerror(errno));
        shutdown(sock_handle, SHUT_RDWR);
        exit(EXIT_FAILURE);
    }
    if (listen(sock_handle, SOCKET_MAX_QUEUED) < 0) {
        log_fatal("Falha ao fazer listen (%s)", strerror(errno));
        shutdown(sock_handle, SHUT_RDWR);
        exit(EXIT_FAILURE);
    }
}

void poll_push(struct pollfd *pfds, int fd, nfds_t *nfds) {
    int i;

    if (*nfds >= SOCKET_MAX_POLL) {
        log_error("Limite de sockets ativos atingido (%d)", SOCKET_MAX_POLL);
        return;
    }

    for (i = 0; i < SOCKET_MAX_POLL && pfds[i].fd != -1; i++);
    if (i < SOCKET_MAX_POLL) {
        pfds[i].fd = fd;
        pfds[i].revents = 0;
        (*nfds)++;
    }
}

void poll_pop(struct pollfd *pfds, int fd, nfds_t *nfds) {
    int i;
    for (i = 0; i < SOCKET_MAX_POLL && pfds[i].fd != fd; i++);
    if (i < SOCKET_MAX_POLL) {
        pfds[i].fd = -1;
        (*nfds)--;
    }
}

void poll_init(struct pollfd *pfds) {
    for (int i = 0; i < SOCKET_MAX_POLL; i++) {
        pfds[i].fd = -1;
        pfds[i].events = POLLIN;
    }
}

void socket_read(int sd, llhttp_t *parser) {
    char buffer[SOCKET_MAX_READ_BUFFER] = { 0 };
    int valread;
    char *response;

    while ((valread = recv(sd, buffer, SOCKET_MAX_READ_BUFFER, MSG_DONTWAIT)) > 0) {
        buffer[valread] = 0;
        if (http_parser_parse(parser, buffer, valread) && http_parser_request_is_complete(parser)) {
            uint8_t method = parser->method;
            char *url = ((HttpRequest *)parser->data)->url;
            char *body = ((HttpRequest *)parser->data)->body;
            response = router_route(method, url, body);
            send(sd, response, strlen(response), 0);
            http_parser_reset_request(parser);
        } else {
            log_error("Falha no parse do request http (%s)", buffer);
        }
    }
}

void socket_loop(int sock_handle, struct sockaddr_in *address, llhttp_t *parser) {
    int new_socket;
    int addrlen = sizeof(*address);

    struct pollfd pfds[SOCKET_MAX_POLL];
    int poll_ready;

    nfds_t nfds = 0;
    poll_init(pfds);
    poll_push(pfds, sock_handle, &nfds);

    while (!shoud_terminate) {
        poll_ready = poll(pfds, SOCKET_MAX_POLL, -1);

        if (poll_ready <= 0) {
            if (errno != EINTR) log_error("Falha ao fazer poll (%s)", strerror(errno));
            continue;
        }

        for (int i = 0; i < SOCKET_MAX_POLL; i++) {
            if (pfds[i].revents == 0) continue;

            if (pfds[i].revents & POLLIN) {
                if (IS_SERVER_SCK(i)) {
                    if (IS_POLL_FULL(nfds)) continue;
                    
                    if ((new_socket = accept(sock_handle, (struct sockaddr *) address, (socklen_t *) &addrlen)) >= 0) {
                        poll_push(pfds, new_socket, &nfds);
                    } else {
                        log_error("Falha ao fazer accept (%s)", strerror(errno));
                    }
                } else {
                    socket_read(pfds[i].fd, parser);
                    shutdown(pfds[i].fd, SHUT_RDWR);
                    close(pfds[i].fd);
                    poll_pop(pfds, pfds[i].fd, &nfds);
                }
            } else {
                if (!IS_SERVER_SCK(i)) {
                    shutdown(pfds[i].fd, SHUT_RDWR);
                    close(pfds[i].fd);
                    poll_pop(pfds, pfds[i].fd, &nfds);
                } 
            }
        }
    }
}

void server_init(uint16_t port) {
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);

    log_info("Iniciando servidor ...");
    int sock_handle = socket_create();

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    socket_listen(sock_handle, &address);
    
    llhttp_t *parser = http_parser_init();

    char ipv4[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &address.sin_addr, ipv4, INET_ADDRSTRLEN);

    log_info("Servidor iniciado em %s:%d", ipv4, port);
    socket_loop(sock_handle, &address, parser);
    log_info("Servidor encerrado");

    http_parser_close(parser);
    shutdown(sock_handle, SHUT_RDWR);    
}