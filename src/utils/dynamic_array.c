#include <utils/dynamic_array.h>
#include <stdlib.h>

dynamic_array_t* dynamic_array_new() {
    dynamic_array_t *array = malloc(sizeof(dynamic_array_t));
    array->size = 0;
    array->capacity = 1;
    array->array = calloc(array->capacity, sizeof(void *));
    return array;
}

void dynamic_array_add(dynamic_array_t *array, void *data) {
    if (array->size == array->capacity) {
        array->capacity *= 2;
        array->array = realloc(array->array, array->capacity * sizeof(void *));
    }
    array->array[array->size++] = data;
}

void* dynamic_array_get(dynamic_array_t *array, int index) {
    if (index >= 0 && index < array->size) {
        return array->array[index];
    }
    return NULL;
}

void dynamic_array_free(dynamic_array_t *array, dynamic_array_free_function_t free_function) {
    int i;
    for (i = 0; i < array->size; i++) {
        free_function(array->array[i]);
    }
    free(array->array);
    free(array);
}

int dynamic_array_index(dynamic_array_t *array, void *pointer) {
    int i;
    for (i = 0; i < array->size; i++) {
        if (array->array[i] == pointer) {
            return i;
        }
    }
    return -1;
}

void dynamic_array_free_empty(void *element) {}

void dynamic_array_free_generic(void *pointer) {
    free(pointer);
}
