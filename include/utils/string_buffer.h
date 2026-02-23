#ifndef CCOMPILER_STRING_BUFFER_H
#define CCOMPILER_STRING_BUFFER_H

typedef struct string_buffer {
    char *buffer;
    int buffer_size;
    int buffer_capacity;
    int buffer_capacity_original;
} string_buffer_t;

string_buffer_t *string_buffer_new(int capacity);
int string_buffer_add(string_buffer_t *string_buffer, char c);
void string_buffer_reset(string_buffer_t *string_buffer);
void string_buffer_free(string_buffer_t *string_buffer);
#endif //CCOMPILER_STRING_BUFFER_H