#ifndef CCOMPILER_STATE_MACHINE_H
#define CCOMPILER_STATE_MACHINE_H
#include <lexical_analyzer/lexical_analyzer.h>

typedef struct state_machine_params {
    int numerical_constant_index;
    int comment_index;
    int string_literal_index;
    int character_literal_index;
    int identifier_index;
    int operator_index;
    int punctuator_index;
    int whitespace_index;
    int preprocessor_directive_index;
} state_machine_params_t;

typedef token_type_t (*state_machine_func)(state_machine_params_t*, char);

token_type_t state_machine_run(
    state_machine_params_t *params,
    char input
);

state_machine_params_t state_machine_params_new();
#endif //CCOMPILER_STATE_MACHINE_H
