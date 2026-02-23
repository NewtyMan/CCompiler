#include <utils/string_buffer.h>
#include <stdlib.h>

string_buffer_t *string_buffer_new(int capacity) {
    string_buffer_t *string_buffer = malloc(sizeof(string_buffer_t));
    string_buffer->buffer_capacity_original = capacity;
    string_buffer->buffer_capacity = capacity;
    string_buffer->buffer_size = 0;
    string_buffer->buffer = calloc(capacity, sizeof(char));
    return string_buffer;
}

int string_buffer_add(string_buffer_t *buffer, char c) {
    // We check if we have space for two more characters, since on every character insert, we also need to append a
    // zero-character which terminates the buffer/string.
    if (buffer->buffer_size + 1 >= buffer->buffer_capacity) {
        buffer->buffer_capacity *= 2;
        char *string_buffer_new = realloc(buffer->buffer, buffer->buffer_capacity);
        if (string_buffer_new == NULL) {
            string_buffer_free(buffer);
            return 1;
        }
        buffer->buffer = string_buffer_new;
    }

    buffer->buffer[buffer->buffer_size++] = c;
    buffer->buffer[buffer->buffer_size] = '\0';
    return 0;
}

void string_buffer_reset(string_buffer_t *buffer) {
    free(buffer->buffer);
    buffer->buffer = calloc(buffer->buffer_capacity_original, sizeof(char));
    buffer->buffer_size = 0;
    buffer->buffer_capacity = buffer->buffer_capacity_original;
}

void string_buffer_free(string_buffer_t *string_buffer) {
    free(string_buffer->buffer);
    free(string_buffer);
}
