#include "env_util.h"
#include "log.h"
#include <stdlib.h>
#include <math.h>

char *env_util_get(const char* name) {
    char *ret = getenv(name);

    if (ret == NULL) {
        log_error("Variável de ambiente não encontrada %s", name);
    }
    return ret;
}

int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1) {
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}

void timeval_print(struct timeval *tv) {
    char buffer[30];
    time_t curtime;

    printf("%ld.%06ld", tv->tv_sec, tv->tv_usec);
    curtime = tv->tv_sec;
    strftime(buffer, 30, "%m-%d-%Y  %T", localtime(&curtime));
    printf(" = %s.%06ld\n", buffer, tv->tv_usec);
}

bool is_main_process() {
    char *var = env_util_get(ENV_MAIN);
    if (var != NULL) {
        return atoi(var) == 1;
    }
    return false;
}

void timeval_print_elapsed_if_greater(struct timeval *tv_begin, struct timeval *tv_end,  time_t seconds, suseconds_t micro_seconds, char *suffix) {
    struct timeval tv_diff;
    timeval_subtract(&tv_diff, tv_end, tv_begin);
    if (tv_diff.tv_sec > seconds || tv_diff.tv_usec > micro_seconds) {
        log_info("Elapsed: %ld.%06ld : %s", tv_diff.tv_sec, tv_diff.tv_usec, suffix);
    }
}

void get_current_time_as_iso_str(char *buffer) {
    char tmp[MAX_BUFFER_DATE_TIME_ISO_STR];
    struct timeval tval;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    gettimeofday(&tval, NULL);
    strftime(tmp, MAX_BUFFER_DATE_TIME_ISO_STR, "%FT%T.%%06ldZ", tm);
    sprintf(buffer, tmp, tval.tv_usec);
}
