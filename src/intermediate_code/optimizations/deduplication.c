#include <intermediate_code/intermediate_code.h>
#include <utils/string.h>
#include <stdlib.h>

/*
 *  Due to the way that the intermediate code generator works, there are a lot of cross-pointer references among
 *  different TAC instructions and arguments. This makes the register re-allocation and cleanup/freeing of instructions
 *  very hard. For this reason, we need a deduplication "optimization", which will make each TAC and argument a
 *  standalone pointer.
 */

extern dynamic_array_t *reference_pool;

intermediate_code_arg_t* clone_intermediate_code_argument(intermediate_code_arg_t *src) {
    if (src == NULL) {
        return NULL;
    }
    intermediate_code_arg_t *argument = malloc(sizeof(intermediate_code_arg_t));
    argument->attrs = src->attrs;
    argument->type = src->type;
    argument->integer_value = src->integer_value;
    if (src->string_value != NULL) {
        argument->string_value = calloc(string_len(src->string_value) + 1, sizeof(char));
        string_cpy(argument->string_value, src->string_value);
    }
    return argument;
}

intermediate_code_tac_t* clone_intermediate_code_tac(intermediate_code_tac_t *src) {
    intermediate_code_tac_t *instruction = malloc(sizeof(intermediate_code_tac_t));
    instruction->op = src->op;
    instruction->attrs = src->attrs;
    instruction->arg1 = clone_intermediate_code_argument(src->arg1);
    instruction->arg2 = clone_intermediate_code_argument(src->arg2);
    instruction->result = clone_intermediate_code_argument(src->result);
    return instruction;
}

void dynamic_array_deduplication_free_arg(void *element) {
    intermediate_code_arg_t *argument = element;
    if (argument->string_value != NULL) {
        free(argument->string_value);
    }
    free(argument);
}

void intermediate_code_optimization_deduplication(dynamic_array_t *intermediate_code) {
    dynamic_array_t *intermediate_code_old = dynamic_array_new();

    // Deduplicate instructions and write them back into original array
    int i;
    for (i = 0; i < intermediate_code->size; i++) {
        intermediate_code_tac_t *instruction_old = dynamic_array_get(intermediate_code, i);
        dynamic_array_add(intermediate_code_old, instruction_old);

        intermediate_code_tac_t *instruction_new = clone_intermediate_code_tac(instruction_old);
        intermediate_code->array[i] = instruction_new;
    }

    // Cleanup the old duplicated instructions
    dynamic_array_free(reference_pool, dynamic_array_deduplication_free_arg);
    dynamic_array_free(intermediate_code_old, dynamic_array_free_generic);
}
