#ifndef CCOMPILER_FILE_BUFFER_H
#define CCOMPILER_FILE_BUFFER_H

#include <stdio.h>

typedef struct file_buffer {
    FILE *file;
    char *buffer;
    int buffer_capacity, buffer_index, is_eof;
    size_t buffer_size;
} file_buffer_t;

file_buffer_t *file_buffer_new_from_path(
    char *filepath,
    int buffer_capacity
);

void file_buffer_free(file_buffer_t *file_buffer);
void file_buffer_load_next(file_buffer_t *file_buffer);
char file_buffer_read_char(file_buffer_t *file_buffer);
int file_buffer_is_eof(file_buffer_t *file_buffer);
void file_buffer_print(file_buffer_t *file_buffer);
#endif //CCOMPILER_FILE_BUFFER_H
