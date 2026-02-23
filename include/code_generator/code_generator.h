#ifndef CCOMPILER_CODE_GENERATOR_H
#define CCOMPILER_CODE_GENERATOR_H
#include <utils/dynamic_array.h>

dynamic_array_t* code_generator_generate(
    dynamic_array_t *intermediate_code,
    dynamic_array_t *string_literals
);

void code_generator_output(
    dynamic_array_t *generated_code,
    char *file_path
);

int code_generator_add_syscalls(
    dynamic_array_t *generated_code
);

void code_generator_print(
    dynamic_array_t *generated_code
);

void code_generator_free(
    dynamic_array_t *generated_code
);
#endif //CCOMPILER_CODE_GENERATOR_H
