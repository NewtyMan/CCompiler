#ifndef CCOMPILER_INTERMEDIATE_CODE_H
#define CCOMPILER_INTERMEDIATE_CODE_H
#include <syntax_analyzer/syntax_analyzer.h>
#include <symbol_table/symbol_table.h>

typedef enum intermediate_code_op {
    INTERMEDIATE_CODE_OP_NONE,

    INTERMEDIATE_CODE_OP_ADD,
    INTERMEDIATE_CODE_OP_SUB,
    INTERMEDIATE_CODE_OP_MUL,
    INTERMEDIATE_CODE_OP_DIV,
    INTERMEDIATE_CODE_OP_MOD,

    INTERMEDIATE_CODE_OP_BITWISE_AND,
    INTERMEDIATE_CODE_OP_BITWISE_OR,
    INTERMEDIATE_CODE_OP_BITWISE_XOR,
    INTERMEDIATE_CODE_OP_BITWISE_COMPLEMENT,
    INTERMEDIATE_CODE_OP_BITWISE_LSHIFT,
    INTERMEDIATE_CODE_OP_BITWISE_RSHIFT,

    INTERMEDIATE_CODE_OP_LOGICAL_AND,
    INTERMEDIATE_CODE_OP_LOGICAL_OR,
    INTERMEDIATE_CODE_OP_LOGICAL_LT,
    INTERMEDIATE_CODE_OP_LOGICAL_LTE,
    INTERMEDIATE_CODE_OP_LOGICAL_GT,
    INTERMEDIATE_CODE_OP_LOGICAL_GTE,
    INTERMEDIATE_CODE_OP_LOGICAL_EQUAL,
    INTERMEDIATE_CODE_OP_LOGICAL_NOT_EQUAL,

    INTERMEDIATE_CODE_OP_ASSIGN,

    // Memory
    INTERMEDIATE_CODE_OP_MEMORY_ADDRESS,
    INTERMEDIATE_CODE_OP_MEMORY_READ,
    INTERMEDIATE_CODE_OP_MEMORY_WRITE,

    // Function
    INTERMEDIATE_CODE_OP_FUNCTION,
    INTERMEDIATE_CODE_OP_FUNCTION_CALL,
    INTERMEDIATE_CODE_OP_FUNCTION_PARAMETER,
    INTERMEDIATE_CODE_OP_FUNCTION_RETURN,

    // Control
    INTERMEDIATE_CODE_OP_IF_GOTO,
    INTERMEDIATE_CODE_OP_GOTO,
    INTERMEDIATE_CODE_OP_LABEL,

    // Misc
    INTERMEDIATE_CODE_OP_STRING_READ
} intermediate_code_op_t;

typedef enum intermediate_code_arg_type {
    INTERMEDIATE_CODE_ARG_TYPE_STRING_CONSTANT,
    INTERMEDIATE_CODE_ARG_TYPE_INTEGER_CONSTANT,
    INTERMEDIATE_CODE_ARG_TYPE_CONTROL_LABEL,
    INTERMEDIATE_CODE_ARG_TYPE_TMP_LABEL,
} intermediate_code_arg_type_t;

typedef struct intermediate_code_arg_attrs {
    char is_address, is_parameter;
    int type_size, base_size, offset, no_dereference;
} intermediate_code_arg_attrs_t;

typedef struct intermediate_code_arg {
    intermediate_code_arg_type_t type;
    intermediate_code_arg_attrs_t attrs;
    int integer_value;
    char *string_value;
} intermediate_code_arg_t;

typedef struct intermediate_code_tac_attrs {
    int parameter_size;
    int local_size;
} intermediate_code_tac_attrs_t;

typedef struct intermediate_code_tac {
    intermediate_code_tac_attrs_t attrs;
    intermediate_code_op_t op;
    intermediate_code_arg_t *result;
    intermediate_code_arg_t *arg1;
    intermediate_code_arg_t *arg2;
} intermediate_code_tac_t;

dynamic_array_t* intermediate_code_generate(
    ast_node_t *node,
    symbol_table_symbol_t *symbol_table
);

void intermediate_code_print(
    dynamic_array_t *intermediate_code
);

void intermediate_code_free(
    dynamic_array_t *intermediate_code
);

void intermediate_code_optimization_liveliness_check(
    dynamic_array_t *intermediate_code
);

void intermediate_code_optimization_deduplication(
    dynamic_array_t *intermediate_code
);

int intermediate_code_get_character_constant(char *constant);
#endif //CCOMPILER_INTERMEDIATE_CODE_H
