#include <sys/time.h>

#define MAX_BUFFER_DATE_TIME_ISO_STR 64

extern char *env_util_get(const char* name);
extern void timeval_print(struct timeval *tv);
extern void timeval_print_elapsed_if_greater(struct timeval *tv_begin, struct timeval *tv_end,  time_t seconds, suseconds_t micro_seconds, char *suffix);
extern void get_current_time_as_iso_str(char *buffer);