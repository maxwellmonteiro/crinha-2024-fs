CC = gcc
SOURCES_C = src/main.c src/util/fs_connection.c src/service/cliente_service.c src/service/transacao_service.c src/repository/cliente_repository.c src/repository/transacao_repository.c src/util/uuid_util.c src/util/string_util.c src/util/log.c src/util/array_list.c src/util/env_util.c src/server/server.c src/server/http_parser.c src/server/router.c src/controller/cliente_controller.c
INCLUDEDIR = ./include
LIBDIR = ./lib
CFLAGS_DEBUG = -g -Wl,-rpath -Wl,$(LIBDIR)
CFLAGS_RELEASE = -pipe -Wrestrict -Wall -O3 -Wl,-rpath -static -Wl,$(LIBDIR)
CFLAGS = $(CFLAGS_RELEASE)

all: rinha

rinha: $(SOURCES_C)
	$(CC) $(CFLAGS) $(SOURCES_C) -o $@ -ljansson -lllhttp -luuid -I$(INCLUDEDIR) -L$(LIBDIR)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.c: %.h	

clean:
	rm -f *.o rinha
