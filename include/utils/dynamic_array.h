#ifndef CCOMPILER_DYNAMIC_ARRAY_H
#define CCOMPILER_DYNAMIC_ARRAY_H

typedef struct dynamic_array {
    void **array;
    int size, capacity;
} dynamic_array_t;

typedef void (*dynamic_array_free_function_t)(void *);

dynamic_array_t* dynamic_array_new();

void dynamic_array_add(
    dynamic_array_t *dynamic_array,
    void *element
);

void* dynamic_array_get(
    dynamic_array_t *dynamic_array,
    int index
);

void dynamic_array_free(
    dynamic_array_t *dynamic_array,
    dynamic_array_free_function_t free_function
);

int dynamic_array_index(
    dynamic_array_t *array,
    void *element
);

void dynamic_array_free_empty(void *element);
void dynamic_array_free_generic(void *element);
#endif //CCOMPILER_DYNAMIC_ARRAY_H
