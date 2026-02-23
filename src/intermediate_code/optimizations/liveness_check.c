#include <intermediate_code/intermediate_code.h>
#include <utils/logger.h>
#include <utils/string.h>
#include <stdlib.h>

/*
 *  Liveness check implements a simple optimization that reduces the amount of temporary registers/labels needed to
 *  compile the program. This is an established algorithm that works as follows:
 *      1. Calculate the first and last occurrence of each temporary label
 *      2. Sort temporary labels in ascending order based on their first occurrence
 *      3. Iterate over all temporary labels again
 *          3.1 For each, find first register where the register is no longer in use, before the temporary label starts
 *          3.2 If such register doesn't exist, throw an error or spill
 *          3.3 Assign the newly retrieved register into the mapping table and mark the new register end
 *      4. Iterate over all instructions and replace all temporary labels with their new indexes
 */

typedef struct metadata {
    int first_occurrence;
    int last_occurrence;
    int label_index;
    int mapping;
} metadata_t;

void find_largest_tmp_label(intermediate_code_arg_t *argument, int *counter) {
    if (argument == NULL || argument->type != INTERMEDIATE_CODE_ARG_TYPE_TMP_LABEL) {
        return;
    }

    if (argument->integer_value > *counter) {
        *counter = argument->integer_value;
    }
}

void calculate_temporary_label_occurrence(intermediate_code_arg_t *argument, metadata_t **metadata_table, int index) {
    if (argument == NULL || argument->type != INTERMEDIATE_CODE_ARG_TYPE_TMP_LABEL) {
        return;
    }

    if (metadata_table[argument->integer_value] == NULL) {
        metadata_table[argument->integer_value] = malloc(sizeof(metadata_t));
        metadata_table[argument->integer_value]->label_index = argument->integer_value;
        metadata_table[argument->integer_value]->first_occurrence = index;
        metadata_table[argument->integer_value]->last_occurrence = index;
        metadata_table[argument->integer_value]->mapping = -1;
    } else {
        metadata_table[argument->integer_value]->last_occurrence = index;
    }
}

void reallocate_temporary_label(intermediate_code_arg_t *argument, metadata_t **metadata_table) {
    if (argument == NULL || argument->type != INTERMEDIATE_CODE_ARG_TYPE_TMP_LABEL) {
        return;
    }

    metadata_t *metadata = metadata_table[argument->integer_value];
    argument->integer_value = metadata->mapping;

    char buffer[32];
    sprintf(buffer, "t%d", argument->integer_value);
    free(argument->string_value);
    argument->string_value = calloc(string_len(buffer) + 1, sizeof(char));
    string_cpy(argument->string_value, buffer);
}

void intermediate_code_optimization_liveliness_check(dynamic_array_t *intermediate_code_array) {
    int i, j;

    // Calculate the amount of temporary registers that exist in the intermediate code
    int temporary_label_count = 0;
    for (i = 0; i < intermediate_code_array->size; i++) {
        intermediate_code_tac_t *intermediate_code = dynamic_array_get(intermediate_code_array, i);
        find_largest_tmp_label(intermediate_code->arg1, &temporary_label_count);
        find_largest_tmp_label(intermediate_code->arg2, &temporary_label_count);
        find_largest_tmp_label(intermediate_code->result, &temporary_label_count);
    }

    if (temporary_label_count == 0) {
        return;
    }

    // We must increment the temporary label counter, since the indexing begins at 0
    temporary_label_count++;

    int register_end[temporary_label_count];
    for (i = 0; i < temporary_label_count; i++) {
        register_end[i] = -1;
    }

    metadata_t **metadata_table = calloc(temporary_label_count, sizeof(metadata_t*));
    if (metadata_table == NULL) {
        logger_error("Failed to initialize memory for metadata table!");
        return;
    }

    // Calculate first and last occurrence of every temporary label
    for (i = 0; i < intermediate_code_array->size; i++) {
        intermediate_code_tac_t *intermediate_code = dynamic_array_get(intermediate_code_array, i);
        calculate_temporary_label_occurrence(intermediate_code->arg1, metadata_table, i);
        calculate_temporary_label_occurrence(intermediate_code->arg2, metadata_table, i);
        calculate_temporary_label_occurrence(intermediate_code->result, metadata_table, i);
    }

    // Sort temporary labels by first occurrence in ascending order
    for (i = 0; i < temporary_label_count; i++) {
        for (j = 0; j < temporary_label_count - i - 1; j++) {
            if (metadata_table[j]->first_occurrence > metadata_table[j + 1]->first_occurrence) {
                metadata_t *temp = metadata_table[j];
                metadata_table[j] = metadata_table[j + 1];
                metadata_table[j + 1] = temp;
            }
        }
    }

    // Assign register reallocation mappings
    for (i = 0; i < temporary_label_count; i++) {
        metadata_t *metadata = metadata_table[i];

        int r = 0;
        while (register_end[r] >= metadata->first_occurrence) r++;

        metadata->mapping = r;
        register_end[r] = metadata->last_occurrence;
    }

    // Remap all registers
    for (i = 0; i < intermediate_code_array->size; i++) {
        intermediate_code_tac_t *intermediate_code = dynamic_array_get(intermediate_code_array, i);
        reallocate_temporary_label(intermediate_code->arg1, metadata_table);
        reallocate_temporary_label(intermediate_code->arg2, metadata_table);
        reallocate_temporary_label(intermediate_code->result, metadata_table);
    }

    // Cleanup the metadata table
    for (i = 0; i < temporary_label_count; i++) {
        free(metadata_table[i]);
    }
    free(metadata_table);
}
