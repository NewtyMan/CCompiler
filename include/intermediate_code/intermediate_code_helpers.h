#ifndef CCOMPILER_INTERMEDIATE_CODE_HELPERS_H
#define CCOMPILER_INTERMEDIATE_CODE_HELPERS_H
#include <intermediate_code/intermediate_code.h>

typedef enum intermediate_code_parse_type {
    INTERMEDIATE_CODE_PARSE_TYPE_LHS,
    INTERMEDIATE_CODE_PARSE_TYPE_RHS,
} intermediate_code_parse_type_t;

typedef struct intermediate_code_visitor_params {
    dynamic_array_t *intermediate_code;
    symbol_table_symbol_t *symbol_table;

    intermediate_code_arg_t *break_label;
    intermediate_code_arg_t *continue_label;
    intermediate_code_arg_t *switch_expression_label;

    intermediate_code_arg_t *return_value;

    intermediate_code_parse_type_t parse_type;
} intermediate_code_visitor_params_t;

void intermediate_code_add(
    intermediate_code_visitor_params_t *params,
    intermediate_code_tac_t *intermediate_code
);

intermediate_code_arg_t* intermediate_code_maybe_load_address(
    intermediate_code_arg_t *address,
    intermediate_code_visitor_params_t *params
);

intermediate_code_op_t get_intermediate_code_operator(token_type_t token_type);
int is_assignable_arithmetic_operator(token_type_t token_type);
#endif //CCOMPILER_INTERMEDIATE_CODE_HELPERS_H