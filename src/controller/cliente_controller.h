#define _GNU_SOURCE

extern void cliente_controller_init();
extern char *cliente_controller_find_by_id(const char *url);
extern char *cliente_controller_transacoes_saldo(const char *url);
extern char *cliente_controller_save_transacao(const char *url, const char *body);
extern char *cliente_controller_transacoes_dump(const char *url);