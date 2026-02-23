#include <utils/file_buffer.h>
#include <utils/logger.h>
#include <stdlib.h>

file_buffer_t *file_buffer_new_from_path(char *file_path, int buffer_size) {
    file_buffer_t *file_buffer = malloc(sizeof(file_buffer_t));

    file_buffer->file = fopen(file_path, "r");
    if (file_buffer->file == NULL) {
        logger_error("Failed to open file: %s\n", file_path);
        return NULL;
    }

    file_buffer->buffer = calloc(buffer_size, sizeof(char));
    if (file_buffer->buffer == NULL) {
        logger_error("Failed to allocate memory for file buffer");
        return NULL;
    }

    file_buffer->buffer_capacity = buffer_size;
    file_buffer->buffer_index = 0;
    file_buffer->buffer_size = 0;
    file_buffer->is_eof = 0;

    return file_buffer;
}

void file_buffer_free(file_buffer_t *file_buffer) {
    fclose(file_buffer->file);
    free(file_buffer->buffer);
    free(file_buffer);
}

void file_buffer_load_next(file_buffer_t *file_buffer) {
    file_buffer->buffer_size = fread(file_buffer->buffer, sizeof(char), file_buffer->buffer_capacity, file_buffer->file);
    file_buffer->buffer_index = 0;
    file_buffer->is_eof = feof(file_buffer->file);

    // Fill the remaining buffer indexes that weren't read with zero character
    size_t i = file_buffer->buffer_size;
    for (; i < file_buffer->buffer_capacity; i++) {
        file_buffer->buffer[i] = '\0';
    }
}

char file_buffer_read_char(file_buffer_t *file_buffer) {
    if (file_buffer->buffer_index == file_buffer->buffer_size) {
        file_buffer_load_next(file_buffer);
    }
    return file_buffer->buffer[file_buffer->buffer_index++];
}

int file_buffer_is_eof(file_buffer_t *file_buffer) {
    return file_buffer->is_eof > 0 && file_buffer->buffer[file_buffer->buffer_index] == '\0';
}

void file_buffer_print(file_buffer_t *file_buffer) {
    char c = file_buffer_read_char(file_buffer);
    for (; c != '\0'; c = file_buffer_read_char(file_buffer)) {
        printf("%c", c);
    }
}
