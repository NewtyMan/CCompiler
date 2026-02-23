#include <code_generator/code_generator.h>
#include <utils/logger.h>
#include <utils/string.h>
#include <stdio.h>
#include <stdlib.h>

void code_generator_output(dynamic_array_t *generated_code, char *file_path) {
    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
        printf("[ERROR] [GEN] Failed to create output file!");
        return;
    }

    int i;
    for (i = 0; i < generated_code->size; i++) {
        char *instruction = dynamic_array_get(generated_code, i);
        fwrite(instruction, sizeof(char), string_len(instruction), file);
        fwrite("\n", sizeof(char), 1, file);
    }
}
