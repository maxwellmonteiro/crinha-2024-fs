#include <stdbool.h>

typedef struct IntArrayList {
    int max_size;
    int size;
    int *values;
    void (*push) (struct IntArrayList *, int);
    int (*pop) (struct IntArrayList *);
    bool (*is_full) (struct IntArrayList *);
    void (*destroy) (struct IntArrayList *);
} IntArrayList;

extern IntArrayList *int_array_list_new();
